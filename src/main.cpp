#include "Logger.h"
#include "V4l2Capture.h"
#include "FFmpegStreamer.h"
#include "ThreadPool.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>

#define DEVICE_NAME_1 "/dev/video1"
#define DEVICE_NAME_2 "/dev/video3"
#define DEVICE_NAME_3 "/dev/video6"

static std::atomic_bool g_running{true};

void SignalHandler(int)
{
    g_running = false;
}

int main()
{
    // std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    auto log = LoggerWithTag::GetLogger("main");

    std::vector<std::unique_ptr<V4L2Capture>> capture_v;
    std::vector<std::unique_ptr<FFmpegStreamer>> stream_v;

    std::vector<std::string> device_name_v = {
        DEVICE_NAME_1,
        DEVICE_NAME_2,
        DEVICE_NAME_3
    };

    const int camera_len = static_cast<int>(device_name_v.size());

    capture_v.reserve(camera_len);
    stream_v.reserve(camera_len);

    /*
     * 先初始化所有摄像头和推流器
     */
    for (int i = 0; i < camera_len; i++) {
        auto capture = std::make_unique<V4L2Capture>(
            device_name_v[i],
            1280,
            720,
            30
        );

        if (!capture->OpenDevice()) {
            log->error("摄像头 {} 打开失败: {}", i, device_name_v[i]);
            return -1;
        }

        auto stream = std::make_unique<FFmpegStreamer>(
            "rtsp",
            "live" + std::to_string(i)
        );

        if (stream->Initialize(1280, 720, 30) != 0) {
            log->error("摄像头 {} 推流器初始化失败", i);
            return -1;
        }

        capture_v.push_back(std::move(capture));
        stream_v.push_back(std::move(stream));
    }

    /*
     * 三个摄像头，三个线程
     */
    ThreadPool pool(camera_len);
    pool.Start();

    std::vector<std::future<void>> futures;
    futures.reserve(camera_len);

    for (int i = 0; i < camera_len; i++) {
        futures.emplace_back(
            pool.Enqueue([i, &capture_v, &stream_v, log]() {
                cv::Mat img;

                log->info("摄像头 {} 线程启动", i);

                while (g_running.load()) {
                    if (!capture_v[i]->CaptureFrame(img)) {
                        log->warn("摄像头 {} 采集失败", i);

                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(10)
                        );

                        continue;
                    }

                    stream_v[i]->EncoderPushStream(img);
                }

                log->info("摄像头 {} 线程退出", i);
            })
        );
    }

    log->info("开始运行，按 Ctrl+C 退出...");

    /*
     * 等待三个摄像头线程退出
     */
    for (auto& f : futures) {
        try {
            f.get();
        } catch (const std::exception& e) {
            log->error("线程异常: {}", e.what());
        }
    }

    pool.Stop();

    log->info("程序退出");

    return 0;
}