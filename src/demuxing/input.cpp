#include "../util/common.h"
#include "demuxing.h"

extern "C"
{
#include <libavformat/avformat.h>
}

void demux_close_input(DemuxingContext *ctx)
{
    av_read_pause(ctx->fmt);

    avcodec_free_context(&(ctx->dec));
    avformat_close_input(&(ctx->fmt));
}

int demux_open_input(DemuxingContext *ctx)
{
    int ret;

    ret = avformat_open_input(&(ctx->fmt), ctx->url, nullptr, nullptr);
    ENSURE_RESULT(ret == 0, ret, "avformat_open_input", __func__)

    ret = avformat_find_stream_info(ctx->fmt, nullptr);
    ENSURE_RESULT(ret >= 0, ret, "avformat_find_stream_info", __func__)

    AVCodec *codec;
    ret = av_find_best_stream(ctx->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    ENSURE_RESULT(ret >= 0, ret, "av_find_best_stream", __func__)

    auto stream = ctx->stream = ctx->fmt->streams[ret];

    ctx->dec = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->dec, stream->codecpar);
    ctx->dec->thread_count = ctx->threads;

    ret = avcodec_open2(ctx->dec, codec, nullptr);
    ENSURE_RESULT(ret == 0, ret, "avcodec_open2", __func__)

    return 0;
}