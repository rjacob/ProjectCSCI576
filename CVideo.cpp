#include "CVideo.h"

/*************************************
* Function: Constructor
* Description: 
*************************************/
CVideo::CVideo()
	:
	m_ulNoFrames(0),
	m_pCurrentFrame(NULL),
	m_pPrevFrame(NULL),
	m_pOutputFrame(NULL),
	m_ulCurrentFrameIndex(0),
	m_unWidth(0),
	m_unHeight(0),
	m_eVideoState(VIDEO_STATE_UNKNOWN),
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
	m_pCurrentFrame(NULL),
	m_pPrevFrame(NULL),
	m_pOutputFrame(NULL),
	m_ulCurrentFrameIndex(0),
	m_unWidth(_nWidth),
	m_unHeight(_nHeight),
	m_eVideoState(VIDEO_STATE_UNKNOWN),
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

	entropyValues = new double[m_ulNoFrames];
	templateValues = new double[m_ulNoFrames];
	colorHistValues = new int[m_ulNoFrames];
	xSquaredValues = new double[m_ulNoFrames];

	m_pCurrentFrame = new MyImage();
	m_pCurrentFrame->setWidth(m_unWidth);
	m_pCurrentFrame->setHeight(m_unHeight);

	m_pPrevFrame = new MyImage();
	m_pPrevFrame->setHeight(m_unHeight);
	m_pPrevFrame->setWidth(m_unWidth);

	m_pMatcher = new BFMatcher(NORM_L2);
}//constructor

/*************************************
* Function:
* Description:
*************************************/
CVideo::~CVideo()
{
	fclose(m_pFile);
	m_pFile = 0;

	if(m_pCurrentFrame)
		delete m_pCurrentFrame;

	if (m_pPrevFrame)
		delete m_pPrevFrame;

	if (m_pMatcher)
		delete m_pMatcher;

	if (entropyValues)
		delete entropyValues;

	if (templateValues)
		delete templateValues;

	if (colorHistValues)
		delete colorHistValues;

	if (xSquaredValues)
		delete xSquaredValues;

}//destructor

/*************************************
* Function: threadPlayingLoop
* Description: Sleep time is with the time it takes to compute 15Hz
*************************************/
void CVideo::threadPlayingLoop()
{
	clock_t iterationTime;
	unsigned short unOnTime_ms;

	do
	{
		unOnTime_ms = 0;

		if (m_eVideoState != VIDEO_STATE_PAUSED)
		{
			iterationTime = clock();
			copyVideoFrame(*m_pCurrentFrame, m_ulCurrentFrameIndex++);

			if(m_bCorrect)
				m_pCurrentFrame->Modify();

			if(m_pOutputFrame)//is there a output buffer we can write to?
				*m_pOutputFrame = *m_pCurrentFrame;

			unOnTime_ms = (clock() - iterationTime);

			if (unOnTime_ms > 1000 / 15)
			{
				char str[128] = { 0 };
				sprintf(str, "[%d] %d\n", m_ulCurrentFrameIndex, unOnTime_ms);
				unOnTime_ms = (1000 / 15);//clamp
				OutputDebugString(_T(str));
			}
		}
		Sleep(1000/15 - unOnTime_ms);
	} while(m_eVideoState != VIDEO_STATE_STOPPED && m_ulCurrentFrameIndex < m_ulNoFrames);
}//threadProcessingLoop

/*************************************
* Function: copyVideoFrame
* Description: 
*************************************/
bool CVideo::copyVideoFrame(MyImage& _image, unsigned int _nFrameNo)
{
	return _image.ReadImage(m_pFile, _nFrameNo);
}//copyVideoFrame

/*************************************
* Function: playVideo
* Description:
*************************************/
bool CVideo::playVideo(bool _bCorrect)
{
	bool bReturn = false;
	m_bCorrect = _bCorrect;

	//Don't spawn additional threads when going from pause to replay
	if (m_eVideoState == VIDEO_STATE_UNKNOWN ||
		m_eVideoState == VIDEO_STATE_STOPPED)
	{
		m_threadPlayingHandle = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			(LPTHREAD_START_ROUTINE)&spawnPlayingThread,       // thread function name
			this,						// argument to thread function 
			0,                      // use default creation flags 
			&m_dwThreadId);   // returns the thread identifier
	}

	if (m_threadPlayingHandle == NULL)
	{
		fprintf(stderr, "Create Thread FAIL");
		bReturn = false;
	}
	else
	{
		m_eVideoState = VIDEO_STATE_PLAYING;
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
	m_eVideoState = VIDEO_STATE_STOPPED;
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
	m_eVideoState = VIDEO_STATE_PAUSED;
	m_bPlaying = false;
	return true;
}//pauseVideo

