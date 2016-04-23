//*****************************************************************************
//
// Image.h : Defines the class operations on images
//
// Author - Parag Havaldar
// Main Image class structure 
//
//*****************************************************************************

#ifndef IMAGE_DISPLAY
#define IMAGE_DISPLAY

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "resource.h"
//#include "afxwin.h"

// C RunTime Header Files
#include <windows.h>//REMOVE
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cmath>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"

// Class structure of Image 
// Use to encapsulate an RGB image
using namespace cv;
class MyImage 
{

private:
	int		m_nWidth;					// Width of Image
	int		m_nHeight;					// Height of Image
	unsigned char*	m_Data;					// RGB data of the image
	Mat*	m_pDataMat;				// Open CV data matrix
	bool	m_bFeatureDet;
	SurfFeatureDetector  *m_pDetector;//For real-time processing

public:
	// Constructor
	MyImage();
	// Copy Constructor
	//MyImage::MyImage( MyImage *otherImage);
	// Destructor
	~MyImage();

	// operator overload
	MyImage & operator= (const MyImage & otherImage);

	// Reader & Writer functions
	void	setWidth( const int w) { m_nWidth = w; }; 
	void	setHeight(const int h) { m_nHeight = h; };
	int		getWidth() const { return m_nWidth; };
	int		getHeight() const { return m_nHeight; };
	unsigned char*	getImageData() { return m_Data; };

	// Input Output operations
	bool	ReadImage(FILE*, unsigned int);
	bool	WriteImage(FILE*, Mat&);

	// Modifications
	bool	Modify();
	
	// Calculations
	void	countSymbols();
	double	calcEntropy();
	double	templateMatchDifference(MyImage &previousFrame);
	int		colorHistogramDifference(MyImage &previousFrame);
	double	xSquaredHistogramDifference(MyImage &previousFrame);
	void	featuresDetec(Mat&, vector<KeyPoint>&);
	void	frameWarping();
};

#endif //IMAGE_DISPLAY
