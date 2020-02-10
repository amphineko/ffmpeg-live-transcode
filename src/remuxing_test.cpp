#include "demuxing/demuxing.h"
#include "remuxing/remuxing.h"
#include "util/common.h"

#define OUTPUT_CODEC_ID AV_CODEC_ID_VP9
#define OUTPUT_FILENAME "output.webm"
#define OUTPUT_FRAME_FORMAT AV_PIX_FMT_YUV422P
#define OUTPUT_FRAME_HEIGHT 1080
#define OUTPUT_FRAME_RATE 30
#define OUTPUT_FRAME_WIDTH 1920
#define OUTPUT_FORMAT_NAME "webm"
#define OUTPUT_FORMAT_MIME_TYPE "video/webm"

int main(int arg_count, char *args[])
{
    if (arg_count < 2)
    {
        fprintf(stderr, "filename not provided\n");
        return -1;
    }
    const char *filename = args[1];

    int ret;

    /* initialize demuxing */

    DemuxingContext demux{0};
    demux.url = filename;

    ret = demux_open_input(&demux);
    ENSURE_RESULT(ret == 0, ret, "demux_open_input", __func__)

    /* initialize muxing */

    AVIOContext *file = nullptr;
    ret = avio_open(&file, OUTPUT_FILENAME, AVIO_FLAG_WRITE);
    ENSURE_RESULT(ret >= 0, ret, "avio_open", __func__)

    RemuxingContext remux{0};
    remux.codec_id = OUTPUT_CODEC_ID;
    remux.format_mime_type = OUTPUT_FORMAT_MIME_TYPE;
    remux.format_name = OUTPUT_FORMAT_NAME;
    remux.frame_rate = OUTPUT_FRAME_RATE;
    remux.height = OUTPUT_FRAME_HEIGHT;
    remux.io = file;
    remux.nb_frames = demux.stream->nb_frames;
    remux.pix_fmt = OUTPUT_FRAME_FORMAT;
    remux.width = OUTPUT_FRAME_WIDTH;

    ret = remux_open_output(&remux);
    ENSURE_RESULT(ret == 0, ret, "remux_open_output", __func__)

    /* do the job */

    FrameQueue fq;
    fq.queue_limit = 2 * OUTPUT_FRAME_RATE;

    std::thread demuxing([&] {
        auto ret = demux_read_frames(&fq, &demux);
        ENSURE_RESULT(ret == 0 || ret == AVERROR_EOF, ret, "demux_read_frames", __func__)

        fq_push_frame(&fq, nullptr); // signal remuxer eof

        printf("demux exit\n");
        return 0;
    });

    std::thread muxing([&] {
        auto ret = remux_write_frames(&fq, remux.video_stream, remux.enc, remux.fmt);
        ENSURE_RESULT(ret == 0 || ret == AVERROR_EOF, ret, "remux_write_frames", __func__)
        
        remux_close_output(&remux);
        avio_close(file);

        printf("remux exit\n");
        return 0;
    });

    demuxing.join();
    muxing.join();

    fq.signal.notify_all();

    return 0;
}