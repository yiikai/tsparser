#include "BufferManager.h"

static const int MAX_BUFFER_SIZE_IN_SECONDS = 3000;

BufferManager::BufferManager() :
m_buffer_in_duration_total(MAX_BUFFER_SIZE_IN_SECONDS)
, m_buffer_in_duration_remain(MAX_BUFFER_SIZE_IN_SECONDS)

{
	m_localbuffer = std::make_shared<std::list<PACKET>>();
}


BufferManager::~BufferManager()
{

}

void BufferManager::init()
{

}

void BufferManager::AddDurationRemain(double duration)
{
	std::unique_lock<std::mutex> Lck(m_localbufferMutex);
	m_buffer_in_duration_remain += duration;
}

void BufferManager::SubDurationRemain(double duration)
{
	std::unique_lock<std::mutex> Lck(m_localbufferMutex);
	m_buffer_in_duration_remain -= duration;
}

bool BufferManager::PutIn(PACKET& packet)
{
	std::unique_lock<std::mutex> Lck(m_localbufferMutex);
	if (GetRemainBufferInSecond() <= 0)
	{
		m_localbufferCV.wait(Lck);
	}
	m_localbuffer->push_back(packet);
	return true;
}

bool BufferManager::PullOut(PACKET& packet)
{
	std::unique_lock<std::mutex> Lck(m_localbufferMutex);
	if (!m_localbuffer->empty())
	{
		packet = m_localbuffer->front();
		m_localbuffer->pop_front();
		if (GetRemainBufferInSecond() <= MAX_BUFFER_SIZE_IN_SECONDS / 2)
		{
			m_localbufferCV.notify_one();
		}
	}
	else
	{
		//std::cout << "buffer is empty" << std::endl;
		return false;
	}
	return true;
}

double BufferManager::GetRemainBufferInSecond()
{
	if (m_localbuffer->empty())
	{
		return MAX_BUFFER_SIZE_IN_SECONDS;
	}
	else
	{
		//std::cout << "pts is " << m_localbuffer->back().pts << std::endl;
		double duration = m_localbuffer->back().pts / 90 - m_localbuffer->front().pts / 90;
		m_buffer_in_duration_remain = MAX_BUFFER_SIZE_IN_SECONDS - duration;
	}
	return m_buffer_in_duration_remain;
}

bool BufferManager::CheckBufferIsFull()
{
	if (m_buffer_in_duration_remain <= 0)
	{
		return true;
	}
	return false;
}