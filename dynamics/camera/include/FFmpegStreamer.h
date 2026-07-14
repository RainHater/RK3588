#ifndef _FFMPEG_STREAMER_H
#define _FFMPEG_STREAMER_H

#include <iostream>
#include <string.h>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/dict.h>
}

#include "Logger.h"

#ifndef MPP_ALIGN
#define MPP_ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

class FFmpegStreamer {
public:
    FFmpegStreamer(std::string encoder_name = "h264_rkmpp", std::string stream_mode = "rtsp", std::string stream_name = "live");
    ~FFmpegStreamer();
    int Initialize(int width, int height, int fps);
    void EncoderPushStream(const cv::Mat& frame);
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
    };
private:
    FFmpegInfo m_ffmpeg;
    std::string m_stream_name;
    std::string m_stream_mode;
    std::string m_encoder_name;
    std::shared_ptr<LoggerWithTag> m_logger;
};

#endif
