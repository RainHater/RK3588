#ifndef _SHARE_MEMORY_DEF_H
#define _SHARE_MEMORY_DEF_H

#include <stdint.h>

#define SHARE_MEMORY_NAME       "SharedMemoryTest"

struct SharedData{
    int         sequence;
    double      value;
    uint64_t    timestamp;
};

#endif
