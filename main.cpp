// TSParser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TsFileParser.h"

int _tmain(int argc, _TCHAR* argv[])
{
	TsFileParser tsparser;
	std::string filepath("D:\\main2.ts");
	tsparser.readLocalFile(filepath);
	tsparser.Parse();
	return 0;
}

