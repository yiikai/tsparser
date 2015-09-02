// TSParser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TsFileParser.h"
#include "Mp4FileParser.h"
#include "HlsParser.h"
//ffmpeg API
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <SDL.h>
#include <curl/curl.h>
}
#include "HttpDownload.h"

static AVFormatContext *fmt_ctx = NULL;
static AVPacket pkt;
static AVStream *videostream = NULL;
//static AVFrame *videoframe = NULL;
static uint8_t *video_dst_data[4] = { NULL };
static int      video_dst_linesize[4];
static int video_dst_bufsize;

//long writer(void *data, int size, int nmemb, void *userdata)
//{
//	long sizes = size * nmemb;
//	std::vector<unsigned char> tmpvec((unsigned char*)data,(unsigned char*)data+sizes);
//	((std::vector<unsigned char>*)userdata)->insert(((std::vector<unsigned char>*)userdata)->end(), tmpvec.begin(),tmpvec.end());
//	return sizes;
//}

int _tmain(int argc, _TCHAR* argv[])
{
	//////////////////////////This is parse download M3U////////////////////////////////////
	/*std::shared_ptr<std::vector<char>> buf;
	const char* url = "http://10.2.68.7:8082/hls/v8/bipbop_16x9_variant.m3u8";
	{
		HttpDownloader dm;
		dm.init();
		dm.startDownload(url, buf);
		HlsParser hls;
		hls.Parser(buf, url);
		std::shared_ptr<std::vector<STREAMINFO>> streaminfo;
		playlist video;
		playlist audio;
		playlist subtrack;
		hls.getSelectTrackPlaylist(video,audio,subtrack);
		if (video.chunklist.empty())
		{
			return 0;
		}
		std::list<chunk>::iterator itr = video.chunklist.begin();
		for (; itr != video.chunklist.end(); itr++)
		{
			std::shared_ptr<std::vector<char>> chunkdata;
			HttpDownloader dm;
			dm.init();
			char buf[100] = {0};
			sprintf(buf, "%d-%d", itr->byterange_low, itr->byterange_up);
			dm.setDownloadRange(buf);
			dm.startDownload(itr->url.c_str(),chunkdata);
			std::cout << "download chunk sucess" << std::endl;

		}
		
	}

	
	return 0;*/

	/////////////////////////This is play video////////////////////////////////
	//SDL
	std::shared_ptr<std::vector<unsigned char>> buf;
	const char* url = "http://10.2.68.7:8082/hls/v8/bipbop_16x9_variant.m3u8";
	HttpDownloader dm;
	dm.init();
	dm.startDownload(url, buf);
	HlsParser hls;
	hls.Parser(buf, url);
	std::shared_ptr<std::vector<STREAMINFO>> streaminfo;
	playlist video;
	playlist audio;
	playlist subtrack;
	hls.getSelectTrackPlaylist(video, audio, subtrack);
	if (video.chunklist.empty())
	{
		return 0;
	}
	std::list<chunk>::iterator itr = video.chunklist.begin();
	/*for (; itr != video.chunklist.end(); itr++)
	{*/
		std::shared_ptr<std::vector<unsigned char>> chunkdata;
		HttpDownloader dm2;
		dm2.init();
		char buf2[100] = { 0 };
		sprintf(buf2, "%d-%d", itr->byterange_low, itr->byterange_up);
		dm2.setDownloadRange(buf2);
		dm2.startDownload(itr->url.c_str(), chunkdata);
		std::cout << "download chunk sucess" << std::endl;

	//}

	


	int screen_width = 0, screen_height = 0;
	SDL_Window *screen;
	SDL_Renderer *sdlRender;
	SDL_Texture *sdlTexture;
	SDL_Rect *sdlrect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	struct SwsContext *img_convert_ctx;
	/////

	TsFileParser tsfileparser;
	tsfileparser.read(std::string(), chunkdata, false);
	tsfileparser.Parse();
	tsfileparser.printvideo();


	////ffmpeg

	av_register_all();
	/*if (avformat_open_input(&fmt_ctx, filepath.c_str(), NULL, NULL) < 0) {
		return 0;
		}
		if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		return 0;
		}
		av_dump_format(fmt_ctx, 0, filepath.c_str(), 0);
		videostream = fmt_ctx->streams[0];*/
	
	AVCodec* videocodec = NULL;
	videocodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!videocodec) {
		return 0;
	}
	AVCodecContext *codecctx = NULL;
	codecctx = avcodec_alloc_context3(videocodec);
	if (!codecctx)
	{
		std::cout << "alloc codec ctx error" << std::endl;
		return 0;
	}
	int ret = 0;
	if ((ret = avcodec_open2(codecctx, videocodec, NULL)) < 0) {
		return 0;
	}

	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	/*video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
		416, 234, videostream->codec->pix_fmt, 1);
	img_convert_ctx = sws_getContext(videostream->codec->width, videostream->codec->height, videostream->codec->pix_fmt,
		videostream->codec->width, videostream->codec->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);*/

	////SDL create
	int sdlret = SDL_Init(SDL_INIT_VIDEO);
	if (sdlret)
	{
		std::cout << "SDL init error" << SDL_GetError() << std::endl;
		return 0;
	}
	screen_width = 416;
	screen_height = 234;
	screen = SDL_CreateWindow("Player window 1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		std::cout << "window create error" << std::endl;
		return 0;
	}
	sdlRender = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 416, 234);
	sdlrect->x = 0;
	sdlrect->y = 0;
	sdlrect->w = screen_width;
	sdlrect->h = screen_height;
	
	int got_frame = 0;
	tsfileparser.setVideoItr();
	while (1)
	{
		std::shared_ptr<std::vector<unsigned char>> buf = tsfileparser.startgetvideobuf();
		if (buf == NULL)
			break;
		AVPacket videopacket;
		av_init_packet(&videopacket);
		videopacket.data = &((*buf)[0]);
		videopacket.size = buf->size();
	
		AVFrame *videoframe = av_frame_alloc();
		ret = avcodec_decode_video2(codecctx, videoframe, &got_frame, &videopacket);
		if (ret < 0) {
			return 0;
		}
		video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
		416, 234, codecctx->pix_fmt, 1);
		img_convert_ctx = sws_getContext(codecctx->width, codecctx->height, codecctx->pix_fmt,
			codecctx->width, codecctx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		if (got_frame) {

			av_image_copy(video_dst_data, video_dst_linesize,
				(const uint8_t **)(videoframe->data), videoframe->linesize,
				codecctx->pix_fmt, codecctx->width, codecctx->height);

			
			///* write to rawvideo file */

			SDL_UpdateYUVTexture(sdlTexture, sdlrect,
				videoframe->data[0], videoframe->linesize[0],
				videoframe->data[1], videoframe->linesize[1],
				videoframe->data[2], videoframe->linesize[2]);
			SDL_RenderClear(sdlRender);
			SDL_RenderCopy(sdlRender, sdlTexture, NULL, sdlrect);
			SDL_RenderPresent(sdlRender);
			//SDL End-----------------------  
			//Delay 40ms  
			//SDL_Delay(40);

		}
		else
		{
			std::cout << "not got a frame" << std::endl;
		}
		av_frame_free(&videoframe);
		//}
	}
	return 0;
}