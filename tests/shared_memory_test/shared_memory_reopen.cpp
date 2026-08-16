#include "SharedMemoryDef.h"
#include "SharedMemory.h"

#include <string>

#include <unistd.h>

int main() {
    rkplatform::platform::SharedMemory<SharedData> shared_memory;
    const std::string name =
        "/rkplatform_shared_memory_reopen_" + std::to_string(::getpid());

    if (!shared_memory.Open(name)) {
        return 1;
    }

    SharedData data{1, 1.0};
    if (!shared_memory.Write(data)) {
        return 1;
    }
    shared_memory.Close();

    if (!shared_memory.Open(name)) {
        return 1;
    }

    data.sequence = 2;
    if (!shared_memory.Write(data)) {
        return 1;
    }
    shared_memory.Close();

    return 0;
}
