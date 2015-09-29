#include "HlsParser.h"
#include "HttpDownload.h"
#include <iostream>
HlsParser::HlsParser():
m_hasvideo(false)
, m_hasaudio(false)
, m_hassub(false)
{
	m_pStreamContainer = std::make_shared<std::vector<STREAMINFO>>();
}

HlsParser::~HlsParser()
{

}

void HlsParser::Parser(std::shared_ptr<std::vector<unsigned char>> pDatabuf,std::string url)
{
	m_masterurl = url;
	M3uParser m3u(pDatabuf);
	m3u.Parser();
	m3utype type = m3u.getCurrentType();
	GenerateStreamInfo(m3u.getTagContainer(),m3u);
	if (type != PLAYLIST)
	{
		GenerateSelectTrackChunkList(0, 1, 0);
	}
	
}

void HlsParser::GenerateStreamInfo(std::shared_ptr<std::vector<TAG>> tag, M3uParser& m3u)
{
	switch (m3u.getCurrentType())
	{
	case m3utype::PLAYLIST:
	{
		std::string playlisturl;
		playlisturl = m_masterurl.substr(0, m_masterurl.find_last_of('/')) + '/';
		CreateSinglePlaylist(tag, playlisturl);
	}break;
	case m3utype::MASTER:
	{
		CreateStreaminfo(tag);
	}break;
	default:break;
	}
}

void HlsParser::getSelectTrackPlaylist(playlist& vplist, playlist& aplist, playlist& splist)
{
	vplist = m_videoplaylist;
	aplist = m_audioplaylist;
	splist = m_subplaylist;
}

bool HlsParser::GenerateSelectTrackChunkList(int videoid, int audioid, int subid)
{
	
	STREAMINFO stream = (*m_pStreamContainer)[videoid];
	videotrack vt = stream.vt;
	audiotrack at = {0};
	subtrack st = {0};
	if (!vt.url.empty())
	{
		HttpDownloader dm;
		dm.init();
		std::shared_ptr<std::vector<unsigned char>> pVideobufdata;
		std::string videoplaylisturl;
		videoplaylisturl = m_masterurl.substr(0,m_masterurl.find_last_of('/')) + '/' + vt.url;
		std::cout << "video playlist url is " << videoplaylisturl << std::endl;
		dm.startDownload((unsigned char*)videoplaylisturl.c_str(), pVideobufdata);
		M3uParser playlistparser(pVideobufdata);
		playlistparser.Parser();
		std::string playlisturldir = videoplaylisturl.substr(0, videoplaylisturl.find_last_of('/')) + '/';
		m_videoplaylist = CreateSinglePlaylist(playlistparser.getTagContainer(), playlisturldir);
		m_hasvideo = true;
	}
	else
	{
		m_hasvideo = false;
		std::cout << "not get video playlist url" << std::endl;
		return false;
	}
	if (!stream.audiotracks.empty())
	{
		at = stream.audiotracks[audioid];
		if (!at.url.empty())
		{
			HttpDownloader dm;
			dm.init();
			std::shared_ptr<std::vector<unsigned char>> pAudiobufdata;
			std::string audioplaylisturl;
			audioplaylisturl = m_masterurl.substr(0, m_masterurl.find_last_of('/')) + '/' + at.url;
			std::cout << "video playlist url is " << audioplaylisturl << std::endl;
			dm.startDownload((unsigned char*)audioplaylisturl.c_str(), pAudiobufdata);
			M3uParser playlistparser(pAudiobufdata);
			playlistparser.Parser();
			std::string playlisturldir = audioplaylisturl.substr(0, audioplaylisturl.find_last_of('/')) + '/';
			m_audioplaylist = CreateSinglePlaylist(playlistparser.getTagContainer(), playlisturldir);
		}
		else
		{
			std::cout << "This audio track is muexed in video track" << std::endl;
		}
		m_hasaudio = true;
	}
	else
	{
		m_hasaudio = false;
		std::cout << "not get audio playlist url" << std::endl;
		return false;
	}
	if (!stream.subtracks.empty())
	{
		st = stream.subtracks[subid];
		if (!st.url.empty())
		{
			HttpDownloader dm;
			dm.init();
			std::shared_ptr<std::vector<unsigned char>> pSubbufdata;
			std::string subplaylisturl;
			subplaylisturl = m_masterurl.substr(0, m_masterurl.find_last_of('/')) + '/' + at.url;
			std::cout << "video playlist url is " << subplaylisturl << std::endl;
			dm.startDownload((unsigned char*)subplaylisturl.c_str(), pSubbufdata);
			M3uParser playlistparser(pSubbufdata);
			playlistparser.Parser();
			std::string playlisturldir = subplaylisturl.substr(0, subplaylisturl.find_last_of('/')) + '/';
			m_subplaylist = CreateSinglePlaylist(playlistparser.getTagContainer(), playlisturldir);
		}
		m_hassub = true;
	}
	else
	{
		std::cout << "not get sub playlist url" << std::endl;
		m_hassub = false;
		return false;
	}
	return true;
}


