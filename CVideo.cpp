#include "CVideo.h"

CVideo::CVideo()
:
	m_ulNoFrames(0),
	m_unWidth(0),
	m_unHeight(0)
{
	m_videoPath[0] = 0;
}//constructor

CVideo::CVideo(char* _cstrVideoPath, int _nWidth, int _nHeight)
:
	m_ulNoFrames(0),
	m_unWidth(_nWidth),
	m_unHeight(_nHeight)
{
	// Create a valid output file pointer
	m_pFile = fopen(_cstrVideoPath, "rb");

	if (m_pFile == NULL)
	{
		fprintf(stderr, "Error Opening File for Reading");
		return;
	}
}//constructor

CVideo::~CVideo()
{
	fclose(m_pFile);
	m_pFile = 0;
}//destructor

MyImage* CVideo::getVideoFrame()
{
	//fprintf(stderr, "Usage is `Image.exe Imagefile w h`");
	return m_pCurrentFrame->ReadImage(m_pFile);
}
