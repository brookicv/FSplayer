
#include "PacketQueue.h"

extern bool quit;

PacketQueue::PacketQueue()
{
	nb_packets = 0;
	size       = 0;

	mutex      = SDL_CreateMutex();
	cond       = SDL_CreateCond();
}

bool PacketQueue::enQueue(const AVPacket *packet)
{
	AVPacket *pkt = av_packet_alloc();
	if (av_packet_ref(pkt, packet) < 0)
		return false;

	SDL_LockMutex(mutex);
	queue.push(*pkt);

	size += pkt->size;
	nb_packets++;

	SDL_CondSignal(cond);
	SDL_UnlockMutex(mutex);

	return true;
}

bool PacketQueue::deQueue(AVPacket *packet, bool block)
{
	bool ret = false;

	SDL_LockMutex(mutex);
	while (true)
	{
		if (quit)
		{
			ret = false;
			break;
		}

		if (!queue.empty())
		{
			if (av_packet_ref(packet, &queue.front()) < 0)
			{
				ret = false;
				break;
			}

			//av_packet_free(&queue.front());
			av_packet_unref(&queue.front());
			queue.pop();
			nb_packets--;
			size -= packet->size;

			ret = true;
			break;
		}
		else if (!block)
		{
			ret = false;
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