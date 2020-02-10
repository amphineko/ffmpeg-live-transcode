extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct RemuxingContext
{
    /* encoding parameters */

    int height, width;

    AVCodecID codec_id;

    AVPixelFormat pix_fmt;

    /* stream parameters */

    int frame_rate;

    int nb_frames;

    /* format parameters */

    const char *format_name;

    const char *format_mime_type;

    AVIOContext *io;

    /* internal members */

    AVRational time_base;

    AVCodecParameters *codec_par;

    AVCodecContext *enc;

    AVStream *video_stream;

    AVFormatContext *fmt;
};

int remux_close_output(RemuxingContext *ctx);

int remux_open_output(RemuxingContext *ctx);

int remux_vp9_encoder_open(const AVRational time_base, RemuxingContext *ctx);
