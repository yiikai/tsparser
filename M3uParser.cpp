#include "M3uParser.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <utility>

#define MASTERTAGNUM 2;
#define PLAYLISTTAGNUM 1;
static char *M3U8HEAD = "#EXTM3U";
//Materlist Tag
static const char* EXT_X_MEDIA = "#EXT-X-MEDIA";
static const char* EXT_X_STREAM_INF = "#EXT-X-STREAM-INF";
static const char* EXT_X_I_FRAME_STREAM_INF = "#EXT-X-I-FRAME-STREAM-INF";
///////////////

//EXT_X_MEDIA Attr
static const char* EXT_X_MEDIA_TYPE = "TYPE";
static const char* EXT_X_MEDIA_URI = "URI";
static const char* EXT_X_MEDIA_GROUP_ID = "GROUP-ID";
static const char* EXT_X_MEDIA_LANGUAGE = "LANGUAGE";
static const char* EXT_X_MEDIA_ASSOC_LANGUAGE = "ASSOC-LANGUAGE";
static const char* EXT_X_MEDIA_NAME = "NAME";
static const char* EXT_X_MEDIA_DEFAULT = "DEFAULT";
static const char* EXT_X_MEDIA_AUTOSELECT = "AUTOSELECT";
static const char* EXT_X_MEDIA_FORCED = "FORCED";

//////////////

//playlist Tag
static const char* EXT_X_TARGETDURATTION = "#EXT-X-TARGETDURATION";
static const char* EXT_X_VERSION = "#EXT-X-VERSION";
static const char* EXT_X_MEDIA_SEQUENCE = "#EXT-X-MEDIA-SEQUENCE";
static const char* EXT_X_PLAYLIST_TYPE = "#EXT-X-PLAYLIST-TYPE";
static const char* EXTINF = "#EXTINF";
static const char* EXT_X_BYTERANGE = "#EXT-X-BYTERANGE";
static const char* EXT_X_ENDLIST = "#EXT-X-ENDLIST";
//////////////

//Tagparser
//TAG TagParser::ParseEXTTag(std::string linedata, TAGID tagid)
//{
//	TAG mediatag;
//	mediatag.tagname = tagid;
//	char* buf = NULL;
//	buf = strtok((char*)(linedata.c_str()), ",");
// 	while (buf != NULL)
//	{
//		//增加的容错机制，如果获取的数据里面没有‘=’，就说明现在的这个数据是之前的值的一部分
//		if (!strstr(buf, "="))
//		{
//			mediatag.attrccontainer.back().second.append(",");
//			mediatag.attrccontainer.back().second.append(buf);
//			buf = strtok(NULL, ",");
//			continue;
//		}
//		printf("%s\n", buf);
//		std::string name;
//		std::string value;
//		ATTRID id = UNKNOW;
//		getAttrNameAndValue(buf, id, value);
//		mediatag.attrccontainer.push_back(std::make_pair(id, value));
//		buf = strtok(NULL, ",");
//	}
//	return mediatag;
//}
///////////////


//////////Matser tag parser
MasterTagParser::MasterTagParser() :TagParser()
{
	m_MTContainer = std::make_shared<std::vector<TAG>>();
}

MasterTagParser::~MasterTagParser()
{

}

void MasterTagParser::ParseTag(std::shared_ptr<std::vector<unsigned char>> linedata)
{
	TAGID id = TAG_UNKNOW;
	std::string line;
	if (memcmp(linedata->data(), EXT_X_MEDIA, strlen(EXT_X_MEDIA)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_MEDIA) + 1, linedata->end()); //+1 is for skip the : after tag name
		id = TAG_EXT_X_MEDIA;
	}
	else if (memcmp(linedata->data(), EXT_X_STREAM_INF, strlen(EXT_X_STREAM_INF)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_STREAM_INF) + 1, linedata->end()); //+1 is for skip the : after tag name
		id = TAG_EXT_X_STREAM_INF;
	}
	else if (memcmp(linedata->data(), EXT_X_I_FRAME_STREAM_INF, strlen(EXT_X_I_FRAME_STREAM_INF)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_I_FRAME_STREAM_INF) + 1, linedata->end());
		id = TAG_EXT_X_I_FRAME_STREAM_INF;
	}
	else
	{
		//非空行就是前一个tag的url
		line.assign(linedata->begin(), linedata->end());
		m_MTContainer->back().tagurl = line;
		return;
	}
	m_MTContainer->push_back(ParseEXTTag(line, id));
}

