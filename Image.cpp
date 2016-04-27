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
	m_DataThumbnail = NULL;
	m_pDataMat = NULL;
	m_nWidth = -1;
	m_nHeight = -1;
	m_bFeatureDet = false;
}

//Overloaded Constructor
MyImage::MyImage(int _w, int _h)
{
	m_Data = NULL;
	m_DataThumbnail = NULL;
	m_pDataMat = NULL;
	m_nWidth = _w;
	m_nHeight = _h;
	m_bFeatureDet = false;
}

MyImage::~MyImage()
{
	if (m_Data)
		delete m_Data;
	if (m_DataThumbnail)
		delete m_DataThumbnail;
	if (m_pDataMat)
		delete m_pDataMat;

	//if (m_pDetector)
	//	delete m_pDetector;
}

// Copy constructor, TODO: this is not called yet???
/*
MyImage::MyImage(MyImage *otherImage)
:
	m_bFeatureDet(false)
{
	m_nHeight = otherImage->m_nHeight;
	m_nWidth = otherImage->m_nWidth;
	m_Data = new unsigned char[m_nWidth*m_nHeight *3];

	for ( int i=0; i<(m_nHeight*m_nWidth *3); i++ )
	{
		m_Data[i] = otherImage->m_Data[i];
	}
}
*/

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
}//operator=

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

	m_pDetector = new SurfFeatureDetector(400);//threshold

	// Clean up and return
	delete Rbuf;
	delete Gbuf;
	delete Bbuf;

	return true;
}

//TODO: use bicubic, which has a natural sharpening effect.
//Emphasizing the data that remains in the new, 
//smaller image after discarding all that extra detail from the original image.
//For ease, used Bilinear Interpolation
unsigned char* MyImage::getImageThumbnailData()
{
	Mat	cvData(m_nHeight, m_nWidth, CV_8UC3, m_Data);
	Mat cvDataThumbnail(m_nHeight / SCALE, m_nWidth / SCALE, CV_8UC3, Scalar(0, 0, 0));

	if(m_DataThumbnail == NULL)
		m_DataThumbnail = new unsigned char[(m_nWidth/ SCALE)*(m_nHeight/ SCALE) * 3];

	resize(cvData, cvDataThumbnail, cvDataThumbnail.size(), INTER_LINEAR);

	memcpy(m_DataThumbnail, cvDataThumbnail.data, cvDataThumbnail.rows*cvDataThumbnail.cols * 3);

	return m_DataThumbnail;
}//getImageThumbnailData

