
#include "VideoDisplay.h"
#include <iostream>

extern "C"{

#include <libswscale\swscale.h>

}

// ÑÓ³Ùdelay msºóË¢ÐÂvideoÖ¡
void schedule_refresh(MediaState *media, int delay)
{
	SDL_AddTimer(delay, sdl_refresh_timer_cb, media);
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
	MediaState *media = (MediaState*)userdata;
	VideoState *video = media->video;

	if (video->video_stream >= 0)
	{
		if (video->videoq->queue.empty())
			schedule_refresh(media, 1);
		else
		{
			//std::cout << "Audio Clock:" << media->audio->get_audio_clock() << std::endl;
			/* Now, normally here goes a ton of code
			about timing, etc. we're just going to
			guess at a delay for now. You can
			increase and decrease this value and hard code
			the timing - but I don't suggest that ;)
			We'll learn how to do it for real later.
			*/
			schedule_refresh(media, 40);

			video->frameq.deQueue(&video->frame);

			SwsContext *sws_ctx = sws_getContext(video->video_ctx->width, video->video_ctx->height, video->video_ctx->pix_fmt,
			video->displayFrame->width,video->displayFrame->height,(AVPixelFormat)video->displayFrame->format, SWS_BILINEAR, nullptr, nullptr, nullptr);

			sws_scale(sws_ctx, (uint8_t const * const *)video->frame->data, video->frame->linesize, 0, 
				video->video_ctx->height, video->displayFrame->data, video->displayFrame->linesize);

			// Display the image to screen
			SDL_UpdateTexture(video->bmp, &(video->rect), video->displayFrame->data[0], video->displayFrame->linesize[0]);
			SDL_RenderClear(video->renderer);
			SDL_RenderCopy(video->renderer, video->bmp, &video->rect, &video->rect);
			SDL_RenderPresent(video->renderer);

			sws_freeContext(sws_ctx);
			av_frame_unref(video->frame);
		}
	}
	else
	{
		schedule_refresh(media, 100);
	}
}