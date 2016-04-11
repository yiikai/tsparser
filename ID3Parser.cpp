#include "ID3Parser.h"
#include <iostream>
ID3Parser::ID3Parser()
{
	memset(&m_id3, 0, sizeof(m_id3));
}


ID3Parser::~ID3Parser()
{

}


bool ID3Parser::parse(std::shared_ptr<std::vector<unsigned char>> data)
{
	int num = 0;
	num += 3; //skip id3 head 3 bytes of name
	num += 3;
	int id3headsize = (data->at(num) & 0x7f) * 0x200000 \
		+ (data->at(num+1) & 0x7f) * 0x4000 \
		+ (data->at(num+2) & 0x7f) * 0x80 \
		+ (data->at(num+3) & 0x7f); //total 10 bytes of id3 head
	num += 4;
	if (data->at(num) == 'P' && data->at(num+1) == 'R' && data->at(num+2) == 'I' && data->at(num+3) == 'V')
	{
		num += 4;
		int privtagsize = data->at(num) * 0x1000000 + data->at(num+1) * 0x10000 + data->at(num+2) * 0x100 + data->at(num+3);
		num += 4;
		num += 2; //skip priv falgs
		num += privtagsize - 8 + 4;
		int timestamp = data->at(num) << 24 | data->at(num + 1) << 16 | data->at(num + 2) << 8 | data->at(num + 3);
		m_id3.pts = timestamp;
		data->erase(data->begin(), data->begin() + num+4);
		std::cout << "time stamp of aac chunk start is " << timestamp << std::endl;
	}
	return true;
}

bool ID3Parser::checkID3(std::shared_ptr<std::vector<unsigned char>> data)
{
	if (data->size() < 10)
	{
		return false;
	}
	if (data->at(0) == 0x49 && data->at(1) == 0x44 && data->at(2) == 0x33)
	{
		return true;
	}
	return false;
}