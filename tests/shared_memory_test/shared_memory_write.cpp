#include "SharedMemoryDef.h"
#include "SharedMemory.h"

#include <chrono>
#include <thread>

int main(){
    
    SharedMemory<SharedData> share_memory;
    std::shared_ptr<LoggerWithTag>  log = LoggerWithTag::GetLogger("SharedMemoryWrite");

    if (!share_memory.Open(SHARE_MEMORY_NAME)){
        log->error("share_memory 初始化失败!");
        return 1;
    }

    auto data = share_memory.Data();

    log->info("开始运行!");
    for (int i = 0; i < 10; i ++){
        share_memory.WriteWait();
        data->sequence = i;
        data->value = float(100.0f / (i+1));
        data->timestamp = share_memory.GetTimestampUs();
        share_memory.WriteFinish();
        log->info("发送消息: {}", data->timestamp);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    
    }

    share_memory.Close();

    return 0;
}
