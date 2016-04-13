//*****************************************************************************
//
// Image.cpp : Defines the class operations on images
//
// Author - Parag Havaldar
// Code used by students as starter code to display and modify images
//
//*****************************************************************************
#include "Image.h"

// Constructor and Desctructors
MyImage::MyImage() 
{
	m_Data = NULL;
	m_pDataMat = NULL;
	m_nWidth = -1;
	m_nHeight = -1;
	m_bFeatureDet = false;
}

MyImage::~MyImage()
{
	if (m_Data)
		delete m_Data;
	if (m_pDataMat)
		delete m_pDataMat;
}

// Copy constructor, TODO: this is not called yet???
MyImage::MyImage(MyImage *otherImage)
:
	m_bFeatureDet(false)
{
	m_nHeight = otherImage->m_nHeight;
	m_nWidth = otherImage->m_nWidth;
	m_Data = new unsigned char[m_nWidth*m_nHeight *3];

	for ( int i=0; i<(m_nHeight*m_nWidth *3); i++ )
	{
		m_Data[i]	= otherImage->m_Data[i];
	}
}

// = operator overload
MyImage& MyImage::operator= (const MyImage &otherImage)
{
	m_nHeight = otherImage.m_nHeight;
	m_nWidth = otherImage.m_nWidth;

	if (m_Data == NULL)
		m_Data = new unsigned char[m_nWidth*m_nHeight * 3];

	for (int i = 0; i < (m_nHeight*m_nWidth * 3); i++)
	{
		m_Data[i] = otherImage.m_Data[i];
	}

	return *this;
}

// MyImage::ReadImage
// Function to read the image given a path
bool MyImage::ReadImage(FILE* _inFile, unsigned int _nFrameNo)
{
	int i;
	// Verify ImagePath
	if (m_nWidth < 0 || m_nHeight < 0)
	{
		fprintf(stderr, "Image or Image properties not defined");
		return false;
	}
	
	// Create and populate RGB buffers
	unsigned char *Rbuf = new unsigned char[m_nHeight*m_nWidth];
	unsigned char *Gbuf = new unsigned char[m_nHeight*m_nWidth];
	unsigned char *Bbuf = new unsigned char[m_nHeight*m_nWidth];

	fseek(_inFile, _nFrameNo*m_nHeight*m_nWidth*3, SEEK_SET);

	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		Rbuf[i] = fgetc(_inFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		Gbuf[i] = fgetc(_inFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		Bbuf[i] = fgetc(_inFile);
	}
	
	// Allocate Data structure and copy
	if(m_Data == NULL)
		m_Data = new unsigned char[m_nWidth*m_nHeight *3];

	for (i = 0; i < m_nHeight*m_nWidth; i++)
	{
		m_Data[3*i]	= Bbuf[i];
		m_Data[3*i+1] = Gbuf[i];
		m_Data[3*i+2] = Rbuf[i];
	}

	// Clean up and return
	delete Rbuf;
	delete Gbuf;
	delete Bbuf;

	return true;
}

// MyImage functions defined here
bool MyImage::WriteImage(FILE* _pImageFile)
{
	// Create and populate RGB buffers
	int i;
	unsigned char *Rbuf = new unsigned char[m_nHeight*m_nWidth];
	unsigned char *Gbuf = new unsigned char[m_nHeight*m_nWidth];
	unsigned char *Bbuf = new unsigned char[m_nHeight*m_nWidth];

	for (i = 0; i < m_nHeight*m_nWidth; i++)
	{
		Bbuf[i] = m_Data[3*i];
		Gbuf[i] = m_Data[3*i+1];
		Rbuf[i] = m_Data[3*i+2];
	}

	// Write data to file
	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		fputc(Rbuf[i], _pImageFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		fputc(Gbuf[i], _pImageFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i ++)
	{
		fputc(Bbuf[i], _pImageFile);
	}
	
	// Clean up and return
	delete Rbuf;
	delete Gbuf;
	delete Bbuf;

	return true;
}


// Here is where you would place your code to modify an image
// eg Filtering, Transformation, Cropping, etc.
bool MyImage::Modify()
{
	siftFeaturesDetec();
	//calcEntropy();
	return false;
}

// Calculate entropy of image
// Calculation based on flattening RGB image into 1D array
double MyImage::calcEntropy()
{
	// Count number of symbols in image
	int symbolHistogram[256] = { 0 };
	for (int i = 0; i < (m_nHeight*m_nWidth * 3); i++)
	{
		unsigned char currentSymbol = m_Data[i];
		symbolHistogram[currentSymbol]++;
	}

	//Calculate entropy
	double entropy = 0;
	double numSymbols = m_nHeight * m_nWidth * 3;

	for (int i = 0; i < 256; i++){
		double symbolProbability = symbolHistogram[i] / numSymbols;
		entropy += (symbolProbability * log2(symbolProbability));
	}

	entropy = -entropy;

	return entropy;
}//calcEntropy

//Using OpenCV, compute SIFT features
void MyImage::siftFeaturesDetec()
{
	Mat	dataMat(m_nHeight, m_nWidth, CV_8UC3, m_Data); // Open CV data matrix

	if (!dataMat.empty())
	{
		//namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("Display window", dataMat);
		m_detectorCurr.detect(dataMat, m_keypoints);
		drawKeypoints(dataMat, m_keypoints, dataMat);
		//waitKey(0);
		//destroyWindow("Display window");
	}
}//siftFeatures

//Brute-Force Matching (Hamming)
void MyImage::featuresMatch(Mat _framePrev, Mat _framCurr)
{
	BFMatcher matcher(NORM_HAMMING);
	vector<DMatch> matches;

	//match(kpts_1, kpts_2, matcher, _framePrev, _framCurr, matches);
}//featuresMatch

void MyImage::outlierRejection()
{

}//outlierRejection

void MyImage::calcHomography()
{
	//Mat H = findHomography(mpts_2, mpts_1, RANSAC, 1, outlier_mask);
}//calcHomography

void MyImage::frameWarping()
{

}//frameWarping