#include <iostream>
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#define DEVICE_NAME "/dev/video1"

class V4L2Capture {
public:
    V4L2Capture(const std::string& device_name) : device_name_(device_name), fd_(-1), buffer_(nullptr), buffer_size_(0) {}
    ~V4L2Capture() { close_device(); }

    bool open_device() {
        setenv("DISPLAY", ":10.0", 1);
        fd_ = open(device_name_.c_str(), O_RDWR);
        if (fd_ == -1) {
            std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
            return false;
        }

        // Query the device capabilities
        struct v4l2_capability cap;
        if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) == -1) {
            std::cerr << "Failed to query capabilities: " << strerror(errno) << std::endl;
            return false;
        }

        // Check if the device supports video capture
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            std::cerr << "Device does not support video capture" << std::endl;
            return false;
        }

        // Set video format to MJPEG
        struct v4l2_format fmt;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 1920;
        fmt.fmt.pix.height = 1080;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        if (ioctl(fd_, VIDIOC_S_FMT, &fmt) == -1) {
            std::cerr << "Failed to set format: " << strerror(errno) << std::endl;
            return false;
        }

        // Request buffers
        struct v4l2_requestbuffers req;
        req.count = 1;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd_, VIDIOC_REQBUFS, &req) == -1) {
            std::cerr << "Failed to request buffers: " << strerror(errno) << std::endl;
            return false;
        }

        // Map the buffer
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;

        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
            std::cerr << "Failed to query buffer: " << strerror(errno) << std::endl;
            return false;
        }

        buffer_size_ = buf.length;
        buffer_ = mmap(NULL, buffer_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);

        if (buffer_ == MAP_FAILED) {
            std::cerr << "Failed to map buffer: " << strerror(errno) << std::endl;
            return false;
        }

        return true;
    }

    bool capture_frame() {
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;

        // Queue the buffer to capture a frame
        if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
            std::cerr << "Failed to queue buffer: " << strerror(errno) << std::endl;
            return false;
        }

        // Start capturing
        if (ioctl(fd_, VIDIOC_STREAMON, &buf.type) == -1) {
            std::cerr << "Failed to start stream: " << strerror(errno) << std::endl;
            return false;
        }

        // Dequeue the buffer after capture
        if (ioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
            std::cerr << "Failed to dequeue buffer: " << strerror(errno) << std::endl;
            return false;
        }

        // Convert MJPEG to OpenCV format
        cv::Mat img = cv::imdecode(cv::Mat(1, buffer_size_, CV_8UC1, buffer_), cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cerr << "Failed to decode MJPEG frame" << std::endl;
            return false;
        }

        // Display the image using OpenCV
        cv::imshow("MJPEG Stream", img);
        cv::waitKey(1);

        return true;
    }

    void close_device() {
        if (fd_ != -1) {
            ioctl(fd_, VIDIOC_STREAMOFF, nullptr);
            close(fd_);
            fd_ = -1;
        }
    }

private:
    std::string device_name_;
    int fd_;
    void* buffer_;
    size_t buffer_size_;
};

int main() {
    V4L2Capture capture(DEVICE_NAME);
    if (!capture.open_device()) {
        return -1;
    }

    while (true) {
        if (!capture.capture_frame()) {
            break;
        }
    }

    return 0;
}
