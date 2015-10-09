#ifndef BASE_PARSER_H
#define BASE_PARSER_H
#include "FileOperator.h"
#include <list>
#include <memory>
#include <mutex>


typedef struct PACKET_ST
{
	double pts = 0;
	std::shared_ptr<std::vector<unsigned char>> data;
	c_int64 size = 0;
	c_int64 duration = 0;
}PACKET;


typedef enum TRACKTYPE
{
	TYPE_VIDEO = 0,
	TYPE_AUDIO,
	TYPE_SUB
}TType;

class BaseParser
{
public:
	BaseParser();
	virtual ~BaseParser();
public:
	bool read(std::string filename, std::shared_ptr<std::vector<unsigned char>> streamdata, bool isloacl);
	virtual int Parse() = 0;
	virtual c_int64 getPTS(std::shared_ptr<std::vector<unsigned char>>& packet, int offset){ return 0; }
	virtual c_int64 getDTS(std::shared_ptr<std::vector<unsigned char>>& packet, int offset){ return 0; }
	virtual bool GetPacket(TType type, PACKET& packet);
protected:
	bool m_islocalparser;   //not local  , it must be push mode
	IoOPerator *m_fileoperator;
	std::shared_ptr<std::list<PACKET>> m_videoPacketBuf;
	std::shared_ptr<std::list<PACKET>> m_audioPacketBuf;
	std::shared_ptr<std::list<PACKET>> m_subPacketBuf;

	std::mutex m_AudioPacketMutex;
	std::mutex m_VideoPacketMutex;
};

#endif
