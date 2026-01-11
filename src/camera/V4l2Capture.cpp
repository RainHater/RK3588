#include "V4l2Capture.h"
#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

V4L2Capture::V4L2Capture(const std::string& deviceName)
    : m_device_name(deviceName)
    , m_fd(-1)
{
    for (int i = 0; i < BUFFER_COUNT; ++i) {
        m_buffers[i] = nullptr;
        m_buffer_sizes[i] = 0;
    }

    if (m_streamer.Initialize(1920, 1080, 24) != 0) {
        spdlog::error("推流初始化失败");
    }else {
        spdlog::info("推流初始化成功");
    }
}

V4L2Capture::~V4L2Capture(){
    CloseDevice();
}

bool V4L2Capture::OpenDevice(){
    m_fd = open(m_device_name.c_str(), O_RDWR);
    if (m_fd < 0) {
        spdlog::error("打开设备失败!");
        return false;
    }

    v4l2_capability cap{};
    if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        spdlog::error("VIDIOC_QUERYCAP 失败!");
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        spdlog::error("不是摄像头设备");
        return false;
    }

    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 1920;
    fmt.fmt.pix.height = 1080;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0) {
        spdlog::error("VIDIOC_S_FMT 失败");
        return false;
    }

    v4l2_requestbuffers req{};
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0) {
        spdlog::error("VIDIOC_REQBUFS 失败");
        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(m_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            spdlog::error("VIDIOC_QUERYBUF 失败");
            return false;
        }

        m_buffers[i] = mmap(nullptr, buf.length,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED, m_fd, buf.m.offset);
        m_buffer_sizes[i] = buf.length;

        if (m_buffers[i] == MAP_FAILED) {
            spdlog::error("mmap 失败");
            return false;
        }

        ioctl(m_fd, VIDIOC_QBUF, &buf);
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
        spdlog::error("VIDIOC_STREAMON 失败");
        return false;
    }

    return true;
}

bool V4L2Capture::CaptureFrame(){
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_DQBUF, &buf) < 0) {
        spdlog::error("VIDIOC_DQBUF 失败");
        return false;
    }

    cv::Mat img = cv::imdecode(
        cv::Mat(1, buf.bytesused, CV_8UC1, m_buffers[buf.index]),
        cv::IMREAD_COLOR
    );

    if (!img.empty()) {
        m_streamer.EncoderPushStream(img);
        cv::imshow("MJPEG Stream", img);
        cv::waitKey(1);
    }

    // 关键：用完必须放回去
    if (ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) {
        spdlog::error("VIDIOC_QBUF 失败");
        return false;
    }

    return true;
}

void V4L2Capture::CloseDevice(){
    if (m_fd < 0) return;

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(m_fd, VIDIOC_STREAMOFF, &type);

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        if (m_buffers[i]) {
            munmap(m_buffers[i], m_buffer_sizes[i]);
        }
    }

    close(m_fd);
    m_fd = -1;
}
