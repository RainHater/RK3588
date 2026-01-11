#ifndef _FFMPEG_STREAMER_H
#define _FFMPEG_STREAMER_H

#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/dict.h>
}

#ifndef MPP_ALIGN
#define MPP_ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

class FFmpegStreamer {
public:
    FFmpegStreamer();
    ~FFmpegStreamer();
    int Initialize(int width, int height, int fps);
    void EncoderPushStream(cv::Mat frame);
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
};

#endif
