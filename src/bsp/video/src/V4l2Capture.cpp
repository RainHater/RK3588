#include "V4l2Capture.h"
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <cerrno>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

namespace rkplatform::bsp {
namespace {

int IoctlRetry(int fd, unsigned long request, void* argument) {
    int result = 0;
    do {
        result = ::ioctl(fd, request, argument);
    } while (result < 0 && errno == EINTR);
    return result;
}

}  // namespace

V4L2Capture::V4L2Capture(
    const std::string& deviceName,
    int width, 
    int height,
    int fps
)
    : m_device_name(deviceName)
    , m_logger(component::logging::GetLogger("V4L2Capture_" + deviceName))
    , m_fd(-1)
    , m_width(width)
    , m_height(height)
    , m_fps(fps)
    , m_buffer_count(0)
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
    CloseDevice();

    if (m_device_name.empty() || m_width <= 0 || m_height <= 0 || m_fps <= 0) {
        m_logger->error("摄像头配置无效");
        return false;
    }

    m_fd = open(m_device_name.c_str(), O_RDWR | O_NONBLOCK);
    if (m_fd < 0) {
        m_logger->error("打开设备失败!");
        return false;
    }

    v4l2_capability cap{};
    if (IoctlRetry(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        m_logger->error("VIDIOC_QUERYCAP 失败!");
        CloseDevice();
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        m_logger->error("不是摄像头设备");
        CloseDevice();
        return false;
    }

    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_width;
    fmt.fmt.pix.height = m_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (IoctlRetry(m_fd, VIDIOC_S_FMT, &fmt) < 0) {
        m_logger->error("VIDIOC_S_FMT 失败");
        CloseDevice();
        return false;
    }

    v4l2_streamparm parm{};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // 设置帧率：fps = denominator / numerator
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = m_fps;
    if (IoctlRetry(m_fd, VIDIOC_S_PARM, &parm) < 0) {
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

    if (IoctlRetry(m_fd, VIDIOC_REQBUFS, &req) < 0) {
        m_logger->error("VIDIOC_REQBUFS 失败");
        CloseDevice();
        return false;
    }

    m_buffer_count = std::min<std::size_t>(req.count, BUFFER_COUNT);
    if (m_buffer_count == 0) {
        m_logger->error("摄像头没有分配可用缓冲区");
        CloseDevice();
        return false;
    }

    for (std::size_t i = 0; i < m_buffer_count; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (IoctlRetry(m_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            m_logger->error("VIDIOC_QUERYBUF 失败");
            CloseDevice();
            return false;
        }

        m_buffers[i] = mmap(nullptr, buf.length,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED, m_fd, buf.m.offset);
        m_buffer_sizes[i] = buf.length;

        if (m_buffers[i] == MAP_FAILED) {
            m_buffers[i] = nullptr;
            m_buffer_sizes[i] = 0;
            m_logger->error("mmap 失败");
            CloseDevice();
            return false;
        }

        if (IoctlRetry(m_fd, VIDIOC_QBUF, &buf) < 0) {
            m_logger->error("VIDIOC_QBUF 初始化失败");
            CloseDevice();
            return false;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (IoctlRetry(m_fd, VIDIOC_STREAMON, &type) < 0) {
        m_logger->error("VIDIOC_STREAMON 失败");
        CloseDevice();
        return false;
    }

    return true;
}

bool V4L2Capture::CaptureFrame(cv::Mat& img, int timeout_ms){
    if (m_fd < 0) {
        m_logger->error("摄像头设备尚未打开");
        return false;
    }

    if (timeout_ms < 0) {
        m_logger->error("摄像头捕获超时时间无效");
        return false;
    }

    pollfd descriptor{};
    descriptor.fd = m_fd;
    descriptor.events = POLLIN;

    int poll_result = 0;
    do {
        poll_result = ::poll(&descriptor, 1, timeout_ms);
    } while (poll_result < 0 && errno == EINTR);

    if (poll_result == 0) {
        m_logger->warn("摄像头捕获超时: {} ms", timeout_ms);
        return false;
    }

    if (poll_result < 0 || !(descriptor.revents & POLLIN)) {
        m_logger->error("等待摄像头数据失败");
        return false;
    }

    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (IoctlRetry(m_fd, VIDIOC_DQBUF, &buf) < 0) {
        if (errno == EAGAIN) {
            m_logger->warn("摄像头数据暂不可用");
            return false;
        }
        m_logger->error("VIDIOC_DQBUF 失败");
        return false;
    }

    if (buf.index >= m_buffer_count || m_buffers[buf.index] == nullptr) {
        m_logger->error("V4L2 返回了无效缓冲区索引: {}", buf.index);
        return false;
    }

    if (buf.bytesused > m_buffer_sizes[buf.index]) {
        m_logger->error("V4L2 返回的数据长度超过缓冲区");
        IoctlRetry(m_fd, VIDIOC_QBUF, &buf);
        return false;
    }

    img = cv::imdecode(
        cv::Mat(1, buf.bytesused, CV_8UC1, m_buffers[buf.index]),
        cv::IMREAD_COLOR
    );

    if (IoctlRetry(m_fd, VIDIOC_QBUF, &buf) < 0) {
        m_logger->error("VIDIOC_QBUF 失败");
        return false;
    }

    if (img.empty()) {
        m_logger->error("MJPEG 图像解码失败");
        return false;
    }

    return true;
}

void V4L2Capture::CloseDevice(){
    if (m_fd < 0) return;

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    IoctlRetry(m_fd, VIDIOC_STREAMOFF, &type);

    for (std::size_t i = 0; i < m_buffer_count; ++i) {
        if (m_buffers[i] != nullptr && m_buffers[i] != MAP_FAILED) {
            munmap(m_buffers[i], m_buffer_sizes[i]);
        }
        m_buffers[i] = nullptr;
        m_buffer_sizes[i] = 0;
    }
    m_buffer_count = 0;

    close(m_fd);
    m_fd = -1;
}

}  // namespace rkplatform::bsp
