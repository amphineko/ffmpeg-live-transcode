#include "demuxing.h"
#include "../util/common.h"

int demux_read_frames(FrameQueue *fq, DemuxingContext *ctx)
{
    int ret;

    int candidate_idx = ctx->stream->index;

    auto dec = ctx->dec;
    auto fmt = ctx->fmt;

    auto pkt = make_unique_packet();
    auto frame = make_unique_frame();
    while (true)
    {
        ret = av_read_frame(fmt, pkt.get());
        if (ret == AVERROR_EOF)
            break;
        if (pkt->stream_index != candidate_idx)
            continue;
        ENSURE_RESULT(ret == 0, ret, "av_read_frame", __func__)

        print_packet(pkt.get(), &(dec->time_base), "demux");

        ret = avcodec_send_packet(dec, pkt.get());
        ENSURE_RESULT(ret == 0, ret, "avcodec_send_packet", __func__)

        ret = avcodec_receive_frame(dec, frame.get());
        if (ret == AVERROR(EAGAIN))
            continue;
        ENSURE_RESULT(ret == 0, ret, "avcodec_receive_frame", __func__)

        fq_push_frame(fq, frame.release());
        frame.reset(av_frame_alloc());
    }

    // signal eof to decoder
    ret = avcodec_send_packet(dec, nullptr);
    ENSURE_RESULT(ret == 0, ret, "avcodec_send_packet(nullptr)", __func__)

    // read queued frames from decoder
    while (true)
    {
        auto frame = make_unique_frame();

        ret = avcodec_receive_frame(dec, frame.get());
        if (ret == AVERROR_EOF)
            break;
        ENSURE_RESULT(ret == 0, ret, "avcodec_receive_frame(nullptr)", __func__)

        fq_push_frame(fq, frame.release());
    }

    return 0;
}