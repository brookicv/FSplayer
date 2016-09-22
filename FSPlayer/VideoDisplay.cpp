
#include "VideoDisplay.h"
#include <iostream>

extern "C"{

#include <libswscale\swscale.h>
#include <libavutil\time.h>

}

static const double SYNC_THRESHOLD = 0.01;
static const double NOSYNC_THRESHOLD = 10.0;

// 延迟delay ms后刷新video帧
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

	if (video->stream_index >= 0)
	{
		if (video->videoq->queue.empty())
			schedule_refresh(media, 1);
		else
		{
			video->frameq.deQueue(&video->frame);

			// 将视频同步到音频上，计算下一帧的延迟时间
			double current_pts = *(double*)video->frame->opaque;
			double delay = current_pts - video->frame_last_pts;
			if (delay <= 0 || delay >= 1.0)
				delay = video->frame_last_delay;

			video->frame_last_delay = delay;
			video->frame_last_pts = current_pts;

			// 当前显示帧的PTS来计算显示下一帧的延迟
			double ref_clock = media->audio->get_audio_clock();

			double diff = current_pts - ref_clock;// diff < 0 => video slow,diff > 0 => video quick

			double threshold = (delay > SYNC_THRESHOLD) ? delay : SYNC_THRESHOLD;

			if (fabs(diff) < NOSYNC_THRESHOLD) // 不同步
			{
				if (diff <= -threshold) // 慢了，delay设为0
					delay = 0;
				else if (diff >= threshold) // 快了，加倍delay
					delay *= 2;
			}
			video->frame_timer += delay;
			double actual_delay = video->frame_timer - static_cast<double>(av_gettime()) / 1000000.0;
			if (actual_delay <= 0.010)
				actual_delay = 0.010; 

			schedule_refresh(media, static_cast<int>(actual_delay * 1000 + 0.5));

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