#pragma once

#include <windows.h>
#include "Image.h"
#include "CDoubleBuffer.h"

typedef enum
{
	VIDEO_STATE_UNKNOWN = 0,
	VIDEO_STATE_PLAYING,//Playing
	VIDEO_STATE_PAUSED,//Paused
	VIDEO_STATE_ANALYZING,//Analyzing
	VIDEO_STATE_STOPPED,//Stopped
}VIDEO_STATE_E;

//Thread based
class CVideo
{
public:
	CVideo();
	CVideo(char*, int, int);
	~CVideo();

	//accessors
	unsigned int getVideoWidth() const { return m_unWidth;}
	unsigned int getVideoHeight() const { return m_unHeight; }
	long getNoFrames() const { return m_ulNoFrames;}
	char* getVideoPath() { return m_videoPath;}
	unsigned int getVideoDuration() const { return m_unVideoDurationSubSec; }
	VIDEO_STATE_E getVideoState() const { return m_eVideoState; }
	unsigned long getCurrentFrameNo() const { return (m_ulCurrentFrameIndex + 1); }//TODO: m_ulCurrentFrameIndex

	//mutators
	void setImagePath(const char *path) { strcpy(m_videoPath, path);}\
	void setOutputFrame(MyImage* _outputFrame) { m_pOutputFrame = _outputFrame; }

	//Interface + summarization functions
	bool playVideo(bool _bCorrect);
	bool pauseVideo();
	bool stopVideo();
	bool analyzeVideo();
	
private:
	unsigned long m_ulNoFrames;
	MyImage* m_pCurrentFrame, *m_pOutputFrame;
	unsigned long m_ulCurrentFrameIndex;//0-Indexed
	unsigned int m_unWidth;
	unsigned int m_unHeight;
	char m_videoPath[_MAX_PATH];	// Video location
	FILE* m_pFile;
	VIDEO_STATE_E m_eVideoState;
	HANDLE m_threadPlayingHandle;
	HANDLE m_threadAnalysisHandle;
	DWORD m_dwThreadId;
	bool m_bPlaying;
	unsigned int m_unVideoDurationSubSec;//at 15Hz, 67ms

	bool copyVideoFrame(MyImage&, unsigned int _nFrame);

	bool m_bCorrect;//TODO change into function??
	bool videoSummarization();
	void featuresMatch(Mat, Mat);
	void outlierRejection();
	void calcHomography(Mat&, vector<KeyPoint>&, Mat&, vector<KeyPoint>&);
	void frameWarping();

	//Thread Procesing
	static void spawnPlayingThread(CVideo* _pThis) { _pThis->threadPlayingLoop(); }
	void threadPlayingLoop();

	static void spawnAnalyzingThread(CVideo* _pThis) { _pThis->threadAnalyzingLoop(); }
	void threadAnalyzingLoop();
};