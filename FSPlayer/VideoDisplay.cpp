
#include "VideoDisplay.h"

extern "C"{

#include <libswscale\swscale.h>

}

// 延迟delay ms后刷新video帧
void schedule_refresh(VideoState *video, int delay)
{
	SDL_AddTimer(delay, sdl_refresh_timer_cb, video);
}

uint32_t sdl_refresh_timer_cb(uint32_t interval, void *opaque)
{
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}

void video_refresh_timer(void *userdata)
{
	VideoState *video = (VideoState*)userdata;

	if (video->video_stream >= 0)
	{
		if (video->videoq.queue.empty())
			schedule_refresh(video, 1);
		else
		{
			/* Now, normally here goes a ton of code
			about timing, etc. we're just going to
			guess at a delay for now. You can
			increase and decrease this value and hard code
			the timing - but I don't suggest that ;)
			We'll learn how to do it for real later.
			*/
			schedule_refresh(video, 40);

			AVFrame *frame = av_frame_alloc();
			AVFrame *frameYUV = av_frame_alloc();

			video->frameq.deQueue(&frame);

			// 格式转换
			int numBytes = avpicture_get_size(AV_PIX_FMT_YUVJ420P, video->video_ctx->width, video->video_ctx->height);
			uint8_t *buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

			avpicture_fill((AVPicture*)frameYUV, buffer, AV_PIX_FMT_YUVJ420P, video->video_ctx->width, video->video_ctx->height);

			SwsContext *sws_ctx = sws_getContext(video->video_ctx->width, video->video_ctx->height, video->video_ctx->pix_fmt,
			video->video_ctx->width, video->video_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

			sws_scale(sws_ctx, (uint8_t const * const *)frame->data, frame->linesize, 0, video->video_ctx->height, frameYUV->data, frameYUV->linesize);

			// Display the image to screen
			SDL_UpdateTexture(video->bmp, &(video->rect), frameYUV->data[0], frameYUV->linesize[0]);
			SDL_RenderClear(video->renderer);
			SDL_RenderCopy(video->renderer, video->bmp, &video->rect, &video->rect);
			SDL_RenderPresent(video->renderer);
			
			av_frame_free(&frame);
			//av_frame_free(&frameYUV);
		}
	}
	else
	{
		schedule_refresh(video, 100);
	}
}