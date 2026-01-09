#include <iostream>
#include "V4l2Capture.h"
#include "Logger.h"

#define DEVICE_NAME "/dev/video1"

int main() {
    Logger::init("data/logs");
    auto logger = Logger::defaultLogger();
    V4L2Capture capture(DEVICE_NAME);
    if (!capture.OpenDevice()) {
        return -1;
    }
    logger->info("开始运行...");
    while (true) {
        if (!capture.CaptureFrame()) {
            break;
        }
    }

    return 0;
}
