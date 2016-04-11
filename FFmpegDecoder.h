#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL.h>
#include <curl/curl.h>
}
#include "Decoder.h"

class FFmpegDecoder : Decoder
{
public:
	FFmpegDecoder();
	~FFmpegDecoder();

	void init() override;
	bool initVideoDecoder(AVCODEC_TYPE type) override;
	bool initAudioDecoder(AVCODEC_TYPE type) override;
	bool decodeVideo(PACKET &packet) override;
	bool decodeAudio(PACKET &packet) override;
private:
	AVCodec* m_videocodec;
	AVCodecContext* m_codecctx;
	struct SwsContext *img_convert_ctx;
	AVFrame *pFrameYUV;
	AVCodec* m_audiocodec;
	AVCodecContext* m_audiocodecctx;

	int screen_width = 0;
	int screen_height = 0;
	SDL_Window *screen;
	SDL_Renderer *sdlRender;
	SDL_Texture *sdlTexture;
	SDL_Rect *sdlrect;

	SDL_AudioSpec wanted_spec;
	uint64_t out_channel_layout;
	int out_nb_samples;
	AVSampleFormat out_sample_fmt;
	int out_sample_rate;
	int out_channels;
	//Out Buffer Size  
	int out_buffer_size;
	int64_t in_channel_layout;
	uint8_t         *out_buffer;
	struct SwrContext *au_convert_ctx;
};

#endif