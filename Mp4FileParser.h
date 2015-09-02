/*
 * =====================================================================================
 *
 *       Filename:  Mp4FileParser.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2015/07/16 10时17分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include "BaseParser.h"
#include <list>
typedef struct FTYP_ST
{
	std::string major_brand;
	int minor_version;
	std::list<std::string> compatible_brands_list;
}FTYPST;

typedef struct MVHD_ST
{
	MVHD_ST() :ver(0), timescale(0), duration(0), rate(0), volume(0), nexttrackid(0)
	{

	}
	unsigned char ver;
	int timescale;
	int duration;
	int rate;
	int volume;
	int nexttrackid;
}MVHDST;


class Mp4FileParser;
class BaseBox
{
public:
	BaseBox(){};
	virtual ~BaseBox(){};
	virtual int handlerBox(std::shared_ptr<std::vector<unsigned char>>, Mp4FileParser* mp4file = NULL){ return PARSER_OK; }   //operate in file
	//virtual int handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>>, c_int64& offset,c_int64 fatherboxsize){ return PARSER_OK; }; //operate in memory /*The box not contain headsize and head so need minus 8 bytes*/
	virtual int handlerChildBox(std::shared_ptr<std::vector<unsigned char>>, Mp4FileParser* mp4file = NULL){ return PARSER_OK; };
	virtual int readchildboxsize(std::shared_ptr<std::vector<unsigned char>> databuf, c_int64& offset);
};

class FTYPbox : public BaseBox
{
public:
	FTYPbox();
	~FTYPbox();
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
private:
	
};

class FREEbox : public BaseBox
{
public:
	FREEbox();
	~FREEbox();
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class MDATbox : public BaseBox
{
public:
	MDATbox() :BaseBox(){}
	~MDATbox(){};
	//int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf) override;
};

class MVHDbox : public BaseBox
{
public:
	MVHDbox() :BaseBox(){}
	~MVHDbox(){};
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	//int handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>>, c_int64& offset, c_int64 fatherboxsize) override;

};

class MOOVbox : public BaseBox
{
public:
	MOOVbox();
	~MOOVbox();
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
private:
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>>, Mp4FileParser* mp4file = NULL) override;
	//int readchildboxsize(std::shared_ptr<std::vector<unsigned char>> databuf,c_int64& offset);
};

class TRAKbox : public BaseBox
{
public:
	TRAKbox():BaseBox(){};
	~TRAKbox(){};
	//int handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>>, c_int64& offset, c_int64 fatherboxsize) override;
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>>, Mp4FileParser* mp4file = NULL) override;
};

class TKHDbox : public BaseBox
{
public:
	TKHDbox(){}
	~TKHDbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	//int handlerChildBoxInMemory(std::shared_ptr<std::vector<unsigned char>>, c_int64& offset, c_int64 fatherboxsize) override;
};

class MDIAbox :public BaseBox
{
public:
	MDIAbox(){}
	~MDIAbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class MDHDbox : public BaseBox
{
public:
	MDHDbox(){}
	~MDHDbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class HDLRbox : public BaseBox
{
public:
	HDLRbox(){}
	~HDLRbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class MINFbox : public BaseBox
{
public:
	MINFbox(){}
	~MINFbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	
};

class VMHDbox : public BaseBox
{
public:
	VMHDbox(){}
	~VMHDbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class DINFbox : public BaseBox
{
public:
	DINFbox(){}
	~DINFbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STBLbox : public BaseBox
{
public:
	STBLbox(){}
	~STBLbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STSDbox : public BaseBox
{
public:
	STSDbox(){}
	~STSDbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
	int handlerChildBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STTSbox : public BaseBox
{
public:
	STTSbox(){}
	~STTSbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STSSbox : public BaseBox
{
public:
	STSSbox(){}
	~STSSbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STSCbox : public BaseBox
{
public:
	STSCbox(){}
	~STSCbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STCObox : public BaseBox
{
public:
	STCObox(){}
	~STCObox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

class STSZbox : public BaseBox
{
public:
	STSZbox(){}
	~STSZbox(){}
	int handlerBox(std::shared_ptr<std::vector<unsigned char>> databuf, Mp4FileParser* mp4file = NULL) override;
};

typedef struct TRACKHEAD_ST
{
	int track_id;
	int duration;
	int layer;
	int altergroup;
	int volume;
	int width;
	int height;
}TRACKHEADST;

typedef struct TRACK_ST
{
	std::shared_ptr<TRACKHEADST> trackhead;
	std::shared_ptr<std::vector<std::pair<c_int64, c_int64>>> TStable;
}TRACKST;

typedef struct PROGRAM_ST
{
	std::shared_ptr<MVHDST> mvhdboxdata;  //every program head info
	std::shared_ptr<std::vector<std::shared_ptr<TRACKST>>> trackboxdata; //track every style type
}PROGRAMST;

class Mp4FileParser : public BaseParser
{
	friend class MOOVbox;
	friend class MVHDbox;
	friend class TKHDbox;
	friend class TRAKbox;
public:
	Mp4FileParser();
	~Mp4FileParser();
	int Parse() override;
private:
	std::shared_ptr<std::vector<unsigned char>> readBox();
	std::shared_ptr<BaseBox> BoxTypeCheck(std::shared_ptr<std::vector<unsigned char>> boxbuf);
private:
	std::shared_ptr<std::vector<PROGRAMST>> m_programetable;
	std::shared_ptr<PROGRAMST> m_programst;  //record current program struct
	std::shared_ptr<TRACKST> m_trackst;
	std::shared_ptr<std::vector<std::shared_ptr<TRACKST>>> m_alltrackdata;
};
