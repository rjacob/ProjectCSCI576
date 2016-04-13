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
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>


// Class structure of Image 
// Use to encapsulate an RGB image
using namespace cv;

class MyImage 
{

private:
	int		m_nWidth;					// Width of Image
	int		m_nHeight;					// Height of Image
	char*	m_Data;					// RGB data of the image
	bool	m_bFeatureDet;
	std::vector<KeyPoint> m_keypoints;
	FastFeatureDetector m_detector;

public:
	// Constructor
	MyImage();
	// Copy Constructor
	MyImage::MyImage( MyImage *otherImage);
	// Destructor
	~MyImage();

	// operator overload
	MyImage & operator= (const MyImage & otherImage);

	// Reader & Writer functions
	void	setWidth( const int w)  { m_nWidth = w; }; 
	void	setHeight(const int h) { m_nHeight = h; };
	void	setImageData( const char *img ) { m_Data = (char *)img; };
	int		getWidth() { return m_nWidth; };
	int		getHeight() { return m_nHeight; };
	char*	getImageData() { return m_Data; };

	// Input Output operations
	bool	ReadImage(FILE*, unsigned int);
	bool	WriteImage(FILE*);

	// Modifications
	bool	Modify();

	bool	analyze();
	
	// Calculations
	double	calcEntropy();
	void	siftFeatures();

};

#endif //IMAGE_DISPLAY
