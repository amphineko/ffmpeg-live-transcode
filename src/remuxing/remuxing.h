#include "output.h"

int remux_write_frames(FrameQueue *fq, AVStream *stream, AVCodecContext *enc, AVFormatContext *dst);