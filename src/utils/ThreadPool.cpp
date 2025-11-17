#include "ThreadPool.h"
#include <utility>

ThreadPool::ThreadPool() 
    : m_stop_flag(false){}

ThreadPool::~ThreadPool(){
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

ThreadPool* ThreadPool::instance(){
    static ThreadPool thread_pool;

    return &thread_pool;
}

void ThreadPool::initialize(size_t num_threads){
    m_num_threads = num_threads;
}

void ThreadPool::start(){
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

void ThreadPool::stop(){
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop_flag = true;
    }
    m_cond.notify_all();
}

void ThreadPool::wait(){
    while(!m_stop_flag);
}

bool ThreadPool::get_state(){
    return !m_stop_flag.load();
}
