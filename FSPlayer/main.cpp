extern "C" {

#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>

}

#include <SDL.h>
#include <SDL_thread.h>

#include <iostream>

#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
#include "VideoDisplay.h"
#include <vld.h>
using namespace std;

bool quit = false;

int main(int argv, char* argc[])
{
	av_register_all();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	char* filename = "E:\\Wildlife.wmv";
	MediaState media(filename);

	if (media.openInput())
		SDL_CreateThread(decode_thread, "", &media); // 创建解码线程，读取packet到队列中缓存

	media.audio->audio_play(); // create audio thread

	media.video->video_play(); // create video thread

	SDL_Event event;
	while (true) // SDL event loop
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case FF_QUIT_EVENT:
		case SDL_QUIT:
			quit = 1;
			SDL_Quit();
			return 0;

		case FF_REFRESH_EVENT:
			video_refresh_timer(media.video);
			break;

		default:
			break;
		}
	}

	getchar();
	return 0;
}