
#ifndef AUDIO_H
#define AUDIO_H

#include "PacketQueue.h"
extern "C"{

#include <libavformat/avformat.h>

}


/**
 * 播放audio时所需要的数据封装
 */
struct AudioState
{
	const uint32_t BUFFER_SIZE;// 缓冲区的大小

	PacketQueue audioq;

	double audio_clock; // audio clock
	AVStream *stream; // audio stream

	uint8_t *audio_buff;       // 解码后数据的缓冲空间
	uint32_t audio_buff_size;  // buffer中的字节数
	uint32_t audio_buff_index; // buffer中未发送数据的index
	
	int stream_index;          // audio流index
	AVCodecContext *audio_ctx; // 已经调用avcodec_open2打开

	AudioState();              //默认构造函数
	AudioState(AVCodecContext *audio_ctx, int audio_stream);
	
	~AudioState();

	/**
	* audio play
	*/
	bool audio_play();

	// get audio clock
	double get_audio_clock();
};

/**
 * 向设备发送audio数据的回调函数
 */
void audio_callback(void* userdata, Uint8 *stream, int len);

/**
 * 解码Avpacket中的数据填充到缓冲空间
 */
int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf, int buf_size);


#endif