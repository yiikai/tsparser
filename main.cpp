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
#include <libswresample/swresample.h>
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
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;
void  fill_audio(void *udata, Uint8 *stream, int len){
	//SDL 2.0  
	SDL_memset(stream, 0, len);
	if (audio_len == 0)        /*  Only  play  if  we  have  data  left  */
		return;
	len = (len > audio_len ? audio_len : len);   /*  Mix  as  much  data  as  possible  */

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

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
	//	std::shared_ptr<std::vector<unsigned char>> buf;
	//	const char* url = "http://10.2.68.7:8082/hls/v8/bipbop_16x9_variant.m3u8";
	//	HttpDownloader dm;
	//	dm.init();
	//	dm.startDownload(url, buf);
	//	HlsParser hls;
	//	hls.Parser(buf, url);
	//	std::shared_ptr<std::vector<STREAMINFO>> streaminfo;
	//	playlist video;
	//	playlist audio;
	//	playlist subtrack;
	//	hls.getSelectTrackPlaylist(video, audio, subtrack);
	//	if (video.chunklist.empty())
	//	{
	//		return 0;
	//	}
	//	std::list<chunk>::iterator itr = video.chunklist.begin();
	//	/*for (; itr != video.chunklist.end(); itr++)
	//	{*/
	//		std::shared_ptr<std::vector<unsigned char>> chunkdata;
	//		HttpDownloader dm2;
	//		dm2.init();
	//		char buf2[100] = { 0 };
	//		sprintf(buf2, "%d-%d", itr->byterange_low, itr->byterange_up);
	//		dm2.setDownloadRange(buf2);
	//		dm2.startDownload(itr->url.c_str(), chunkdata);
	//		std::cout << "download chunk sucess" << std::endl;
	//
	//	//}
	//
	//	
	//
	//
	//	int screen_width = 0, screen_height = 0;
	//	SDL_Window *screen;
	//	SDL_Renderer *sdlRender;
	//	SDL_Texture *sdlTexture;
	//	SDL_Rect *sdlrect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	//	struct SwsContext *img_convert_ctx;
	//	/////
	//
	//	TsFileParser tsfileparser;
	//	tsfileparser.read(std::string(), chunkdata, false);
	//	tsfileparser.Parse();
	//	tsfileparser.printvideo();
	//
	//
	//	////ffmpeg
	//
	//	av_register_all();
	//	/*if (avformat_open_input(&fmt_ctx, filepath.c_str(), NULL, NULL) < 0) {
	//		return 0;
	//		}
	//		if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
	//		return 0;
	//		}
	//		av_dump_format(fmt_ctx, 0, filepath.c_str(), 0);
	//		videostream = fmt_ctx->streams[0];*/
	//	
	//	AVCodec* videocodec = NULL;
	//	videocodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	//	if (!videocodec) {
	//		return 0;
	//	}
	//	AVCodecContext *codecctx = NULL;
	//	codecctx = avcodec_alloc_context3(videocodec);
	//	if (!codecctx)
	//	{
	//		std::cout << "alloc codec ctx error" << std::endl;
	//		return 0;
	//	}
	//	int ret = 0;
	//	if ((ret = avcodec_open2(codecctx, videocodec, NULL)) < 0) {
	//		return 0;
	//	}
	//
	//	av_init_packet(&pkt);
	//	pkt.data = NULL;
	//	pkt.size = 0;
	//	/*video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
	//		416, 234, videostream->codec->pix_fmt, 1);
	//	img_convert_ctx = sws_getContext(videostream->codec->width, videostream->codec->height, videostream->codec->pix_fmt,
	//		videostream->codec->width, videostream->codec->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);*/
	//
	//	////SDL create
	//	int sdlret = SDL_Init(SDL_INIT_VIDEO);
	//	if (sdlret)
	//	{
	//		std::cout << "SDL init error" << SDL_GetError() << std::endl;
	//		return 0;
	//	}
	//	screen_width = 416;
	//	screen_height = 234;
	//	screen = SDL_CreateWindow("Player window 1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	//		screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//	if (!screen)
	//	{
	//		std::cout << "window create error" << std::endl;
	//		return 0;
	//	}
	//	sdlRender = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	//	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 416, 234);
	//	sdlrect->x = 0;
	//	sdlrect->y = 0;
	//	sdlrect->w = screen_width;
	//	sdlrect->h = screen_height;
	//	
	//	int got_frame = 0;
	//	tsfileparser.setVideoItr();
	//	while (1)
	//	{
	//		std::shared_ptr<std::vector<unsigned char>> buf = tsfileparser.startgetvideobuf();
	//		if (buf == NULL)
	//			break;
	//		AVPacket videopacket;
	//		av_init_packet(&videopacket);
	//		videopacket.data = &((*buf)[0]);
	//		videopacket.size = buf->size();
	//	
	//		AVFrame *videoframe = av_frame_alloc();
	//		ret = avcodec_decode_video2(codecctx, videoframe, &got_frame, &videopacket);
	//		if (ret < 0) {
	//			return 0;
	//		}
	//		/*video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
	//		416, 234, codecctx->pix_fmt, 1);
	//		img_convert_ctx = sws_getContext(codecctx->width, codecctx->height, codecctx->pix_fmt,
	//		codecctx->width, codecctx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);*/
	//		if (got_frame) {
	//
	//			/*av_image_copy(video_dst_data, video_dst_linesize,
	//				(const uint8_t **)(videoframe->data), videoframe->linesize,
	//				codecctx->pix_fmt, codecctx->width, codecctx->height);
	//*/
	//			
	//			///* write to rawvideo file */
	//
	//			SDL_UpdateYUVTexture(sdlTexture, sdlrect,
	//				videoframe->data[0], videoframe->linesize[0],
	//				videoframe->data[1], videoframe->linesize[1],
	//				videoframe->data[2], videoframe->linesize[2]);
	//			SDL_RenderClear(sdlRender);
	//			SDL_RenderCopy(sdlRender, sdlTexture, NULL, sdlrect);
	//			SDL_RenderPresent(sdlRender);
	//			//SDL End-----------------------  
	//			//Delay 40ms  
	//			//SDL_Delay(40);
	//
	//		}
	//		else
	//		{
	//			std::cout << "not got a frame" << std::endl;
	//		}
	//		av_frame_free(&videoframe);
	//		//}
	//	}
	//	return 0;



	///SDL　AUDIO
#define MAX_AUDIO_FRAME_SIZE 192000






	AVFormatContext *pFormatCtx;
	int             i, audioStream;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;

	uint8_t         *out_buffer;
	AVFrame         *pFrame;
	SDL_AudioSpec wanted_spec;
	int ret;
	uint32_t len = 0;
	int got_picture;
	int index = 0;
	int64_t in_channel_layout;
	struct SwrContext *au_convert_ctx;

	FILE *pFile = NULL;




	std::shared_ptr<std::vector<unsigned char>> buf;
	const char* url = "http://10.2.68.7:8082/hls/v8/bipbop_16x9_variant.m3u8";
	HttpDownloader dm;
	dm.init();
	dm.startDownload((const unsigned char*)url, buf);
	HlsParser hls;
	hls.Parser(buf, (char*)url);
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
	dm2.startDownload((unsigned char*)itr->url.c_str(), chunkdata);
	std::cout << "download chunk sucess" << std::endl;

	//}



	TsFileParser tsfileparser;
	tsfileparser.read(std::string(), chunkdata, false);
	tsfileparser.Parse();
	tsfileparser.printvideo();



	
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, "D:\\main.ts", NULL, NULL) != 0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	// Retrieve stream information  
	if (avformat_find_stream_info(pFormatCtx,NULL) < 0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	// Get a pointer to the codec context for the audio stream  

	audioStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
		audioStream = i;
		break;
		}

	if (audioStream == -1){
		printf("Didn't find a audio stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream  
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL){
		printf("Codec not found.\n");
		return -1;
	}

	// Open codec  
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
		printf("Could not open codec.\n");
		return -1;
	}