/*************************************
* Function: pauseVideo
* Description:
*************************************/
bool CVideo::analyzeVideo()
{
	m_threadAnalysisHandle = CreateThread(
		NULL,                   // default security attributes
		1024*1024*32,                      // use default stack size  
		(LPTHREAD_START_ROUTINE)&spawnAnalyzingThread,       // thread function name
		this,						// argument to thread function 
		0,                      // use default creation flags 
		&m_dwThreadId);   // returns the thread identifier

	return true;
}

/*************************************
* Function: threadAnalyzingLoop
* Description:
*************************************/
void CVideo::threadAnalyzingLoop()
{
	m_eVideoState = VIDEO_STATE_ANALYZING;
	m_ulCurrentFrameIndex = 0;
	m_unVideoDurationSubSec = m_ulNoFrames * 15;
	vector<KeyPoint> keypointsCurr, keypointsPrev;

#if DEBUG_FILE
	debugOutput = fopen("Video Data.txt", "w");
	fprintf(debugOutput, "Number of frames: %d\n", m_ulNoFrames);
	fprintf(debugOutput, "Frame:\t");
	fprintf(debugOutput, "Entropy:\t");
	fprintf(debugOutput, "Template:\t");
	fprintf(debugOutput, "Color:\t");
	fprintf(debugOutput, "X2:\n");
	fclose(debugOutput);
	debugOutput = fopen("Video Data.txt", "a");
#endif

	copyVideoFrame(*m_pPrevFrame, 0);//Get first frame, one step ahead
	//Analyze All frames
	for (unsigned long i = 0; i < m_ulNoFrames; i++)
	{
		if (m_eVideoState != VIDEO_STATE_STOPPED)
		{
			copyVideoFrame(*m_pCurrentFrame, i);
			videoSummarization(i);

#if CORRECT
			// Open CV data matrices
			Mat	dataMatCurrent(m_unHeight, m_unWidth, CV_8UC3, m_pCurrentFrame->getImageData());
			Mat dataMatPrev(m_unHeight, m_unWidth, CV_8UC3, m_pPrevFrame->getImageData());

			if(!dataMatPrev.empty() && !dataMatCurrent.empty())
			{
				//Convert to grayscale
				Mat greyMatPrev, greyMatCurr;
				cvtColor(dataMatPrev, greyMatPrev, CV_BGR2GRAY);
				cvtColor(dataMatCurrent, greyMatCurr, CV_BGR2GRAY);

				//Train
				m_pPrevFrame->featuresDetec(dataMatPrev, keypointsPrev);

				//Query
				m_pCurrentFrame->featuresDetec(dataMatCurrent, keypointsCurr);

				SurfDescriptorExtractor extractor;

				try
				{
					extractor.compute(greyMatPrev, keypointsPrev, descriptorPrev);
					extractor.compute(greyMatCurr, keypointsCurr, descriptorCurr);
				}
				catch (...)
				{
					OutputDebugString(_T("Exception"));
				}

				m_pMatcher->match(descriptorCurr, descriptorPrev, matches);

				//char str[128] = { 0 };
				//sprintf(str, "[%d, %d] %d\n", keypointsPrev.size(), keypointsCurr.size(), matches.size());
				//OutputDebugString(_T(str));

				mpts1.clear();
				mpts2.clear();
				mpts1.reserve(matches.size());
				mpts2.reserve(matches.size());
				for (size_t i = 0; i < matches.size(); i++)
				{
					const DMatch& match = matches[i];
					mpts1.push_back(keypointsPrev[match.queryIdx].pt);
					mpts2.push_back(keypointsCurr[match.trainIdx].pt);
				}

				//homographyMatrix = findHomography(mpts2, mpts1, RANSAC, 1, outlier_mask);

				//featuresMatch(greyMatPrev, keypointsPrev, greyMatCurr, keypointsCurr);
				//	calcHomographyMatrix(dataMatCurrent, keypointsCurr, dataMatPrev, keypointsPrev);
			}
#endif

			*m_pPrevFrame = *m_pCurrentFrame;
			m_ulCurrentFrameIndex = i;
		}

	}
	m_eVideoState = VIDEO_STATE_ANALYSIS_COMPLETE;
	m_ulCurrentFrameIndex = 0;

#if DEBUG_FILE
	fclose(debugOutput);
#endif
}

 /*************************************
 * Function: videoSummarization
 * Description:
 *************************************/
