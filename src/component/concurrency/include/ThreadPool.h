#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <type_traits>
#include <utility>

namespace rkplatform::component {

class ThreadPool {
public:
    // 创建指定工作线程数量的线程池。
    explicit ThreadPool(size_t num_threads)
        : m_num_threads(num_threads)
        , m_stop(false)
        , m_started(false)
    {
        if (num_threads == 0)
            throw std::invalid_argument("ThreadPool: num_threads == 0");
    }

    // 禁止复制线程池。
    ThreadPool(const ThreadPool&) = delete;
    // 禁止复制赋值线程池。
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 停止线程池并回收工作线程。
    ~ThreadPool() {
        Stop();
    }

    // 启动工作线程。
    void Start() {
        if (m_stop.load()) {
            throw std::runtime_error("ThreadPool cannot restart after Stop");
        }

        bool expected = false;
        if (!m_started.compare_exchange_strong(expected, true))
            return;

        for (size_t i = 0; i < m_num_threads; ++i) {
            m_workers.emplace_back([this] {
                WorkerLoop();
            });
        }
    }

    // 提交异步任务并返回结果句柄。
    template<typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        if (!m_started)
            throw std::runtime_error("ThreadPool not started");

        using Ret = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<Ret> future = task->get_future();

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_stop)
                throw std::runtime_error("ThreadPool already stopped");

            m_tasks.emplace([task]() { (*task)(); });
        }

        m_cond.notify_one();
        return future;
    }

    // 停止线程池并等待已有任务完成。
    void Stop() {
        m_stop.store(true);
        m_cond.notify_all();

        for (auto& t : m_workers) {
            if (t.joinable())
                t.join();
        }
        m_workers.clear();
    }

private:
    // 执行工作线程循环。
    void WorkerLoop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cond.wait(lock, [this] {
                    return m_stop || !m_tasks.empty();
                });

                if (m_stop && m_tasks.empty())
                    return;

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            task();
        }
    }

private:
    size_t m_num_threads;
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;

    std::mutex m_mutex;
    std::condition_variable m_cond;

    std::atomic<bool> m_stop;
    std::atomic<bool> m_started;
};

}  // namespace rkplatform::component

#endif
