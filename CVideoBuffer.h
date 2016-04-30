#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

#define BUFFER_SIZE  1100/* (480*270*3*1100) Mb RAM */
#define DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

typedef enum
{
	BUFFER_ELEM_READY,
	BUFFER_ELEM_EMPTY
}BUFF_ELEM_ESTATE;

typedef struct
{
	CMutexLock mutex;
	unsigned short unFrameId;
	MyImage image;
	BUFF_ELEM_ESTATE eBuffElemState;
}BUFFER_STYPE;

class CVideoBuffer
{
private:
	BUFFER_STYPE m_videoBuffer[BUFFER_SIZE];
	unsigned short m_usCurrentWriteIndex;
	unsigned short m_usCurrentReadIndex;

	bool m_bReady2Read;
public:
	CVideoBuffer(int, int);
	virtual ~CVideoBuffer();

	void reset();
	BUFFER_STYPE* nextFrame();
	BUFFER_STYPE* read();
};//CVideoBuffer