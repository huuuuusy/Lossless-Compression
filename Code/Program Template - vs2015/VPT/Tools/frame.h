//	===========================================================
//
//	Copyright 2006-2008 by CyberView Inc. All rights reserved.
//
//	It is prohibited to disclose or redistribute any portion
//	of the code in this file without prior permission from
//	CyberView Inc.
//
//	===========================================================
//
// frame.c/frame.h - Module that provides functions for Image Frame Manipulations

#ifndef FRAME_H
#define FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TYPE_FRAME_YUV420
#define TYPE_FRAME_YUV420

// Video Frame Structure for use with camMotion, videoStab and videoDeinterlace modules
typedef struct {
	unsigned char *Y ;					// Y-Plane pointer
	unsigned char *U ;					// U-Plane pointer
	unsigned char *V ;					// V-Plane pointer
	int xOff ;
	int yOff ;
	int width ;
	int height ;
	int strideY ;						// Stride for Y-Plane
	int strideUV ;						// Stride for UV-Plane
} FRAME_YUV420 ;

#endif // TYPE_FRAME_YUV420

void downSamplingFrameByTwo(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest, int hPad, int vPad) ;
void copyFrame420(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest) ;
void borderExtendFrame420(FRAME_YUV420 *pCur) ;
void blur3x3Frame420_Y(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest) ;

unsigned char *newFrame420(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) ;
void freeFrame420(FRAME_YUV420 *pFrame) ;

unsigned char *getActiveFrame420_Y(FRAME_YUV420 *pCur) ;
unsigned char *getActiveFrame420_U(FRAME_YUV420 *pCur) ;
unsigned char *getActiveFrame420_V(FRAME_YUV420 *pCur) ;
void getPSNRFrame420(FRAME_YUV420 *pCur, FRAME_YUV420 *pRef, double *psnrY, double *psnrU, double *psnrV) ;

#ifdef __cplusplus
}
#endif

#endif	// FRAME_H