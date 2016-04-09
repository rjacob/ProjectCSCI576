#include "CDoubleBuffer.h"

CDoubleBuffer::CDoubleBuffer(int _w, int _h)
	: m_usCurrentIndex(0)
{
	for (int i = 0; i < NO_BUFFERS; ++i)
	{
		m_doubleBuffer[i].image.setHeight(_h);
		m_doubleBuffer[i].image.setWidth(_w);
		m_doubleBuffer[i].eBuffState = BUFFER_EMPTY;
	}
}//Default Constructor

//Driven by processling loop/ thread
void CDoubleBuffer::write(const MyImage& _image)
{
	if (m_doubleBuffer[m_usCurrentIndex].eBuffState == BUFFER_EMPTY)
	{
		m_doubleBuffer[m_usCurrentIndex].image = _image;
		m_doubleBuffer[m_usCurrentIndex].eBuffState = BUFFER_READY;
		swap();
	}
	else
	{
		int i = 0;
		i = 1;
		//how often are we overflowing?
	}
}//write

//Driven by timer in PCMPlay
//Read the one that's not being written to
MyImage& CDoubleBuffer::read()
{
	if(m_doubleBuffer[m_usCurrentIndex].eBuffState == BUFFER_READY)
	{
		m_doubleBuffer[m_usCurrentIndex].eBuffState = BUFFER_EMPTY;
		return m_doubleBuffer[m_usCurrentIndex].image;
	}
	else
	{
		//how often are we underflowing?
	}
}//read

bool CDoubleBuffer::swap()
{
	m_usCurrentIndex++;
	m_usCurrentIndex %= NO_BUFFERS;

	return true;
}//swap