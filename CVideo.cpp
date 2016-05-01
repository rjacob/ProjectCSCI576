#include "CVideo.h"

//Video Player
/*************************************
* Function: Constructor
* Description: 
*************************************/
CVideo::CVideo()
	:
	m_ulNoFrames(0),
	m_ulCurrentFrameIndex(0),
	m_unVideoWidth(0),
	m_unVideoHeight(0),
	m_eVideoState(VIDEO_STATE_UNKNOWN),
	m_bPlaying(FALSE),
	m_bCorrect(false)
{
	m_videoPath[0] = 0;
}//constructor

 /*************************************
 * Function: Destructor
 * Description:
 *************************************/
CVideo::~CVideo()
{
	fclose(m_pFile);
	m_pFile = 0;

	if (m_pVideoBuffer)
		delete m_pVideoBuffer;

	if (entropyValues)
		delete entropyValues;

	if (templateValues)
		delete templateValues;

	if (colorHistValues)
		delete colorHistValues;

	if (xSquaredValues)
		delete xSquaredValues;

	if (m_pMatcher)
	{
		m_pMatcher->clear();
		//delete m_pMatcher;
	}

}//destructor

/*************************************
* Function: Constructor
* Description:
*************************************/
void CVideo::createVideo(int _nWidth, int _nHeight)
{
	m_unVideoWidth = _nWidth;
	m_unVideoHeight = _nHeight;

	if (m_pVideoPath == NULL)
		return;

	strncpy(m_videoPath, m_pVideoPath, strlen(m_pVideoPath));
	// Create a valid output file pointer
	m_pFile = fopen(m_pVideoPath, "rb");

	if (m_pFile == NULL)
	{
		OutputDebugString(_T("Error Opening File for Reading"));
		return;
	}

	if (m_unVideoWidth < 0 || m_unVideoHeight < 0)
	{
		OutputDebugString(_T("Image or Image properties not defined"));
		return;
	}

	//Find the number of frames and reset file pointer
	int nFileSize = 0;
	fseek(m_pFile, 0L, SEEK_END);
	nFileSize = ftell(m_pFile);
	fseek(m_pFile, 0L, SEEK_SET);

	if (nFileSize)
	{
		m_ulNoFrames = nFileSize / (m_unVideoWidth*m_unVideoHeight * 3);
	}

	entropyValues = new double[m_ulNoFrames];
	templateValues = new double[m_ulNoFrames];
	colorHistValues = new int[m_ulNoFrames];
	xSquaredValues = new double[m_ulNoFrames];

	m_pVideoBuffer = new CVideoBuffer(m_unVideoWidth, m_unVideoHeight);
	m_pMatcher = new BFMatcher(NORM_L2SQR);
}//constructor

/*************************************
* Function: threadPlayingLoop
* Description: Sleep time is with the time it takes to compute 15Hz
*************************************/
void CVideo::threadPlayingLoop()
{
	clock_t iterationTime;
	unsigned short unOnTime_ms;
	static unsigned int unTotal = 0;
	unsigned int nBufferIndex = 0;

	if (m_bCorrect)
		m_pFile = fopen(m_pVideoPath, "rb");

	do
	{
		unOnTime_ms = 0;

		if (m_eVideoState != VIDEO_STATE_PAUSED)
		{
			BUFFER_STYPE* pRef = NULL;
			iterationTime = clock();
			pRef = m_pVideoBuffer->nextFrame();

			if (pRef)
			{
				if(nBufferIndex >= BUFFER_SIZE/2)
					m_eVideoState = VIDEO_STATE_PLAYING;

				pRef->unFrameId = m_ulCurrentFrameIndex;
				readVideoFrame(pRef->image, m_ulCurrentFrameIndex++);
				nBufferIndex++;

				unOnTime_ms = (clock() - iterationTime);

				if (unOnTime_ms > round(1000 / FRAME_RATE_HZ))
				{
					char str[128] = { 0 };
					sprintf(str, "[%d] %d %d\n", m_ulCurrentFrameIndex, unOnTime_ms, unTotal);
					unTotal += (unOnTime_ms - (1000 / FRAME_RATE_HZ));
					unOnTime_ms = round(1000 / FRAME_RATE_HZ);//clamp
					OutputDebugString(_T(str));
				}
			}
		}
		Sleep(round(1000/ FRAME_RATE_HZ) - unOnTime_ms);
	} while(m_eVideoState != VIDEO_STATE_STOPPED && m_ulCurrentFrameIndex < m_ulNoFrames);
}//threadProcessingLoop

