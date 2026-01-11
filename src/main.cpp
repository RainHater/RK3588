#include <iostream>
#include <spdlog/spdlog.h>
#include "V4l2Capture.h"

#define DEVICE_NAME "/dev/video1"

int main() {
    V4L2Capture capture(DEVICE_NAME);
    if (!capture.OpenDevice()) {
        return -1;
    }
    spdlog::info("开始运行...");
    while (true) {
        if (!capture.CaptureFrame()) {
            break;
        }
    }

    return 0;
}
