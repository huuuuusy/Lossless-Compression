#pragma once
#include "../processing.h"
#include "../Tools/ImageConvert.h"
#include "../Tools/motionEstimation_yuv420.h"

class CAppME :
	public CProcessing
{
public:
	// Add variables here

	FRAME_YUV420 cur, recon ;
	MotionEstimationHandle *hME[2] ;

	unsigned char *pRefRGB ;
	unsigned char *pHRefRGB ;
	unsigned char *pQRefRGB ;

	int frameNum ;

public:
	CAppME(void);
	~CAppME(void);
	// Add methods here

	unsigned char *NewFrame(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) ;
	void FreeFrame(FRAME_YUV420 *pFrame) ;

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
