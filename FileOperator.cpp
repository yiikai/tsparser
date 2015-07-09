#include "FileOperator.h"


FileOperator::FileOperator()
{

}

FileOperator::~FileOperator()
{

}

void FileOperator::ReadFile(const std::string filepath)
{
	FILE *file = fopen(filepath.c_str(), "rb");
	if (!file)
	{
		std::cout << "openfile error" << std::endl;
		return;
	}
	m_filecontext.filename = filepath;
	m_filecontext.filehandler = file;
}

std::shared_ptr<std::vector<char>> FileOperator::readBytesFromFile(int size)
{
	std::shared_ptr<std::vector<char>> data(new std::vector<char>(size));
	
	if (m_filecontext.filehandler)
	{
		int num = 0;
		
		num = fread((*data).data(), 1, size, m_filecontext.filehandler);
		if (num == size)
		{
			return data;
		}
	}
	return nullptr;
}

std::shared_ptr<std::vector<char>> FileOperator::readBytes(int size)
{
	std::shared_ptr<std::vector<char>> tmpdata;
	tmpdata = readBytesFromFile(size);
		
	/*switch (size)
	{
	case 1:
	{
	tmpdata = readBytesFromFile(1);

	}break;
	case 2:
	{
	tmpdata = readBytesFromFile(2);


	}break;
	case 3:
	{
	tmpdata = readBytesFromFile(3);

	}break;
	default:break;

	}*/
	return tmpdata;
}