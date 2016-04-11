#ifndef AAC_PARSER_H
#define AAC_PARSER_H
#include "BaseParser.h"
#include "ID3Parser.h"
#include "ADTSParser.h"
#include <list>
class AACParser :public BaseParser
{
public:
	AACParser();
	~AACParser();
	int Parse();
	bool SpiltADTSTrack(std::shared_ptr<std::vector<unsigned char>> data);
	void SetChunkDuration(double duration);
private:
	void CalcADTSFramePts(std::shared_ptr<std::list<PACKET>> audiopacketbuf);
private:
	ID3Parser m_id3parser;
	ADTSParser m_adtsparser;
	double m_chunkduration;
};

#endif