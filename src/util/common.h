extern "C"
{
#include <libavformat/avformat.h>
}

#include "frame_queue.h"

/** error handling **/

#define ENSURE_RESULT(cond, ret, callee, caller)                         \
    if (!(cond))                                                         \
    {                                                                    \
        char msg[AV_ERROR_MAX_STRING_SIZE];                              \
        av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, ret);        \
        fprintf(stderr, "[%s] %s: %s (%d)\n", caller, callee, msg, ret); \
        return ret;                                                      \
    }

/** smart objects **/

typedef void (*frame_deleter)(AVFrame *);
typedef std::unique_ptr<AVFrame, frame_deleter> unique_frame;

unique_frame make_unique_frame(AVFrame *);
unique_frame make_unique_frame();

typedef void (*packet_deleter)(AVPacket *);
typedef std::unique_ptr<AVPacket, packet_deleter> unique_packet;

unique_packet make_unique_packet();

/** debugging **/

void print_packet(const AVPacket *pkt, const AVRational *time_base, const char *tag);