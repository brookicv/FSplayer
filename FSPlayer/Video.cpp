
#include "Video.h"
#include "VideoDisplay.h"
#include <vld.h>
extern "C"{

#include <libswscale\swscale.h>

}

VideoState::VideoState()
{
	video_ctx    = nullptr;
	video_stream = -1;

	window       = nullptr;
	bmp          = nullptr;
	renderer     = nullptr;
}

void VideoState::video_play()
{
	// ´´½¨sdl´°¿Ú
	window = SDL_CreateWindow("FFmpeg Decode", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		video_ctx->width, video_ctx->height, SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED);
	renderer = SDL_CreateRenderer(window, -1, 0);
	bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
		video_ctx->width, video_ctx->height);

	rect.x = 0;
	rect.y = 0;
	rect.w = video_ctx->width;
	rect.h = video_ctx->height;

	SDL_CreateThread(decode, "", this);

	schedule_refresh(this, 40); // start display
}

int  decode(void *arg)
{
	VideoState *video = (VideoState*)arg;

	AVFrame *frame = av_frame_alloc();

	AVPacket packet;


	while (true)
	{
		video->videoq.deQueue(&packet, true);

		int ret = avcodec_send_packet(video->video_ctx, &packet);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			continue;

		ret = avcodec_receive_frame(video->video_ctx, frame);
		if (ret < 0 && ret != AVERROR_EOF)
			continue;

		if (video->frameq.nb_frames >= FrameQueue::capacity)
			SDL_Delay(1000 * 10);

		video->frameq.enQueue(frame);

		av_frame_unref(frame);
	}


	av_frame_free(&frame);

	return 0;
}