playlist HlsParser::CreateSinglePlaylist(std::shared_ptr<std::vector<TAG>> tag,std::string playlisturldir)
{
	playlist tmpplaylist;
	std::vector<TAG>::iterator playlisttagitr = tag->begin();
	for (; playlisttagitr != tag->end(); playlisttagitr++)
	{
		switch (playlisttagitr->tagname)
		{
		case TAG_EXT_X_TARGETDURATTION:
		{
			tmpplaylist.targetduration = atoi(playlisttagitr->tagvalue.c_str());
		}break;
		case TAG_EXT_X_MEDIA_SEQUENCE:
		{
			tmpplaylist.sequenceid = atoi(playlisttagitr->tagvalue.c_str());
		}break;
		case TAG_EXT_X_PLAYLIST_TYPE:
		{
			if (playlisttagitr->tagvalue == "VOD")
			{
				tmpplaylist.type = playlisttype::VOD;
			}
			else if (playlisttagitr->tagvalue == "LIVE")
			{
				tmpplaylist.type = playlisttype::LIVE;
			}
		}break;
		case TAG_EXTINF:
		{
			chunk tmpchunk;
			int pos = playlisttagitr->tagvalue.find(',');
			tmpchunk.chunkduration = atof(playlisttagitr->tagvalue.substr(0,pos).c_str());
			tmpplaylist.chunklist.push_back(tmpchunk);
		}break;
		case TAG_EXT_X_BYTERANGE:
		{
			int pos = playlisttagitr->tagvalue.find('@');
			tmpplaylist.chunklist.back().byterange_low = atoi(playlisttagitr->tagvalue.substr(pos + 1).c_str());
			tmpplaylist.chunklist.back().byterange_up = atoi(playlisttagitr->tagvalue.substr(0, pos).c_str()) + tmpplaylist.chunklist.back().byterange_low;
		}break;
		default:
		{

		}break;
		}
		if (!playlisttagitr->tagurl.empty())
		{
			tmpplaylist.chunklist.back().url = playlisturldir + playlisttagitr->tagurl;
		}
	}
	return tmpplaylist;
}


audiotrack HlsParser::CreateAudioTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritrstart, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend)
{
	audiotrack at;
	for (; attritrstart != attritrend; attritrstart++)
	{
		switch (attritrstart->first)
		{
		case ATTRID::GROUPID:
		{
			at.group = attritrstart->second;
		}break;
		case ATTRID::LANGUAGE:
		{
			at.language = attritrstart->second;
		}break;
		case ATTRID::NAME:
		{
			at.name = attritrstart->second;
		}break;
		case ATTRID::AUTOSELECT:
		{
			if (attritrstart->second == "YES")
			{
				at.isautoselect = true;
			}
			else
			{
				at.isautoselect = false;
			}
		}break;
		case ATTRID::DEFAULT:
		{
			if (attritrstart->second == "YES")
			{
				at.isdefault = true;
			}
			else
			{
				at.isdefault = false;
			}
		}break;
		case ATTRID::URI:
		{
			//去掉url的前后两个引号
			attritrstart->second.erase(attritrstart->second.begin(), attritrstart->second.begin() + 1);
			attritrstart->second.erase(attritrstart->second.end()-1, attritrstart->second.end());
			at.url = attritrstart->second;
		}break;
		default:break;
		}
	}
	return at;
}

