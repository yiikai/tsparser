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
	m_decoder.init();
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

void HLSStrreamController::VideoThreadFunc(playlist video, HLSStrreamController *controller)
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
		std::cout << "download url is " << itr->url << "range is " << buf << std::endl;
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

void HLSStrreamController::AudioThreadFunc(playlist audio, HLSStrreamController *controller)     //这里有个问题， 现在如果audio和video是在一起的话没办法获取出audio的packet
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
#define MAX_AUDIO_FRAME_SIZE 192000
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
void HLSStrreamController::GetPacketFunc(TRACKTYPE type, HLSStrreamController *controller)
{
	controller->m_decoder.initVideoDecoder(AVCODEC_TYPE::VIDEO_TYPE_H264);
	while (1)
	{
		if (type == TYPE_VIDEO)
		{
			PACKET packet;
			if (controller->m_videobuffer.PullOut(packet) == false)
			{
				continue;
			}
			controller->m_decoder.decodeVideo(packet);
		}
		else if (type == TYPE_AUDIO)
		{

		}
	}
//	AVFormatContext *pFormatCtx;
//	int             i, audioStream;
//	AVCodecContext  *pCodecCtx;
//	AVCodec         *pCodec;
//
//	uint8_t         *out_buffer;
//	AVFrame         *pFrame;
//	SDL_AudioSpec wanted_spec;
//	int ret;
//	uint32_t len = 0;
//	int got_picture;
//	int index = 0;
//	int64_t in_channel_layout;
//	struct SwrContext *au_convert_ctx;
//	av_register_all();
//	avformat_network_init();
//	pFormatCtx = avformat_alloc_context();
//	if (avformat_open_input(&pFormatCtx, "D:\\main.ts", NULL, NULL) != 0){
//		printf("Couldn't open input stream.\n");
//		return;
//	}
//	// Retrieve stream information  
//	if (avformat_find_stream_info(pFormatCtx, NULL) < 0){
//		printf("Couldn't find stream information.\n");
//		return;
//	}
//	// Get a pointer to the codec context for the audio stream  
//
//	audioStream = -1;
//	for (i = 0; i < pFormatCtx->nb_streams; i++)
//		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
//		audioStream = i;
//		break;
//		}
//
//	if (audioStream == -1){
//		printf("Didn't find a audio stream.\n");
//		return;
//	}
//
//	pCodecCtx = pFormatCtx->streams[audioStream]->codec;
//
//	// Find the decoder for the audio stream  
//	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
//	if (pCodec == NULL){
//		printf("Codec not found.\n");
//		return;
//	}
//
//	// Open codec  
//	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
//		printf("Could not open codec.\n");
//		return;
//	}
//
//#if OUTPUT_PCM  
//	pFile = fopen("output.pcm", "wb");
//#endif  
//
//
//
//	//Out Audio Param  
//	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
//	//nb_samples: AAC-1024 MP3-1152  
//	int out_nb_samples = pCodecCtx->frame_size;
//	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
//	int out_sample_rate = 44100;
//	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
//	//Out Buffer Size  
//	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
//
//	out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
//	pFrame = av_frame_alloc();
//	//SDL------------------  
//
//	//Init  
//	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
//		printf("Could not initialize SDL - %s\n", SDL_GetError());
//		return;
//	}
//	//SDL_AudioSpec  
//	wanted_spec.freq = out_sample_rate;
//	wanted_spec.format = AUDIO_S16SYS;
//	wanted_spec.channels = out_channels;
//	wanted_spec.silence = 0;
//	wanted_spec.samples = out_nb_samples;
//	wanted_spec.callback = fill_audio;
//	wanted_spec.userdata = pCodecCtx;
//
//	if (SDL_OpenAudio(&wanted_spec, NULL) < 0){
//		printf("can't open audio.\n");
//		return ;
//	}
//
//
//	//FIX:Some Codec's Context Information is missing  
//	in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
//	//Swr  
//
//	au_convert_ctx = swr_alloc();
//	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
//		in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
//	swr_init(au_convert_ctx);
//	//tsfileparser.setVideoItr();
//	
//	//tsfileparser.setVideoItr();
//	while (1){
//
//
//		//解码音频帧的大小从avpkt->size 到avpkt->data 成帧。
//		PACKET packet;
//		if (controller->m_audiobuffer.PullOut(packet) == false)
//		{
//			continue;
//		}
//		//std::shared_ptr<std::vector<unsigned char>> audiobuf = tsfileparser.startgetvideobuf();
//		AVPacket fuckpacket;
//		av_init_packet(&fuckpacket);
//		fuckpacket.data = &((*(packet.data))[0]);
//		fuckpacket.size = packet.size;
//		fuckpacket.pts = packet.pts;
//
//		ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, &fuckpacket);
//		if (ret < 0) {
//			printf("Error in decoding audio frame.\n");
//			return ;
//		}
//		if (got_picture > 0){//got_picture,是否有音频数据被解码  
//
//			//转化音频数据  
//			swr_convert(au_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);
//#if 0  
//			printf("index:%5d\t pts:%10d\t packet size:%d\n", index, packet->pts, packet->size);
//#endif  
//#if 1  
//			//FIX:FLAC,MP3,AAC Different number of samples  
//			/*在解码循环中添加了一小段代码，可以根据解码后AVFrame中的nb_samples调整
//			*SDL_AudioSpec中的samples的大小。这样不用改代码就可以正常播放AAC，MP3这
//			*些每帧采样数不同的音频流了。
//			*/
//			if (wanted_spec.samples != pFrame->nb_samples){
//				SDL_CloseAudio();
//				out_nb_samples = pFrame->nb_samples;
//				out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
//				wanted_spec.samples = out_nb_samples;
//				SDL_OpenAudio(&wanted_spec, NULL);
//			}
//#endif  
//
//#if OUTPUT_PCM  
//			//Write PCM  
//			fwrite(out_buffer, 1, out_buffer_size, pFile);
//#endif  
//
//			index++;
//		}
//		//SDL------------------  
//#if 1  
//		//Set audio buffer (PCM data)  
//		audio_chunk = (Uint8 *)out_buffer;
//		//Audio buffer length  
//		audio_len = out_buffer_size;
//
//		audio_pos = audio_chunk;
//		//Play开始播放  
//		SDL_PauseAudio(0);
//		while (audio_len > 0)//Wait until finish  
//			SDL_Delay(1); //延迟一毫秒  
//#endif  
//	}
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
