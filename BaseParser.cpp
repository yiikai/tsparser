#include "BaseParser.h"

BaseParser::BaseParser()
{

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