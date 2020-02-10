#include "frame_queue.h"

AVFrame *fq_pop_frame(FrameQueue *fq, bool peek)
{
    std::unique_lock lock(fq->mutex);
    fq->signal.wait(lock, [&fq] { return !fq->queue.empty(); });

    auto frame = fq->queue.front();
    if (!peek)
        fq->queue.pop();

    fq->signal.notify_all();
    return frame;
}

void fq_push_frame(FrameQueue *fq, AVFrame *frame)
{
    std::unique_lock lock(fq->mutex);
    fq->signal.wait(lock, [&fq] { return fq->queue.size() < fq->queue_limit; });
    fq->queue.push(frame);
    fq->signal.notify_all();
}
