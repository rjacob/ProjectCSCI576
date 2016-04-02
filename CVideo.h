#pragma once

#include <windows.h>
#include "Image.h"

typedef enum
{
	THREAD_STATE_UNKNOWN = 0,
	THREAD_STATE_ALIVE,
	THREAD_STATE_KILLED
}THREAD_STATE_E;

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
	bool playVideo();

	//mutators
	void setImagePath(const char *path) { strcpy(m_videoPath, path); }
	void setOutputBuff(MyImage* _pOutputBuff) { m_pOutputBuffer = _pOutputBuff; }
	
private:
	unsigned long m_ulNoFrames;
	unsigned int m_unWidth;
	unsigned int m_unHeight;
	MyImage* m_pCurrentFrame;
	MyImage* m_pOutputBuffer;

	char m_videoPath[_MAX_PATH];	// Video location

	FILE* m_pFile;

	THREAD_STATE_E m_eThreadState;
	HANDLE m_threadHandle;
	DWORD m_dwThreadId;

	static void spawnThread(CVideo* _pThis) { _pThis->threadProcessingLoop(); }
	void threadProcessingLoop();
	bool copyVideoFrame(MyImage&);
};