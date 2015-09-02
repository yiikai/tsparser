#include <memory>
#include <vector>

void readBytesTo(void* data,std::shared_ptr<std::vector<unsigned char>> datdbuf,int readsize,int& offset)
{
	int size = readsize;
	unsigned char* buf = new unsigned char[size];
	memset(buf, 0, size);
	while (size != 0)
	{
		buf[--size] = (*datdbuf)[offset];
		offset++;
	}
	memcpy(data, buf, readsize);
	delete[] buf;
}	

void readBytesTo_SP(void* data, std::shared_ptr<std::vector<unsigned char>> datdbuf, int readsize, int& offset)  //special for track head data for width and height get different bytes order
{
	int size = 0;
	unsigned char* buf = new unsigned char[readsize];
	memset(buf, 0, size);
	while (size != readsize)
	{
		buf[size++] = (*datdbuf)[offset];
		offset++;
	}
	char tmpbuf = buf[1];
	buf[1] = buf[0];
	buf[0] = tmpbuf;
	memcpy(data, buf, readsize);
	delete[] buf;
}
