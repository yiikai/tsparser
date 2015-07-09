#include "BaseParser.h"

BaseParser::BaseParser()
{

}

BaseParser::~BaseParser()
{

}

bool BaseParser::readLocalFile(std::string filename)
{
	m_fileoperator.ReadFile(filename);
	return true;
}