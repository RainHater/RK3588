#include "VideoDevice.h"

#include "FFmpegStreamer.h"
#include "V4l2Capture.h"

#include <limits>
#include <utility>

#include <opencv2/core/mat.hpp>

namespace rkplatform::device {

struct CameraDevice::Impl {
    explicit Impl(const CameraConfig& config)
        : capture(config.device_name, config.width, config.height, config.fps)
    {}

    bsp::V4L2Capture capture;
};

CameraDevice::CameraDevice(CameraConfig config)
    : m_impl(std::make_unique<Impl>(config))
{}

CameraDevice::~CameraDevice() = default;

bool CameraDevice::Open() {
    return m_impl->capture.OpenDevice();
}

bool CameraDevice::Capture(VideoFrame& frame, int timeout_ms) {
    cv::Mat image;
    if (!m_impl->capture.CaptureFrame(image, timeout_ms)) {
        return false;
    }

    if (!image.isContinuous()) {
        image = image.clone();
    }

    frame.width = image.cols;
    frame.height = image.rows;
    frame.stride_bytes = image.step[0];
    frame.format = PixelFormat::kBgr24;
    frame.data.assign(
        image.data,
        image.data + frame.stride_bytes * static_cast<std::size_t>(frame.height)
    );
    return true;
}

void CameraDevice::Close() noexcept {
    m_impl->capture.CloseDevice();
}

struct FrameStreamerDevice::Impl {
    explicit Impl(StreamConfig stream_config)
        : config(std::move(stream_config))
        , streamer(
              config.encoder_name,
              config.stream_mode,
              config.stream_name)
    {}

    StreamConfig config;
    bsp::FFmpegStreamer streamer;
    bool initialized = false;
};

FrameStreamerDevice::FrameStreamerDevice(StreamConfig config)
    : m_impl(std::make_unique<Impl>(std::move(config)))
{}

FrameStreamerDevice::~FrameStreamerDevice() = default;

bool FrameStreamerDevice::Initialize() {
    m_impl->initialized =
        m_impl->streamer.Initialize(
            m_impl->config.width,
            m_impl->config.height,
            m_impl->config.fps) == 0;
    return m_impl->initialized;
}

bool FrameStreamerDevice::PushFrame(const VideoFrame& frame) {
    if (!m_impl->initialized || frame.format != PixelFormat::kBgr24 ||
        frame.width <= 0 || frame.height <= 0 || frame.stride_bytes == 0) {
        return false;
    }

    const auto height = static_cast<std::size_t>(frame.height);
    if (height > std::numeric_limits<std::size_t>::max() / frame.stride_bytes ||
        frame.data.size() < height * frame.stride_bytes) {
        return false;
    }

    const auto minimum_stride = static_cast<std::size_t>(frame.width) * 3U;
    if (frame.stride_bytes < minimum_stride) {
        return false;
    }

    cv::Mat image(
        frame.height,
        frame.width,
        CV_8UC3,
        const_cast<std::uint8_t*>(frame.data.data()),
        frame.stride_bytes
    );
    return m_impl->streamer.EncoderPushStream(image);
}

}  // namespace rkplatform::device
