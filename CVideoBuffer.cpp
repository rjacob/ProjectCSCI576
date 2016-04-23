#include "CVideoBuffer.h"

CVideoBuffer::CVideoBuffer(int _w, int _h)
:
	m_usCurrentWriteIndex(0),
	m_usCurrentReadIndex(0),
	m_bRead2Read(false)
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

void CVideoBuffer::reset()
{
	m_usCurrentWriteIndex = 0;
	m_usCurrentReadIndex = 0;
	m_bRead2Read = false;
}//reset

//Driven by processling loop/ thread
MyImage* CVideoBuffer::nextFrame()
{
	MyImage* pRef = NULL;

	if (m_videoBuffer[m_usCurrentWriteIndex].eBuffElemState == BUFFER_ELEM_EMPTY)
	{
		pRef = &m_videoBuffer[m_usCurrentWriteIndex].image;
		m_videoBuffer[m_usCurrentWriteIndex].eBuffElemState = BUFFER_ELEM_READY;

		if (m_usCurrentWriteIndex == BUFFER_SIZE / 2)
			m_bRead2Read = true;

		m_usCurrentWriteIndex = ++m_usCurrentWriteIndex % BUFFER_SIZE;
	}
	else
	{
		OutputDebugString(_T("Overflow!\n"));
	}

	return pRef;
}//write

//Driven by timer in PCMPlay
//Read the one that's not being written to
MyImage* CVideoBuffer::read()
{
	MyImage* pImage = NULL;

	if(m_bRead2Read)
	{
		if(m_videoBuffer[m_usCurrentReadIndex].eBuffElemState == BUFFER_ELEM_READY)
		{
			pImage = &m_videoBuffer[m_usCurrentReadIndex].image;
			m_videoBuffer[m_usCurrentReadIndex].eBuffElemState = BUFFER_ELEM_EMPTY;//TODO: what if the copy is not done?? MUTEX
			m_usCurrentReadIndex = ++m_usCurrentReadIndex % BUFFER_SIZE;
		}
		else
		{
			OutputDebugString(_T("Underlfow!\n"));
			//how often are we underflowing?
		}
	}

	return pImage;
}//read