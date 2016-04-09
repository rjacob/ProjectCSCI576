#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

enum
{
	OUT_BUFFER = 0,
	IN_BUFFER,
	NO_BUFFERS
};

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

class CDoubleBuffer
{
private:
	BUFFER_STYPE m_doubleBuffer[NO_BUFFERS];
	unsigned short m_usCurrentIndex;

	bool swap();
public:
	CDoubleBuffer(int, int);
	virtual ~CDoubleBuffer() {}

	void write(const MyImage&);
	MyImage& read();
};//CDoubleBuffer