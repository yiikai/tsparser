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
	
	std::shared_ptr<StreamContrller> CreateWantController(unsigned char* url, STREAMTYPE type){}
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

	void start();
	void getTrackPlayList(playlist& vplist, playlist& aplist, playlist& splist);
private:
	static void VideoThreadFunc(playlist video, HLSStrreamController *controller);
	static void AudioThreadFunc(playlist audio, HLSStrreamController *controller);
	static void SubThreadFunc(playlist sub, HLSStrreamController *controller);
private:
	HlsParser m_hlsParser;
	TsFileParser m_tsParser;

	std::thread m_readvideothread;
	std::thread m_readaudiothread;
	std::thread m_readsubthread;

	std::list<PACKET> m_videopacketbuf;
	int m_videobufduration = 0;
	std::mutex m_videomutex;
	std::condition_variable m_cvV;

	std::list<PACKET> m_audiopacketbuf;
	int m_audiobufduration = 0;
	std::mutex m_audiomutex;
	std::condition_variable m_cvA;

	std::list<PACKET> m_subpacketbuf;
	int m_subbufduration = 0;
	std::mutex m_submutex;
	std::condition_variable m_cvS;
};
#endif