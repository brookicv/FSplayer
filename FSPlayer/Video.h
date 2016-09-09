
#ifndef VIDEO_H
#define VIDEO_H

#include "PacketQueue.h"
#include "FrameQueue.h"

/**
 * 播放音频所需的数据封装
 */
struct AudioState
{
	PacketQueue videoq;        // 保存的video packet的队列缓存

	int video_stream;          // index of video stream
	AVCodecContext *video_ctx; // have already be opened by avcodec_open2

	FrameQueue frameq;         // 保存解码后的原始帧数据,已经根据需要转换成了相应的格式

	void decode(const AVPacket *packet); // 将packet解码，并将
	
};

#endif