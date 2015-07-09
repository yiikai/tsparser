#ifndef BASE_PARSER_H
#define BASE_PARSER_H
#include "FileOperator.h"
class BaseParser
{
public:
	BaseParser();
	virtual ~BaseParser();
public:
	bool readLocalFile(std::string filename);
	virtual int Parse() = 0;
	virtual c_int64 getPTS(std::shared_ptr<std::vector<char>>& packet, int offset) = 0;
	virtual c_int64 getDTS(std::shared_ptr<std::vector<char>>& packet, int offset) = 0;
protected:
	FileOperator m_fileoperator;
};

#endif
