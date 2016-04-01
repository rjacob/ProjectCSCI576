#pragma once

#include "Image.h"

class CVideo
{
public:
	CVideo();
	CVideo(char*, int, int);
	~CVideo();

	//accessors
	long getNoFrames() const { return m_ulNoFrames;}
	char* getVideoPath() const { return m_videoPath;}

	//mutators
	void setImagePath(const char *path) { strcpy(m_videoPath, path); }
	
private:
	unsigned long m_ulNoFrames;
	unsigned int m_unWidth;
	unsigned int m_unHeight;
	MyImage m_currentFrame;

	char m_videoPath[_MAX_PATH];	// Video location

	FILE* m_pFile;
};