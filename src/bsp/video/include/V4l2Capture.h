#ifndef _V4L2_CAPTURE_H
#define _V4L2_CAPTURE_H

#include <cstddef>
#include <memory>
#include <string>
#include <opencv2/core/mat.hpp>

#include "Logger.h"

namespace rkplatform::bsp {

class V4L2Capture {
public:
    // 创建 V4L2 摄像头适配器。
    V4L2Capture(
        const std::string& device_name, 
        int width = 1280, 
        int height = 720,
        int fps = 30
    );
    // 释放摄像头资源。
    ~V4L2Capture();

    // 打开并启动摄像头设备。
    bool OpenDevice();
    // 在超时时间内捕获摄像头的一帧图像。
    bool CaptureFrame(cv::Mat& img, int timeout_ms = 1000);
    // 停止并关闭摄像头设备。
    void CloseDevice();
public:
    static constexpr int BUFFER_COUNT = 4;
private:
    std::string m_device_name;
    std::shared_ptr<spdlog::logger> m_logger;
    int m_fd;
    int m_width;
    int m_height;
    int m_fps;
    std::size_t m_buffer_count;
    void* m_buffers[BUFFER_COUNT];
    size_t m_buffer_sizes[BUFFER_COUNT];
};

}  // namespace rkplatform::bsp

#endif
