#ifndef DECODER_H
#define DECODER_H
#include "TsFileParser.h"
typedef enum AVCODEC_TYPE
{
	TYPE_UNKONW = -1,
	AUDIO_TYPE_AAC = 0,
	VIDEO_TYPE_H264
};

class Decoder
{
public:
	Decoder(){}
	virtual ~Decoder(){}
	virtual void init() = 0;
	virtual bool initVideoDecoder(AVCODEC_TYPE type) = 0;
	virtual bool initAudioDecoder(AVCODEC_TYPE type) = 0;

	virtual bool decodeVideo(PACKET &packet) = 0;
	virtual bool decodeAudio(PACKET &packet) = 0;
};

#endif