TAG MasterTagParser::ParseEXTTag(std::string linedata, TAGID tagid)
{
	TAG mediatag;
	mediatag.tagname = tagid;
	char* buf = NULL;
	buf = strtok((char*)(linedata.c_str()), ",");
	while (buf != NULL)
	{
		//增加的容错机制，如果获取的数据里面没有‘=’，就说明现在的这个数据是之前的值的一部分
		if (!strstr(buf, "="))
		{
			mediatag.attrccontainer.back().second.append(",");
			mediatag.attrccontainer.back().second.append(buf);
			buf = strtok(NULL, ",");
			continue;
		}
		printf("%s\n", buf);
		std::string name;
		std::string value;
		ATTRID id = UNKNOW;
		getAttrNameAndValue(buf, id, value);
		mediatag.attrccontainer.push_back(std::make_pair(id, value));
		buf = strtok(NULL, ",");
	}
	return mediatag;
}

std::shared_ptr<std::vector<TAG>> MasterTagParser::getTag()
{
	return m_MTContainer;
}

void MasterTagParser::getAttrNameAndValue(char* data, ATTRID& attrid, std::string& value)
{
	char* spilt = strstr(data, "=");
	std::string Attrname;
	Attrname.assign(data, spilt);
	value.assign(spilt + 1);
	std::cout << "Attr：" << Attrname << std::endl;
	std::cout << "Name :" << value << std::endl;

	if (memcmp(Attrname.c_str(), "TYPE", strlen("TYPE")) == 0)
	{
		attrid = ATTRID::TYPE;
	}
	else if (memcmp(Attrname.c_str(), "GROUP-ID", strlen("GROUP-ID")) == 0)
	{
		attrid = ATTRID::GROUPID;
	}
	else if (memcmp(Attrname.c_str(), "LANGUAGE", strlen("LANGUAGE")) == 0)
	{
		attrid = ATTRID::LANGUAGE;
	}
	else if (memcmp(Attrname.c_str(), "NAME", strlen("NAME")) == 0)
	{
		attrid = ATTRID::NAME;
	}
	else if (memcmp(Attrname.c_str(), "AUTOSELECT", strlen("AUTOSELECT")) == 0)
	{
		attrid = ATTRID::AUTOSELECT;
	}
	else if (memcmp(Attrname.c_str(), "DEFAULT", strlen("DEFAULT")) == 0)
	{
		attrid = ATTRID::DEFAULT;
	}
	else if (memcmp(Attrname.c_str(), "URI", strlen("URI")) == 0)
	{
		attrid = ATTRID::URI;
	}
	else if (memcmp(Attrname.c_str(), "PROGRAM-ID", strlen("PROGRAM-ID")) == 0)
	{
		attrid = ATTRID::PROGRAMID;
	}
	else if (memcmp(Attrname.c_str(), "BANDWIDTH", strlen("BANDWIDTH")) == 0)
	{
		attrid = ATTRID::BANDWIDTH;
	}
	else if (memcmp(Attrname.c_str(), "CODECS", strlen("CODECS")) == 0)
	{
		attrid = ATTRID::CODECS;
	}
	else if (memcmp(Attrname.c_str(), "RESOLUTION", strlen("RESOLUTION")) == 0)
	{
		attrid = ATTRID::RESOLUTION;
	}
	else if (memcmp(Attrname.c_str(), "AUDIO", strlen("AUDIO")) == 0)
	{
		attrid = ATTRID::AUDIO;
	}
	else
	{
		std::cout << "no support ATTR ID" << std::endl;
	}
}



/////////////////////////

PlaylistTagParser::PlaylistTagParser()
{
	m_PLContainer = std::make_shared<std::vector<TAG>>();
}

PlaylistTagParser::~PlaylistTagParser()
{

}

TAG PlaylistTagParser::ParseEXTTag(std::string linedata, TAGID tagid)
{
	TAG mediatag;
	mediatag.tagname = tagid;
	mediatag.tagvalue = linedata;
	return mediatag;
}

