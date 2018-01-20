#include "StdAfx.h"
#include "AppVideoEnhance.h"
#include "../Tools/videoEnhance.h"
#include "../Tools/ImageConvert.h"

CAppVideoEnhance::CAppVideoEnhance(void)
{
	// Class Constructor
}

CAppVideoEnhance::~CAppVideoEnhance(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppVideoEnhance::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
	frameCurYUV = new unsigned char[width * height * 2] ;
	frameOutYUV = new unsigned char[width * height * 2] ;
	frameCurRGBA = new unsigned char[width * height * 4] ;
	frameOutRGBA = new unsigned char[width * height * 4] ;
}

void CAppVideoEnhance::CustomFinal(void) {
	// Add custom finalization code here
	delete [] frameCurRGBA ;
	delete [] frameOutRGBA ;
	delete [] frameCurYUV ;
	delete [] frameOutYUV ;
}

void CAppVideoEnhance::Process(void) {

	CImageConvert imgConvert ;

	GetSrc() ;
	imgConvert.RGB24_to_YUV420(pInput, frameCurYUV, width, height) ;
	imgConvert.RGB24_to_RGBA(pInput, frameCurRGBA, width, height) ;

	videoEnhance_yuv(frameCurYUV, frameOutYUV, width, height) ;
	videoEnhance_rgba(frameCurRGBA, frameOutRGBA, width, height) ;

//	imgConvert.YUV420_to_RGB24(frameOutYUV, pOutput, width, height) ;
	imgConvert.RGBA_to_RGB24(frameOutRGBA, pOutput, width, height) ;

	PutDC(pOutput) ;
}
