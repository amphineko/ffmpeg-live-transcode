#include "../util/common.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

inline SwsContext *make_scaler(const AVFrame *out_frame, const AVFrame *src_frame)
{
    return sws_getContext(src_frame->width, src_frame->height, static_cast<AVPixelFormat>(src_frame->format),
                          out_frame->width, out_frame->height, static_cast<AVPixelFormat>(out_frame->format),
                          SWS_BILINEAR, nullptr, nullptr, nullptr);
}

inline void transform(AVFrame *out_frame, AVFrame *in_frame, SwsContext *scaler)
{
    sws_scale(scaler, in_frame->data, in_frame->linesize, 0, in_frame->height, out_frame->data, out_frame->linesize);
}

inline int write_packet(AVPacket *pkt, const AVRational src_tb, AVStream *stream, AVFormatContext *fmt)
{
    av_packet_rescale_ts(pkt, src_tb, stream->time_base);
    pkt->stream_index = stream->index;

    print_packet(pkt, const_cast<AVRational *>(&stream->time_base), __func__);

    auto ret = av_interleaved_write_frame(fmt, pkt);
    ENSURE_RESULT(ret == 0, ret, "av_interleaved_write_frame", __func__);

    return ret;
}

inline int send_frame(const AVFrame *frame, AVStream *stream, AVCodecContext *enc, AVFormatContext *fmt)
{
    int ret;

    while (AVERROR(EAGAIN) == (ret = avcodec_send_frame(enc, frame)))
    {
        auto pkt = make_unique_packet();

        ret = avcodec_receive_packet(enc, pkt.get());
        ENSURE_RESULT(ret == 0, ret, "avcodec_receive_packet", __func__)

        ret = write_packet(pkt.release(), enc->time_base, stream, fmt);
        ENSURE_RESULT(ret == 0, ret, "write_packet", __func__)
    }
    ENSURE_RESULT(ret == 0 || ret == AVERROR_EOF, ret, "avcodec_send_frame", __func__)

    return 0;
}

int flush_encoder(AVStream *stream, AVCodecContext *enc, AVFormatContext *fmt)
{
    int ret;

    // send null frame to signal encoder flushing
    ret = send_frame(nullptr, stream, enc, fmt);
    ENSURE_RESULT(ret == 0, ret, "send_frame", __func__)

    // receive queued packets
    while (true)
    {
        auto pkt = make_unique_packet();

        ret = avcodec_receive_packet(enc, pkt.get());
        if (ret == AVERROR_EOF)
            break;
        ENSURE_RESULT(ret == 0 || ret == AVERROR_EOF, ret, "avcodec_receive_packet(nullptr)", __func__)

        ret = write_packet(pkt.release(), enc->time_base, stream, fmt);
        ENSURE_RESULT(ret == 0, ret, "write_packet", __func__)
    }

    return 0;
}

const auto RESCALE_ROUNDING = static_cast<AVRounding>(AV_ROUND_NEAR_INF);

int remux_write_frames(FrameQueue *fq, AVStream *stream, AVCodecContext *enc, AVFormatContext *dst)
{
    int ret;
    auto out_time_base = enc->time_base;
    size_t nb_frames = 0;

    // peek one frame as sample input
    auto sample_frame = fq_pop_frame(fq, true);
    if (sample_frame == nullptr)
        return AVERROR_EOF;
    auto base_pts = sample_frame->pts;

    // initialize output frame
    auto out = make_unique_frame();
    out->format = enc->pix_fmt;
    out->height = enc->height;
    out->width = enc->width;
    av_frame_get_buffer(out.get(), 32);

    // initialize scaler
    const static auto delete_scaler = [](auto p) {
        if (p != nullptr)
            sws_freeContext(p);
    };
    std::unique_ptr<SwsContext, decltype(delete_scaler)> scaler(make_scaler(out.get(), sample_frame), delete_scaler);

    while (true)
    {
        // retrieve one frame from queue
        auto src = make_unique_frame(fq_pop_frame(fq, false));
        if (src == nullptr)
            break;

        // scale and convert pixel format
        transform(out.get(), src.get(), scaler.get());
        out->pts = nb_frames++; // TODO: rescale to encoder timebase

        ret = send_frame(out.get(), stream, enc, dst);
        ENSURE_RESULT(ret == 0, ret, "try_send_frame", __func__)
    }

    ret = flush_encoder(stream, enc, dst);
    ENSURE_RESULT(ret == 0, ret, "flush_encoder", __func__)

    return 0;
}