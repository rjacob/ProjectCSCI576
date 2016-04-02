#include "CVideo.h"

/*************************************
* Function: Constructor
* Author: Roeil Jacob
* Description: 
*************************************/
CVideo::CVideo()
:
	m_ulNoFrames(0),
	m_unWidth(0),
	m_unHeight(0),
	m_eThreadState(THREAD_STATE_UNKNOWN)
{
	m_videoPath[0] = 0;
}//constructor

/*************************************
* Function: Constructor
* Author: Roeil Jacob
* Description:
*************************************/
CVideo::CVideo(char* _cstrVideoPath, int _nWidth, int _nHeight)
:
	m_ulNoFrames(0),
	m_unWidth(_nWidth),
	m_unHeight(_nHeight),
	m_eThreadState(THREAD_STATE_UNKNOWN)
{
	strncpy(m_videoPath, _cstrVideoPath, strlen(_cstrVideoPath));
	// Create a valid output file pointer
	m_pFile = fopen(_cstrVideoPath, "rb");

	if (m_pFile == NULL)
	{
		fprintf(stderr, "Error Opening File for Reading");
		return;
	}

	if (m_unWidth < 0 || m_unHeight < 0)
	{
		fprintf(stderr, "Image or Image properties not defined");
		return;
	}

	m_pCurrentFrame = new MyImage();
	m_pCurrentFrame->setWidth(m_unWidth);
	m_pCurrentFrame->setHeight(m_unHeight);
}//constructor

/*************************************
* Function:
* Author: Roeil Jacob
* Description:
*************************************/
CVideo::~CVideo()
{
	fclose(m_pFile);
	m_pFile = 0;
}//destructor

/*************************************
* Function:
* Author: Roeil Jacob
* Description:
*************************************/
void CVideo::threadProcessingLoop()
{
	m_eThreadState = THREAD_STATE_ALIVE;

	do
	{
		copyVideoFrame(*m_pOutputBuffer);
		Sleep(1000 / 30);//30Hz
	} while (m_eThreadState == THREAD_STATE_ALIVE);
}//threadProcessingLoop

/*************************************
* Function:
* Author: Roeil Jacob
* Description:
*************************************/
bool CVideo::copyVideoFrame(MyImage& _image)
{
	//fprintf(stderr, "Usage is `Image.exe Imagefile w h`");
	m_pCurrentFrame->ReadImage(m_pFile);
	_image = *m_pCurrentFrame;
	return true;
}//copyVideoFrame

/*************************************
* Function:
* Author: Roeil Jacob
* Description:
*************************************/
bool CVideo::playVideo()
{
	bool bReturn = false;

	m_threadHandle = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		(LPTHREAD_START_ROUTINE)&spawnThread,       // thread function name
		this,						// argument to thread function 
		0,                      // use default creation flags 
		&m_dwThreadId);   // returns the thread identifier 

	if (m_threadHandle == NULL)
	{
		fprintf(stderr, "CreateThread");
		bReturn = false;
	}
	else
		bReturn = true;

	return bReturn;
}//playVideo