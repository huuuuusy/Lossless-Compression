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
// motionEstimation.c/motionEstimation.h - Module that provides functions for Motion Estimation

#ifndef MOTIONESTIMATION_YUV420_H
#define MOTIONESTIMATION_YUV420_H

#include "frame_yuv420.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ME_FS	0x00
#define	ME_HFS	0x01
#define ME_RCS	0x02

typedef struct {
	int width, height ;					//	Width and Height of the input image
	int searchRange ;
	int mode ;
	int nMBx ;
	int nMBy ;
	FRAME_YUV420 ref ;
	FRAME_YUV420 hRef ;
	FRAME_YUV420 qRef ;
	FRAME_YUV420 hCur ;
	FRAME_YUV420 qCur ;
	short (*pMVs)[2] ;
	short (*hMVs)[2] ;
	short (*qMVs)[2] ;
} MotionEstimationHandle ;

void meSetReference(MotionEstimationHandle *hME, FRAME_YUV420 *pRef) ;
MotionEstimationHandle *motionEstimationInit(int width, int height, int searchRange, int mode) ;
void motionEstimationFinal(MotionEstimationHandle *hME) ;
void motionEstimation(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int meAlg, int mvCost) ;
void motionCompensation(MotionEstimationHandle *hME, FRAME_YUV420 *pCon) ;

void meFSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) ;
void meHierFullSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) ;
void meRCSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) ;

#ifdef __cplusplus
}
#endif

#endif