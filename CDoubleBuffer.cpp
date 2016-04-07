#include "CDoubleBuffer.h"

template <class T>
CDoubleBuffer<T>::CDoubleBuffer()
:
	m_usCurrentIndex(0)
{

}//constructor

template <class T>
CDoubleBuffer<T>::~CDoubleBuffer()
{

}//destructor

template <class T>
void CDoubleBuffer<T>::write(const T&)
{
	m_doubleBuffer[m_usCurrentIndex] = T;
	m_usCurrentIndex++;
	m_usCurrentIndex %= NO_BUFFERS;
}//write

template <class T>
T CDoubleBuffer<T>::read() const
{
	unsigned short unCurrentIndex = ++m_usCurrentIndex;
	unCurrentIndex %= NO_BUFFERS;

	return m_doubleBuffer[m_usCurrentIndex];
}//read