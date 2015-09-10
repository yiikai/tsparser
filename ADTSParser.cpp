#include "ADTSParser.h"
#include <iostream>

ADTSParser::ADTSParser()
{
	memset(&m_adtshead, 0, sizeof(m_adtshead));
}

ADTSParser::~ADTSParser()
{

}

void ADTSParser::Init()
{
	
}

bool ADTSParser::Parser(std::shared_ptr<std::vector<unsigned char>> data)
{
	if (data->size() < 7)
	{
		return false;
	}
	m_adtshead.syncword = 0xfff;
	int num = 0;
	num++;
	m_adtshead.mpegversion = ((*data)[num] & 0x08) >> 3;
	m_adtshead.layer = ((*data)[num] & 0x06) >> 1;
	m_adtshead.protectionabsent = (*data)[num] & 0x01;
	num++;
	m_adtshead.profile = ((*data)[num] & 0xC0) >> 6;	
	m_adtshead.samplefrequencyindex = ((*data)[num] & 0x3C) >> 2;
	m_adtshead.privatebit = (*data)[num] & 0x02 >> 1;

	unsigned char bit;
	bit = (*data)[num] & 0x01;
	unsigned char bit2;
	num++;
	bit2 = ((*data)[num] & 0xE0) >> 5;
	bit = bit << 3;
	m_adtshead.channelconfiguration = bit | bit2;
	m_adtshead.originality = ((*data)[num] & 0x10) >> 4;
	m_adtshead.home = ((*data)[num] & 0x08) >> 3;
	m_adtshead.copyrighted = ((*data)[num] & 0x04) >> 2;
	m_adtshead.copyrightstart = ((*data)[num] & 0x02) >> 1;
	bit = 0;
	bit = (*data)[num] & 0x01;
	num++;
	bit2 = 0;
	bit2 = (*data)[num];
	num++;
	unsigned char bit3;
	bit3 = ((*data)[num] & 0xE0) >> 5;
	int value = 0;
	value = (value | bit) << 11;
	value = (value | bit2) << 3;
	value = value | bit3;
	m_adtshead.framelength = value;

	value = 0;
	bit = 0;
	bit = (*data)[num] & 0x1F;
	value = (value | bit) << 6;
	bit2 = 0;
	num++;
	bit2 = ((*data)[num] & 0xFC) >> 2;
	value = value | bit2;
	m_adtshead.bufferfullness = value;
	m_adtshead.numberofaacframe = (*data)[num] & 0x03;
	if (m_adtshead.protectionabsent != 1)
	{
		//ªÒ»°CRCdata
	}
	return true;
}

int ADTSParser::GetAdtsHeaderLength(std::shared_ptr<std::vector<unsigned char>> data)
{
	if ((*data)[0] == 0xFF && ((*data)[1] & 0xF0) == 0xF0)
	{
		if ((*data)[1] & 0x01 == 1)
		{
			return 7;  
		}
		else if ((*data)[1] & 0x01 == 0)
		{
			return 9;
		}
		else
		{
			std::cout << "crc bit juge is error" << std::endl;
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int ADTSParser::GetAACWholeDataSize()
{
	return m_adtshead.framelength;
}
