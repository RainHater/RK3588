#include <iostream>
#include <chrono>
#include "ThreadPool.h"

int main() {
    ThreadHeadler::Instance()->Initialize(10);
    ThreadHeadler::Instance()->Start();
    ThreadHeadler::Instance()->Enqueue([](){
        while (ThreadHeadler::Instance()->GetState()){
            std::cout << "Thread1 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadHeadler::Instance()->Enqueue([](){
        while (ThreadHeadler::Instance()->GetState()){
            std::cout << "Thread2 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadHeadler::Instance()->Enqueue([](){
        while (ThreadHeadler::Instance()->GetState()){
            std::cout << "Thread3 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    ThreadHeadler::Instance()->Stop();
    ThreadHeadler::Instance()->Wait();
    return 0;
}
