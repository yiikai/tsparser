#ifndef BASE_PARSER_H
#define BASE_PARSER_H
#include "FileOperator.h"

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
protected:
	bool m_islocalparser;   //not local  , it must be push mode
	IoOPerator *m_fileoperator;
};

#endif
