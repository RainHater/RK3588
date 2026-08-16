#ifndef RKPLATFORM_DEVICE_VIDEO_DEVICE_H
#define RKPLATFORM_DEVICE_VIDEO_DEVICE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace rkplatform::device {

enum class PixelFormat {
    kBgr24,
};

struct VideoFrame {
    int width = 0;
    int height = 0;
    std::size_t stride_bytes = 0;
    PixelFormat format = PixelFormat::kBgr24;
    std::vector<std::uint8_t> data;
};

struct CameraConfig {
    std::string device_name;
    int width = 1280;
    int height = 720;
    int fps = 30;
};

class ICamera {
public:
    // 释放摄像头接口。
    virtual ~ICamera() = default;

    // 打开并启动摄像头。
    virtual bool Open() = 0;

    // 在超时时间内捕获一帧图像。
    virtual bool Capture(VideoFrame& frame, int timeout_ms) = 0;

    // 停止并关闭摄像头。
    virtual void Close() noexcept = 0;
};

class CameraDevice final : public ICamera {
public:
    // 创建摄像头设备。
    explicit CameraDevice(CameraConfig config);

    // 释放摄像头设备。
    ~CameraDevice() override;

    // 禁止复制摄像头设备。
    CameraDevice(const CameraDevice&) = delete;
    // 禁止复制赋值摄像头设备。
    CameraDevice& operator=(const CameraDevice&) = delete;

    // 打开并启动摄像头。
    bool Open() override;

    // 在超时时间内捕获一帧图像。
    bool Capture(VideoFrame& frame, int timeout_ms) override;

    // 停止并关闭摄像头。
    void Close() noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

struct StreamConfig {
    std::string encoder_name = "h264_rkmpp";
    std::string stream_mode = "rtsp";
    std::string stream_name = "live";
    int width = 1280;
    int height = 720;
    int fps = 30;
};

class IFrameStreamer {
public:
    // 释放视频推流接口。
    virtual ~IFrameStreamer() = default;

    // 初始化视频推流能力。
    virtual bool Initialize() = 0;

    // 推送一帧视频数据。
    virtual bool PushFrame(const VideoFrame& frame) = 0;
};

class FrameStreamerDevice final : public IFrameStreamer {
public:
    // 创建视频推流设备。
    explicit FrameStreamerDevice(StreamConfig config);

    // 释放视频推流设备。
    ~FrameStreamerDevice() override;

    // 禁止复制视频推流设备。
    FrameStreamerDevice(const FrameStreamerDevice&) = delete;
    // 禁止复制赋值视频推流设备。
    FrameStreamerDevice& operator=(const FrameStreamerDevice&) = delete;

    // 初始化视频推流能力。
    bool Initialize() override;

    // 推送一帧视频数据。
    bool PushFrame(const VideoFrame& frame) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace rkplatform::device

#endif  // RKPLATFORM_DEVICE_VIDEO_DEVICE_H
