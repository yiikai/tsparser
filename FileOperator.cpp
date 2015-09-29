#include "FileOperator.h"


void MemOperator::setInputData(std::shared_ptr<std::vector<unsigned char>> bufdata)
{
	m_readdata = bufdata;
	bufend = bufdata->size();
}

std::shared_ptr<std::vector<unsigned char>> MemOperator::readBytes(int size)
{
	std::shared_ptr<std::vector<unsigned char>> readbytes = std::make_shared<std::vector<unsigned char>>();
	if (bufstart == bufend)
	{
		readbytes = nullptr;
	}
	else if (bufstart + size > bufend)
	{
		readbytes->assign(m_readdata->begin() + bufstart, m_readdata->end());
		bufstart = bufend;
	}
	else if (bufstart + size >= 0 && bufstart + size <= bufend)
	{
		readbytes->assign(m_readdata->begin() + bufstart, m_readdata->begin() + bufstart + size);
		bufstart += size;
	}
	else
	{
		readbytes = nullptr;
		std::cout << "MemOperator: readBytes is at end" << std::endl;
	}
	return readbytes;
}


std::shared_ptr<std::vector<unsigned char>> MemOperator::getAllBytes()
{
	return m_readdata;
}

FileOperator::FileOperator():IoOPerator()
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

std::shared_ptr<std::vector<unsigned char>> FileOperator::readBytesFromFile(int size)
{
	std::shared_ptr<std::vector<unsigned char>> data(new std::vector<unsigned char>(size));
	
	if (m_filecontext.filehandler)
	{
		int num = 0;
		
		num = fread((*data).data(), 1, size, m_filecontext.filehandler);
		if (num != 0 )
		{
			return data;
		}
		else
		{
			if (isLocalFileEof())
			{
				std::cout << "read local file at end and need over local parser" << std::endl;
				return nullptr;
			}

		}
	}
	return nullptr;
}

bool FileOperator::isLocalFileEof()
{
	if (m_filecontext.filehandler)
	{
		if (feof(m_filecontext.filehandler))
			return true;
	}
	return false;
}

std::shared_ptr<std::vector<unsigned char>> FileOperator::readBytes(int size)
{
	std::shared_ptr<std::vector<unsigned char>> tmpdata;
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