void PlaylistTagParser::ParseTag(std::shared_ptr<std::vector<unsigned char>> linedata)
{
	TAGID id = TAG_UNKNOW;
	std::string line;
	if (memcmp(linedata->data(), EXT_X_TARGETDURATTION, strlen(EXT_X_TARGETDURATTION)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_TARGETDURATTION) + 1, linedata->end()); //+1 is for skip the : after tag name
		id = TAG_EXT_X_TARGETDURATTION;
	}
	else if (memcmp(linedata->data(), EXT_X_VERSION, strlen(EXT_X_VERSION)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_VERSION) + 1, linedata->end()); //+1 is for skip the : after tag name
		id = TAG_EXT_X_VERSION;
	}
	else if (memcmp(linedata->data(), EXT_X_MEDIA_SEQUENCE, strlen(EXT_X_MEDIA_SEQUENCE)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_MEDIA_SEQUENCE) + 1, linedata->end());
		id = TAG_EXT_X_MEDIA_SEQUENCE;
	}
	else if (memcmp(linedata->data(), EXT_X_PLAYLIST_TYPE, strlen(EXT_X_PLAYLIST_TYPE)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_PLAYLIST_TYPE) + 1, linedata->end());
		id = TAG_EXT_X_PLAYLIST_TYPE;
	}
	else if (memcmp(linedata->data(), EXTINF, strlen(EXTINF)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXTINF) + 1, linedata->end());
		id = TAG_EXTINF;
	}
	else if (memcmp(linedata->data(), EXT_X_BYTERANGE, strlen(EXT_X_BYTERANGE)) == 0)
	{
		line.assign(linedata->begin() + strlen(EXT_X_BYTERANGE) + 1, linedata->end());
		id = TAG_EXT_X_BYTERANGE;
	}
	else if (memcmp(linedata->data(), EXT_X_ENDLIST, strlen(EXT_X_ENDLIST)) == 0)
	{
		id = TAG_EXT_X_ENDLIST;
	}
	else
	{
		//非空行就是前一个tag的url
		line.assign(linedata->begin(), linedata->end());
		m_PLContainer->back().tagurl = line;
		return;
	}
	m_PLContainer->push_back(ParseEXTTag(line, id));
}

std::shared_ptr<std::vector<TAG>> PlaylistTagParser::getTag()
{
	return m_PLContainer;
}


M3uParser::M3uParser(std::shared_ptr<std::vector<unsigned char>> databuf) :
m_currenttype(m3utype::UNKONOW)
, m_tagparser(NULL)
{
	m_databuf = databuf;
}

M3uParser::~M3uParser()
{

}


bool M3uParser::Parser()
{
	std::vector<unsigned char>::iterator curitr = m_databuf->begin();
	std::vector<unsigned char>::iterator enditr = m_databuf->end();
	if (!checkM3u8Fomat(curitr, enditr))
		return false;
	if (!switchTagType())
		return false;
	const char** usetagarray = NULL;
	int arraycount = 0;
	switch (m_currenttype)
	{
	case MASTER:
	{
		m_tagparser = new MasterTagParser();
	}break;
	case PLAYLIST:
	{
		m_tagparser = new PlaylistTagParser();
	}break;
	default:
	{
		std::cout << "TAG error specif error" << std::endl;
		return false;
	}break;
	}
	while (curitr != enditr)
	{
		std::shared_ptr<std::vector<unsigned char>> line;
		getLine(curitr, enditr, line);
		if (line.use_count() == 0)
		{
			continue;
		}
		else
		{
			m_tagparser->ParseTag(line);
		}
	}
	m_manifesttagcontainer = m_tagparser->getTag();

	return true;
}

bool M3uParser::switchTagType()
{
	std::string alldatainsring(m_databuf->begin(), m_databuf->end());
	int ret = alldatainsring.find("#EXT-X-STREAM-INF");
	if (ret != -1)
	{
		m_currenttype = m3utype::MASTER;
		return true;
	}
	else
	{
		ret = alldatainsring.find("#EXT-X-TARGETDURATION");
		if (ret != -1)
		{
			m_currenttype = m3utype::PLAYLIST;
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool M3uParser::checkM3u8Fomat(std::vector<unsigned char>::iterator& itrcur, std::vector<unsigned char>::iterator endcur)
{

	std::vector<unsigned char>::iterator tmpcurbegin = itrcur;
	while (itrcur != endcur && (*itrcur) != '\r' && (*itrcur) != '\n')
	{
		itrcur++;
	}
	if (itrcur == endcur)
	{
		return false;
	}
	else if (itrcur == tmpcurbegin)
	{
		return false;
	}
	else
	{
		std::shared_ptr<std::vector<unsigned char>> line;
		line = std::make_shared<std::vector<unsigned char>>(tmpcurbegin, itrcur);
		if (memcmp(M3U8HEAD, line->data(), line->size()) != 0)
		{
			std::cout << "This manifest not m3u8 format " << std::endl;
			return false;
		}
	}
	return true;
}

void M3uParser::getLine(std::vector<unsigned char>::iterator& itrcur, std::vector<unsigned char>::iterator endcur, std::shared_ptr<std::vector<unsigned char>>& line)
{
	std::vector<unsigned char>::iterator tmpcurbegin = itrcur;
	while (itrcur != endcur && (*itrcur) != '\r' && (*itrcur) != '\n')
	{
		itrcur++;
	}
	if (itrcur == endcur)
	{
		return;
	}
	else if (itrcur == tmpcurbegin)
	{
		itrcur++;
		return;
	}
	line = std::make_shared<std::vector<unsigned char>>(tmpcurbegin, itrcur);
	std::string linedata(line->begin(), line->end());
	std::cout << "LINE IS:   " << linedata << std::endl;
	itrcur++;
}