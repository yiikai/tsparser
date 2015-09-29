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
	if (audio.chunklist.empty() && controller->m_hlsParser.HasAudio())
	{
		while (1)
		{
			PACKET audiopacket;
			if (!controller->m_tsParser.GetPacket(TYPE_AUDIO, audiopacket))
			{
				std::cout << "Audio packet get end" << std::endl;
				continue;
			}
			controller->m_audiobuffer.PutIn(audiopacket);
		}
	}
	else
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

			//controller->m_tsParser.read(std::string(), chunkdata, false);
			//controller->m_tsParser.Parse();
			controller->m_aacparser.read(std::string(), chunkdata, false);
			controller->m_aacparser.Parse();
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
}

void HLSStrreamController::SubThreadFunc(playlist sub, HLSStrreamController *controller)
{

}

void HLSStrreamController::GetPacketFunc(TRACKTYPE type, HLSStrreamController *controller)
{
	if (type == TYPE_VIDEO)
		controller->m_decoder.initVideoDecoder(AVCODEC_TYPE::VIDEO_TYPE_H264);
	else if (type == TYPE_AUDIO)
		controller->m_decoder.initAudioDecoder(AVCODEC_TYPE::AUDIO_TYPE_AAC);
	while (1)
	{
		if (type == TYPE_VIDEO)
		{
			PACKET packet;
			if (controller->m_videobuffer.PullOut(packet) == false)
			{
				continue;
			}

			std::unique_lock<std::mutex> lck(controller->m_syncvideomutex);
			if (controller->m_syncvideotime >= controller->m_syncaudiotime)
			{
				
				double sleeptime = (controller->m_syncvideotime - controller->m_syncaudiotime) / 1000;
				lck.unlock();
				Sleep(sleeptime);
				controller->m_decoder.decodeVideo(packet);
				lck.lock();
				controller->m_syncvideotime = packet.pts;
			}
			else
			{
				controller->m_decoder.decodeVideo(packet);
				controller->m_syncvideotime = packet.pts;
			}

		}
		else if (type == TYPE_AUDIO)
		{
			PACKET packet;
			if (controller->m_audiobuffer.PullOut(packet) == false)
			{
				continue;
			}
			std::unique_lock<std::mutex> lck(controller->m_syncvideomutex);
			if (controller->m_syncaudiotime >= controller->m_syncvideotime)
			{
				
				double sleeptime = (controller->m_syncaudiotime - controller->m_syncvideotime) / 1000;
				lck.unlock();
				Sleep(sleeptime);
				controller->m_decoder.decodeAudio(packet);
				lck.lock();
				controller->m_syncaudiotime = packet.pts;
			}
			else
			{
				controller->m_decoder.decodeAudio(packet);
				controller->m_syncaudiotime = packet.pts;
			}
		}
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
	if (m_hlsParser.HasVideo())
	{
		m_hasvideo = true;
		m_readvideothread = std::thread(VideoThreadFunc, video, this);
	}
	else
	{
		m_hasvideo = false;
	}
	if (m_hlsParser.HasAudio())
	{
		m_hasaudio = true;
		m_readaudiothread = std::thread(AudioThreadFunc, audio, this);
	}
	else
	{
		m_hasaudio = false;
	}
	if (m_hlsParser.HasSub())
	{
		m_hassub = true;
		m_readsubthread = std::thread(SubThreadFunc, sub, this);
	}
	else
	{
		m_hassub = false;
	}


	if (m_hlsParser.HasVideo())
	{
		m_getvideothread = std::thread(GetPacketFunc, TYPE_VIDEO, this);
	}
	if (m_hlsParser.HasAudio())
	{
		m_getaudiothread = std::thread(GetPacketFunc, TYPE_AUDIO, this);
	}
	while (1)
	{

	}


}

void HLSStrreamController::getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist)
{
	m_hlsParser.getSelectTrackPlaylist(vplist, aplist, splist);
}
