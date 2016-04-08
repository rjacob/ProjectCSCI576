#include "CDoubleBuffer.h"

template <class T>
CDoubleBuffer<T>::CDoubleBuffer()
:
	m_usCurrentIndex(0)
{

}//default constructor

template <class T>
CDoubleBuffer<T>::CDoubleBuffer(MyImage)
{

}

template <class T>
CDoubleBuffer<T>::~CDoubleBuffer()
{

}//destructor

template <class T>
void CDoubleBuffer<T>::write(const T&)
{
	CMutexEclusiveLock lock;
	m_doubleBuffer[m_usCurrentIndex] = T;
	m_usCurrentIndex++;
	m_usCurrentIndex %= NO_BUFFERS;
}//write

template <class T>
T CDoubleBuffer<T>::read() const
{
	CMutexEclusiveLock lock;
	unsigned short unCurrentIndex = ++m_usCurrentIndex;
	unCurrentIndex %= NO_BUFFERS;

	return m_doubleBuffer[m_usCurrentIndex];
}//read