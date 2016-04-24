#pragma once

#include <windows.h>
#include <time.h>
#include "Image.h"
#include "CVideoBuffer.h"

#define DEBUG_FILE 0
#define FRAME_RATE_HZ 15


typedef enum
{
	VIDEO_STATE_UNKNOWN = 0,
	VIDEO_STATE_BUFFERING,
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
	unsigned int getVideoWidth() const { return m_unVideoWidth;}
	unsigned int getVideoHeight() const { return m_unVideoHeight; }
	long getNoFrames() const { return m_ulNoFrames;}
	unsigned int getVideoDuration() const { return m_unVideoDurationSubSec; }
	VIDEO_STATE_E getVideoState() const { return m_eVideoState; }
	unsigned long getCurrentFrameNo() const { return (m_ulCurrentFrameIndex); }//TODO: m_ulCurrentFrameIndex
	vector<unsigned short> getIFrames() const { return m_iFrames; }

	//mutators
	void createVideo(int, int);
	void setVideoPath(char* _videoPath) { m_pVideoPath = _videoPath; }

	//Interface + summarization functions
	bool playVideo(bool _bCorrect);
	bool pauseVideo();
	bool stopVideo();
	bool analyzeVideo();

	unsigned short copyVideoFrame(MyImage&);
	
private:
	char* m_pVideoPath;
	unsigned long m_ulNoFrames;
	CVideoBuffer *m_pVideoBuffer;
	unsigned long m_ulCurrentFrameIndex;//0-Indexed
	unsigned int m_unVideoWidth;
	unsigned int m_unVideoHeight;
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

	bool readVideoFrame(MyImage&, unsigned int _nFrame);

	FILE* m_correctFile;

	bool m_bCorrect;//TODO change into function??
	bool videoSummarization(unsigned long, MyImage&, MyImage&);
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