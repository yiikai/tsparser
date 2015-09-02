#ifndef HLS_PARSER_H
#define HLS_PARSER_H
#include <vector>
#include <memory>
#include <string>
#include <list>
#include "M3uParser.h"
typedef struct AUDIOTRACK_ST
{
	int audioid;
	std::string language;
	std::string name;
	std::string group;
	bool isautoselect;
	bool isdefault;
	std::string url;
}audiotrack;

typedef struct SUBTRACK_ST
{
	int subid;
	std::string language;
	std::string name;
	std::string group;
	bool isautoselect;
	bool isdefault;
	std::string url;
}subtrack;

typedef struct VIDEOTRACK_ST
{
	int streamid;
	std::string programid;
	std::string codecs;
	int bandwidth;
	std::string audiogroup;
	std::string subgroup;
	std::string url;
	std::string resolution;
}videotrack;

typedef struct STREAMINFO_ST
{
	videotrack vt;
	std::vector<audiotrack> audiotracks;
	std::vector<subtrack> subtracks;
}STREAMINFO;

typedef struct CHUNK_ST
{
	CHUNK_ST():byterange_low(0),byterange_up(0),chunkduration(0){
	}
	std::string url;
	int byterange_low;
	int byterange_up;
	float chunkduration;
}chunk;

typedef struct PLAYLIST_ST
{
	int targetduration;
	int sequenceid;
	playlisttype type;
	std::list<chunk> chunklist;
}playlist;

class HlsParser
{
public:
	HlsParser();
	~HlsParser();

	void Parser(std::shared_ptr<std::vector<unsigned char>> pDatabuf, std::string url);
	std::shared_ptr<std::vector<STREAMINFO>> GetSelectStream(){ return m_pStreamContainer; }   //目前都只是写死了选择av第一路，没有sub
	void getSelectTrackPlaylist(playlist& vplist, playlist& aplist, playlist& splist);
private:
	void GenerateStreamInfo(std::shared_ptr<std::vector<TAG>> tag , M3uParser& m3u);
	playlist CreateSinglePlaylist(std::shared_ptr<std::vector<TAG>> tag, std::string playlisturldir);
	void CreateStreaminfo(std::shared_ptr<std::vector<TAG>> tag);
	void GenerateStreaminfo(std::vector<audiotrack>& at_vec, std::vector<subtrack>& sb_vec, std::vector<videotrack>& vt_vec);
	bool GenerateSelectTrackChunkList(int videoid,int audioid,int subid);
	audiotrack CreateAudioTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritr, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend);
	subtrack CreateSubTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritr, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend);
	videotrack CreateVideoTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritr, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend);
private:
	std::shared_ptr<std::vector<STREAMINFO>> m_pStreamContainer;
	std::string m_masterurl;
	playlist m_videoplaylist;
	playlist m_audioplaylist;
	playlist m_subplaylist;
};

#endif