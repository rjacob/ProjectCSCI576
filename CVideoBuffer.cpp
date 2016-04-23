#include "CVideoBuffer.h"

CVideoBuffer::CVideoBuffer(int _w, int _h)
	: m_usCurrentIndex(0)
{
	for (int i = 0; i < CIRCULAR_BUFFER_SIZE; ++i)
	{
		m_videoBuffer[i].image.setHeight(_h);
		m_videoBuffer[i].image.setWidth(_w);
		m_videoBuffer[i].eBuffState = BUFFER_EMPTY;
	}
}//Default Constructor

//Driven by processling loop/ thread
void CVideoBuffer::write(const MyImage& _image)
{
	if (m_videoBuffer[m_usCurrentIndex].eBuffState == BUFFER_EMPTY)
	{
		m_videoBuffer[m_usCurrentIndex].image = _image;
		m_videoBuffer[m_usCurrentIndex].eBuffState = BUFFER_READY;
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
MyImage& CVideoBuffer::read()
{
	if(m_videoBuffer[m_usCurrentIndex].eBuffState == BUFFER_READY)
	{
		m_videoBuffer[m_usCurrentIndex].eBuffState = BUFFER_EMPTY;
		return m_videoBuffer[m_usCurrentIndex].image;
	}
	else
	{
		//how often are we underflowing?
	}
}//read