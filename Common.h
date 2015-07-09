#ifndef COMMON_H
#define COMMON_H
#include <iostream>
#include <string>
#include <memory>
struct FileContext
{
	int filetype = {0};
	std::string filename;
	FILE* filehandler = nullptr;
	~FileContext()
	{
		if (filehandler)
			fclose(filehandler);
		filehandler = nullptr;
	}
};

#define MKWORD(h, l) (((h) << 8) | (l))
typedef long long int c_int64;

#define PTS_NO_VALUE 0
#define DTS_NO_VALUE 0 

const int PARSER_OK = 0;
const int PARSER_FAIL = 1;
#endif