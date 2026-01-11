#include "FFmpegStreamer.h"

#include <spdlog/spdlog.h>

FFmpegStreamer::FFmpegStreamer(){
    m_ffmpeg.param = nullptr;
    m_ffmpeg.fmt_ctx = nullptr;
}

FFmpegStreamer::~FFmpegStreamer() {
    av_write_trailer(m_ffmpeg.fmt_ctx);
    avcodec_free_context(&m_ffmpeg.enc_ctx);
    av_frame_free(&m_ffmpeg.frame);
    av_packet_free(&m_ffmpeg.pkt);
    sws_freeContext(m_ffmpeg.sws_ctx);
    if (!(m_ffmpeg.fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_close(m_ffmpeg.fmt_ctx->pb);
    }
    avformat_free_context(m_ffmpeg.fmt_ctx);
}

int FFmpegStreamer::Initialize(int width, int height, int fps) {
    m_ffmpeg.width = width;
    m_ffmpeg.height = height;
    m_ffmpeg.fps = fps;

    const AVCodec *codec = avcodec_find_encoder_by_name("h264_rkmpp");
    if (!codec) {
        spdlog::error("未找到编码器 h264_rkmpp");
        return -1;
    }

    m_ffmpeg.enc_ctx = avcodec_alloc_context3(codec);
    m_ffmpeg.enc_ctx->width = m_ffmpeg.width;
    m_ffmpeg.enc_ctx->height = m_ffmpeg.height;
    m_ffmpeg.enc_ctx->pix_fmt = AV_PIX_FMT_NV12;
    m_ffmpeg.enc_ctx->time_base = AVRational{1, m_ffmpeg.fps};
    m_ffmpeg.enc_ctx->framerate = AVRational{m_ffmpeg.fps, 1};
    m_ffmpeg.enc_ctx->bit_rate = 2000000;
    m_ffmpeg.enc_ctx->gop_size = 30;

    av_dict_set(&m_ffmpeg.param, "profile", "main", 0);
    av_dict_set(&m_ffmpeg.param, "preset", "ultrafast", 0);
    av_dict_set_int(&m_ffmpeg.param, "rc_mode", 0, 0);  // CBR 模式

    if (avcodec_open2(m_ffmpeg.enc_ctx, codec, &m_ffmpeg.param) < 0) {
        spdlog::error("编码器打开失败");
        return -1;
    }

    av_dict_free(&m_ffmpeg.param);

    avformat_alloc_output_context2(&m_ffmpeg.fmt_ctx, nullptr, "rtsp", "rtsp://0.0.0.0:8554/live");

    if (!m_ffmpeg.fmt_ctx) {
        spdlog::error("RTSP 输出上下文创建失败");
        return -1;
    }

    m_ffmpeg.video_st = avformat_new_stream(m_ffmpeg.fmt_ctx, nullptr);
    m_ffmpeg.video_st->codecpar->codec_id = AV_CODEC_ID_H264;
    m_ffmpeg.video_st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    m_ffmpeg.video_st->codecpar->width = m_ffmpeg.enc_ctx->width;
    m_ffmpeg.video_st->codecpar->height = m_ffmpeg.enc_ctx->height;
    m_ffmpeg.video_st->codecpar->format = m_ffmpeg.enc_ctx->pix_fmt;
    m_ffmpeg.video_st->time_base = m_ffmpeg.enc_ctx->time_base;

    if (!(m_ffmpeg.fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_ffmpeg.fmt_ctx->pb, m_ffmpeg.fmt_ctx->url, AVIO_FLAG_WRITE) < 0) {
            spdlog::error("无法打开 RTSP 输出 URL");
            return -1;
        }
    }

    if (avformat_write_header(m_ffmpeg.fmt_ctx, nullptr) < 0) {
        spdlog::error("写入 RTSP 头部失败");
        return -1;
    }

    m_ffmpeg.frame = av_frame_alloc();
    m_ffmpeg.frame->format = m_ffmpeg.enc_ctx->pix_fmt;
    m_ffmpeg.frame->width = m_ffmpeg.enc_ctx->width;
    m_ffmpeg.frame->height = m_ffmpeg.enc_ctx->height;
    if (av_frame_get_buffer(m_ffmpeg.frame, 32) < 0) {
        spdlog::error("frame buffer 分配失败");
        return -1;
    }

    m_ffmpeg.sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_BGR24,
        width, height, AV_PIX_FMT_NV12,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    );

    m_ffmpeg.pkt = av_packet_alloc();
    m_ffmpeg.pts = 0;

    return 0;
}

void FFmpegStreamer::EncoderPushStream(cv::Mat frame){
    // BGR 转 NV12
    uint8_t* src_slices[1] = { frame.data };
    int src_stride[1] = { static_cast<int>(frame.step) };

    sws_scale(
        m_ffmpeg.sws_ctx, 
        src_slices, 
        src_stride, 0, 
        m_ffmpeg.height, 
        m_ffmpeg.frame->data, 
        m_ffmpeg.frame->linesize

    );

    m_ffmpeg.frame->pts = m_ffmpeg.pts++;

    if (avcodec_send_frame(m_ffmpeg.enc_ctx, m_ffmpeg.frame) < 0) 
        return;

    while (avcodec_receive_packet(m_ffmpeg.enc_ctx, m_ffmpeg.pkt) == 0) {
        m_ffmpeg.pkt->stream_index = m_ffmpeg.video_st->index;
        m_ffmpeg.pkt->pts = av_rescale_q(
            m_ffmpeg.pkt->pts, 
            m_ffmpeg.enc_ctx->time_base, 
            m_ffmpeg.video_st->time_base
        );
        m_ffmpeg.pkt->dts = m_ffmpeg.pkt->pts;
        m_ffmpeg.pkt->duration = av_rescale_q(
            m_ffmpeg.pkt->duration, 
            m_ffmpeg.enc_ctx->time_base, 
            m_ffmpeg.video_st->time_base
        );
        av_interleaved_write_frame(m_ffmpeg.fmt_ctx, m_ffmpeg.pkt);
        av_packet_unref(m_ffmpeg.pkt);
    }
}
