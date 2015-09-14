#ifndef STREAM_CONTROLLER_H
#define STREAM_CONTROLLER_H
#include <memory>

enum STREAMTYPE
{
	UNKONW = -1,
	HLS = 0
};

class StreamControllerFactory
{
public:
	StreamControllerFactory(){}
	~StreamControllerFactory(){}
	
	std::shared_ptr<StreamContrller> CreateWantController(unsigned char* url,STREAMTYPE type);
};

class StreamContrller
{
public:
	StreamContrller(){}
	~StreamContrller(){}
	
	void init(){}
	

public:
	
};

class HLSStrreamController : public StreamContrller
{
public:
	HLSStrreamController();
	~HLSStrreamController();

	void init();
};
#endif