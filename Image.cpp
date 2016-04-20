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

	m_pDetector = new SurfFeatureDetector(400);//threshold

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
}//WriteImage


// Here is where you would place your code to modify an image
// eg Filtering, Transformation, Cropping, etc.
bool MyImage::Modify()
{
	//calcEntropy();
	frameWarping();
	return false;
}//Modify

void MyImage::countSymbols()
{

}//countSymbols

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
		m_pDetector->detect(_dataMat, _keypoints);
	}
}//siftFeatures

 //Transformation (Rotation or a Projection)
void MyImage::frameWarping()
{

}//frameWarping
