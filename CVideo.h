#pragma once

#include "Image.h"

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
	MyImage* getVideoFrame();

	//mutators
	void setImagePath(const char *path) { strcpy(m_videoPath, path); }
	
private:
	unsigned long m_ulNoFrames;
	unsigned int m_unWidth;
	unsigned int m_unHeight;
	MyImage* m_pCurrentFrame;

	char m_videoPath[_MAX_PATH];	// Video location

	FILE* m_pFile;
};