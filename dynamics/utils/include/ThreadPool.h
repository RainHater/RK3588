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
#include "Singleton.h"

class ThreadPool : public Singleton<ThreadPool>{
public:
    explicit ThreadPool(size_t num_threads)
        : m_num_threads(num_threads)
        , m_stop(false)
        , m_started(false)
    {
        if (num_threads == 0)
            throw std::invalid_argument("ThreadPool: num_threads == 0");
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        Stop();
    }

    //启动线程池
    void Start() {
        bool expected = false;
        if (!m_started.compare_exchange_strong(expected, true))
            return;

        for (size_t i = 0; i < m_num_threads; ++i) {
            m_workers.emplace_back([this] {
                WorkerLoop();
            });
        }
    }

    //提交任务
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

    //停止线程池
    void Stop() {
        bool expected = false;
        if (!m_stop.compare_exchange_strong(expected, true))
            return;

        m_cond.notify_all();

        for (auto& t : m_workers) {
            if (t.joinable())
                t.join();
        }
    }

private:
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

#endif
