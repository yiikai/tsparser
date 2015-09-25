#ifndef STREAM_CONTROLLER_H
#define STREAM_CONTROLLER_H

#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "HlsParser.h"
#include "TsFileParser.h"
#include "HttpDownload.h"
#include "BufferManager.h"
#include "FFmpegDecoder.h"
enum STREAMTYPE
{
	STREAMTYPE_UNKONW = -1,
	HLS = 0
};
class StreamContrller;
class StreamControllerFactory
{
public:
	StreamControllerFactory(){}
	~StreamControllerFactory(){}
	
	std::shared_ptr<StreamContrller> CreateWantController(STREAMTYPE type);
};

class StreamContrller
{
public:
	StreamContrller(){}
	~StreamContrller(){}
	
	virtual void init(unsigned char* url)
	{
		m_mainfesturl = std::string((char*)url);
		HttpDownloader dm;
		dm.init();
		dm.startDownload(url, m_manifestdownloaddata);
	}
	virtual void start() = 0;
	virtual void getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist) = 0;
private:
	void InitAVDevice();
protected:
	std::shared_ptr<std::vector<unsigned char>> m_manifestdownloaddata;   //不管什么协议都需要下载manifest的数据
	std::string m_mainfesturl;
};

class HLSStrreamController : public StreamContrller
{
public:
	HLSStrreamController();
	~HLSStrreamController();
	void init(unsigned char* url);
	void start();
	void getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist);
private:
	static void VideoThreadFunc(playlist video, HLSStrreamController *controller);
	static void AudioThreadFunc(playlist audio, HLSStrreamController *controller);
	static void SubThreadFunc(playlist sub, HLSStrreamController *controller);
	static void GetPacketFunc(TRACKTYPE type,HLSStrreamController *controller);
private:
	HlsParser m_hlsParser;
	TsFileParser m_tsParser;

	bool m_hasvideo = false;
	bool m_hasaudio = false;
	bool m_hassub = false;

	//BufferManager m_videobuffer;
	//BufferManager m_audiobuffer;

	std::thread m_readvideothread;
	std::thread m_readaudiothread;
	std::thread m_readsubthread;

	std::thread m_getvideothread;
	std::thread m_getaudiothread;
	std::thread m_getsubthread;

	std::shared_ptr<std::list<PACKET>> m_videopacketbuf;
	int m_videobufduration = 0;
	std::mutex m_videomutex;
	std::condition_variable m_cvV;

	std::shared_ptr<std::list<PACKET>> m_audiopacketbuf;
	int m_audiobufduration = 0;
	std::mutex m_audiomutex;
	std::condition_variable m_cvA;

	std::list<PACKET> m_subpacketbuf;
	int m_subbufduration = 0;
	std::mutex m_submutex;
	std::condition_variable m_cvS;

	BufferManager m_videobuffer;
	BufferManager m_audiobuffer;

	FFmpegDecoder m_decoder;
	
};
#endif