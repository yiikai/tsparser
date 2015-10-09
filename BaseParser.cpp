#include "BaseParser.h"

BaseParser::BaseParser()
{
	m_audioPacketBuf = std::make_shared<std::list<PACKET>>();
	m_videoPacketBuf = std::make_shared<std::list<PACKET>>();
	m_subPacketBuf = std::make_shared<std::list<PACKET>>();
}

BaseParser::~BaseParser()
{

}

bool BaseParser::read(std::string filename, std::shared_ptr<std::vector<unsigned char>> streamdata, bool isloacl)
{
	m_islocalparser = isloacl;
	if (m_islocalparser == true)
	{
		FileOperator *fileoperate = new FileOperator();
		fileoperate->ReadFile(filename);
		m_fileoperator = fileoperate;
	}
	else
	{
		MemOperator *memoperate = new MemOperator();
		memoperate->setInputData(streamdata);
		m_fileoperator = memoperate;
	}
	return true;
}

bool BaseParser::GetPacket(TType type, PACKET& packet)
{
	switch (type)
	{
	case TYPE_VIDEO:
	{
		if (m_videoPacketBuf->empty())
		{
			return false;
		}
		std::unique_lock<std::mutex> lck(m_VideoPacketMutex);
		packet = m_videoPacketBuf->front();
		m_videoPacketBuf->pop_front();
	}break;
	case TYPE_AUDIO:
	{
		if (m_audioPacketBuf->empty())
		{
			return false;
		}
		std::unique_lock<std::mutex> lck(m_AudioPacketMutex);
		packet = m_audioPacketBuf->front();
		m_audioPacketBuf->pop_front();
	}break;
	case TYPE_SUB:
	{

	}break;
	default:break;
	}
	return true;
}