#include "output.h"
#include "../util/common.h"

int remux_open_stream(const AVCodec *codec, RemuxingContext *ctx)
{
    auto stream = ctx->video_stream = avformat_new_stream(ctx->fmt, codec);
    ENSURE_RESULT(stream != nullptr, AVERROR_UNKNOWN, "avformat_new_stream", __func__)

    stream->codecpar = avcodec_parameters_alloc();
    avcodec_parameters_from_context(stream->codecpar, ctx->enc);
    stream->id = ctx->fmt->nb_streams - 1;
    stream->time_base = ctx->time_base;

    return 0;
}

int remux_open_format(RemuxingContext *ctx)
{
    auto format = av_guess_format(ctx->format_name, nullptr, ctx->format_mime_type);
    ENSURE_RESULT(format != nullptr, AVERROR_MUXER_NOT_FOUND, "av_guess_format", __func__)

    auto ret = avformat_alloc_output_context2(&(ctx->fmt), format, nullptr, nullptr);
    ENSURE_RESULT(ret >= 0, ret, "avformat_alloc_output_context2", __func__)

    ctx->fmt->pb = ctx->io;

    return 0;
}

int remux_open_output(RemuxingContext *ctx)
{
    int ret;

    auto time_base = ctx->time_base = {1, ctx->frame_rate};

    auto par = ctx->codec_par = avcodec_parameters_alloc();
    par->codec_id = ctx->codec_id;
    par->codec_type = AVMEDIA_TYPE_VIDEO;
    par->format = ctx->pix_fmt;
    par->height = ctx->height;
    par->width = ctx->width;

    /* open encoder */

    switch (ctx->codec_id)
    {
    case AV_CODEC_ID_VP9:
        ret = remux_vp9_encoder_open(time_base, ctx);
        ENSURE_RESULT(ret == 0, ret, "remux_vp9_encoder_open", __func__)
        break;
    default:
        ENSURE_RESULT(false, AVERROR_ENCODER_NOT_FOUND, "remux_unsupported_encoder_open", __func__)
    }

    /* open container */

    ret = remux_open_format(ctx);
    ENSURE_RESULT(ret == 0, ret, "remux_open_format", __func__)

    /* open stream */

    ret = remux_open_stream(ctx->enc->codec, ctx);
    ENSURE_RESULT(ret == 0, ret, "remux_open_stream", __func__)

    ret = avformat_write_header(ctx->fmt, nullptr);
    ENSURE_RESULT(ret >= 0, ret, "avformat_write_header", __func__)

    return 0;
}

int remux_close_output(RemuxingContext *ctx)
{
    av_interleaved_write_frame(ctx->fmt, nullptr);
    av_write_trailer(ctx->fmt);
    avformat_free_context(ctx->fmt);
    return 0;
}