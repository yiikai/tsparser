#ifndef FILE_OPERATOR_H
#define FILE_OPERATOR_H
#include "Common.h"
#include <bitset>
#include <vector>


class IoOPerator
{
public:
	IoOPerator(){}
	virtual ~IoOPerator(){}

	virtual std::shared_ptr<std::vector<unsigned char>> readBytes(int size) = 0;
	virtual std::shared_ptr<std::vector<unsigned char>> getAllBytes() = 0;
};


class MemOperator : public IoOPerator
{
public:
	MemOperator():bufstart(0),bufend(0){}
	~MemOperator(){}

	void setInputData(std::shared_ptr<std::vector<unsigned char>> bufdata);
	std::shared_ptr<std::vector<unsigned char>> readBytes(int size) override;
	std::shared_ptr<std::vector<unsigned char>> getAllBytes();

private:
	std::shared_ptr<std::vector<unsigned char>> m_readdata;
	int bufstart;
	int bufend;
};



class FileOperator : public IoOPerator
{
public:
	FileOperator();
	~FileOperator();
	//local file operator
	void ReadFile(const std::string filepath);
	std::shared_ptr<std::vector<unsigned char>> readBytes(int size) override;
	std::shared_ptr<std::vector<unsigned char>> getAllBytes(){ return NULL; }
	
private:
	bool isLocalFileEof();
	std::shared_ptr<std::vector<unsigned char>> readBytesFromFile(int size);
private:
	FileContext m_filecontext;
};

#endif

