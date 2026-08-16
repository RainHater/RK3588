#ifndef RKPLATFORM_PLATFORM_SHARED_MEMORY_H
#define RKPLATFORM_PLATFORM_SHARED_MEMORY_H

#include "Logger.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <memory>
#include <new>
#include <string>
#include <type_traits>

#include <fcntl.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace rkplatform::platform {

template<typename T>
class SharedMemory {
    static_assert(
        std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>,
        "SharedMemory requires a default-constructible, trivially copyable type"
    );

private:
    static constexpr std::uint32_t kMagic = 0x524B5348U;

    struct Message {
        mutable pthread_mutex_t mutex;
        std::uint32_t magic;
        std::uint64_t sequence;
        std::uint64_t timestamp_us;
        T data;
    };

public:
    // 创建未打开的共享内存对象。
    SharedMemory()
        : m_log(component::logging::GetLogger("SharedMemory"))
    {}

    // 关闭共享内存并释放资源。
    ~SharedMemory() {
        Close();
    }

    // 禁止复制共享内存对象。
    SharedMemory(const SharedMemory&) = delete;
    // 禁止复制赋值共享内存对象。
    SharedMemory& operator=(const SharedMemory&) = delete;

    // 创建或打开指定名称的共享内存。
    bool Open(const std::string& name) {
        Close();

        if (name.empty()) {
            m_log->error("共享内存名称不能为空");
            return false;
        }

        m_name = name.front() == '/' ? name : "/" + name;
        m_fd = ::shm_open(
            m_name.c_str(),
            O_CREAT | O_EXCL | O_RDWR,
            0660
        );

        if (m_fd >= 0) {
            m_owner = true;
        } else if (errno == EEXIST) {
            m_fd = ::shm_open(m_name.c_str(), O_RDWR, 0);
        }

        if (m_fd < 0) {
            m_log->error("shm_open 打开失败: {}", m_name);
            return false;
        }

        if (::flock(m_fd, LOCK_EX) != 0) {
            m_log->error("共享内存初始化锁获取失败: {}", m_name);
            Close();
            return false;
        }

        const auto fail_open = [this]() {
            ::flock(m_fd, LOCK_UN);
            Close();
            return false;
        };

        if (m_owner) {
            if (::ftruncate(m_fd, static_cast<off_t>(sizeof(Message))) != 0) {
                m_log->error("设置共享内存大小失败: {}", m_name);
                return fail_open();
            }
        } else {
            struct stat info{};
            if (::fstat(m_fd, &info) != 0 ||
                info.st_size != static_cast<off_t>(sizeof(Message))) {
                m_log->error("共享内存布局不匹配: {}", m_name);
                return fail_open();
            }
        }

        m_address = ::mmap(
            nullptr,
            sizeof(Message),
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            m_fd,
            0
        );

        if (m_address == MAP_FAILED) {
            m_address = nullptr;
            m_log->error("mmap 失败: {}", m_name);
            return fail_open();
        }

        if (m_owner) {
            auto* message = new (m_address) Message{};
            pthread_mutexattr_t attributes;
            if (::pthread_mutexattr_init(&attributes) != 0) {
                m_log->error("共享内存互斥锁属性初始化失败: {}", m_name);
                return fail_open();
            }

            const int shared_result = ::pthread_mutexattr_setpshared(
                &attributes,
                PTHREAD_PROCESS_SHARED
            );
            const int robust_result = ::pthread_mutexattr_setrobust(
                &attributes,
                PTHREAD_MUTEX_ROBUST
            );
            const int mutex_result =
                shared_result == 0 && robust_result == 0
                    ? ::pthread_mutex_init(&message->mutex, &attributes)
                    : -1;
            ::pthread_mutexattr_destroy(&attributes);

            if (mutex_result != 0) {
                m_log->error("进程共享互斥锁初始化失败: {}", m_name);
                return fail_open();
            }

            message->sequence = 0;
            message->timestamp_us = 0;
            message->data = {};
            message->magic = kMagic;
        } else if (GetMessage()->magic != kMagic) {
            m_log->error("共享内存尚未完成初始化: {}", m_name);
            return fail_open();
        }

        ::flock(m_fd, LOCK_UN);
        ::close(m_fd);
        m_fd = -1;
        return true;
    }

    // 在进程共享锁保护下写入完整数据。
    bool Write(const T& data) noexcept {
        auto* message = GetMessage();
        if (message == nullptr || !LockMessage()) {
            return false;
        }

        message->data = data;
        message->timestamp_us = GetTimestampUs();
        ++message->sequence;
        if (message->sequence == 0) {
            message->sequence = 1;
        }
        return UnlockMessage();
    }

    // 读取尚未消费的最新完整数据。
    bool ReadIfNew(T& data, std::uint64_t& timestamp_us) noexcept {
        auto* message = GetMessage();
        if (message == nullptr || !LockMessage()) {
            return false;
        }

        if (message->timestamp_us == 0 || message->sequence == 0 ||
            message->sequence == m_last_sequence) {
            UnlockMessage();
            return false;
        }

        data = message->data;
        timestamp_us = message->timestamp_us;
        m_last_sequence = message->sequence;
        return UnlockMessage();
    }

    // 获取当前系统时间戳，单位为微秒。
    std::uint64_t GetTimestampUs() const noexcept {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    // 关闭映射并按所有权删除共享内存名称。
    void Close() noexcept {
        if (m_address != nullptr) {
            ::munmap(m_address, sizeof(Message));
            m_address = nullptr;
        }

        if (m_fd >= 0) {
            ::close(m_fd);
            m_fd = -1;
        }

        if (m_owner && !m_name.empty()) {
            ::shm_unlink(m_name.c_str());
        }

        m_owner = false;
        m_last_sequence = 0;
        m_name.clear();
    }

private:
    // 获取共享消息地址。
    Message* GetMessage() noexcept {
        return static_cast<Message*>(m_address);
    }

    // 获取只读共享消息地址。
    const Message* GetMessage() const noexcept {
        return static_cast<const Message*>(m_address);
    }

    // 获取进程共享互斥锁。
    bool LockMessage() noexcept {
        auto* message = GetMessage();
        if (message == nullptr) {
            return false;
        }

        const int result = ::pthread_mutex_lock(&message->mutex);
        if (result == EOWNERDEAD) {
            message->timestamp_us = 0;
            message->data = {};
            if (::pthread_mutex_consistent(&message->mutex) == 0) {
                return true;
            }
            ::pthread_mutex_unlock(&message->mutex);
            return false;
        }
        return result == 0;
    }

    // 释放进程共享互斥锁。
    bool UnlockMessage() noexcept {
        auto* message = GetMessage();
        return message != nullptr &&
               ::pthread_mutex_unlock(&message->mutex) == 0;
    }

private:
    bool m_owner = false;
    int m_fd = -1;
    void* m_address = nullptr;
    std::string m_name;
    std::shared_ptr<spdlog::logger> m_log;
    std::uint64_t m_last_sequence = 0;
};

}  // namespace rkplatform::platform

#endif  // RKPLATFORM_PLATFORM_SHARED_MEMORY_H
