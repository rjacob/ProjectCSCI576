#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

#define NO_BUFFERS 2

class CDoubleBuffer
{
private:
	MyImage m_doubleBuffer[NO_BUFFERS];
	CMutexLock m_mutex;
	unsigned short m_usCurrentIndex;
public:
	CDoubleBuffer() : m_usCurrentIndex(0) {}//Default Constructor
	virtual ~CDoubleBuffer() {}

	void write(const MyImage&);
	MyImage read();
};//CDoubleBuffer