#include "StreamController.h"
#define MAX_BUF_SIZE 20000  //ms

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
		dm.startDownload((unsigned char*)itr->url.c_str(), chunkdata);
		std::cout << "download chunk sucess" << std::endl;

		controller->m_tsParser.read(std::string(), chunkdata, false);
		controller->m_tsParser.Parse();
		controller->m_videobufduration += controller->m_tsParser.GetTsFileDuration();
		std::unique_lock<std::mutex> lck(controller->m_videomutex);
		if (controller->m_videobufduration > MAX_BUF_SIZE)
		{
			controller->m_cvV.wait(lck);
		}
		while (1)
		{
			PACKET videopacket;
			if (!controller->m_tsParser.GetPacket(TYPE_VIDEO, videopacket))
			{
				std::cout << "packet get end" << std::endl;
				break;
			}
			controller->m_videopacketbuf.push_back(videopacket);
		}
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
		std::unique_lock<std::mutex> lck(controller->m_audiomutex);
		if (controller->m_audiobufduration > MAX_BUF_SIZE)
		{
			controller->m_cvV.wait(lck);
		}
		while (1)
		{
			PACKET audiopacket;
			if (!controller->m_tsParser.GetPacket(TYPE_AUDIO, audiopacket))
			{
				std::cout << "packet get end" << std::endl;
				break;
			}
			controller->m_audiopacketbuf.push_back(audiopacket);
		}
	}
}

void HLSStrreamController::SubThreadFunc(playlist sub, HLSStrreamController *controller)
{

}

void HLSStrreamController::start()
{
	m_hlsParser.Parser(m_manifestdownloaddata, m_mainfesturl);
	//开三个线程去拿audio,video,subititle 给render和decode
	playlist video;
	playlist audio;
	playlist sub;
	m_hlsParser.getSelectTrackPlaylist(video, audio, sub);
	m_readvideothread = std::thread(VideoThreadFunc,video,this);
	m_readaudiothread = std::thread(AudioThreadFunc,audio,this);
	m_readsubthread = std::thread(SubThreadFunc,sub,this);

	
}

void HLSStrreamController::getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist)
{
	m_hlsParser.getSelectTrackPlaylist(vplist, aplist, splist);
}
