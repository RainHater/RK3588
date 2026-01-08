#ifndef _V4L2_CAPTURE_H
#define _V4L2_CAPTURE_H

#include <iostream>

class V4L2Capture{
public:
    V4L2Capture(const std::string& device_name);
    ~V4L2Capture();

    //打开设备
    bool OpenDevice();
    //捕获摄像头的一帧图像
    bool CaptureFrame();
    //关闭设备
    void CloseDevice();
public:
    static constexpr int BUFFER_COUNT = 4;
private:
    std::string m_device_name;
    int m_fd;
    void* m_buffers[BUFFER_COUNT];
    size_t m_buffer_sizes[BUFFER_COUNT];
};

#endif
