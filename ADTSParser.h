#ifndef ADTS_PARSER_H
#define ADTS_PARSER_H

#include <memory>
#include <vector>

typedef struct ADTSHEADER_ST
{
	unsigned int syncword : 12;
	unsigned int samplefrequencyindex : 4;
	unsigned char mpegversion : 1;
	unsigned char layer : 2;
	unsigned char protectionabsent : 1;
	unsigned char profile : 2;
	unsigned char privatebit : 1;
	unsigned char channelconfiguration : 3;
	unsigned char originality : 1;
	unsigned char home : 1;
	unsigned char copyrighted : 1;
	unsigned char copyrightstart : 1;
	unsigned char numberofaacframe : 2;
	unsigned int framelength : 13;
	unsigned int bufferfullness : 11;
	unsigned int crc : 16;
}ADTSHEADER;

class ADTSParser
{
public:
	ADTSParser();
	~ADTSParser();

	void Init();
	bool Parser(std::shared_ptr<std::vector<unsigned char>> data);
	int GetAdtsHeaderLength(std::shared_ptr<std::vector<unsigned char>> data);
	int GetAACWholeDataSize(); //include adtsHeader
private:
	ADTSHEADER m_adtshead;
 };

#endif