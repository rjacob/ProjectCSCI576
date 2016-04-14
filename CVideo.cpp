#include "CVideo.h"

/*************************************
* Function: Constructor
* Description: 
*************************************/
CVideo::CVideo()
	:
	m_ulNoFrames(0),
	m_pOutputFrame(NULL),
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
	m_pOutputFrame(NULL),
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
			copyVideoFrame(*m_pCurrentFrame, m_ulCurrentFrameIndex++);
			m_pCurrentFrame->Modify();

			if(m_pOutputFrame)
				*m_pOutputFrame = *m_pCurrentFrame;
		}
		Sleep(1000 / 15);//15Hz TODO: consider time it takes to readVideoFrame
	} while(m_eThreadState != THREAD_STATE_KILLED && m_ulCurrentFrameIndex < m_ulNoFrames);

	m_ulCurrentFrameIndex = 0;
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

/*************************************
* Function: pauseVideo
* Description:
*************************************/
bool CVideo::analyzeVideo()
{
	//Analyze All frames
	videoSummarization();
	m_unVideoDurationSubSec = 1234;
	return true;

	vector<KeyPoint> keypointsCurr, keypointsPrev;

	Mat	dataMatCurrent(m_unHeight, m_unWidth, CV_8UC3, m_pCurrentFrame->getImageData()); // Open CV data matrixs
	Mat dataMatPrev;

	m_pCurrentFrame->siftFeaturesDetec(dataMatCurrent, keypointsCurr);
	featuresMatch(dataMatCurrent, dataMatPrev);
	outlierRejection();
	//calcHomography(dataMatCurrent, dataMatPrev);

	return false;
}

 /*************************************
 * Function: videoSummarization
 * Description:
 *************************************/
bool CVideo::videoSummarization()
{
	MyImage currentFrame;
	currentFrame.setWidth(m_unWidth);
	currentFrame.setHeight(m_unHeight);

	//Cycle through each frame in video
	//Testing output write
	//Apply RGB histogram, entropy here
	for (unsigned long i = 0; i < m_ulNoFrames; i++) {
		currentFrame.ReadImage(m_pFile, i);
		FILE* outImageFile;
		char imagePath[_MAX_PATH] = "Test.rgb";
		outImageFile = fopen(imagePath, "wb");

		if (outImageFile = NULL) {
			fprintf(stderr, "Error Opening File for Output Write");
			return false;
		}

		currentFrame.WriteImage(outImageFile);

		fclose(outImageFile);
	}

	//Create array of "I-frames" here

	//Choose best "GOPs" here

	//Output summarized video (or "I-frame" array) here

	return true;
}//videoSummarization

//Brute-Force Matching (Hamming)
void CVideo::featuresMatch(Mat _framePrev, Mat _framCurr)
{
	BFMatcher matcher(NORM_HAMMING);
	vector<DMatch> matches;

	matcher.match(_framePrev, _framCurr, matches);
}//featuresMatch

void CVideo::outlierRejection()
{

}//outlierRejection

void CVideo::calcHomography(Mat& _matCurr, vector<KeyPoint>& _keyPtCurr, Mat& _matPrev, vector<KeyPoint>& _keyPtPrev)
{
	//Descriptor Extraction
	Mat descriptorPrev, descriptorCurr;
	SiftDescriptorExtractor extractor;

	extractor.compute(_matPrev, _keyPtPrev, descriptorPrev);
	extractor.compute(_matCurr, _keyPtCurr, descriptorCurr);

	//Matching descriptor vectors using FLANN matcher

	//Quick calculation of max and min distances between keypoints
	std::vector<DMatch> matches;
	double max_dist = 0; double min_dist = 100;

	for (int i = 0; i < descriptorPrev.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//Mat H = findHomography(mpts_2, mpts_1, RANSAC, 1, outlier_mask);
}//calcHomography

 //Transformation (Rotation or a Projection)
void CVideo::frameWarping()
{

}//frameWarping