// MyImage functions defined here
bool MyImage::WriteImage(FILE* _pImageFile, Mat& _data)
{
	// Create and populate RGB buffers
	int i;
	char *Rbuf = new char[m_nHeight*m_nWidth];
	char *Gbuf = new char[m_nHeight*m_nWidth];
	char *Bbuf = new char[m_nHeight*m_nWidth];
	unsigned char Data[480*270 * 3] = { 0 };

	memcpy(Data, _data.data, _data.rows*_data.cols * 3);

	for (i = 0; i < m_nHeight*m_nWidth; i++)
	{
		Bbuf[i] = Data[3 * i];
		Gbuf[i] = Data[3 * i + 1];
		Rbuf[i] = Data[3 * i + 2];
	}
	
	// Write data to file
	for (i = 0; i < m_nWidth*m_nHeight; i++)
	{
		fputc(Rbuf[i], _pImageFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i++)
	{
		fputc(Gbuf[i], _pImageFile);
	}
	for (i = 0; i < m_nWidth*m_nHeight; i++)
	{
		fputc(Bbuf[i], _pImageFile);
	}

	fflush(_pImageFile);
	delete Rbuf;
	delete Gbuf;
	delete Bbuf;
	
	return true;
}//WriteImage

bool MyImage::WriteImage2(FILE* _pImageFile, Mat& _data)
{
	// Create and populate RGB buffers
	unsigned char *Rbuf = new unsigned char[m_nHeight * m_nWidth];
	unsigned char *Gbuf = new unsigned char[m_nHeight * m_nWidth];
	unsigned char *Bbuf = new unsigned char[m_nHeight * m_nWidth];

	for (int i = 0; i < m_nHeight * m_nWidth; i++) {
		Rbuf[i] = _data.data[3 * i + 2];
		Gbuf[i] = _data.data[3 * i + 1];
		Bbuf[i] = _data.data[3 * i];
	}

	// Write data to file
	fwrite(Rbuf, sizeof(unsigned char), m_nHeight * m_nWidth, _pImageFile);
	fwrite(Gbuf, sizeof(unsigned char), m_nHeight * m_nWidth, _pImageFile);
	fwrite(Bbuf, sizeof(unsigned char), m_nHeight * m_nWidth, _pImageFile);

	delete Rbuf;
	delete Gbuf;
	delete Bbuf;

	return true;
}//WriteImage2

// Calculate entropy of image
// Calculation based on flattening RGB image into 1D array
double MyImage::calcEntropy()
{
	// Count number of symbols in image
	unsigned long symbolHistogram[256] = { 0 };
	for (int i = 0; i < m_nHeight * m_nWidth * 3; i++){
		unsigned char currentSymbol = m_Data[i];
		symbolHistogram[currentSymbol]++;
	}

	//Calculate entropy
	double entropy = 0;
	double numSymbols = m_nHeight * m_nWidth * 3;
	double summationValue = 0;
	double symbolProbability;
	for (int i = 0; i < 256; i++){

		//If number of occurences is 0, set sum value to 0
		//Otherwise, use entropy equation
		summationValue = 0;
		if (symbolHistogram[i] > 0) {
			symbolProbability = symbolHistogram[i] / numSymbols;
			summationValue = symbolProbability * log2(symbolProbability);
		}

		entropy += summationValue;
	}

	entropy = -entropy;

	return entropy;
}//calcEntropy

// Calculates temporal difference using template matching
// Euclidean distance between 2 pixels
double MyImage::templateMatchDifference(MyImage &previousFrame) 
{
	unsigned char* previousFrameData = previousFrame.getImageData();
	double differenceSum = 0;

	for (int i = 0; i < m_nHeight * m_nWidth; i++) {
		differenceSum += sqrt(pow(m_Data[i] - previousFrameData[i], 2.0)
							+ pow(m_Data[i + 1] - previousFrameData[i + 1], 2.0)
							+ pow(m_Data[i + 2] - previousFrameData[i + 2], 2.0));
	}

	return differenceSum;
}//templateMatchDifference

// Calculates temporal difference using color histograms
int MyImage::colorHistogramDifference(MyImage &previousFrame) 
{
	unsigned char* previousFrameData = previousFrame.getImageData();

	//Create histograms for each R,G,B value for both images
	long PrevHist_B[256] = { 0 };
	long PrevHist_G[256] = { 0 };
	long PrevHist_R[256] = { 0 };
	long CurrHist_B[256] = { 0 };
	long CurrHist_G[256] = { 0 };
	long CurrHist_R[256] = { 0 };

	for (int i = 0; i < m_nHeight * m_nWidth; i++) {
		PrevHist_B[previousFrameData[i]]++;
		PrevHist_G[previousFrameData[i + 1]]++;
		PrevHist_R[previousFrameData[i + 2]]++;

		CurrHist_B[m_Data[i]]++;
		CurrHist_G[m_Data[i + 1]]++;
		CurrHist_R[m_Data[i + 2]]++;
	}

	//Get difference sum between histograms
	int differenceSum = 0;
	for (int i = 0; i < 256; i++) {
		differenceSum += (abs(PrevHist_B[i] - CurrHist_B[i])
						+ abs(PrevHist_G[i] - CurrHist_G[i])
						+ abs(PrevHist_R[i] - CurrHist_R[i]));
	}

	return differenceSum;
}

// Calculates temporal difference using X Squared calculation
double MyImage::xSquaredHistogramDifference(MyImage &previousFrame) 
{
	unsigned char* previousFrameData = previousFrame.getImageData();

	//Create histograms for each R,G,B value for both images
	long PrevHist_B[256] = { 0 };
	long PrevHist_G[256] = { 0 };
	long PrevHist_R[256] = { 0 };
	long CurrHist_B[256] = { 0 };
	long CurrHist_G[256] = { 0 };
	long CurrHist_R[256] = { 0 };

	for (int i = 0; i < m_nHeight * m_nWidth; i++) {
		PrevHist_B[previousFrameData[i]]++;
		PrevHist_G[previousFrameData[i + 1]]++;
		PrevHist_R[previousFrameData[i + 2]]++;

		CurrHist_B[m_Data[i]]++;
		CurrHist_G[m_Data[i + 1]]++;
		CurrHist_R[m_Data[i + 2]]++;
	}

	//Get difference sum between histograms
	double differenceSum = 0;
	for (int i = 0; i < 256; i++) {
		double summationValue = 0;
		if (CurrHist_B[i] + CurrHist_G[i] + CurrHist_R[i] > 0) {
			summationValue = pow((PrevHist_B[i] - CurrHist_B[i])
				+ abs(PrevHist_G[i] - CurrHist_G[i])
				+ abs(PrevHist_R[i] - CurrHist_R[i]), 2.0)
				/ (CurrHist_B[i] + CurrHist_G[i] + CurrHist_R[i]);
		}

		differenceSum += summationValue;
	}

	return differenceSum;
}

//Using OpenCV, compute FAST features
void MyImage::featuresDetec(Mat &_dataMat, vector<KeyPoint> &_keypoints)
{
	if (!_dataMat.empty())
	{
		Mat mask = Mat::zeros(_dataMat.size(), _dataMat.type());
		// select a ROI
		Mat regionOfInterest(mask, Rect(0, 0, 240, 135));

		// fill the ROI with (255, 255, 255) (which is white in RGB space);
		// the original image will be modified
		regionOfInterest = Scalar(255, 255, 255);

		m_pDetector->detect(_dataMat, _keypoints, mask);
	}
}//siftFeatures