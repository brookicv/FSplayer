
# include "Media.h"
#include <iostream>

extern bool quit;

MediaState::MediaState(char* input_file)
	:filename(input_file)
{
	pFormatCtx = nullptr;
	audio = new AudioState();

	video = new VideoState();
	//quit = false;
}

MediaState::~MediaState()
{
	if(audio)
		delete audio;

	if (video)
		delete video;
}

bool MediaState::openInput()
{
	// Open input file
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) < 0)
		return false;

	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		return false;

	// Output the stream info to standard 
	av_dump_format(pFormatCtx, 0, filename, 0);

	for (uint32_t i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio->audio_stream < 0)
			audio->audio_stream = i;

		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video->video_stream < 0)
			video->video_stream = i;
	}

	if (audio->audio_stream < 0 || video->video_stream < 0)
		return false;

	// Find audio decoder
	AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio->audio_stream]->codec->codec_id);
	if (!pCodec)
		return false;

	audio->audio_ctx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(audio->audio_ctx, pFormatCtx->streams[audio->audio_stream]->codec) != 0)
		return false;

	avcodec_open2(audio->audio_ctx, pCodec, nullptr);

	// Find video decoder
	AVCodec *pVCodec = avcodec_find_decoder(pFormatCtx->streams[video->video_stream]->codec->codec_id);
	if (!pVCodec)
		return false;

	video->video_ctx = avcodec_alloc_context3(pVCodec);
	if (avcodec_copy_context(video->video_ctx, pFormatCtx->streams[video->video_stream]->codec) != 0)
		return false;

	avcodec_open2(video->video_ctx, pVCodec, nullptr);

	return true;
}

int decode_thread(void *data)
{
	MediaState *media = (MediaState*)data;

	AVPacket *packet = av_packet_alloc();

	while (true)
	{
		int ret = av_read_frame(media->pFormatCtx, packet);
		if (ret < 0)
		{
			if (ret == AVERROR_EOF)
				break;
			if (media->pFormatCtx->pb->error == 0) // No error,wait for user input
			{
				SDL_Delay(100);
				continue;
			}
			else
				break;
		}

		if (packet->stream_index == media->audio->audio_stream) // audio stream
		{
			media->audio->audioq.enQueue(packet);
			av_packet_unref(packet);
		}		

		else if (packet->stream_index == media->video->video_stream) // video stream
		{
			media->video->videoq->enQueue(packet);
			av_packet_unref(packet);
			;
		}		
		else
			av_packet_unref(packet);
	}

	

	av_packet_free(&packet);

	std::cout << "demutex finished " << std::endl << "audio packet queue size:" << media->audio->audioq.size << std::endl
		<< "video packet queue size :" << media->video->videoq->size << std::endl;

	// All done,wait for it
	//while (!quit)
		//SDL_Delay(100);

	return 0;
}