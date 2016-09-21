
#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include "Video.h"

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

// ÑÓ³Ùdelay msºóË¢ÐÂvideoÖ¡
void schedule_refresh(MediaState *media, int delay);

uint32_t sdl_refresh_timer_cb(uint32_t interval, void *opaque);

void video_refresh_timer(void *userdata);

//void video_display(VideoState *video);


#endif