/*************************************
* Function: readVideoFrame
* Description: 
*************************************/
bool CVideo::readVideoFrame(MyImage& _image, unsigned int _nFrameNo)
{
	return _image.ReadImage(m_pFile, _nFrameNo);
}//readVideoFrame

 /*************************************
 * Function: copyVideoFrame
 * Description: This is executed from the scope of another thread
 *				TODO: Mutex protect
 *************************************/
unsigned short CVideo::copyVideoFrame(MyImage& _buff)
{
	BUFFER_STYPE *pRef = m_pVideoBuffer->read();
	if (pRef)
	{
		_buff = pRef->image;
		return pRef->unFrameId;
	}
	else
		return 0;
}//copyVideoFrame

 /*************************************
 * Function: setCurrentFrameNo
 * Description:
 *************************************/
MyImage CVideo::setCurrentFrameNo(unsigned long _frameNo)
{
	MyImage image(m_unVideoWidth, m_unVideoHeight);
	if (_frameNo < m_ulNoFrames)
	{
		m_ulCurrentFrameIndex = _frameNo;
		readVideoFrame(image, m_ulCurrentFrameIndex);
	}
	return image;
}//setCurrentFrameNo

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
		m_eVideoState = VIDEO_STATE_BUFFERING;
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
	m_pVideoBuffer->reset();
	m_bPlaying = false;
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
* Function: analyzeVideo
* Description: Spawns Video analyzing Thread
*************************************/
bool CVideo::analyzeVideo()
{
	m_threadAnalysisHandle = CreateThread(
		NULL,                   // default security attributes
		1024*1024,                      // use default stack size  
		(LPTHREAD_START_ROUTINE)&spawnAnalyzingThread,       // thread function name
		this,						// argument to thread function 
		0,                      // use default creation flags 
		&m_dwThreadIdAnalysis);   // returns the thread identifier

	return true;
}//analyzeVideo

