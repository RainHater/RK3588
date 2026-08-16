#ifndef _FFMPEG_STREAMER_H
#define _FFMPEG_STREAMER_H

#include <memory>
#include <string>
#include <opencv2/core/mat.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/dict.h>
}

#include "Logger.h"

namespace rkplatform::bsp {

class FFmpegStreamer {
public:
    // 创建 Rockchip FFmpeg 推流适配器。
    FFmpegStreamer(std::string encoder_name = "h264_rkmpp", std::string stream_mode = "rtsp", std::string stream_name = "live");
    // 释放 FFmpeg 推流资源。
    ~FFmpegStreamer();
    // 初始化编码器和输出流。
    int Initialize(int width, int height, int fps);
    // 编码并推送一帧图像。
    bool EncoderPushStream(const cv::Mat& frame);
private:
    struct FFmpegInfo{
        AVCodecContext *enc_ctx;
        AVDictionary* param;
        AVFormatContext *fmt_ctx;
        AVStream *video_st;
        AVFrame *frame;
        SwsContext *sws_ctx;
        AVPacket *pkt;
        int64_t pts;
        int width;
        int height;
        int fps;
        bool header_written;
        bool initialized;
    };
    // 释放已初始化的 FFmpeg 资源。
    void Release() noexcept;
private:
    FFmpegInfo m_ffmpeg{};
    std::string m_stream_name;
    std::string m_stream_mode;
    std::string m_encoder_name;
    std::shared_ptr<spdlog::logger> m_logger;
};

}  // namespace rkplatform::bsp

#endif
