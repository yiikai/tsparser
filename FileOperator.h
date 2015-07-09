#ifndef FILE_OPERATOR_H
#define FILE_OPERATOR_H
#include "Common.h"
#include <bitset>
#include <vector>


class FileOperator
{
public:
	FileOperator();
	~FileOperator();
	void ReadFile(const std::string filepath);
	std::shared_ptr<std::vector<char>> readBytes(int size);
private:
	std::shared_ptr<std::vector<char>> readBytesFromFile(int size);
private:
	FileContext m_filecontext;
};

#endif

