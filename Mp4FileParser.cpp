/*
 * =====================================================================================
 *
 *       Filename:  Mp4FileParser.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015/07/16 10时18分00秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "Mp4FileParser.h"
#include "IOHandler.h"

const int BOXTYPE_FTYP = 1;


int BaseBox::readchildboxsize(std::shared_ptr<std::vector<unsigned char>> databuf, c_int64& offset)
{
	c_int64 size = 0;
	unsigned char bufsize[4] = { 0 };
	bufsize[3] = (*databuf)[offset++];
	bufsize[2] = (*databuf)[offset++];
	bufsize[1] = (*databuf)[offset++];
	bufsize[0] = (*databuf)[offset++];
	memcpy(&size, bufsize, 4);
	/*std::vector<char>
	::iterator itr, enditr;
	itr = databuf->begin();
	enditr = databuf->begin() + 4;
	databuf->erase(itr, enditr);*/
	return size;
}

FTYPbox::FTYPbox() :BaseBox()
{

}

FTYPbox::~FTYPbox()
{

}

int FTYPbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	std::cout << "FTYP Handler" << std::endl;
	FTYPST ftyp_st;
	int offset = 0;
	char buf[5] = { 0 };
	buf[0] = (*databuf)[offset];
	offset++;
	buf[1] = (*databuf)[offset];
	offset++;
	buf[2] = (*databuf)[offset];
	offset++;
	buf[3] = (*databuf)[offset];
	ftyp_st.major_brand = buf;

	offset++;
	char minor[5] = { 0 };
	minor[3] = (*databuf)[offset];
	offset++;
	minor[2] = (*databuf)[offset];
	offset++;
	minor[1] = (*databuf)[offset];
	offset++;
	minor[0] = (*databuf)[offset];
	memcpy(&(ftyp_st.minor_version), minor, 4);

	offset++;
	while (offset < databuf->size())
	{
		char buf[5] = { 0 };
		buf[0] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[3] = (*databuf)[offset];
		std::string compaatiblebrand(buf);
		ftyp_st.compatible_brands_list.push_back(compaatiblebrand);
		offset++;
	}
	return PARSER_OK;
}

FREEbox::FREEbox() :BaseBox()
{

}
FREEbox::~FREEbox()
{

}
int FREEbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	std::cout << "Free box hadnler do nothing and skip it " << std::endl;
	return PARSER_OK;
}


MOOVbox::MOOVbox()
{

}

MOOVbox::~MOOVbox()
{

}

int MOOVbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	
	handlerChildBox(databuf, mp4file);
	return PARSER_OK;
}

int MOOVbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 totalcount = databuf->size();
	c_int64 offset = 0;
	while (offset < totalcount)
	{
		c_int64 childboxsize = readchildboxsize(databuf, offset);
		c_int64 childboxdatasize = childboxsize - 4;  //box size is minus head size
		std::shared_ptr<BaseBox> box;
		if ((*databuf)[offset] == 'm' && (*databuf)[offset+1] == 'v' && (*databuf)[offset+2] == 'h' && (*databuf)[offset+3] == 'd')
		{
			box =std::make_shared<MVHDbox>();
		}
		else if ((*databuf)[offset] == 't' && (*databuf)[offset+1] == 'r' && (*databuf)[offset+2] == 'a' && (*databuf)[offset+3] == 'k')
		{
			box = std::make_shared<TRAKbox>();
		}
		else
		{
			std::cout << "what fuck" << std::endl;
		}
		offset += 4;
		childboxdatasize -= 4;   //box size also need minus 4 for head
		std::vector<unsigned char>::iterator itr,itrend;
		itr = databuf->begin() + offset;
		itrend = itr + childboxdatasize;
		std::shared_ptr<std::vector<unsigned char>> currentboxdatabuf = std::make_shared<std::vector<unsigned char>>(itr,itrend);
		box->handlerBox(currentboxdatabuf, mp4file);
		//box->handlerChildBoxInMemory(databuf, offset, childboxdatasize);
		offset += childboxdatasize;
	}
	return PARSER_OK;
}

int MVHDbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	mp4file->m_programst = std::make_shared<PROGRAMST>();  //when start of mvhd box need create a new program
	mp4file->m_alltrackdata = std::make_shared<std::vector<std::shared_ptr<TRACKST>>>();
	std::shared_ptr<MVHDST> mvhdst = std::make_shared<MVHDST>();
	int offset = 0;
	int totalcount = databuf->size();
	while (offset < totalcount)
	{
		mvhdst->ver = (*databuf)[offset];
		offset += 12; //skip ver,flag,create,modify byte
		unsigned char buf[4] = { 0 };
		buf[3] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[0] = (*databuf)[offset];
		memcpy(&(mvhdst->timescale), buf, 4);
		offset++;

		buf[3] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[0] = (*databuf)[offset];
		memcpy(&(mvhdst->duration), buf, 4);
		offset++;

		buf[3] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[0] = (*databuf)[offset];
		memcpy(&(mvhdst->rate), buf, 4);
		offset++;

		unsigned char buf2[2] = { 0 };
		buf2[1] = (*databuf)[offset];
		offset++;
		buf2[0] = (*databuf)[offset];
		offset++;
		memcpy(&(mvhdst->volume), buf2, 2);

		offset += 70;
		buf[3] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[0] = (*databuf)[offset];
		memcpy(&(mvhdst->nexttrackid), buf, 4);
		offset++;
	}
	mp4file->m_programst->mvhdboxdata = mvhdst;
	return PARSER_OK;
}

int TRAKbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	mp4file->m_trackst = std::make_shared<TRACKST>();
	handlerChildBox(databuf,mp4file);
	return PARSER_OK;
}
int TRAKbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 databufsize = databuf->size();
	c_int64 offset = 0;
	while (offset < databufsize)
	{
		std::shared_ptr<BaseBox> box;
		c_int64 childboxsize = readchildboxsize(databuf, offset);
		c_int64 childboxdatasize = childboxsize - 4;
		if((*databuf)[offset] == 't' && (*databuf)[offset + 1] == 'k' && (*databuf)[offset + 2] == 'h' && (*databuf)[offset + 3] == 'd')
		{
			box = std::make_shared<TKHDbox>();
			mp4file->m_alltrackdata->push_back(mp4file->m_trackst);
			
		}
		else if ((*databuf)[offset] == 'm' && (*databuf)[offset + 1] == 'd' && (*databuf)[offset + 2] == 'i' && (*databuf)[offset + 3] == 'a')
		{
			box = std::make_shared<MDIAbox>();
		}
		else
		{

		}
		offset += 4;
		childboxdatasize -= 4;
		std::vector<unsigned char>::iterator itr, itrend;
		itr = databuf->begin() + offset;
		itrend = itr + childboxdatasize;
		std::shared_ptr<std::vector<unsigned char>> currentboxdatabuf = std::make_shared<std::vector<unsigned char>>(itr, itrend);
		box->handlerBox(currentboxdatabuf, mp4file);
		offset += childboxdatasize;
	}
	return PARSER_OK;
}

int TKHDbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	std::shared_ptr<TRACKHEADST> trackH = std::make_shared<TRACKHEADST>();
	c_int64 totalsize = databuf->size();
	int offset = 0;
	while (offset < totalsize)
	{
		offset += 12; //skip version creattime modifytime
		readBytesTo(&(trackH->track_id), databuf, 4, offset);
		offset += 4; //skip reserved
		readBytesTo(&(trackH->duration), databuf, 4, offset);
		offset += 8; //skip reserved
		readBytesTo(&(trackH->layer), databuf, 2, offset);
		readBytesTo(&(trackH->altergroup), databuf, 2, offset);
		readBytesTo(&(trackH->volume), databuf, 2, offset);
		offset += 2;  //skip reserved
		offset += 36; //skip martix
		readBytesTo_SP(&(trackH->width), databuf, 4, offset);
		readBytesTo_SP(&(trackH->height), databuf, 4, offset);
	}
	mp4file->m_trackst->trackhead = trackH;
	return PARSER_OK;
}

int TKHDbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int MDIAbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	handlerChildBox(databuf);
	return PARSER_OK;
}

int MDIAbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 databufsize = databuf->size();
	c_int64 offset = 0;
	while (offset < databufsize)
	{
		std::shared_ptr<BaseBox> box;
		c_int64 childboxsize = readchildboxsize(databuf, offset);
		c_int64 childboxdatasize = childboxsize - 4;
		if ((*databuf)[offset] == 'm' && (*databuf)[offset + 1] == 'd' && (*databuf)[offset + 2] == 'h' && (*databuf)[offset + 3] == 'd')
		{
			box = std::make_shared<MDHDbox>();
		}
		else if ((*databuf)[offset] == 'h' && (*databuf)[offset + 1] == 'd' && (*databuf)[offset + 2] == 'l' && (*databuf)[offset + 3] == 'r')
		{
			box = std::make_shared<HDLRbox>();
		}
		else if ((*databuf)[offset] == 'm' && (*databuf)[offset + 1] == 'i' && (*databuf)[offset + 2] == 'n' && (*databuf)[offset + 3] == 'f')
		{
			box = std::make_shared<MINFbox>();
		}
		else
		{

		}
		offset += 4;
		childboxdatasize -= 4;
		std::vector<unsigned char>::iterator itr, itrend;
		itr = databuf->begin() + offset;
		itrend = itr + childboxdatasize;
		std::shared_ptr<std::vector<unsigned char>> currentboxdatabuf = std::make_shared<std::vector<unsigned char>>(itr, itrend);
		box->handlerBox(currentboxdatabuf);
		offset += childboxdatasize;
	}
	return PARSER_OK;
}

int MDHDbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int HDLRbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int MINFbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	handlerChildBox(databuf);
	return PARSER_OK;
}

int MINFbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 databufsize = databuf->size();
	c_int64 offset = 0;
	while (offset < databufsize)
	{
		std::shared_ptr<BaseBox> box;
		c_int64 childboxsize = readchildboxsize(databuf, offset);
		c_int64 childboxdatasize = childboxsize - 4;
		if ((*databuf)[offset] == 'v' && (*databuf)[offset + 1] == 'm' && (*databuf)[offset + 2] == 'h' && (*databuf)[offset + 3] == 'd')
		{
			box = std::make_shared<VMHDbox>();
		}
		else if ((*databuf)[offset] == 'd' && (*databuf)[offset + 1] == 'i' && (*databuf)[offset + 2] == 'n' && (*databuf)[offset + 3] == 'f')
		{
			box = std::make_shared<DINFbox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 'b' && (*databuf)[offset + 3] == 'l')
		{
			box = std::make_shared<STBLbox>();
		}
		else
		{

		}
		offset += 4;
		childboxdatasize -= 4;
		std::vector<unsigned char>::iterator itr, itrend;
		itr = databuf->begin() + offset;
		itrend = itr + childboxdatasize;
		std::shared_ptr<std::vector<unsigned char>> currentboxdatabuf = std::make_shared<std::vector<unsigned char>>(itr, itrend);
		box->handlerBox(currentboxdatabuf);
		offset += childboxdatasize;
	}
	return PARSER_OK;
}

int VMHDbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int DINFbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	handlerChildBox(databuf);
	return PARSER_OK;
}

int DINFbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	return PARSER_OK;
}

int STBLbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	handlerChildBox(databuf);
	return PARSER_OK;
}

int STBLbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 databufsize = databuf->size();
	c_int64 offset = 0;
	while (offset < databufsize)
	{
		std::shared_ptr<BaseBox> box;
		c_int64 childboxsize = readchildboxsize(databuf, offset);
		c_int64 childboxdatasize = childboxsize - 4;
		if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 's' && (*databuf)[offset + 3] == 'd')
		{
			box = std::make_shared<STSDbox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 't' && (*databuf)[offset + 3] == 's')
		{
			box = std::make_shared<STTSbox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 's' && (*databuf)[offset + 3] == 's')
		{
			box = std::make_shared<STSSbox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 's' && (*databuf)[offset + 3] == 'c')
		{
			box = std::make_shared<STSCbox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 'c' && (*databuf)[offset + 3] == 'o')
		{
			box = std::make_shared<STCObox>();
		}
		else if ((*databuf)[offset] == 's' && (*databuf)[offset + 1] == 't' && (*databuf)[offset + 2] == 's' && (*databuf)[offset + 3] == 'z')
		{
			box = std::make_shared<STSZbox>();
		}
		else
		{

		}
		offset += 4;
		childboxdatasize -= 4;
		std::vector<unsigned char>::iterator itr, itrend;
		itr = databuf->begin() + offset;
		itrend = itr + childboxdatasize;
		std::shared_ptr<std::vector<unsigned char>> currentboxdatabuf = std::make_shared<std::vector<unsigned char>>(itr, itrend);
		box->handlerBox(currentboxdatabuf);
		offset += childboxdatasize;
	}
	return PARSER_OK;
}

int STSDbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	handlerChildBox(databuf);
	return PARSER_OK;
}

int STSDbox::handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int STTSbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	c_int64 totatsize = databuf->size();
	int offset = 0;
	std::shared_ptr<std::vector<std::pair<c_int64, c_int64>>> STTStable = std::make_shared<std::vector<std::pair<c_int64, c_int64>>>();
	while (offset < totatsize)
	{
		offset += 4; //skip version and flags
		int entry_count = 0;
		unsigned char buf[4] = { 0 };
		buf[3] = (*databuf)[offset];
		offset++;
		buf[2] = (*databuf)[offset];
		offset++;
		buf[1] = (*databuf)[offset];
		offset++;
		buf[0] = (*databuf)[offset];
		offset++;
		memcpy(&entry_count, buf, 4);
		for (int i = 0; i < entry_count; i++)
		{
			int samplecount = 0;
			readBytesTo(&samplecount, databuf, 4, offset);
			int sample_delta = 0;
			readBytesTo(&sample_delta, databuf, 4, offset);
			STTStable->push_back(std::make_pair(samplecount, sample_delta));
		}
	}

	return PARSER_OK;
}

int STSSbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file)
{
	return PARSER_OK;
}

int STSCbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	return PARSER_OK;
}

int STCObox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	return PARSER_OK;
}

int STSZbox::handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file )
{
	return PARSER_OK;
}

//int MVHDbox::handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>> databuf, c_int64& offset,c_int64 fatherboxsize)
//{
//	std::cout << "MVHD box " << std::endl;
//	if (1)
//		std::cout << "fuck" << std::endl;
//	return PARSER_OK;
//}

//int MOOVbox::readchildboxsize(std::shared_ptr<std::vector<unsigned char>> databuf,c_int64& offset)
//{
//	c_int64 size = 0;
//	unsigned char bufsize[4] = { 0 };
//	bufsize[3] = (*databuf)[offset++];
//	bufsize[2] = (*databuf)[offset++];
//	bufsize[1] = (*databuf)[offset++];
//	bufsize[0] = (*databuf)[offset++];
//	memcpy(&size,bufsize,4);
//	/*std::vector<char>::iterator itr, enditr;
//	itr = databuf->begin();
//	enditr = databuf->begin() + 4;
//	databuf->erase(itr, enditr);*/
//	return size;
//}


