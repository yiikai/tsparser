#include "StreamController.h"
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
#define MAX_BUF_SIZE 1800000  //ms   Ts时间的计算方法 s * 1000 * 90     *1000表示毫秒 *90表示timescale


std::shared_ptr<StreamContrller> StreamControllerFactory::CreateWantController(STREAMTYPE type)
{
	std::shared_ptr<StreamContrller> controller;
	switch (type)
	{
	case HLS:
	{
		controller = std::make_shared<HLSStrreamController>();
	}break;
	default:
		break;
	}
	return controller;
}

void HLSStrreamController::init(unsigned char* url)
{
	StreamContrller::init(url);
}



void StreamContrller::InitAVDevice()
{
	
}


HLSStrreamController::HLSStrreamController()
{
	
}

HLSStrreamController::~HLSStrreamController()
{

}

void HLSStrreamController::VideoThreadFunc(playlist video,HLSStrreamController *controller)
{
	std::list<chunk>::iterator itr = video.chunklist.begin();
	while (itr != video.chunklist.end())
	{
		std::shared_ptr<std::vector<unsigned char>> chunkdata;
		HttpDownloader dm;
		dm.init();
		char buf[100] = { 0 };
		sprintf(buf, "%d-%d", itr->byterange_low, itr->byterange_up);
		dm.setDownloadRange(buf);
		std::cout << "download url is " << itr->url <<"range is "<<buf <<std::endl;
		dm.startDownload((unsigned char*)itr->url.c_str(), chunkdata);
		std::cout << "download chunk sucess" << std::endl;

		controller->m_tsParser.read(std::string(), chunkdata, false);
		controller->m_tsParser.Parse();
		while (1)
		{
				PACKET videopacket;
				if (!controller->m_tsParser.GetPacket(TYPE_VIDEO, videopacket))
				{
					std::cout << "Video packet get end" << std::endl;
					break;
				}
				controller->m_videobuffer.PutIn(videopacket);
		}
		itr++;
	}
}

void HLSStrreamController::AudioThreadFunc(playlist audio, HLSStrreamController *controller)
{
	std::list<chunk>::iterator itr = audio.chunklist.begin();
	while (itr != audio.chunklist.end())
	{
		std::shared_ptr<std::vector<unsigned char>> chunkdata;
		HttpDownloader dm;
		dm.init();
		char buf[100] = { 0 };
		sprintf(buf, "%d-%d", itr->byterange_low, itr->byterange_up);
		dm.setDownloadRange(buf);
		dm.startDownload((unsigned char*)itr->url.c_str(), chunkdata);
		std::cout << "download chunk sucess" << std::endl;

		controller->m_tsParser.read(std::string(), chunkdata, false);
		controller->m_tsParser.Parse();
		controller->m_audiobufduration += controller->m_tsParser.GetTsFileDuration();
		while (1)
		{
				PACKET audiopacket;
				if (!controller->m_tsParser.GetPacket(TYPE_AUDIO, audiopacket))
				{
					std::cout << "Audio packet get end" << std::endl;
					break;
				}
				controller->m_audiobuffer.PutIn(audiopacket);
		}
		itr++;
	}
}

void HLSStrreamController::SubThreadFunc(playlist sub, HLSStrreamController *controller)
{

}

void HLSStrreamController::GetPacketFunc(TRACKTYPE type, HLSStrreamController *controller)
{
	int screen_width = 0, screen_height = 0;
	SDL_Window *screen;
	SDL_Renderer *sdlRender;
	SDL_Texture *sdlTexture;
	SDL_Rect *sdlrect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	struct SwsContext *img_convert_ctx;
	av_register_all();
	AVCodec* videocodec = NULL;
	videocodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!videocodec) {
		return;
	}
	AVCodecContext *codecctx = NULL;
	codecctx = avcodec_alloc_context3(videocodec);
	if (!codecctx)
	{
		std::cout << "alloc codec ctx error" << std::endl;
		return;
	}
	int ret = 0;
	if ((ret = avcodec_open2(codecctx, videocodec, NULL)) < 0) {
		return;
	}

	int sdlret = SDL_Init(SDL_INIT_VIDEO);
	if (sdlret)
	{
		std::cout << "SDL init error" << SDL_GetError() << std::endl;
		return;
	}
	screen_width = 416;
	screen_height = 234;
	screen = SDL_CreateWindow("Player window 1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		std::cout << "window create error" << std::endl;
		return;
	}
	sdlRender = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 416, 234);
	sdlrect->x = 0;
	sdlrect->y = 0;
	sdlrect->w = screen_width;
	sdlrect->h = screen_height;

	int got_frame = 0;
	while (1)
	{
		PACKET packet;
		if (type == TYPE_VIDEO)
		{
			
			if (controller->m_videobuffer.PullOut(packet) == false)
			{
				continue;
			}
			FILE * f264dump = NULL; 
			f264dump = fopen("264dump", "wb");
			fwrite(&(*(packet.data))[0], 1, packet.size, f264dump);
			fclose(f264dump);
		}
		else if (type == TYPE_AUDIO)
		{
			//PACKET packet;
			//controller->m_audiobuffer.PullOut(packet);
		}
	
			AVPacket videopacket;
			av_init_packet(&videopacket);
			videopacket.data = &(*(packet.data))[0];
			videopacket.size = packet.size;

			AVFrame *videoframe = av_frame_alloc();
			ret = avcodec_decode_video2(codecctx, videoframe, &got_frame, &videopacket);
			if (ret < 0) {
				return;
			}
			/*video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
			416, 234, codecctx->pix_fmt, 1);
			img_convert_ctx = sws_getContext(codecctx->width, codecctx->height, codecctx->pix_fmt,
			codecctx->width, codecctx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);*/
			if (got_frame) {

				/*av_image_copy(video_dst_data, video_dst_linesize,
				(const uint8_t **)(videoframe->data), videoframe->linesize,
				codecctx->pix_fmt, codecctx->width, codecctx->height);
				*/

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
	
}

void HLSStrreamController::start()
{
	m_hlsParser.Parser(m_manifestdownloaddata, m_mainfesturl);
	//开三个线程去拿audio,video,subititle 给render和decode
	playlist video;
	playlist audio;
	playlist sub;
	m_hlsParser.getSelectTrackPlaylist(video, audio, sub);
	if (!video.chunklist.empty())
	{
		m_hasvideo = true;
		m_readvideothread = std::thread(VideoThreadFunc, video, this);
	}
	else
	{
		m_hasvideo = false;
	}
	if (!audio.chunklist.empty())
	{
		m_hasaudio = true;
		m_readaudiothread = std::thread(AudioThreadFunc, audio, this);
	}
	else
	{
		m_hasaudio = false;
	}
	if (!sub.chunklist.empty())
	{
		m_hassub = true;
		m_readsubthread = std::thread(SubThreadFunc, sub, this);
	}
	else
	{
		m_hassub = false;
	}


	if (m_hasvideo)
	{
		m_getvideothread = std::thread(GetPacketFunc, TYPE_VIDEO, this);
	}
	while (1)
	{

	}

	
}

void HLSStrreamController::getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist)
{
	m_hlsParser.getSelectTrackPlaylist(vplist, aplist, splist);
}
