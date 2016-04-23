#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

#define BUFFER_SIZE 100 //480*270*3*100 = 38.88 Mb RAM
#define DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

typedef enum
{
	BUFFER_ELEM_READY,
	BUFFER_ELEM_EMPTY
}BUFF_ELEM_ESTATE;

typedef struct
{
	MyImage image;
	//CMutexLock mutex;
	BUFF_ELEM_ESTATE eBuffElemState;
}BUFFER_STYPE;

class CVideoBuffer
{
private:
	BUFFER_STYPE m_videoBuffer[BUFFER_SIZE];
	unsigned short m_usCurrentWriteIndex;
	unsigned short m_usCurrentReadIndex;
public:
	CVideoBuffer(int, int);
	virtual ~CVideoBuffer();

	MyImage& nextFrame();
	BUFFER_STYPE& temporary(unsigned short);//Replace with nextFrame or Write
	MyImage& read();
};//CVideoBuffer