subtrack HlsParser::CreateSubTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritr, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend)
{
    subtrack st;
	return st;
}

videotrack HlsParser::CreateVideoTrack(std::vector<std::pair<ATTRID, std::string>>::iterator attritr, std::vector<std::pair<ATTRID, std::string>>::iterator attritrend)
{
	videotrack si;
	for (; attritr != attritrend; attritr++)
	{
		switch (attritr->first)
		{
		case ATTRID::PROGRAMID:
		{
			si.programid = attritr->second;
		}break;
		case ATTRID::BANDWIDTH:
		{
			si.bandwidth = atoi(attritr->second.c_str());
		}break;
		case ATTRID::CODECS:
		{
			si.codecs = attritr->second;
		}break;
		case ATTRID::RESOLUTION:
		{
			si.resolution = attritr->second;
		}break;
		case ATTRID::AUDIO:
		{
			si.audiogroup = attritr->second;
		}break;
		case ATTRID::URI:
		{
			si.url = attritr->second;
		}break;
		case ATTRID::SUBTITLE :
		{
			si.subgroup = attritr->second;
		}break;
		default:break;
		}
	}
	return si;
}


void HlsParser::CreateStreaminfo(std::shared_ptr<std::vector<TAG>> tag)
{
	std::vector<audiotrack> tmpaudiomedia;
	std::vector<subtrack> tmpsubmedia;
	std::vector<videotrack> tmpvideomedia;
	std::vector<TAG>::iterator tagitr = tag->begin();
	for (; tagitr != tag->end(); tagitr++)
	{
		switch ((*tagitr).tagname)
		{
		case TAGID::TAG_EXT_X_MEDIA:
		{
			std::vector<std::pair<ATTRID, std::string>>::iterator attritr = (*tagitr).attrccontainer.begin();
			std::vector<std::pair<ATTRID, std::string>>::iterator attritrend = (*tagitr).attrccontainer.end();
			std::vector<std::pair<ATTRID, std::string>>::iterator tmpiter = attritr;
			for (; tmpiter != (*tagitr).attrccontainer.end(); tmpiter++)
			{
				if (tmpiter->first == ATTRID::TYPE)
				{
					if (tmpiter->second == "AUDIO")
					{
						tmpaudiomedia.push_back(CreateAudioTrack(attritr, attritrend));
					}
					else if (tmpiter->second == "SUBTITLES")
					{
						tmpsubmedia.push_back(CreateSubTrack(attritr, attritrend));
					}
					break;
				}
			}
		}break;
		case TAGID::TAG_EXT_X_STREAM_INF:
		{
			std::vector<std::pair<ATTRID, std::string>>::iterator attritr = (*tagitr).attrccontainer.begin();
			std::vector<std::pair<ATTRID, std::string>>::iterator attritrend = (*tagitr).attrccontainer.end();
			videotrack tmpvideo = CreateVideoTrack(attritr, attritrend);
			tmpvideo.url = tagitr->tagurl;
			tmpvideomedia.push_back(tmpvideo);
		}break;
		default:break;
		}
	}

	GenerateStreaminfo(tmpaudiomedia, tmpsubmedia, tmpvideomedia);
}

void HlsParser::GenerateStreaminfo(std::vector<audiotrack>& at_vec, std::vector<subtrack>& sb_vec, std::vector<videotrack>& vt_vec)
{
	std::vector<videotrack>::iterator vtiter = vt_vec.begin();
	for (; vtiter != vt_vec.end(); vtiter++)
	{
		STREAMINFO stream;
		stream.vt = (*vtiter);
		std::vector<audiotrack>::iterator atiter = at_vec.begin();
		for (; atiter != at_vec.end(); atiter++)
		{
			if (atiter->group == vtiter->audiogroup)
			{
				stream.audiotracks.push_back((*atiter));
			}
		}
		std::vector<subtrack>::iterator sbiter = sb_vec.begin();
		for (; sbiter != sb_vec.end(); sbiter++)
		{
			if (sbiter->group == vtiter->subgroup)
			{
				stream.subtracks.push_back((*sbiter));
			}
		}
		m_pStreamContainer->push_back(stream);
	}
}