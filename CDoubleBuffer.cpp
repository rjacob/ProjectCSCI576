#include "CDoubleBuffer.h"

void CDoubleBuffer::write(const MyImage& _image)
{
	CMutexExlusiveLock lock(m_mutex);
	m_doubleBuffer[m_usCurrentIndex] = _image;
	m_usCurrentIndex++;
	m_usCurrentIndex %= NO_BUFFERS;
}//write

MyImage CDoubleBuffer::read()
{
	CMutexExlusiveLock lock(m_mutex);
	unsigned short unCurrentIndex = ++m_usCurrentIndex;
	unCurrentIndex %= NO_BUFFERS;

	return m_doubleBuffer[m_usCurrentIndex];
}//read