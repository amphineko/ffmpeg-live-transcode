extern "C"
{
#include <libavformat/avformat.h>
}

#include "../util/frame_queue.h"

struct DemuxingContext
{
    const char *url;

    size_t threads;

    /** internal members **/

    AVStream *stream;

    AVCodecContext *dec;

    AVFormatContext *fmt;
};

void demux_close_input(DemuxingContext *ctx);

int demux_open_input(DemuxingContext *ctx);

int demux_read_frames(FrameQueue *fq, DemuxingContext *ctx);
