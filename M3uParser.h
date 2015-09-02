//This M3u8 Version is V16 but current not support all

#ifndef M3U_PARSER_H
#define M3U_PARSER_H

#include <memory>
#include <vector>

typedef enum
{
	TAG_UNKNOW = -1,
	TAG_EXT_X_MEDIA,
	TAG_EXT_X_STREAM_INF,
	TAG_EXT_X_I_FRAME_STREAM_INF,
	TAG_EXT_X_TARGETDURATTION,
	TAG_EXT_X_VERSION,
	TAG_EXT_X_MEDIA_SEQUENCE,
	TAG_EXT_X_PLAYLIST_TYPE,
	TAG_EXTINF,
	TAG_EXT_X_BYTERANGE,
	TAG_EXT_X_ENDLIST

}TAGID;

typedef enum 
{
	UNKNOW = -1,
	TYPE = 0,
	GROUPID,
	LANGUAGE,
	NAME,
	AUTOSELECT,
	DEFAULT,
	URI,
	PROGRAMID,
	BANDWIDTH,
	CODECS,
	RESOLUTION,
	AUDIO,
	SUBTITLE
}ATTRID;

typedef enum 
{
	UNKONW = -1,
	VOD = 0,
	LIVE
}playlisttype;

typedef struct ATTR_ST
{
	ATTRID attid;
	std::string attrvalue;
}ATTR;

typedef struct TAG_ST
{
	TAGID tagname;
	std::string tagvalue;
	std::vector<std::pair<ATTRID,std::string>> attrccontainer;
	std::string tagurl;  //m3u8文件中单独的非空行
}TAG;

typedef enum m3utype
{
	UNKONOW = -1,
	MASTER = 0,
	PLAYLIST
};

class TagParser
{
public:
	TagParser(){};
	virtual ~TagParser(){};
	virtual void ParseTag(std::shared_ptr<std::vector<unsigned char>> linedata) = 0;
	virtual TAG ParseEXTTag(std::string linedata, TAGID tagid) = 0;
	virtual void getAttrNameAndValue(char* data, ATTRID& attrid, std::string& value) = 0;
	virtual std::shared_ptr<std::vector<TAG>> getTag() = 0;

};

class MasterTagParser : public TagParser
{
public:
	MasterTagParser();
	~MasterTagParser();

	void ParseTag(std::shared_ptr<std::vector<unsigned char>> linedata);
	TAG ParseEXTTag(std::string linedata, TAGID tagid);
	std::shared_ptr<std::vector<TAG>> getTag();
private:
	void getAttrNameAndValue(char* data, ATTRID& attrid, std::string& value);
private:
	std::shared_ptr<std::vector<TAG>> m_MTContainer;
};

class PlaylistTagParser : public TagParser
{
public:
	PlaylistTagParser();
	~PlaylistTagParser();

	void ParseTag(std::shared_ptr<std::vector<unsigned char>> linedata);
	TAG ParseEXTTag(std::string linedata, TAGID tagid);
	std::shared_ptr<std::vector<TAG>> getTag();
private:
	void getAttrNameAndValue(char* data, ATTRID& attrid, std::string& value){}
private:
	std::shared_ptr<std::vector<TAG>> m_PLContainer;
};

class M3uParser
{
public:
	M3uParser(std::shared_ptr<std::vector<unsigned char>> databuf);
	~M3uParser();
	bool Parser();
	m3utype getCurrentType(){ return m_currenttype; }
	std::shared_ptr<std::vector<TAG>> getTagContainer(){ return m_manifesttagcontainer; }
private:
	
private:
	//void TagParser(std::shared_ptr<std::vector<unsigned char>> linedata);
	void getLine(std::vector<unsigned char>::iterator& itrcur, std::vector<unsigned char>::iterator endcur, std::shared_ptr<std::vector<unsigned char>>& line);
	bool checkM3u8Fomat(std::vector<unsigned char>::iterator& itrcur, std::vector<unsigned char>::iterator endcur);
	bool switchTagType();
	
	

private:
	std::shared_ptr<std::vector<TAG>> m_manifesttagcontainer;
	m3utype m_currenttype;
	std::shared_ptr<std::vector<unsigned char>> m_databuf;

	TagParser *m_tagparser;


};
#endif