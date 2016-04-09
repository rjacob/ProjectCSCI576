#include "CVideo.h"

/*************************************
* Function: Constructor
* Description: 
*************************************/
CVideo::CVideo()
	:
	m_ulNoFrames(0),
	m_ulCurrentFrameIndex(0),
	m_unWidth(0),
	m_unHeight(0),
	m_eThreadState(THREAD_STATE_UNKNOWN),
	m_bPlaying(FALSE),
	m_bCorrect(false)
{
	m_videoPath[0] = 0;
}//constructor

/*************************************
* Function: Constructor
* Description:
*************************************/
CVideo::CVideo(char* _cstrVideoPath, int _nWidth, int _nHeight)
:
	m_ulNoFrames(0),
	m_ulCurrentFrameIndex(0),
	m_unWidth(_nWidth),
	m_unHeight(_nHeight),
	m_eThreadState(THREAD_STATE_UNKNOWN),
	m_bPlaying(FALSE),
	m_bCorrect(false)
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

	//Find the number of frames and reset file pointer
	int nFileSize = 0;
	fseek(m_pFile, 0L, SEEK_END);
	nFileSize = ftell(m_pFile);
	fseek(m_pFile, 0L, SEEK_SET);

	if (nFileSize)
	{
		m_ulNoFrames = nFileSize / (m_unWidth*m_unHeight * 3);
	}

	m_pCurrentFrame = new MyImage();
	m_pCurrentFrame->setWidth(m_unWidth);
	m_pCurrentFrame->setHeight(m_unHeight);
}//constructor

/*************************************
* Function:
* Description:
*************************************/
CVideo::~CVideo()
{
	fclose(m_pFile);
	m_pFile = 0;
}//destructor

/*************************************
* Function:
* Description:
*************************************/
void CVideo::threadProcessingLoop()
{
	do
	{
		if (m_eThreadState != THREAD_STATE_PAUSED)
		{
			copyVideoFrame(*m_pOutputBuffer, m_ulCurrentFrameIndex++);
		}
		Sleep(1000 / 15);//15Hz TODO: consider time it takes to readVideoFrame
	} while(m_eThreadState != THREAD_STATE_KILLED && m_ulCurrentFrameIndex < m_ulNoFrames);

	m_ulCurrentFrameIndex = 0;
}//threadProcessingLoop

/*************************************
* Function: readVideoFrame
* Description: 
*************************************/
bool CVideo::copyVideoFrame(MyImage& _image, unsigned int _nFrameNo)
{
	m_pCurrentFrame->ReadImage(m_pFile, _nFrameNo);
	_image = *m_pCurrentFrame;
	return true;
}//copyVideoFrame

/*************************************
* Function: playVideo
* Description:
*************************************/
bool CVideo::playVideo(bool _bCorrect)
{
	bool bReturn = false;
	m_bCorrect = _bCorrect;

	//Dont spawn additional threads when going from pause to replay
	if (m_eThreadState == THREAD_STATE_UNKNOWN ||
		m_eThreadState == THREAD_STATE_KILLED)
	{
		m_threadHandle = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			(LPTHREAD_START_ROUTINE)&spawnThread,       // thread function name
			this,						// argument to thread function 
			0,                      // use default creation flags 
			&m_dwThreadId);   // returns the thread identifier
	}

	if (m_threadHandle == NULL)
	{
		fprintf(stderr, "CreateThread");
		bReturn = false;
	}
	else
	{
		m_eThreadState = THREAD_STATE_ALIVE;
		m_bPlaying = true;
		bReturn = true;
	}

	return bReturn;
}//playVideo

/*************************************
* Function: stopVideo
* Description:
*************************************/
bool CVideo::stopVideo()
{
	m_eThreadState = THREAD_STATE_KILLED;
	m_ulCurrentFrameIndex = 0;
	m_bPlaying = false;
	//Clean-up
	return true;
}//stopVideo

/*************************************
* Function: pauseVideo
* Description:
*************************************/
bool CVideo::pauseVideo()
{
	m_eThreadState = THREAD_STATE_PAUSED;
	m_bPlaying = false;
	return true;
}//pauseVideo