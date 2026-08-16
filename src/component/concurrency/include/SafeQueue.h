#ifndef _SAFE_QUEUE_H
#define _SAFE_QUEUE_H

#include <cstddef>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace rkplatform::component {

template<typename T>
class SafeQueue {
public:
    // 向队列尾部添加元素。
    void Push(const T& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // 阻塞等待并弹出队首元素。
    T Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

    // 尝试弹出队首元素。
    bool TryPop(T& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        data = m_queue.front();
        m_queue.pop();
        return true;
    }

    // 判断队列是否为空。
    bool Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    // 获取当前队列元素数量。
    std::size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

}  // namespace rkplatform::component

#endif
