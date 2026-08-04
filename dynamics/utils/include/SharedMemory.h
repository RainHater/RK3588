#ifndef _SHARE_MEMORY_H
#define _SHARE_MEMORY_H

#include "Logger.h"

#include <cstddef>
#include <cstring>

#include <fcntl.h>
#include <string>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <semaphore.h>

static int FutexWait(
    std::uint32_t* address,
    std::uint32_t expected
)
{
    return static_cast<int>(
        ::syscall(
            SYS_futex,
            address,
            FUTEX_WAIT,
            expected,
            nullptr,
            nullptr,
            0
        )
    );
}

static int FutexWake(
    std::uint32_t* address,
    int count = 1
)
{
    return static_cast<int>(
        ::syscall(
            SYS_futex,
            address,
            FUTEX_WAKE,
            count,
            nullptr,
            nullptr,
            0
        )
    );
}

template<typename T>
class SharedMemory{
public:
    SharedMemory()
    : m_owner(false)
    , m_fd(-1)
    , m_address(nullptr)
    , m_shared_memory_size(0)
    , m_can_write(SEM_FAILED)
    , m_can_read(SEM_FAILED)
    , m_overtime(1000)
    , m_log(LoggerWithTag::GetLogger("SharedMemory"))
    {

    }
    ~SharedMemory(){
        Close();
    }

    bool Open(std::string name, std::uint64_t overtime = 1000){
        Close();

        m_name = name;
        m_can_write_name = m_name + "CanWrite";
        m_can_read_name = m_name + "CanRead";
        m_overtime = overtime;
        m_shared_memory_size = sizeof(Message);

        // 创建或打开共享内存对象
        m_fd = ::shm_open(
            name.c_str(), 
            O_CREAT | O_EXCL  |  O_RDWR,
            0660
        );
        
        if (m_fd > 0){
            m_owner = true;
        }else if (errno == EEXIST) {
            m_fd = ::shm_open(
                m_name.c_str(),
                O_RDWR,
                0
            );
            m_owner = false;
        }

        if (m_fd == -1){
            m_log->error("shm_open 打开失败!");
            return false;
        }

        if (::ftruncate(m_fd, static_cast<off_t>(m_shared_memory_size)) == -1){
            m_log->error("设置共享内存大小失败!");
            ::close(m_fd);
            return false;
        }

        m_address = ::mmap(
            nullptr,
            m_shared_memory_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            m_fd,
            0
        );

        ::close(m_fd);

        if (m_address == MAP_FAILED) {
            m_log->error("设置 mmap 失败!");
            return false;
        }

        // 初始值为1, 相当于互斥锁
        m_can_write = ::sem_open(
            m_can_write_name.c_str(),
            O_CREAT,
            0660,
            1
        );

        // 初始值为0, 表示暂时没有新数据
        m_can_read = ::sem_open(
            m_can_read_name.c_str(),
            O_CREAT,
            0660,
            0
        );

        if (m_can_write == SEM_FAILED || m_can_read == SEM_FAILED) {
            m_log->error("sem_open 失败: {}", std::strerror(errno));
            Close();
            return false;
        }

        return true;
    }

    T* Data(){
        return static_cast<Message<T>*>(m_address);
    }

    // bool Wait(sem_t* semaphore){
    //     if (semaphore == nullptr || semaphore == SEM_FAILED) {
    //         return false;
    //     }

    //     timespec deadline {};

    //     if (::clock_gettime(CLOCK_REALTIME, &deadline) == -1) {
    //         return false;
    //     }

    //     deadline.tv_sec += static_cast<time_t>(m_overtime / 1000);
    //     deadline.tv_nsec += static_cast<long>(
    //         (m_overtime % 1000) * 1000000ULL
    //     );

    //     if (deadline.tv_nsec >= 1000000000L) {
    //         ++deadline.tv_sec;
    //         deadline.tv_nsec -= 1000000000L;
    //     }

    //     int result;

    //     do {
    //         result = ::sem_timedwait(semaphore, &deadline);
    //     } while (result == -1 && errno == EINTR);

    //     return result == 0;
    // }

    bool WriteWait(){
        while (true) {
            const std::uint32_t state =
                __atomic_load_n(
                    &m_msg.state,
                    __ATOMIC_ACQUIRE
                );

            if (state == 0) {
                return true;
            }

            const int result =
                FutexWait(&m_msg.state, 1);

            if (result == -1 &&
                errno != EAGAIN &&
                errno != EINTR) {
                return false;
            }
        }
    }

    void WriteFinish(){
        __atomic_store_n(
            &m_msg.state,
            1,
            __ATOMIC_RELEASE
        );

        FutexWake(&m_msg.state, 1);
    }

    bool ReadWait(){
        while (true) {
            const std::uint32_t state =
                __atomic_load_n(
                    &m_msg.state,
                    __ATOMIC_ACQUIRE
                );

            if (state == 1) {
                return true;
            }

            const int result =
                FutexWait(&m_msg.state, 0);

            if (result == -1 &&
                errno != EAGAIN &&
                errno != EINTR) {
                return false;
            }
        }
    }

    void ReadFinish(){
        __atomic_store_n(
            &m_msg.state,
            0,
            __ATOMIC_RELEASE
        );

        FutexWake(&m_msg.state, 1);
    }

    std::uint64_t GetTimestampUs(){
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    void Close(){
        if (m_address != nullptr &&
            m_address != MAP_FAILED) {
            ::munmap(
                m_address,
                m_shared_memory_size
            );

            m_address = nullptr;
            m_shared_memory_size = 0;
        }

        if (m_fd != -1) {
            ::close(m_fd);
            m_fd = -1;
        }

        if (IsSemaphoreValid(m_can_write)) {
            ::sem_close(m_can_write);
            m_can_write = SEM_FAILED;
        }

        if (IsSemaphoreValid(m_can_read)) {
            ::sem_close(m_can_read);
            m_can_read = SEM_FAILED;
        }

        if (m_owner) {
            Unlink();
        }

        m_owner = false;
    }
protected:
    bool IsSemaphoreValid(sem_t* semaphore) const {
        return semaphore != nullptr &&
            semaphore != SEM_FAILED;
    }

    void Unlink(){
        ::shm_unlink(m_name.c_str());
        ::sem_unlink(m_can_write_name.c_str());
        ::sem_unlink(m_can_read_name.c_str());
    }
private:
    struct Message{
        alignas(4) std::uint32_t    state;
        T                           data;
    }                               m_msg;
    bool                            m_owner;
    int                             m_fd;
    void*                           m_address;
    std::string                     m_name;
    std::string                     m_can_write_name;
    std::string                     m_can_read_name;
    size_t                          m_shared_memory_size;
    sem_t*                          m_can_write;
    sem_t*                          m_can_read;
    std::uint64_t                   m_overtime;
    std::shared_ptr<LoggerWithTag>  m_log;
};

#endif
