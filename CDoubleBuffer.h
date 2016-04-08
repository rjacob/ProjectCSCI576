#pragma once
#include "CMutexExclusiveLock.h"
#include "Image.h"

#define NO_BUFFERS 2

template <class T>
class CDoubleBuffer
{
private:
	T m_doubleBuffer[NO_BUFFERS];

	unsigned short m_usCurrentIndex;
public:
	CDoubleBuffer();//Default Constructor
	CDoubleBuffer(MyImage);//Constructor
	virtual ~CDoubleBuffer();

	void write(const T&);
	T read() const;
};//CDoubleBuffer