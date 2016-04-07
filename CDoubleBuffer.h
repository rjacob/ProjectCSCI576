#pragma once
#include "CMutexExclusiveLock.h"

#define NO_BUFFERS 2

template <class T>
class CDoubleBuffer
{
private:
	T m_doubleBuffer[NO_BUFFERS];

	unsigned short m_usCurrentIndex;
public:
	CDoubleBuffer();
	virtual ~CDoubleBuffer();

	void write(const T&);
	T read() const;
};//CDoubleBuffer