#include <iostream>
#include "Logger.h"
#include "V4l2Capture.h"

#define DEVICE_NAME_1 "/dev/video1"
#define DEVICE_NAME_2 "/dev/video3"

int main() {
    auto log = LoggerWithTag::GetLogger("main");
    V4L2Capture capture_1(DEVICE_NAME_1, 1280, 720, 30);
    V4L2Capture capture_2(DEVICE_NAME_2, 1280, 720, 30);
    FFmpegStreamer stream_1("rtsp", "live1");
    FFmpegStreamer stream_2("rtsp", "live2");
    stream_1.Initialize(1280, 720, 30);
    stream_2.Initialize(1280, 720, 30);
    cv::Mat img_1;
    cv::Mat img_2;
    log->info("开始运行...");
    if (!capture_1.OpenDevice()) {
        return -1;
    }
    if (!capture_2.OpenDevice()) {
        return -1;
    }
    while (true) {
        if (!capture_1.CaptureFrame(img_1)) {
            break;
        }else {
            stream_1.EncoderPushStream(img_1);
        }
        if (!capture_2.CaptureFrame(img_2)) {
            break;
        }else {
            stream_2.EncoderPushStream(img_2);
        }
    }

    return 0;
}
