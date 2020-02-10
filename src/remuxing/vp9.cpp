#include "../util/common.h"
#include "output.h"

int remux_vp9_encoder_open(const AVRational time_base, RemuxingContext *ctx)
{
    auto codec = avcodec_find_encoder(ctx->codec_id);
    ENSURE_RESULT(codec != nullptr, AVERROR_ENCODER_NOT_FOUND, "avcodec_find_encoder", "remux_open")

    // initialize codec
    ctx->enc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->enc, ctx->codec_par);
    ctx->enc->bit_rate = 5000;
    ctx->enc->time_base = time_base;

    // initialize options
    AVDictionary *opts = nullptr;
    // TODO: remove constants here
    av_dict_set(&opts, "quality", "realtime", 0);
    av_dict_set(&opts, "speed", "6", 0);
    av_dict_set(&opts, "tile-columns", "4", 0);
    av_dict_set(&opts, "frame-parallel", "1", 0);
    av_dict_set(&opts, "threads", "8", 0);
    av_dict_set(&opts, "static-thresh", "0", 0);
    av_dict_set(&opts, "max-intra-rate", "300", 0);
    av_dict_set(&opts, "lag-in-frames", "0", 0);
    av_dict_set(&opts, "qmin", "4", 0);
    av_dict_set(&opts, "qmax", "48", 0);
    av_dict_set(&opts, "row-mt", "1", 0);

    auto ret = avcodec_open2(ctx->enc, codec, &opts);

    auto nb_inval_opts = av_dict_count(opts);
    // TODO: tell invalid option names
    if (opts != nullptr)
    {
        AVDictionaryEntry *i = nullptr;
        while (i = av_dict_get(opts, "", i, AV_DICT_IGNORE_SUFFIX))
            fprintf(stderr, "[%s] unused encoder option: %s=%s\n", __func__, i->key, i->value);

        av_dict_free(&opts);
    }

    ENSURE_RESULT(ret == 0, ret, "avcodec_open2", __func__);
    ENSURE_RESULT(nb_inval_opts == 0, AVERROR_UNKNOWN, "assert(av_dict_count(opts) == 0)", __func__)

    return 0;
}