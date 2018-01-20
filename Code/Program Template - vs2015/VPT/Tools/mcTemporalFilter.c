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

#include "mcTemporalFilter.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

MCTFHandle *mcTemporalFilterInit(int width, int height, int strength, int searchRange) {

	MCTFHandle *hMCTF ;

	hMCTF = (MCTFHandle *) malloc(sizeof(MCTFHandle)) ;

	if(!hMCTF) return NULL ;

	hMCTF->width = width ;
	hMCTF->height = height ;
	hMCTF->searchRange = searchRange ;
	hMCTF->strength = strength ;
	hMCTF->hME = motionEstimationInit(width, height, searchRange, 0) ;
	hMCTF->refInit = 0 ;
	hMCTF->noiseLevel = 0 ;

	if(!hMCTF->hME) {
		free(hMCTF) ;

		return NULL ;
	}

	newFrame420(&hMCTF->blurredCur, width, height, 0, 0) ;
	newFrame420(&hMCTF->filtered, width, height, 0, 0) ;
	newFrame420(&hMCTF->recon, width, height, 0, 0) ;

	return hMCTF ;
}

void mcTemporalFilterFinal(MCTFHandle *hMCTF) {

	motionEstimationFinal(hMCTF->hME) ;
	freeFrame420(&hMCTF->blurredCur) ;
	freeFrame420(&hMCTF->filtered) ;
	freeFrame420(&hMCTF->recon) ;
	free(hMCTF) ;
}