/*************************************
* Function: threadAnalyzingLoop
* Description:
*************************************/
//Descriptor Extaction
void CVideo::threadAnalyzingLoop()
{
	m_eVideoState = VIDEO_STATE_ANALYZING;
	m_ulCurrentFrameIndex = 0;
	m_unVideoDurationSubSec = m_ulNoFrames * FRAME_RATE_HZ;

	//For Analysis we dont have to worry about buffering
	MyImage currentFrame(m_unVideoWidth, m_unVideoHeight);
	MyImage prevFrame(m_unVideoWidth, m_unVideoHeight);
	//Descriptor Extaction
	SurfDescriptorExtractor extractor;
	//vector<KeyPoint> keypointsCurr;
	//vector<KeyPoint> keypointsPrev;


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

	char pCorrectedFilePath[128] = { 0 };
	char* addr;
	sprintf(pCorrectedFilePath, "%s", m_pVideoPath);
	addr = strchr(pCorrectedFilePath, 'r');
	sprintf(--addr, "%s", "C.rgb");
	m_correctFile = fopen(pCorrectedFilePath, "wb");

	if(m_correctFile == NULL)
		OutputDebugString(_T("Shit!"));


	readVideoFrame(prevFrame, 0);//Get first frame, one step ahead
	//Analyze All frames
	for (unsigned long i = 0; i < m_ulNoFrames; i++)//m_ulNoFrames
	{
		if (m_eVideoState != VIDEO_STATE_STOPPED)
		{
			readVideoFrame(currentFrame, i);
			videoSummarization(i, prevFrame, currentFrame);

#if 1
			// Open CV data matrices
			Mat	dataMatCurrent(m_unVideoHeight, m_unVideoWidth, CV_8UC3, currentFrame.getImageData());
			Mat dataMatPrev(m_unVideoHeight, m_unVideoWidth, CV_8UC3, prevFrame.getImageData());

			if(!dataMatPrev.empty() && !dataMatCurrent.empty())
			{
				//Convert to grayscale
				Mat greyMatPrev, greyMatCurr;
				cvtColor(dataMatPrev, greyMatPrev, CV_BGR2GRAY);
				cvtColor(dataMatCurrent, greyMatCurr, CV_BGR2GRAY);

				m_descriptorPrev.resize(0);
				m_descriptorCurr.resize(0);

				try
				{
					//Feature detection (throws exception if not grayscale)
					prevFrame.featuresDetec(greyMatPrev, keypointsPrev);//Train
					currentFrame.featuresDetec(greyMatCurr, keypointsCurr);//Query

					extractor.compute(greyMatPrev, keypointsPrev, m_descriptorPrev);
					extractor.compute(greyMatCurr, keypointsCurr, m_descriptorCurr);
				}
				catch (...)
				{
					OutputDebugString(_T("Exception"));
				}
				
				m_descriptors.push_back(m_descriptorCurr);

				//Descriptor Match
				//Brute-Force Matching (Hamming)
				m_pMatcher->match(m_descriptorCurr, m_descriptorPrev, m_matches);
				m_pts1.clear();//Current
				m_pts2.clear();//Train
				m_pts1.reserve(m_matches.size());
				m_pts2.reserve(m_matches.size());

				//char str[32] = { 0 };
				//sprintf(str, "[%d] Mathced: %d\n", i, m_matches.size());
				//OutputDebugString(_T(str));

				//extact points from keypoints based on matches
				for (size_t i = 0; i < m_matches.size(); i++)
				{
					const DMatch& match = m_matches[i];
					Point2f query, train;

					query = keypointsPrev[match.queryIdx].pt;
					train = keypointsCurr[match.trainIdx].pt;

					volatile float dX = train.x - query.x;
					volatile float dY = train.y - query.y;
					double dist = sqrt(pow(dX,2) + pow(dY,2));

					if (dist < sqrt(162))
					{
						m_pts1.push_back(keypointsPrev[match.queryIdx].pt);//Query (Curr)
						m_pts2.push_back(keypointsCurr[match.trainIdx].pt);//Train (Train)
					}
				}//for matches

				if (m_pts1.size() >= 30)
				{
					Mat homographyMatrix;
					//Use RANSAC to determine the highly unreliable correspondences(the outliers)
					//then use all of the remaining(still noisy) correspondences
					try
					{
						homographyMatrix = findHomography(m_pts1, m_pts2, RANSAC);
					}
					catch (...)
					{
						char str[128] = { 0 };
						sprintf(str, "Exception: 0x%X\n", 0);

						OutputDebugString(_T(str));
					}

					//Transformation (Rotation or a Projection)
					Mat warped(m_unVideoHeight, m_unVideoWidth, CV_8UC3, Scalar(0, 0, 0));
					warpPerspective(dataMatCurrent, warped, homographyMatrix, dataMatCurrent.size());
					currentFrame.WriteImage(m_correctFile, warped);
				}//if #pts
				else
				{
					currentFrame.WriteImage(m_correctFile, dataMatCurrent);
				}//else
			}
#endif
			prevFrame = currentFrame;
			m_ulCurrentFrameIndex = i;
		}//if Video has not stopped
	}//for frames

	m_eVideoState = VIDEO_STATE_ANALYSIS_COMPLETE;
	m_ulCurrentFrameIndex = 0;

	generateIFrames();	//Generate vector of I frames
	//generateSummarizationFrames();	//Generate vector of summarization frames

#if DEBUG_FILE
	fclose(debugOutput);
#endif

	fclose(m_correctFile);
	keypointsCurr.clear();
	keypointsPrev.clear();
}

 /*************************************
 * Function: videoSummarization
 * Description:
 *************************************/
