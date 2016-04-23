#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

#define CIRCULAR_BUFFER_SIZE 100

typedef enum
{
	BUFFER_READY,
	BUFFER_EMPTY
}BUFF_STATE;

typedef struct
{
	MyImage image;
	//CMutexLock mutex;
	BUFF_STATE eBuffState;
}BUFFER_STYPE;

class CVideoBuffer
{
private:
	BUFFER_STYPE m_videoBuffer[CIRCULAR_BUFFER_SIZE];
	unsigned short m_usCurrentIndex;
public:
	CVideoBuffer(int, int);
	virtual ~CVideoBuffer() {}

	void write(const MyImage&);
	MyImage& read();
};//CVideoBuffer