#if OUTPUT_PCM  
	pFile = fopen("output.pcm", "wb");
#endif  



	//Out Audio Param  
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	//nb_samples: AAC-1024 MP3-1152  
	int out_nb_samples = pCodecCtx->frame_size;
	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = 44100;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	//Out Buffer Size  
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

	out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
	pFrame = av_frame_alloc();
	//SDL------------------  

	//Init  
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	//SDL_AudioSpec  
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL) < 0){
		printf("can't open audio.\n");
		return -1;
	}


	//FIX:Some Codec's Context Information is missing  
	in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
	//Swr  

	au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);
	//tsfileparser.setVideoItr();
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//tsfileparser.setVideoItr();
	while (av_read_frame(pFormatCtx, packet) >= 0){
		if (packet->stream_index == audioStream){

			//解码音频帧的大小从avpkt->size 到avpkt->data 成帧。
			PACKET audiopacket;
			if (!tsfileparser.GetPacket(TYPE_AUDIO, audiopacket))
			{
				std::cout << "packet get end" << std::endl;
				return 0;
			}
			//std::shared_ptr<std::vector<unsigned char>> audiobuf = tsfileparser.startgetvideobuf();
			FILE* myts = NULL;
			myts = fopen("mytsaudiodump.txt", "wb");
			if (!myts)
			{
			return 0;
			}
			fwrite(&((*(audiopacket.data))[0]), 1, audiopacket.size, myts);
			fclose(myts);
			FILE* fftsaudio = NULL;
			fftsaudio = fopen("fftsaudiodump.txt", "wb");
			if (!fftsaudio)
			{
			return 0;
			}
			fwrite(packet->data, 1, packet->size, fftsaudio);
			fclose(fftsaudio);
			AVPacket fuckpacket;
			av_init_packet(&fuckpacket);
			fuckpacket.data = &((*(audiopacket.data))[0]);
			fuckpacket.size = audiopacket.size;
			fuckpacket.pts = audiopacket.pts;

			ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, &fuckpacket);
			if (ret < 0) {
				printf("Error in decoding audio frame.\n");
				return -1;
			}
			if (got_picture > 0){//got_picture,是否有音频数据被解码  

				//转化音频数据  
				swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);
#if 0  
				printf("index:%5d\t pts:%10d\t packet size:%d\n", index, packet->pts, packet->size);
#endif  
#if 1  
				//FIX:FLAC,MP3,AAC Different number of samples  
				/*在解码循环中添加了一小段代码，可以根据解码后AVFrame中的nb_samples调整
				*SDL_AudioSpec中的samples的大小。这样不用改代码就可以正常播放AAC，MP3这
				*些每帧采样数不同的音频流了。
				*/
				if (wanted_spec.samples != pFrame->nb_samples){
					SDL_CloseAudio();
					out_nb_samples = pFrame->nb_samples;
					out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
					wanted_spec.samples = out_nb_samples;
					SDL_OpenAudio(&wanted_spec, NULL);
				}
#endif  

#if OUTPUT_PCM  
				//Write PCM  
				fwrite(out_buffer, 1, out_buffer_size, pFile);
#endif  

				index++;
			}
			//SDL------------------  
#if 1  
			//Set audio buffer (PCM data)  
			audio_chunk = (Uint8 *)out_buffer;
			//Audio buffer length  
			audio_len = out_buffer_size;

			audio_pos = audio_chunk;
			//Play开始播放  
			SDL_PauseAudio(0);
			while (audio_len>0)//Wait until finish  
				SDL_Delay(1); //延迟一毫秒  
#endif  
		}
		av_free_packet(packet);
	}

	swr_free(&au_convert_ctx);


	SDL_CloseAudio();//Close SDL  
	SDL_Quit();

	// Close file  
#if OUTPUT_PCM  
	fclose(pFile);
#endif  
	av_free(out_buffer);
	// Close the codec  
	avcodec_close(pCodecCtx);
	// Close the video file  
	avformat_close_input(&pFormatCtx);

	return 0;

	//////////////
}