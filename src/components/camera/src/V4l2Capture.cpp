#include "V4l2Capture.h"
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

V4L2Capture::V4L2Capture(
    const std::string& deviceName,
    int width, 
    int heigt,
    int fps
)
    : m_device_name(deviceName)
    , m_logger(LoggerWithTag::GetLogger("V4L2Capture_" + deviceName))
    , m_fd(-1)
    , m_width(width)
    , m_heigt(heigt)
    , m_fps(fps)
{   
    for (int i = 0; i < BUFFER_COUNT; ++i) {
        m_buffers[i] = nullptr;
        m_buffer_sizes[i] = 0;
    }
}

V4L2Capture::~V4L2Capture(){
    CloseDevice();
}

bool V4L2Capture::OpenDevice(){
    m_fd = open(m_device_name.c_str(), O_RDWR);
    if (m_fd < 0) {
        m_logger->error("打开设备失败!");
        return false;
    }

    v4l2_capability cap{};
    if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        m_logger->error("VIDIOC_QUERYCAP 失败!");
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        m_logger->error("不是摄像头设备");
        return false;
    }

    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_width;
    fmt.fmt.pix.height = m_heigt;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0) {
        m_logger->error("VIDIOC_S_FMT 失败");
        return false;
    }

    v4l2_streamparm parm{};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // 设置帧率：fps = denominator / numerator
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = m_fps;
    if (ioctl(m_fd, VIDIOC_S_PARM, &parm) < 0) {
    m_logger->warn("VIDIOC_S_PARM 设置帧率失败，摄像头可能不支持手动设置 fps");
    } else {
        m_logger->info(
            "设置摄像头帧率: {}/{} fps",
            parm.parm.capture.timeperframe.denominator,
            parm.parm.capture.timeperframe.numerator
        );
    }

    v4l2_requestbuffers req{};
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0) {
        m_logger->error("VIDIOC_REQBUFS 失败");
        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(m_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            m_logger->error("VIDIOC_QUERYBUF 失败");
            return false;
        }

        m_buffers[i] = mmap(nullptr, buf.length,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED, m_fd, buf.m.offset);
        m_buffer_sizes[i] = buf.length;

        if (m_buffers[i] == MAP_FAILED) {
            m_logger->error("mmap 失败");
            return false;
        }

        ioctl(m_fd, VIDIOC_QBUF, &buf);
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
        m_logger->error("VIDIOC_STREAMON 失败");
        return false;
    }

    return true;
}

bool V4L2Capture::CaptureFrame(cv::Mat &img){
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_DQBUF, &buf) < 0) {
        m_logger->error("VIDIOC_DQBUF 失败");
        return false;
    }

    img = cv::imdecode(
        cv::Mat(1, buf.bytesused, CV_8UC1, m_buffers[buf.index]),
        cv::IMREAD_COLOR
    );

    if (!img.empty()) {
        // cv::imshow("MJPEG Stream", img);
        // cv::waitKey(1);
    }

    // 关键：用完必须放回去
    if (ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) {
        m_logger->error("VIDIOC_QBUF 失败");
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
