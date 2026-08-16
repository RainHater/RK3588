#include "SharedMemoryDef.h"
#include "SharedMemory.h"

#include <chrono>
#include <thread>

int main(){
    
    rkplatform::platform::SharedMemory<SharedData> share_memory;
    auto log = rkplatform::component::logging::GetLogger("SharedMemoryWrite");

    if (!share_memory.Open(SHARE_MEMORY_NAME)){
        log->error("share_memory 初始化失败!");
        return 1;
    }

    log->info("share_memory 初始化成功"); 

    log->info("开始运行!");
    for (int i = 0; i < 10; i ++){
        SharedData data{};
        data.sequence = i;
        data.value = float(100.0f / (i+1));
        log->info("发送消息");
        if (!share_memory.Write(data)) {
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));    
    }

    share_memory.Close();

    return 0;
}
