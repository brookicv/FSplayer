
#include "Audio.h"

#include <iostream>
#include <fstream>
extern "C" {

#include <libswresample\swresample.h>

}

extern bool quit;

AudioState::AudioState()
	:BUFFER_SIZE(192000)
{
	audio_ctx = nullptr;
	stream_index = -1;
	stream = nullptr;
	audio_clock = 0;

	audio_buff = new uint8_t[BUFFER_SIZE];
	audio_buff_size = 0;
	audio_buff_index = 0;
}

AudioState::AudioState(AVCodecContext *audioCtx, int index)
	:BUFFER_SIZE(192000)
{
	audio_ctx = audioCtx;
	stream_index = index;
	

	audio_buff = new uint8_t[BUFFER_SIZE];
	audio_buff_size = 0;
	audio_buff_index = 0;
}

AudioState::~AudioState()
{
	if (audio_buff)
		delete[] audio_buff;
}

bool AudioState::audio_play()
{
	SDL_AudioSpec desired;
	desired.freq = audio_ctx->sample_rate;
	desired.channels = audio_ctx->channels;
	desired.format = AUDIO_S16SYS;
	desired.samples = 1024;
	desired.silence = 0;
	desired.userdata = this;
	desired.callback = audio_callback;

	if (SDL_OpenAudio(&desired, nullptr) < 0)
	{
		return false;
	}

	SDL_PauseAudio(0); // playing

	return true;
}

double AudioState::get_audio_clock()
{
	int hw_buf_size = audio_buff_size - audio_buff_index;
	int bytes_per_sec = stream->codec->sample_rate * audio_ctx->channels * 2;

	double pts = audio_clock - static_cast<double>(hw_buf_size) / bytes_per_sec;

	
	return pts;
}

/**
* 向设备发送audio数据的回调函数
*/
void audio_callback(void* userdata, Uint8 *stream, int len)
{
	AudioState *audio_state = (AudioState*)userdata;

	SDL_memset(stream, 0, len);

	int audio_size = 0;
	int len1 = 0;
	while (len > 0)// 向设备发送长度为len的数据
	{
		if (audio_state->audio_buff_index >= audio_state->audio_buff_size) // 缓冲区中无数据
		{
			// 从packet中解码数据
			audio_size = audio_decode_frame(audio_state, audio_state->audio_buff, sizeof(audio_state->audio_buff));
			if (audio_size < 0) // 没有解码到数据或出错，填充0
			{
				audio_state->audio_buff_size = 0;
				memset(audio_state->audio_buff, 0, audio_state->audio_buff_size);
			}
			else
				audio_state->audio_buff_size = audio_size;

			audio_state->audio_buff_index = 0;
		}
		len1 = audio_state->audio_buff_size - audio_state->audio_buff_index; // 缓冲区中剩下的数据长度
		if (len1 > len) // 向设备发送的数据长度为len
			len1 = len;

		SDL_MixAudio(stream, audio_state->audio_buff + audio_state->audio_buff_index, len, SDL_MIX_MAXVOLUME);

		len -= len1;
		stream += len1;
		audio_state->audio_buff_index += len1;
	}
}

int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf, int buf_size)
{
	AVFrame *frame = av_frame_alloc();
	int data_size = 0;
	AVPacket pkt;
	SwrContext *swr_ctx = nullptr;
	static double clock = 0;
	
	if (quit)
		return -1;
	if (!audio_state->audioq.deQueue(&pkt, true))
		return -1;

	if (pkt.pts != AV_NOPTS_VALUE)
	{
		audio_state->audio_clock = av_q2d(audio_state->stream->time_base) * pkt.pts;
	}
	int ret = avcodec_send_packet(audio_state->audio_ctx, &pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return -1;

	ret = avcodec_receive_frame(audio_state->audio_ctx, frame);
	if (ret < 0 && ret != AVERROR_EOF)
		return -1;

	// 设置通道数或channel_layout
	if (frame->channels > 0 && frame->channel_layout == 0)
		frame->channel_layout = av_get_default_channel_layout(frame->channels);
	else if (frame->channels == 0 && frame->channel_layout > 0)
		frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);

	AVSampleFormat dst_format = AV_SAMPLE_FMT_S16;//av_get_packed_sample_fmt((AVSampleFormat)frame->format);
	Uint64 dst_layout = av_get_default_channel_layout(frame->channels);
	// 设置转换参数
	swr_ctx = swr_alloc_set_opts(nullptr, dst_layout, dst_format, frame->sample_rate,
		frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, nullptr);
	if (!swr_ctx || swr_init(swr_ctx) < 0)
		return -1;

	// 计算转换后的sample个数 a * b / c
	uint64_t dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, frame->sample_rate, frame->sample_rate, AVRounding(1));
	// 转换，返回值为转换后的sample个数
	int nb = swr_convert(swr_ctx, &audio_buf, static_cast<int>(dst_nb_samples), (const uint8_t**)frame->data, frame->nb_samples);
	data_size = frame->channels * nb * av_get_bytes_per_sample(dst_format);

	// 每秒钟音频播放的字节数 sample_rate * channels * sample_format(一个sample占用的字节数)
	audio_state->audio_clock += static_cast<double>(data_size) / (2 * audio_state->stream->codec->channels * audio_state->stream->codec->sample_rate);


	av_frame_free(&frame);
	swr_free(&swr_ctx);

	return data_size;
}

