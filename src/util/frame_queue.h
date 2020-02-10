#ifndef SHIRONEKO_FRAME_QUEUE
#define SHIRONEKO_FRAME_QUEUE

#include <condition_variable>
#include <mutex>
#include <queue>

extern "C"
{
#include <libavutil/frame.h>
}

struct FrameQueue
{
    std::condition_variable signal;
    std::mutex mutex;
    std::queue<AVFrame *> queue;
    size_t queue_limit;
};

AVFrame *fq_pop_frame(FrameQueue *, bool peek);
void fq_push_frame(FrameQueue *, AVFrame *);

#endif
