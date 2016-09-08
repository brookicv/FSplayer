
#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include "Audio.h"

extern "C" {

#include <libavformat\avformat.h>

}

struct MediaState
{
	AudioState *audio;
	AVFormatContext *pFormatCtx;

	char* filename;
	//bool quit;

	MediaState(char *filename);

	~MediaState();

	bool openInput();
};

int decode_thread(void *data);

#endif