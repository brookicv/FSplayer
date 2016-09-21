
#include "FrameQueue.h"

FrameQueue::FrameQueue()
{
	nb_frames = 0;

	mutex     = SDL_CreateMutex();
	cond      = SDL_CreateCond();
}

bool FrameQueue::enQueue(const AVFrame* frame)
{
	AVFrame* p = av_frame_alloc();

	int ret = av_frame_ref(p, frame);
	if (ret < 0)
		return false;

	uint64_t pts = av_frame_get_best_effort_timestamp(frame);
	SDL_LockMutex(mutex);
	queue.push(p);

	nb_frames++;
	
	SDL_CondSignal(cond);
	SDL_UnlockMutex(mutex);
	
	return true;
}

bool FrameQueue::deQueue(AVFrame **frame)
{
	bool ret = true;

	SDL_LockMutex(mutex);
	while (true)
	{
		if (!queue.empty())
		{
			if (av_frame_ref(*frame, queue.front()) < 0)
			{
				ret = false;
				break;
			}

			av_frame_free(&queue.front());
			//av_frame_unref(queue.front());
			queue.pop();

			nb_frames--;

			ret = true;
			break;
		}
		else
		{
			SDL_CondWait(cond, mutex);
		}
	}

	SDL_UnlockMutex(mutex);
	return ret;
}