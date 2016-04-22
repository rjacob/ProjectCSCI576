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
void CVideo::createVideo(int _nWidth, int _nHeight)
{
	m_unWidth = _nWidth;
	m_unHeight = _nHeight;

	if (m_pVideoPath == NULL)
		return;

	strncpy(m_videoPath, m_pVideoPath, strlen(m_pVideoPath));
	// Create a valid output file pointer
	m_pFile = fopen(m_pVideoPath, "rb");

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

	if (entropyValues)
		delete entropyValues;

	if (templateValues)
		delete templateValues;

	if (colorHistValues)
		delete colorHistValues;

	if (xSquaredValues)
		delete xSquaredValues;

	//if (m_pMatcher)
	//	delete m_pMatcher;

}//destructor

/*************************************
* Function: threadPlayingLoop
* Description: Sleep time is with the time it takes to compute 15Hz
*************************************/
void CVideo::threadPlayingLoop()
{
	clock_t iterationTime;
	unsigned short unOnTime_ms;
	static unsigned int unTotal = 0;

	if (m_bCorrect)
		m_pFile = fopen(m_pVideoPath, "rb");

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

			if (unOnTime_ms > round(1000 / 15))
			{
				char str[128] = { 0 };
				sprintf(str, "[%d] %d %d\n", m_ulCurrentFrameIndex, unOnTime_ms, unTotal);
				unTotal += (unOnTime_ms - (1000 / 15));
				unOnTime_ms = round(1000 / 15);//clamp
				OutputDebugString(_T(str));
			}
		}
		Sleep(round(1000/15) - unOnTime_ms);
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
			&m_dwThreadIdPlay);   // returns the thread identifier
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
		&m_dwThreadIdAnalysis);   // returns the thread identifier

	return true;
}

/*************************************
* Function: threadAnalyzingLoop
* Description:
*************************************/
FILE *outFile;
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

	if (1)
	{
		char pCorrectedFilePath[128] = { 0 };
		char* addr;
		sprintf(pCorrectedFilePath, "%s", m_pVideoPath);
		addr = strchr(pCorrectedFilePath, 'r');
		sprintf(--addr, "%s", "C.rgb");
		m_correctFile = fopen(pCorrectedFilePath, "a");
	}

	copyVideoFrame(*m_pPrevFrame, 0);//Get first frame, one step ahead
	//Analyze All frames
	for (unsigned long i = 0; i < m_ulNoFrames; i++)
	{
		if (m_eVideoState != VIDEO_STATE_STOPPED)
		{
			copyVideoFrame(*m_pCurrentFrame, i);
			videoSummarization(i);

if(1)
{
			// Open CV data matrices
			Mat	dataMatCurrent(m_unHeight, m_unWidth, CV_8UC3, m_pCurrentFrame->getImageData());
			Mat dataMatPrev(m_unHeight, m_unWidth, CV_8UC3, m_pPrevFrame->getImageData());

			if(!dataMatPrev.empty() && !dataMatCurrent.empty())
			{
				//Convert to grayscale
				Mat greyMatPrev, greyMatCurr;
				cvtColor(dataMatPrev, greyMatPrev, CV_BGR2GRAY);
				cvtColor(dataMatCurrent, greyMatCurr, CV_BGR2GRAY);

				//Feature detection
				//Train
				m_pPrevFrame->featuresDetec(dataMatPrev, keypointsPrev);

				//Query
				m_pCurrentFrame->featuresDetec(dataMatCurrent, keypointsCurr);

				//Descriptor Extaction
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

				//Descriptor Match
				m_pMatcher->match(descriptorCurr, descriptorPrev, matches);

char buff[32] = { 0 };
sprintf(buff, "[%d] Matches %d, ",i, matches.size());
OutputDebugString(_T(buff));

				m_pts1.clear();//Current
				m_pts2.clear();//Train
				m_pts1.reserve(matches.size());
				m_pts2.reserve(matches.size());

				//extact points from keypoints based on matches
				for (size_t i = 0; i < matches.size(); i++)
				{
					const DMatch& match = matches[i];
					Point2f query, train;

					query = keypointsPrev[match.queryIdx].pt;
					train = keypointsCurr[match.trainIdx].pt;

					double dist = sqrt((train.x - query.x) * (train.x - query.x) +
						(train.y - query.y) * (train.y - query.y));

					if (dist < 12.0f)
					{
						m_pts1.push_back(keypointsPrev[match.queryIdx].pt);//Query (Curr)
						m_pts2.push_back(keypointsCurr[match.trainIdx].pt);//Train (Train)
					}
				}

sprintf(buff, "(%d,%d)\n", m_pts1.size(), m_pts2.size());
OutputDebugString(_T(buff));

				if (m_pts1.size() >= 20)
				{
					//Use RANSAC to determine the highly unreliable correspondences(the outliers)
					//then use all of the remaining(still noisy) correspondences
					try
					{
						homographyMatrix = findHomography(m_pts1, m_pts2, RANSAC);
						//homographyMatrix = getPerspectiveTransform(mpts1, mpts2);
					}
					catch (...)
					{
						char str[128] = { 0 };
						sprintf(str, "Exception: 0x%X\n", 0);

						OutputDebugString(_T(str));
					}

					Mat warped;
					warpPerspective(dataMatCurrent, warped, homographyMatrix, dataMatCurrent.size());
imshow("warped", warped);
//m_pCurrentFrame->WriteImage(m_correctFile, (char*)dataMatCurrent.data);
					fclose(m_correctFile);
				}
				else
				{
imshow("warped", dataMatCurrent);
					//m_pCurrentFrame->WriteImage(m_correctFile, (char*)dataMatCurrent.data);
					//fwrite(dataMatCurrent.data, CV_8UC3, m_unWidth*m_unHeight, m_correctFile);
				}
waitKey(60);
			}
}

			*m_pPrevFrame = *m_pCurrentFrame;
			m_ulCurrentFrameIndex = i;
		}

	}

	m_eVideoState = VIDEO_STATE_ANALYSIS_COMPLETE;
	m_ulCurrentFrameIndex = 0;

	generateIFrames();	//Generate vector of I frames
	generateSummarizationFrames();	//Generate vector of summarization frames

