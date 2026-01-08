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
    ThreadPool()
        : m_stop_flag(false){}
    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_stop_flag = true;
        }
        m_cond.notify_all();
        for (std::thread &t : m_workers){
            if (t.joinable()){
                t.join();
            }
        }
    }
    static ThreadPool* instance(){
        static ThreadPool thread_pool;

        return &thread_pool;
    }

    //初始化
    void Initialize(size_t num_threads){
        m_num_threads = num_threads;
    }
    //线程开始
    void Start(){
        for (size_t i = 0; i < m_num_threads; i ++){
            m_workers.emplace_back([this](){
                while(true){
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_queue_mutex);
                        m_cond.wait(lock, [this]() {return m_stop_flag || !m_tasks.empty();});
                        
                        if (m_stop_flag && m_tasks.empty())
                            return;
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    //线程结束
    void Stop(){
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_stop_flag = true;
        }
    }
    //等待线程
    void Wait(){
        while(!m_stop_flag);
    }
    //添加任务
    template <typename F, typename... Args>
    void Enqueue(F&& f, Args&&... args){
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
    bool GetState(){
        return !m_stop_flag.load();
    }
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    
    std::mutex m_queue_mutex;
    std::condition_variable m_cond;
    std::atomic<bool> m_stop_flag;
    
    size_t m_num_threads;
};

#endif
