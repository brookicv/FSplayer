
#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include "Audio.h"
#include "Video.h"

extern "C" {

#include <libavformat\avformat.h>

}

struct VideoState;

struct MediaState
{
	AudioState *audio;
	VideoState *video;
	AVFormatContext *pFormatCtx;

	char* filename;
	//bool quit;

	MediaState(char *filename);

	~MediaState();

	bool openInput();
};

int decode_thread(void *data);

#endif