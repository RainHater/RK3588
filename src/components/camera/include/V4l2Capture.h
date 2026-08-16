#ifndef _V4L2_CAPTURE_H
#define _V4L2_CAPTURE_H

#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>

#include "Logger.h"

class V4L2Capture{
public:
    V4L2Capture(
        const std::string& device_name, 
        int width = 1280, 
        int heigt = 720,
        int fps = 30
    );
    ~V4L2Capture();

    //打开设备
    bool OpenDevice();
    //捕获摄像头的一帧图像
    bool CaptureFrame(cv::Mat &img);
    //关闭设备
    void CloseDevice();
public:
    static constexpr int BUFFER_COUNT = 4;
private:
    std::string m_device_name;
    std::shared_ptr<LoggerWithTag> m_logger;
    int m_fd;
    int m_width;
    int m_heigt;
    int m_fps;
    void* m_buffers[BUFFER_COUNT];
    size_t m_buffer_sizes[BUFFER_COUNT];
};

#endif