bool CVideo::videoSummarization(unsigned long _ulFrameIndex)
{
	if (m_ulNoFrames == 0)
		return false;

	//Cycle through each frame in video
	//Apply RGB histogram, entropy here
	//double *entropyValues = new double[m_ulNoFrames];
	//double *templateValues = new double[m_ulNoFrames];
	//int *colorHistValues = new int[m_ulNoFrames];
	//double *xSquaredValues = new double[m_ulNoFrames];

	//Add analysis values to arrays
	entropyValues[_ulFrameIndex] = m_pCurrentFrame->calcEntropy();//70ms
	templateValues[_ulFrameIndex] = m_pCurrentFrame->templateMatchDifference(*m_pPrevFrame);
	colorHistValues[_ulFrameIndex] = m_pCurrentFrame->colorHistogramDifference(*m_pPrevFrame);//50ms
	xSquaredValues[_ulFrameIndex] = m_pCurrentFrame->xSquaredHistogramDifference(*m_pPrevFrame);

#if DEBUG_FILE
	//Create output file of data
	fprintf(debugOutput, "%d\t", _ulFrameIndex);
	fprintf(debugOutput, "%f\t", entropyValues[_ulFrameIndex]);
	fprintf(debugOutput, "%f\t", templateValues[_ulFrameIndex]);
	fprintf(debugOutput, "%d\t", colorHistValues[_ulFrameIndex]);
	fprintf(debugOutput, "%f\n", xSquaredValues[_ulFrameIndex]);
#endif

	/*
	//Create array of "I-frames" here
	FILE *outFile;
	outFile = fopen("Average.txt", "w");
	iFrames.push_back(0);	//Frame 0 is always an I-frame
	double average = 0;
	for (unsigned long i = 0; i < m_ulNoFrames; i++) {
		fprintf(outFile, "%f\n", xSquaredValues[i] / m_ulNoFrames);
		average += (xSquaredValues[i] / m_ulNoFrames);
	}

	fprintf(outFile, "%f", average);
	fclose(outFile);*/

	//Choose best "GOPs" here

	//Output summarized video (or "I-frame" array) here

	return true;
}//videoSummarization

//Brute-Force Matching (Hamming)
void CVideo::featuresMatch(Mat& _framePrev, vector<KeyPoint>& _keyPtsPrev, Mat&  _framCurr, vector<KeyPoint>& _keyPtsCurr)
{
	SurfDescriptorExtractor extractor;
	BFMatcher matcher(NORM_L2);

	vector<DMatch> matches;
	Mat descriptorPrev, descriptorCurr;

	try
	{
		extractor.compute(_framePrev, _keyPtsPrev, descriptorPrev);
		extractor.compute(_framCurr, _keyPtsCurr, descriptorCurr);
	}
	catch (...)
	{
		OutputDebugString(_T("Exception"));
	}

	matcher.match(descriptorCurr, descriptorPrev, matches);

	//char str[128] = { 0 };
	//sprintf(str, "[%d, %d] %d\n", _keyPtsPrev.size(), _keyPtsCurr.size(), matches.size());
	//OutputDebugString(_T(str));

	//vector<Point2f> mpts1, mpts2;
	//vector<char> outlier_mask;
	//Mat homographyMatrix;
	//outlierRejection(matches, _keyPtsPrev, _keyPtsCurr, mpts1, mpts2);
	//homographyMatrix = findHomography(mpts2, mpts1, RANSAC, 1, outlier_mask);
}//featuresMatch

void CVideo::outlierRejection(
	vector<DMatch>& _matches,
	vector<KeyPoint>& _keyPts1,
	vector<KeyPoint>& _keyPts2,
	vector<Point2f>& _mpts_1,
	vector<Point2f>& _mpts_2)
{
	_mpts_1.clear();
	_mpts_2.clear();
	_mpts_1.reserve(_matches.size());
	_mpts_2.reserve(_matches.size());
	for (size_t i = 0; i < _matches.size(); i++)
	{
		const DMatch& match = _matches[i];
		_mpts_1.push_back(_keyPts1[match.queryIdx].pt);
		_mpts_2.push_back(_keyPts2[match.trainIdx].pt);
	}
}//outlierRejection

void CVideo::transformFrame(Mat _homographyMatrix)
{
	Mat warped;
	warpPerspective(_homographyMatrix, warped, _homographyMatrix, _homographyMatrix.size());
}//transformFrame