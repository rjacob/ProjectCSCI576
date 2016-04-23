#include "CVideoBuffer.h"

CVideoBuffer::CVideoBuffer(int _w, int _h)
:
	m_usCurrentWriteIndex(0),
	m_usCurrentReadIndex(0)
{
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		m_videoBuffer[i].image.setHeight(_h);
		m_videoBuffer[i].image.setWidth(_w);
		m_videoBuffer[i].eBuffElemState = BUFFER_ELEM_EMPTY;
	}
}//Default Constructor

CVideoBuffer::~CVideoBuffer()
{
}//Destructor

//Driven by processling loop/ thread
MyImage& CVideoBuffer::nextFrame()
{
	m_videoBuffer[m_usCurrentWriteIndex].eBuffElemState = BUFFER_ELEM_READY;
	return m_videoBuffer[m_usCurrentWriteIndex].image;
}//write

BUFFER_STYPE& CVideoBuffer::temporary(unsigned short _index)
{
	return m_videoBuffer[_index];
}

//Driven by timer in PCMPlay
//Read the one that's not being written to
MyImage* CVideoBuffer::read()
{
	MyImage* pImage = NULL;

	if(m_videoBuffer[m_usCurrentReadIndex].eBuffElemState == BUFFER_ELEM_READY)
	{
		m_videoBuffer[m_usCurrentReadIndex].eBuffElemState = BUFFER_ELEM_EMPTY;
		pImage = &m_videoBuffer[m_usCurrentReadIndex].image;
		m_usCurrentReadIndex = ++m_usCurrentReadIndex % BUFFER_SIZE;
	}
	else
	{
		//how often are we underflowing?
	}

	return pImage;
}//read