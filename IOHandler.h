#ifndef IOHANDLER_H
#define IOHANDLER_H
#include <memory>
#include <vector>
extern void readBytesTo(void* data, std::shared_ptr<std::vector<unsigned char>> datdbuf, int readsize, int& offset);
extern void readBytesTo_SP(void* data, std::shared_ptr<std::vector<unsigned char>> datdbuf, int readsize, int& offset);
#endif