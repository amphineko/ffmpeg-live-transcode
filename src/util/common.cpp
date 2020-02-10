#include "common.h"

extern "C"
{
#include <libavutil/timestamp.h>
}

unique_frame make_unique_frame(AVFrame *frame)
{
    const static auto delete_frame = [](auto f) { av_frame_free(&f); };
    return std::unique_ptr<AVFrame, frame_deleter>(frame, delete_frame);
}

unique_frame make_unique_frame()
{
    return make_unique_frame(av_frame_alloc());
}

unique_packet make_unique_packet()
{
    const static auto delete_packet = [](auto f) { av_packet_free(&f); };
    return std::unique_ptr<AVPacket, packet_deleter>(av_packet_alloc(), delete_packet);
}

void print_packet(const AVPacket *pkt, const AVRational *time_base, const char *tag)
{
    char pts[AV_TS_MAX_STRING_SIZE], dts[AV_TS_MAX_STRING_SIZE];
    av_ts_make_time_string(pts, pkt->pts, const_cast<AVRational *>(time_base));
    av_ts_make_time_string(dts, pkt->dts, const_cast<AVRational *>(time_base));
    printf("%s:\tpts=%s\t(%d)\tdts=%s\n", tag, pts, pkt->pts, dts);
}
