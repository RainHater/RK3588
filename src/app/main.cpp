#include <iostream>
#include <chrono>
#include "ThreadPool.h"

int main() {
    ThreadPool::instance()->initialize(10);
    ThreadPool::instance()->start();
    ThreadPool::instance()->enqueue([](){
        while (ThreadPool::instance()->get_state()){
            std::cout << "Thread1 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadPool::instance()->enqueue([](){
        while (ThreadPool::instance()->get_state()){
            std::cout << "Thread2 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadPool::instance()->enqueue([](){
        while (ThreadPool::instance()->get_state()){
            std::cout << "Thread3 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadPool::instance()->stop();
    ThreadPool::instance()->wait();
    return 0;
}
