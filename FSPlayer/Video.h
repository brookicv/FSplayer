
#ifndef VIDEO_H
#define VIDEO_H

#include "PacketQueue.h"
#include "FrameQueue.h"


/**
 * 播放音频所需的数据封装
 */
struct VideoState
{
	PacketQueue videoq;        // 保存的video packet的队列缓存

	int video_stream;          // index of video stream
	AVCodecContext *video_ctx; // have already be opened by avcodec_open2

	FrameQueue frameq;         // 保存解码后的原始帧数据,已经根据需要转换成了相应的格式

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *bmp;
	SDL_Rect rect;

	void video_play();
	
	VideoState();
};


int decode(void *arg); // 将packet解码，并将解码后的Frame放入FrameQueue队列中


#endif