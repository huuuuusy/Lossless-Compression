#pragma once

#include "../processing.h"
#include "../Tools/frame_yuv420.h"
#include "../Tools/ImageConvert.h"
#include "../Tools/motionEstimation_yuv420.h"
#include "../Tools/NoiseGenerator.h"
#include "../Tools/mcTemporalFilter.h"

class CAppMCTF :
	public CProcessing
{
public:
	// Add variables here

	FRAME_YUV420 cur, noisy, filtered ;
	FRAME_YUV420 buf[3] ;
	FRAME_YUV420 *pBuf[3] ;
	MCTFHandle *hMCTF ;
	unsigned char *pFilteredRGB ;
	CNoiseGenerator *pNoiseGen ;
	unsigned char *pNoisyInput ;

	int frameNum ;

public:
	CAppMCTF(void);
	~CAppMCTF(void);
	// Add methods here

	unsigned char *NewFrame(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) ;
	void FreeFrame(FRAME_YUV420 *pFrame) ;

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