#if DEBUG_FILE
	fclose(debugOutput);
#endif

if(1)
	fclose(m_correctFile);
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

	//Add analysis values to arrays
	entropyValues[_ulFrameIndex] = m_pCurrentFrame->calcEntropy();//70ms
	templateValues[_ulFrameIndex] = m_pCurrentFrame->templateMatchDifference(*m_pPrevFrame);
	colorHistValues[_ulFrameIndex] = m_pCurrentFrame->colorHistogramDifference(*m_pPrevFrame);//50ms
	xSquaredValues[_ulFrameIndex] = m_pCurrentFrame->xSquaredHistogramDifference(*m_pPrevFrame);

#if DEBUG_FILE
	//Create array of "I-frames" here

	//Create output file of data
	fprintf(debugOutput, "%d\t", _ulFrameIndex);
	fprintf(debugOutput, "%f\t", entropyValues[_ulFrameIndex]);
	fprintf(debugOutput, "%f\t", templateValues[_ulFrameIndex]);
	fprintf(debugOutput, "%d\t", colorHistValues[_ulFrameIndex]);
	fprintf(debugOutput, "%f\n", xSquaredValues[_ulFrameIndex]);
#endif

	return true;
}//videoSummarization

//Generate vector of I-Frames based on xSquared values
bool CVideo::generateIFrames() 
{
	//Find average of xSquared values
	double average = 0;
	for (unsigned long i = 0; i < m_ulNoFrames; i++) {
		average += xSquaredValues[i];
	}
	average /= m_ulNoFrames;

	//Find variance and standard deviation of xSquared values
	double variance = 0;
	for (unsigned long i = 0; i < m_ulNoFrames; i++) {
		variance += pow(xSquaredValues[i] - average, 2.0);
	}
	variance /= m_ulNoFrames;
	double standardDeviation = sqrt(variance);

	//Generate I-frames
	double limit = average + standardDeviation;		//Need to figure this out
	iFrames.push_back(0);	//Frame 0 is always an I-frame (Maybe?)
	for (unsigned long i = 1; i < m_ulNoFrames; i++) {
		if (xSquaredValues[i] > limit) {
			iFrames.push_back(i);
		}
	}

#if DEBUG_FILE
	debugOutput = fopen("Video Data.txt", "a");
	fprintf(debugOutput, "\nI-Frames:\n");
	for (unsigned long i = 0; i < iFrames.size(); i++) {
		fprintf(debugOutput, "%d\t%f\n", iFrames[i], xSquaredValues[iFrames[i]]);
	}
	fclose(debugOutput);
#endif

	return true;
}//generateIFrames

//Generate summarization frames using I-frames
bool CVideo::generateSummarizationFrames() {
	//If there is an I-frame within the GOP of another I-frame
	//increase the GOP length instead of creating new GOP
	unsigned long minGOPLength = 3 * 15;	//Make minimum GOP size to 3 seconds
	unsigned long lastFrameIndex = 0;
	for (int iFrameIndex = 0; iFrameIndex < iFrames.size(); iFrameIndex++) {
		unsigned long currentIFrame = iFrames[iFrameIndex];
		for (int i = 0; i < minGOPLength; i++) {
			if (currentIFrame + i >= lastFrameIndex) {
				summarizationFrames.push_back(currentIFrame + i);
			}
		}

		lastFrameIndex = iFrames[iFrameIndex] + minGOPLength;
	}

#if DEBUG_FILE
	debugOutput = fopen("Video Data.txt", "a");
	fprintf(debugOutput, "\nSummarization frames:\n");
	for (unsigned long i = 0; i < summarizationFrames.size(); i++) {
		fprintf(debugOutput, "%d\n", summarizationFrames[i]);
	}
	fclose(debugOutput);
#endif

	return true;
}//generateSummarizationFrames

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

void CVideo::outlierRejection(vector<DMatch>& _matches, vector<KeyPoint>&_keyPts1,  vector<KeyPoint>&_keyPts2)
{

}//outlierRejection

void CVideo::transformFrame(Mat _homographyMatrix)
{
	Mat warped;
	warpPerspective(_homographyMatrix, warped, _homographyMatrix, _homographyMatrix.size());
}//transformFrame