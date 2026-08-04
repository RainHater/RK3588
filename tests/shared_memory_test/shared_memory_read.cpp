#include "SharedMemoryDef.h"
#include "SharedMemory.h"
#include "Logger.h"

#include <chrono>
#include <thread>

int main(){
    
    SharedMemory<SharedData> share_memory;
    std::shared_ptr<LoggerWithTag>  log = LoggerWithTag::GetLogger("SharedMemoryRead");

    if (!share_memory.Open(SHARE_MEMORY_NAME)){
        log->error("share_memory 初始化失败!");
        return 1;
    }

    auto data = share_memory.Data();
    uint64_t    last_timestamp = 0;
    int         sequence = 0;

    log->info("开始运行!");
    while(sequence < 9){
        share_memory.ReadWait();
        if (last_timestamp != data->timestamp){
            last_timestamp = data->timestamp;
            uint64_t end_time = share_memory.GetTimestampUs() - last_timestamp;
            sequence = data->sequence;
            log->info("接收到消息过程耗时: {} us -> sequence: {}, value: {}, timestamp: {}", end_time, data->sequence, data->value, data->timestamp);
        }
        share_memory.ReadFinish();
    }

    share_memory.Close();

    return 0;
}
