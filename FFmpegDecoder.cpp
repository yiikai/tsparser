#include "FFmpegDecoder.h"

#include <iostream>
FFmpegDecoder::FFmpegDecoder() :Decoder()
{

}

FFmpegDecoder::~FFmpegDecoder()
{

}

void FFmpegDecoder::init()
{
	av_register_all();
	avformat_network_init();
	sdlrect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
}

bool FFmpegDecoder::initVideoDecoder(AVCODEC_TYPE type)
{
	m_videocodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!m_videocodec) {
		return false;
	}
	m_codecctx = avcodec_alloc_context3(m_videocodec);
	if (!m_codecctx)
	{
		std::cout << "alloc codec ctx error" << std::endl;
		return false;
	}
	int ret = 0;
	if ((ret = avcodec_open2(m_codecctx, m_videocodec, NULL)) < 0) {
		return false;
	}
	int sdlret = SDL_Init(SDL_INIT_VIDEO);
	if (sdlret)
	{
		std::cout << "SDL init error" << SDL_GetError() << std::endl;
		return false;
	}
	screen_width = 320;
	screen_height = 240;
	screen = SDL_CreateWindow("Player window 1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen)
	{
		std::cout << "window create error" << std::endl;
		return false;
	}
	sdlRender = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	sdlTexture = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
	sdlrect->x = 0;
	sdlrect->y = 0;
	sdlrect->w = screen_width;
	sdlrect->h = screen_height;
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, sdlrect->w, sdlrect->h));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, sdlrect->w, sdlrect->h);
	return true;
}

#define MAX_AUDIO_FRAME_SIZE 192000
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;
void  fill_audio(void *udata, Uint8 *stream, int len){
	//SDL 2.0  
	SDL_memset(stream, 0, len);
	if (audio_len == 0)        /*  Only  play  if  we  have  data  left  */
		return ;
	len = (len > audio_len ? audio_len : len);   /*  Mix  as  much  data  as  possible  */

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

bool FFmpegDecoder::initAudioDecoder(AVCODEC_TYPE type)
{
	//AVFormatContext *pFormatCtx = NULL;
	//pFormatCtx = avformat_alloc_context();
	//	if (avformat_open_input(&pFormatCtx, "D:\\main.ts", NULL, NULL) != 0){
	//		printf("Couldn't open input stream.\n");
	//		return false;
	//	}
	//	// Retrieve stream information  
	//	if (avformat_find_stream_info(pFormatCtx, NULL) < 0){
	//		printf("Couldn't find stream information.\n");
	//		return false;
	//	}
	//	// Get a pointer to the codec context for the audio stream  
	//
	//	int audioStream = -1;
	//	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	//		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
	//		audioStream = i;
	//		break;
	//		}
	//
	//	if (audioStream == -1){
	//		printf("Didn't find a audio stream.\n");
	//		return false;
	//	}
	//
		//m_audiocodecctx = pFormatCtx->streams[audioStream]->codec;
	// Find the decoder for the audio stream  
	m_audiocodec = avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (m_audiocodec == NULL){
		printf("Codec not found.\n");
		return false;
	}
	m_audiocodecctx = avcodec_alloc_context3(m_audiocodec);
	if (!m_audiocodecctx)
	{
		std::cout << "alloc codec ctx error" << std::endl;
		return false;
	}
	// Open codec  
	if (avcodec_open2(m_audiocodecctx, m_audiocodec, NULL) < 0){
		printf("Could not open codec.\n");
		return false;
	}

	//Out Audio Param  
	out_channel_layout = AV_CH_LAYOUT_STEREO;
	out_nb_samples = 1024;//m_audiocodecctx->frame_size;
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	out_sample_rate = 24000;
	out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	//Out Buffer Size  
	out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

	out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
	//SDL------------------  
	in_channel_layout = av_get_default_channel_layout(2);
	//Init  
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return false;
	}
	//SDL_AudioSpec  
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = m_audiocodecctx;

	if (SDL_OpenAudio(&wanted_spec, NULL) < 0){
		printf("can't open audio.\n");
		return false;
	}
	au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout, m_audiocodecctx->sample_fmt, out_sample_rate, 0, NULL);
	swr_init(au_convert_ctx);

	return true;
}

bool FFmpegDecoder::decodeVideo(PACKET& packet)
{
	int got_frame;
	AVPacket videopacket;
	av_init_packet(&videopacket);
	videopacket.data = &(*(packet.data))[0];
	videopacket.size = packet.size;
	
	AVFrame *videoframe = av_frame_alloc();
	int ret;
	ret = avcodec_decode_video2(m_codecctx, videoframe, &got_frame, &videopacket);
	if (ret < 0) {
		return false;
	}

	if (got_frame) {
		SDL_UpdateYUVTexture(sdlTexture, sdlrect,
			videoframe->data[0], videoframe->linesize[0],
			videoframe->data[1], videoframe->linesize[1],
			videoframe->data[2], videoframe->linesize[2]);
		SDL_RenderClear(sdlRender);
		SDL_RenderCopy(sdlRender, sdlTexture, NULL, sdlrect);
		SDL_RenderPresent(sdlRender);
	}
	else
	{
		std::cout << "not got a frame" << std::endl;
	}
	av_frame_free(&videoframe);
	return true;
}

bool FFmpegDecoder::decodeAudio(PACKET& packet)
{
	std::cout << "start decode audio data" << std::endl;
	int got_picture;
	AVPacket audiopacket;
	av_init_packet(&audiopacket);
	audiopacket.data = &((*(packet.data))[0]);
	audiopacket.size = packet.size;
	audiopacket.pts = packet.pts;
	
	AVFrame *audioframe = av_frame_alloc();
	int ret;
	ret = avcodec_decode_audio4(m_audiocodecctx, audioframe, &got_picture, &audiopacket);
	if (ret < 0) {
		printf("Error in decoding audio frame.\n");
		return false;
	}
	if (got_picture > 0){//got_picture,是否有音频数据被解码  

		//转化音频数据  

		swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)audioframe->data, audioframe->nb_samples);

		//FIX:FLAC,MP3,AAC Different number of samples  
		/*在解码循环中添加了一小段代码，可以根据解码后AVFrame中的nb_samples调整
		*SDL_AudioSpec中的samples的大小。这样不用改代码就可以正常播放AAC，MP3这
		*些每帧采样数不同的音频流了。
		*/
		if (wanted_spec.samples != audioframe->nb_samples){
			SDL_CloseAudio();
			out_nb_samples = audioframe->nb_samples;
			//out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
			wanted_spec.samples = out_nb_samples;
			SDL_OpenAudio(&wanted_spec, NULL);
		}
	}
	//SDL------------------  
	//Set audio buffer (PCM data)  
	audio_chunk = (Uint8 *)out_buffer;
	//Audio buffer length  
	audio_len = out_buffer_size;
	audio_pos = audio_chunk;
	//Play开始播放  
	SDL_PauseAudio(0);
	while (audio_len > 0)//Wait until finish  
		SDL_Delay(1); //延迟一毫秒 
	av_frame_free(&audioframe);
	return true;
}
