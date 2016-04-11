#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H
#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include "TsFileParser.h"

class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	void init();
	void AddDurationRemain(double duration);
	void SubDurationRemain(double duration);
	bool PutIn(PACKET& packet);
	bool PullOut(PACKET& packet);
	double GetRemainBufferInSecond();
	bool CheckBufferIsFull();
	std::shared_ptr<std::list<PACKET>> Getbuffer(){ return m_localbuffer; }
private:
	std::mutex m_localbufferMutex;
	std::condition_variable m_localbufferCV;
	std::shared_ptr<std::list<PACKET>> m_localbuffer;
	double m_buffer_in_duration_total;
	double m_buffer_in_duration_remain;
};

#endif

