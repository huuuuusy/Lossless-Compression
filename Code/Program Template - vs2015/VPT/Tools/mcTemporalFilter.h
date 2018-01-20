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
// mcTemporalFiltering.c/mcTemporalFiltering.h - Module that provides functions for Motion Compensated Temporal Filtering

#ifndef MCTF_H
#define MCTF_H

#include "./motionEstimation_yuv420.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int width, height ;					//	Width and Height of the input image
	int searchRange ;
	int strength ;
	int refInit ;
	int noiseLevel ;
	FRAME_YUV420 blurredCur ;
	FRAME_YUV420 filtered ;
	FRAME_YUV420 recon ;
	MotionEstimationHandle *hME ;
} MCTFHandle ;

MCTFHandle *mcTemporalFilterInit(int width, int height, int strength, int searchRange) ;
void mcTemporalFilterFinal(MCTFHandle *hMCTF) ;
void mcTemporalFilter(MCTFHandle *hMCTF, FRAME_YUV420 *pCur, FRAME_YUV420 *pOut) ;

#ifdef __cplusplus
}
#endif

#endif	// MCTF_H