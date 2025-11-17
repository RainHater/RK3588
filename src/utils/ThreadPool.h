#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <cstddef>
#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>

class ThreadPool{
public:
    ThreadPool();
    ~ThreadPool();
    static ThreadPool* instance();
    //初始化
    void initialize(size_t num_threads);
    //线程开始
    void start();
    //线程结束
    void stop();
    //等待线程
    void wait();
    //添加任务
    template <typename F, typename... Args>
    void enqueue(F&& f, Args&&... args){
        auto task = std::make_shared<std::packaged_task<void()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_tasks.push([task]() {(*task)(); });
        }
        m_cond.notify_one();
    }
    //获取线程状态
    bool get_state();
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    
    std::mutex m_queue_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_stop_flag;
    
    size_t m_num_threads;
};

#endif
