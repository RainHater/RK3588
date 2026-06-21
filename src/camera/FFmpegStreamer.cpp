#include "FFmpegStreamer.h"

#include <spdlog/spdlog.h>

FFmpegStreamer::FFmpegStreamer(
    std::string stream_mode,
    std::string stream_name
)
    : m_stream_name(stream_name)
    , m_stream_mode(stream_mode)
    , m_logger(LoggerWithTag::GetLogger("FFmpegStreamer_" + m_stream_name))
{
    m_ffmpeg.param = nullptr;
    m_ffmpeg.fmt_ctx = nullptr;
}

FFmpegStreamer::~FFmpegStreamer(){
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
        m_logger->error("未找到编码器 h264_rkmpp");
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
        m_logger->error("编码器打开失败");
        return -1;
    }

    av_dict_free(&m_ffmpeg.param);

    std::string stream_name = m_stream_mode + "://0.0.0.0:8554/" + m_stream_name;
    avformat_alloc_output_context2(&m_ffmpeg.fmt_ctx, nullptr, "rtsp", stream_name.c_str());

    if (!m_ffmpeg.fmt_ctx) {
        m_logger->error("RTSP 输出上下文创建失败");
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
            m_logger->error("无法打开 RTSP 输出 URL");
            return -1;
        }
    }

    if (avformat_write_header(m_ffmpeg.fmt_ctx, nullptr) < 0) {
        m_logger->error("写入 RTSP 头部失败");
        return -1;
    }

    m_ffmpeg.frame = av_frame_alloc();
    m_ffmpeg.frame->format = m_ffmpeg.enc_ctx->pix_fmt;
    m_ffmpeg.frame->width = m_ffmpeg.enc_ctx->width;
    m_ffmpeg.frame->height = m_ffmpeg.enc_ctx->height;
    if (av_frame_get_buffer(m_ffmpeg.frame, 32) < 0) {
        m_logger->error("frame buffer 分配失败");
        return -1;
    }

    m_ffmpeg.sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_BGR24,
        width, height, AV_PIX_FMT_NV12,
        SWS_BICUBIC, nullptr, nullptr, nullptr
    );

    if (!m_ffmpeg.sws_ctx) {
        m_logger->error("sws_getContext 创建失败");
        return -1;
    }

    m_ffmpeg.pkt = av_packet_alloc();
    m_ffmpeg.pts = 0;

    return 0;
}

void FFmpegStreamer::EncoderPushStream(const cv::Mat& input)
{
    if (!m_ffmpeg.sws_ctx || !m_ffmpeg.frame || !m_ffmpeg.enc_ctx) {
        m_logger->error("FFmpegStreamer 未初始化");
        return;
    }

    if (input.empty() || input.data == nullptr) {
        m_logger->warn("输入图像为空，跳过");
        return;
    }

    cv::Mat frame;

    // 确保输入是 BGR24，也就是 CV_8UC3
    if (input.type() == CV_8UC3) {
        frame = input;
    } else if (input.type() == CV_8UC1) {
        cv::cvtColor(input, frame, cv::COLOR_GRAY2BGR);
    } else if (input.type() == CV_8UC4) {
        cv::cvtColor(input, frame, cv::COLOR_BGRA2BGR);
    } else {
        m_logger->error("不支持的 Mat 类型: {}", input.type());
        return;
    }

    // 确保尺寸和编码器尺寸一致
    if (frame.cols != m_ffmpeg.width || frame.rows != m_ffmpeg.height) {
        cv::resize(frame, frame, cv::Size(m_ffmpeg.width, m_ffmpeg.height));
    }

    // 确保数据连续，避免 ROI 或外部 buffer 异常
    if (!frame.isContinuous()) {
        frame = frame.clone();
    }

    if (av_frame_make_writable(m_ffmpeg.frame) < 0) {
        m_logger->error("frame 不可写");
        return;
    }

    const uint8_t* src_slices[4] = {
        frame.data,
        nullptr,
        nullptr,
        nullptr
    };

    int src_stride[4] = {
        static_cast<int>(frame.step[0]),
        0,
        0,
        0
    };

    int ret = sws_scale(
        m_ffmpeg.sws_ctx,
        src_slices,
        src_stride,
        0,
        m_ffmpeg.height,
        m_ffmpeg.frame->data,
        m_ffmpeg.frame->linesize
    );

    if (ret <= 0) {
        m_logger->error("sws_scale 失败: ret={}", ret);
        return;
    }

    m_ffmpeg.frame->pts = m_ffmpeg.pts++;

    ret = avcodec_send_frame(m_ffmpeg.enc_ctx, m_ffmpeg.frame);
    if (ret < 0) {
        m_logger->error("avcodec_send_frame 失败: {}", ret);
        return;
    }

    while (avcodec_receive_packet(m_ffmpeg.enc_ctx, m_ffmpeg.pkt) == 0) {
        m_ffmpeg.pkt->stream_index = m_ffmpeg.video_st->index;

        av_packet_rescale_ts(
            m_ffmpeg.pkt,
            m_ffmpeg.enc_ctx->time_base,
            m_ffmpeg.video_st->time_base
        );

        av_interleaved_write_frame(m_ffmpeg.fmt_ctx, m_ffmpeg.pkt);
        av_packet_unref(m_ffmpeg.pkt);
    }
}