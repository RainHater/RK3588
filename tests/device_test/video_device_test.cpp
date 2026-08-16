#include "VideoDevice.h"

int main() {
    rkplatform::device::CameraConfig camera_config;
    camera_config.device_name = "";
    rkplatform::device::CameraDevice camera(camera_config);
    if (camera.Open()) {
        return 1;
    }

    rkplatform::device::StreamConfig stream_config;
    stream_config.width = 0;
    rkplatform::device::FrameStreamerDevice streamer(stream_config);
    if (streamer.Initialize()) {
        return 1;
    }

    rkplatform::device::VideoFrame empty_frame;
    return streamer.PushFrame(empty_frame) ? 1 : 0;
}