//int TRAKbox::handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>> databuf, c_int64& offset,c_int64 fatherboxsize)
//{
//	std::cout << "TRAK Handler" << std::endl;
//	c_int64 childboxsize = readchildboxsize(databuf,offset);
//	std::shared_ptr<BaseBox> box;
//	if ((*databuf)[offset] == 't' && (*databuf)[offset + 1] == 'k' && (*databuf)[offset + 2] == 'h' && (*databuf)[offset + 3] == 'd')
//	{
//		box = std::make_shared<TKHDbox>();
//	}
//	else
//	{
//
//	}
//	offset += 4;
//	box->handlerChildBoxInMemory(databuf, offset, fatherboxsize);
//	return PARSER_OK;
//}

//int TKHDbox::handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>>, c_int64& offset, c_int64 fatherboxsize)
//{
//	return PARSER_OK;
//}

Mp4FileParser::Mp4FileParser():BaseParser()							
{	
	m_programetable = std::make_shared<std::vector<PROGRAMST>>();
}

Mp4FileParser::~Mp4FileParser()
{

}

int Mp4FileParser::Parse()
{
	while(1)
	{
		std::shared_ptr<std::vector<unsigned char>> boxbuf;
		boxbuf = readBox();
		std::shared_ptr<BaseBox> boxhandler;
		boxhandler = BoxTypeCheck(boxbuf);
		boxhandler->handlerBox(boxbuf,this);
	}
	return PARSER_OK;
}

std::shared_ptr<BaseBox> Mp4FileParser::BoxTypeCheck(std::shared_ptr<std::vector<unsigned char>> boxbuf)
{
	std::shared_ptr<BaseBox> box;
	if ((*boxbuf)[0] == 'f' && (*boxbuf)[1] == 't' && (*boxbuf)[2] == 'y' && (*boxbuf)[3] == 'p')
	{
		std::cout << "FTYP box --The Mp4 file first box " << std::endl;
		box = std::make_shared<FTYPbox>();
	}
	else if ((*boxbuf)[0] == 'f' && (*boxbuf)[1] == 'r' && (*boxbuf)[2] == 'e' && (*boxbuf)[3] == 'e')
	{
		std::cout << "FREE box So skip it" << std::endl;
		box = std::make_shared<FREEbox>();
	}
	else if ((*boxbuf)[0] = 'm' && (*boxbuf)[1] == 'd' && (*boxbuf)[2] == 'a' && (*boxbuf)[3] == 't')
	{
		std::cout << "MDAT box so skip it now Due to this box is Truly track data " << std::endl;
		box = std::make_shared<MDATbox>();
	}
	else if ((*boxbuf)[0] = 'm' && (*boxbuf)[1] == 'o' && (*boxbuf)[2] == 'o' && (*boxbuf)[3] == 'v')
	{
		std::cout << "MOOV box " << std::endl;
		box = std::make_shared<MOOVbox>();
	}
	else
	{
		return NULL;
	}
	std::vector<unsigned char>::iterator itr, enditr;
	itr = boxbuf->begin();
	enditr = boxbuf->begin() + 4;
	boxbuf->erase(itr, enditr);
	return box;
}

std::shared_ptr<std::vector<unsigned char>> Mp4FileParser::readBox()
{	
	//read box header , size and type
	std::shared_ptr< std::vector<unsigned char> > boxsizebuf = m_fileoperator->readBytes(4);
	unsigned char buf[4] = {0};
	buf[0] = (*boxsizebuf)[3];
	buf[1] = (*boxsizebuf)[2];
	buf[2] = (*boxsizebuf)[1];
	buf[3] = (*boxsizebuf)[0];
	c_int64 boxsize = 0;
	memcpy(&boxsize,buf,4);
	std::shared_ptr<std::vector<unsigned char>> boxdatabuf = m_fileoperator->readBytes(boxsize - 4);
	return boxdatabuf;
}