void mcTemporalFilter(MCTFHandle *hMCTF, FRAME_YUV420 *pCur, FRAME_YUV420 *pOut) {

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MED(a, b, c) MIN(MIN(MAX(a, b), MAX(b, c)), MAX(a, c))

	MotionEstimationHandle *hME ;

	hME = hMCTF->hME ;

	blur3x3Frame420_Y(pCur, &hMCTF->blurredCur) ;

	if(!hMCTF->refInit) {
		meSetReference(hME, pCur) ;
	}
	motionEstimation(hME, pCur, ME_RCS, 1) ;

//	copyFrame420(pCur, &hMCTF->filtered) ;
//	memset(hME->pMVs, 0, sizeof(short) * hMCTF->width / 16 * hMCTF->height / 16 * 2) ;

	motionCompensation(hME, &hMCTF->recon) ;

	{
		int i, j ;
		int diff, sumDiff ;
		int diff1, diff2, diff3 ;
		unsigned char *pCurY, *pReconY, *pFilteredY ;
		unsigned char *pCurU, *pReconU, *pFilteredU ;
		unsigned char *pCurV, *pReconV, *pFilteredV ;
		int alpha ;
		int beta ;
		int mvx, mvy ;
		int noiseLevel ;
		int noiseCount ;

		pCurY = getActiveFrame420_Y(pCur) ;
		pReconY = getActiveFrame420_Y(&hMCTF->recon) ;
		pFilteredY = getActiveFrame420_Y(&hMCTF->filtered) ;

		noiseLevel = 0 ;
		noiseCount = 0 ;

		for(j = 0; j < hMCTF->height; j++) {
			for(i = 0; i < hMCTF->width; i++) {

				mvx = hME->pMVs[i / 16 + j / 16 * hME->nMBx][0] ;
				mvy = hME->pMVs[i / 16 + j / 16 * hME->nMBx][1] ;

				mvx = mvx >= 0 ? mvx : -mvx ;
				mvy = mvy >= 0 ? mvy : -mvy ;

				diff = pReconY[i + j * hMCTF->recon.strideY] - hMCTF->blurredCur.Y[i + j * hMCTF->blurredCur.strideY] ;
//				diff = pReconY[i + j * hMCTF->recon.strideY] - pCurY[i + j * pCur->strideY] ;
				if(diff < 0) diff = -diff ;
				diff = diff + (32 - hMCTF->noiseLevel) ;

				if(hMCTF->strength) {
					beta = (mvx + mvy) * 512 / (2 * hME->searchRange) ;
					alpha = (diff * diff) * 512 / (diff * diff + hMCTF->strength) ;
					alpha = beta + alpha ;
					if(alpha > 512) alpha = 512 ;
					if(alpha < 0) alpha = 0 ;
				} else {
					alpha = 512 ;
					beta = 0 ;
				}

				pFilteredY[i + j * hMCTF->filtered.strideY] = (pReconY[i + j * hMCTF->recon.strideY] * (512 - alpha) + pCurY[i + j * pCur->strideY] * alpha) / 512 ;
				diff1 = pCurY[i + j * pCur->strideY] - pReconY[i + j * hMCTF->recon.strideY] ;
				diff2 = pCurY[i + j * pCur->strideY] - hMCTF->blurredCur.Y[i + j * hMCTF->blurredCur.strideY] ;
				diff3 = pCurY[i + j * pCur->strideY] - pFilteredY[i + j * hMCTF->filtered.strideY] ;

				if(diff1 < 0) diff1 = -diff1 ;
				if(diff2 < 0) diff2 = -diff2 ;
				if(diff3 < 0) diff3 = -diff3 ;

				/*
				if(diff1 < diff2) diff = diff1 ;
				else diff = diff2 ;
				*/

				diff = MED(diff1, diff2, diff3) ;
				//diff = MIN(MIN(diff1, diff2), diff3) ;

				noiseLevel += diff * diff ;
				noiseCount++ ;
			}
		}

		if(noiseCount) {
			hMCTF->noiseLevel = noiseLevel / noiseCount ;
			if(hMCTF->noiseLevel > 32) hMCTF->noiseLevel = 32 ;
//			hMCTF->noiseLevel = 0 ;
		}


		pCurU = getActiveFrame420_U(pCur) ;
		pReconU = getActiveFrame420_U(&hMCTF->recon) ;
		pFilteredU = getActiveFrame420_U(&hMCTF->filtered) ;
		pCurV = getActiveFrame420_V(pCur) ;
		pReconV = getActiveFrame420_V(&hMCTF->recon) ;
		pFilteredV = getActiveFrame420_V(&hMCTF->filtered) ;

		for(j = 0; j < hMCTF->height / 2; j++) {
			for(i = 0; i < hMCTF->width / 2; i++) {

				mvx = hME->pMVs[i / 8 + j / 8 * hME->nMBx][0] ;
				mvy = hME->pMVs[i / 8 + j / 8 * hME->nMBx][1] ;

				mvx = mvx >= 0 ? mvx : -mvx ;
				mvy = mvy >= 0 ? mvy : -mvy ;

				sumDiff = 0 ;
				diff = pReconY[2 * i + 2 * j * hMCTF->recon.strideY] - hMCTF->blurredCur.Y[2 * i + 2 * j * hMCTF->blurredCur.strideY] ;
				if(diff < 0) diff = -diff ;
				sumDiff += diff ;

				diff = pReconY[2 * i + 1 + 2 * j * hMCTF->recon.strideY] - hMCTF->blurredCur.Y[2 * i + 1 + 2 * j * hMCTF->blurredCur.strideY] ;
				if(diff < 0) diff = -diff ;
				sumDiff += diff ;

				diff = pReconY[2 * i + 1 + (2 * j + 1) * hMCTF->recon.strideY] - hMCTF->blurredCur.Y[2 * i + 1 + (2 * j + 1) * hMCTF->blurredCur.strideY] ;
				if(diff < 0) diff = -diff ;
				sumDiff += diff ;

				diff = pReconY[2 * i + (2 * j + 1) * hMCTF->recon.strideY] - hMCTF->blurredCur.Y[2 * i + (2 * j + 1) * hMCTF->blurredCur.strideY] ;
				if(diff < 0) diff = -diff ;
				sumDiff += diff ;

				diff = sumDiff >> 2 ;
				diff = diff + (32 - hMCTF->noiseLevel) ;

				if(hMCTF->strength) {
					beta = (mvx + mvy) * 512 / (2 * hME->searchRange) ;
					alpha = (diff * diff) * 512 / (diff * diff + hMCTF->strength + hMCTF->noiseLevel) ;
					alpha = beta + alpha ;
					if(alpha > 512) alpha = 512 ;
					if(alpha < 0) alpha = 0 ;
				} else {
					alpha = 512 ;
					beta = 0 ;
				}

				pFilteredU[i + j * hMCTF->filtered.strideUV] = (pReconU[i + j * hMCTF->recon.strideUV] * (512 - alpha) + pCurU[i + j * pCur->strideUV] * alpha) / 512 ;
				pFilteredV[i + j * hMCTF->filtered.strideUV] = (pReconV[i + j * hMCTF->recon.strideUV] * (512 - alpha) + pCurV[i + j * pCur->strideUV] * alpha) / 512 ;
			}
		}
	}

	copyFrame420(&hMCTF->filtered, pOut) ;
	meSetReference(hME, &hMCTF->filtered) ;

	hMCTF->refInit = 1 ;

#undef MIN
#undef MAX
#undef MED
}

#ifdef __cplusplus
}
#endif
