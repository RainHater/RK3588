#include "Logger.h"
#include "V4l2Capture.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define DEVICE_NAME_1 "/dev/video1"
#define DEVICE_NAME_2 "/dev/video3"
#define DEVICE_NAME_3 "/dev/video5"

int main() {
    auto log = LoggerWithTag::GetLogger("main");

    std::vector<std::unique_ptr<V4L2Capture>> capture_v;
    std::vector<std::unique_ptr<FFmpegStreamer>> stream_v;

    std::vector<std::string> device_name_v = {
        DEVICE_NAME_1,
        DEVICE_NAME_2,
        DEVICE_NAME_3
    };

    int camera_len = device_name_v.size();

    std::vector<cv::Mat> img_v(camera_len);

    capture_v.reserve(camera_len);
    stream_v.reserve(camera_len);

    for (int i = 0; i < camera_len; i++) {
        auto capture = std::make_unique<V4L2Capture>(
            device_name_v[i], 1280, 720, 30
        );

        auto stream = std::make_unique<FFmpegStreamer>(
            "rtsp", "live" + std::to_string(i)
        );

        stream->Initialize(1280, 720, 30);

        if (!capture->OpenDevice()) {
            log->error("摄像头 {} 打开失败: {}", i, device_name_v[i]);
            return -1;
        }

        capture_v.push_back(std::move(capture));
        stream_v.push_back(std::move(stream));
    }

    log->info("开始运行...");

    while (true) {
        for (int i = 0; i < camera_len; i++) {
            if (!capture_v[i]->CaptureFrame(img_v[i])) {
                log->warn("摄像头 {} 打开失败: {}", i);
                continue;
            }

            stream_v[i]->EncoderPushStream(img_v[i]);
        }
    }

    return 0;
}