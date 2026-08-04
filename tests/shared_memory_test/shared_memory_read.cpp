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

    log->info("share_memory 初始化成功");

    auto data = share_memory.Data();
    int         sequence = 0;

    log->info("开始运行!");
    while(sequence < 9){
        if (!share_memory.ReadWait()){
            continue;
        }
        auto end_time = share_memory.GetTimestampUs() - share_memory.GetMessageTimestampUs();
        sequence = data->sequence;
        log->info("耗时: {} us-> sequence: {}, value: {}", end_time, data->sequence, data->value);
    }

    share_memory.Close();

    return 0;
}