bool CVideo::videoSummarization(unsigned long _ulFrameIndex, MyImage& _prev, MyImage&_curr)
{
	if (m_ulNoFrames == 0)
		return false;

	//Cycle through each frame in video

	//Add analysis values to arrays
	entropyValues[_ulFrameIndex] = _curr.calcEntropy();//70ms
	templateValues[_ulFrameIndex] = _curr.templateMatchDifference(_prev);
	colorHistValues[_ulFrameIndex] = _curr.colorHistogramDifference(_prev);//50ms
	xSquaredValues[_ulFrameIndex] = _curr.xSquaredHistogramDifference(_prev);

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

/*************************************
* Function: generateIFrames
* Description: Generate vector of I-Frames based on xSquared values
*************************************/
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
	m_iFrames.push_back(0);	//Frame 0 is always an I-frame (Maybe?)
	for (unsigned long i = 1; i < m_ulNoFrames; i++) {
		if (xSquaredValues[i] > limit) {
			m_iFrames.push_back(i);
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

/*************************************
* Function: generateSummarizationFrames
* Description: Generate summarization frames using I-frames
*************************************/
bool CVideo::generateSummarizationFrames()
{
	//If there is an I-frame within the GOP of another I-frame
	//increase the GOP length instead of creating new GOP
	unsigned short minGOPLength = 3 * FRAME_RATE_HZ;	//Make minimum GOP size to 3 seconds
	unsigned long lastFrameIndex = 0;
	for (int iFrameIndex = 0; iFrameIndex < m_iFrames.size(); iFrameIndex++) {
		unsigned long currentIFrame = m_iFrames[iFrameIndex];
		for (int i = 0; i < minGOPLength; i++) {
			if (currentIFrame + i >= lastFrameIndex) {
				m_summarizationFrames.push_back(currentIFrame + i);
			}
		}

		lastFrameIndex = m_iFrames[iFrameIndex] + minGOPLength;
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

 /*************************************
 * Function: videoIndex
 * Description: 
 *************************************/
static vector<KeyPoint> keypoints, keypointsIndex;
unsigned long CVideo::videoIndex(MyImage& _source)
{
	//Speeded-Up Robust Features
	SurfDescriptorExtractor extractor;
	SurfFeatureDetector detector(400);//For real-time processing
	unsigned long ulFrameIndex = 0;
	Mat descriptors, descriptorsIndex;
	Mat greyMat, greyMatIndex;
	unsigned long ulFrameIndexMaxMatch = 0;
	unsigned long ulMaxMatch = 0;

	double dMinDiff = DBL_MAX;
	double dDiff = 0;

	//have we already analyzed?
//	if (m_descriptors.size() == m_ulNoFrames)
	{
		Mat	dataMat(_source.getHeight(), _source.getWidth(), CV_8UC3, _source.getImageData());
		Mat cvDataResized(m_unVideoHeight,m_unVideoWidth, CV_8UC3, Scalar(0, 0, 0));
		resize(dataMat, cvDataResized, cvDataResized.size(), INTER_LINEAR);
		cvtColor(cvDataResized, greyMat, CV_BGR2GRAY);

		detector.detect(greyMat, keypoints);
		extractor.compute(greyMat, keypoints, descriptors);

		do
		{
			MyImage imageIndex;
			imageIndex.setHeight(m_unVideoHeight);
			imageIndex.setWidth(m_unVideoWidth);
			readVideoFrame(imageIndex, ulFrameIndex);
			Mat	dataMatIndex(m_unVideoHeight, m_unVideoWidth, CV_8UC3, imageIndex.getImageData());
			cvtColor(dataMatIndex, greyMatIndex, CV_BGR2GRAY);

			//imshow("1", greyMatIndex);
			//waitKey(60);

			detector.detect(greyMatIndex, keypointsIndex);
			extractor.compute(greyMatIndex, keypointsIndex, descriptorsIndex);
			m_matches.clear();

			//Mat refDescriptors = m_descriptors.at(ulFrameIndex);
			m_pMatcher->match(descriptors, descriptorsIndex, m_matches);

			//Remove outliers
			std::vector< DMatch > good_matches;
			for (int i = 0; i < m_matches.size(); i++) {
				char str[128] = { 0 };
				Point2f query, train;
				const DMatch& match = m_matches[i];

				query = keypoints[match.queryIdx].pt;
				train = keypointsIndex[match.trainIdx].pt;

				volatile float dX = train.x - query.x;
				volatile float dY = train.y - query.y;
				double dist = sqrt(pow(dX, 2) + pow(dY, 2));

				if (dist < sqrt(8)) //2px offset
				{
					good_matches.push_back(m_matches[i]);
				}
			}

			//Mat img_matches;
			//drawMatches(greyMat, keypoints, greyMatIndex, keypointsIndex, good_matches, img_matches);
			//imshow("Matches", img_matches);
			//waitKey(60);

			char str[128] = { 0 };
			sprintf(str, "[%d] Matched: %d %f\n", ulFrameIndex, good_matches.size());
			OutputDebugString(_T(str));

			if (good_matches.size() > ulMaxMatch)
			{
				ulMaxMatch = good_matches.size();
				ulFrameIndexMaxMatch = ulFrameIndex;
			}
		}while (++ulFrameIndex < m_ulNoFrames);
	}

	char str[128] = { 0 };
	sprintf(str, "Frame Index: %d (%d) \n", ulFrameIndexMaxMatch, ulMaxMatch);
	OutputDebugString(_T(str));

	keypoints.clear();
	return ulFrameIndexMaxMatch;
}//videoIndex