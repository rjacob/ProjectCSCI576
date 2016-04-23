#pragma once

#include <windows.h>
#include <time.h>
#include "Image.h"
#include "CDoubleBuffer.h"

#define DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

#define DEBUG_FILE 0

typedef enum
{
	VIDEO_STATE_UNKNOWN = 0,
	VIDEO_STATE_PLAYING,//Playing
	VIDEO_STATE_PAUSED,//Paused
	VIDEO_STATE_ANALYZING,//Analyzing
	VIDEO_STATE_ANALYSIS_COMPLETE,
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
	unsigned int getVideoDuration() const { return m_unVideoDurationSubSec; }
	VIDEO_STATE_E getVideoState() const { return m_eVideoState; }
	unsigned long getCurrentFrameNo() const { return (m_ulCurrentFrameIndex); }//TODO: m_ulCurrentFrameIndex

	//mutators
	void createVideo(int, int);
	void setVideoPath(char* _videoPath) { m_pVideoPath = _videoPath; }
	void setOutputFrame(MyImage* _outputFrame) { m_pOutputFrame = _outputFrame; }//TODO: Zero-Copy

	//Interface + summarization functions
	bool playVideo(bool _bCorrect);
	bool pauseVideo();
	bool stopVideo();
	bool analyzeVideo();
	
private:
	char* m_pVideoPath;
	unsigned long m_ulNoFrames;
	BUFFER_STYPE *m_pFrameBuffer;
	MyImage *m_pOutputFrame;
	unsigned long m_ulCurrentFrameIndex;//0-Indexed
	unsigned int m_unWidth;
	unsigned int m_unHeight;
	char m_videoPath[_MAX_PATH];	// Video location
	FILE* m_pFile;
	VIDEO_STATE_E m_eVideoState;
	HANDLE m_threadPlayingHandle;
	HANDLE m_threadAnalysisHandle;
	DWORD m_dwThreadIdPlay;
	DWORD m_dwThreadIdAnalysis;
	bool m_bPlaying;
	unsigned int m_unVideoDurationSubSec;//at 15Hz, 67ms
	BFMatcher *m_pMatcher;
	vector<DMatch> m_matches;
	Mat m_descriptorPrev, m_descriptorCurr;
	vector<Point2f> m_pts1, m_pts2;

	double *entropyValues;
	double *templateValues;
	int *colorHistValues;
	double *xSquaredValues;
	vector<unsigned short> m_iFrames;
	vector<unsigned short> m_summarizationFrames;
	FILE* debugOutput;

	bool copyVideoFrame(MyImage&, unsigned int _nFrame);

	FILE* m_correctFile;

	bool m_bCorrect;//TODO change into function??
	bool videoSummarization(unsigned long);
	bool generateIFrames();
	bool generateSummarizationFrames();
	//void featuresMatch(Mat&, vector<KeyPoint>&, Mat&, vector<KeyPoint>&);
	//void outlierRejection(vector<DMatch>&, vector<KeyPoint>&, vector<KeyPoint>&);
	//void transformFrame(Mat);

	//Thread Procesing
	static void spawnPlayingThread(CVideo* _pThis) { _pThis->threadPlayingLoop(); }
	void threadPlayingLoop();

	static void spawnAnalyzingThread(CVideo* _pThis) { _pThis->threadAnalyzingLoop(); }
	void threadAnalyzingLoop();
};