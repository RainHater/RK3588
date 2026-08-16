#include "SharedMemoryDef.h"
#include "SharedMemory.h"
#include "Logger.h"

#include <chrono>
#include <thread>

int main(){
    
    rkplatform::platform::SharedMemory<SharedData> share_memory;
    auto log = rkplatform::component::logging::GetLogger("SharedMemoryRead");

    if (!share_memory.Open(SHARE_MEMORY_NAME)){
        log->error("share_memory 初始化失败!");
        return 1;
    }

    log->info("share_memory 初始化成功");

    SharedData  data{};
    std::uint64_t message_timestamp_us = 0;
    int         sequence = 0;

    log->info("开始运行!");
    while(sequence < 9){
        if (!share_memory.ReadIfNew(data, message_timestamp_us)){
            continue;
        }
        auto end_time = share_memory.GetTimestampUs() - message_timestamp_us;
        sequence = data.sequence;
        log->info("耗时: {} us-> sequence: {}, value: {}", end_time, data.sequence, data.value);
    }

    share_memory.Close();

    return 0;
}
