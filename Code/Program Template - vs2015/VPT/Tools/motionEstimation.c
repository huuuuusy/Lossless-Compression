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

#include "motionEstimation.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *meNewFrame(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) {

	int frameWidth ;
	int frameHeight ;

	frameWidth = hPad * 2 + width ;
	frameHeight = vPad * 2 + height ;

	pFrame->strideY = frameWidth ;
	pFrame->strideUV = pFrame->strideY >> 1 ;

	pFrame->Y = (unsigned char *) malloc(frameWidth * frameHeight + frameWidth * frameHeight / 2) ;
	pFrame->U = pFrame->Y + frameWidth * frameHeight ;
	pFrame->V = pFrame->U + frameWidth * frameHeight / 4 ;

	pFrame->xOff = hPad ;
	pFrame->yOff = vPad ;

	pFrame->width = width ;
	pFrame->height = height ;

	return pFrame->Y ;
}

void meFreeFrame(FRAME_YUV420 *pFrame) {

	if(pFrame->Y) {
		free(pFrame->Y) ;
		pFrame->Y = NULL ;
		pFrame->U = NULL ;
		pFrame->V = NULL ;
	}
}

MotionEstimationHandle *motionEstimationInit(int width, int height, int searchRange, int mode) {

	MotionEstimationHandle *hME ;
	int hPad, vPad ;

	hME = (MotionEstimationHandle *) malloc(sizeof(MotionEstimationHandle)) ;
	if(!hME) return NULL ;

	hME->pMVs = (short (*)[2]) malloc((width * height >> 4) * sizeof(short) * 2) ;
	hME->hMVs = (short (*)[2]) malloc((width * height >> 4) * sizeof(short) * 2) ;
	hME->qMVs = (short (*)[2]) malloc((width * height >> 4) * sizeof(short) * 2) ;

	hME->width = width ;
	hME->height = height ;
	hME->searchRange = searchRange ;
	hME->mode = mode ;

	hME->nMBx = width >> 4 ;
	hME->nMBy = height >> 4 ;

	vPad = hPad = ((searchRange + 15) / 16) * 16 ;
	meNewFrame(&hME->ref, width, height, hPad, vPad) ;
	if(hME->mode) {
		meNewFrame(&hME->hRef, width / 2, height / 2, hPad / 2, vPad / 2) ;
		meNewFrame(&hME->qRef, width / 4, height / 4, hPad / 4, vPad / 4) ;
		meNewFrame(&hME->hCur, width / 2, height / 2, 0, 0) ;
		meNewFrame(&hME->qCur, width / 4, height / 4, 0, 0) ;
	}

	return hME ;
}

void motionEstimationFinal(MotionEstimationHandle *hME) {

	if(hME) {
		free(hME->pMVs) ;
		free(hME->hMVs) ;
		free(hME->qMVs) ;
		meFreeFrame(&hME->ref) ;
		if(hME->mode) {
			meFreeFrame(&hME->hRef) ;
			meFreeFrame(&hME->qRef) ;
			meFreeFrame(&hME->hCur) ;
			meFreeFrame(&hME->qCur) ;
		}
		free(hME) ;
	}
}

int memSAD(unsigned char *cur, unsigned char *ref, int curStride, int refStride, int blockSize, int bsShift) {

	int i, j ;
	int diff ;
	int sum ;

	sum = 0 ;

	for(j = 0; j < blockSize; j++) {
		for(i = 0; i < blockSize; i++) {
			diff = cur[i + j * curStride] - ref[(i >> bsShift) + (j >> bsShift) * refStride] ;
			sum = sum + (diff >= 0 ? diff : -diff) ;
		}
	}

	return sum ;
}

int meSAD(unsigned char *cur, unsigned char *ref, int curStride, int refStride, int blockSize) {

	int i, j ;
	int diff ;
	int sum ;

	sum = 0 ;

	for(j = 0; j < blockSize; j++) {
		for(i = 0; i < blockSize; i++) {
			diff = cur[i + j * curStride] - ref[i + j * refStride] ;
			sum = sum + (diff >= 0 ? diff : -diff) ;
		}
	}

	return sum ;
}

int meMbRCSearch(unsigned char *cur, unsigned char *ref, int curStride, int refStride,
				 int blockSize, int minX, int maxX, int minY, int maxY, short *mv, int mvCost) {
	
	int k ;
	int i, j ;
	int stepSize, stepMax ;
	int absMinX, absMaxX, absMinY, absMaxY ;
	int maxStepX, maxStepY ;
	int cost, minCost ;
	int sad, minSad ;
	int minK ;
	int ci, cj ;

	absMinX = minX >= 0 ? minX : -minX ;
	absMaxX = maxX >= 0 ? maxX : -maxX ;
	absMinY = minY >= 0 ? minY : -minY ;
	absMaxY = maxY >= 0 ? maxY : -maxY ;

	maxStepX = absMinX >= absMaxX ? absMinX : absMaxX ;
	maxStepY = absMinY >= absMaxY ? absMinY : absMaxY ;

	stepMax = maxStepX >= maxStepY ? maxStepX : maxStepY ;

	stepSize = 0 ;
	sad = meSAD(cur, ref,  curStride, refStride, blockSize) ;
	i = j = 0 ;
	mv[0] = i ;
	mv[1] = j ;
	if(mvCost) {
		int ai, aj ;

		ai = i >= 0 ? i : -i ;
		aj = j >= 0 ? j : -j ;
		cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
	} else {
		cost = sad ;
	}
	minCost = cost ;
	minSad = sad ;
	for(stepSize = 1; stepSize <= stepMax; stepSize <<= 1) {
//	for(stepSize = 4; stepSize <= 8; stepSize <<= 1) {
		i = -stepSize ;
		j = -stepSize ;
		minK = 8 ;
		for(k = 0; k < 8; k++) {			
			if(i >= minX && i <= maxX && j >= minY && j <= maxY) {
				sad = meSAD(cur, ref + i + j * refStride,  curStride, refStride, blockSize) ;
				if(mvCost) {
					int ai, aj ;

					ai = i >= 0 ? i : -i ;
					aj = j >= 0 ? j : -j ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
			}
			if(cost <= minCost) {
				if(cost == minCost) {
					if(mv[0] * mv[0] + mv[1] * mv[1] > i * i + j * j) {
						minCost = cost ;
						minSad = sad ;
						mv[0] = i ;
						mv[1] = j ;
						minK = k ;
					}
				} else {
					minCost = cost ;
					minSad = sad ;
					mv[0] = i ;
					mv[1] = j ;
					minK = k ;
				}
			}

			if(k < 2) {
				i += stepSize ;
			} else if(k < 4) {
				j += stepSize ;
			} else if(k < 6) {
				i -= stepSize ;
			} else {
				j -= stepSize ;
			}
		}
		if(minK == 8) break ;
	}

	if(stepSize > stepMax) {
		stepSize = stepMax >> 1 ;
	} else {
		stepSize >>= 1 ;
	}
	ci = mv[0] ;
	cj = mv[1] ;
	for(; stepSize >= 1; stepSize >>= 1) {
		i = ci - stepSize ;
		j = cj - stepSize ;

		for(k = 0; k < 8; k++) {			
			if(i >= minX && i <= maxX && j >= minY && j <= maxY) {
				sad = meSAD(cur, ref + i + j * refStride,  curStride, refStride, blockSize) ;
				if(mvCost) {
					int ai, aj ;

					ai = i >= 0 ? i : -i ;
					aj = j >= 0 ? j : -j ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
			}
			if(cost <= minCost) {
				if(cost == minCost) {
					if(mv[0] * mv[0] + mv[1] * mv[1] > i * i + j * j) {
						minCost = cost ;
						minSad = sad ;
						mv[0] = i ;
						mv[1] = j ;
						minK = k ;
					}
				} else {
					minCost = cost ;
					minSad = sad ;
					mv[0] = i ;
					mv[1] = j ;
					minK = k ;
				}
			}

			if(k < 2) {
				i += stepSize ;
			} else if(k < 4) {
				j += stepSize ;
			} else if(k < 6) {
				i -= stepSize ;
			} else {
				j -= stepSize ;
			}
		}

		ci = mv[0] ;
		cj = mv[1] ;
	}

	return minSad ;
}

int memMbFullSearch(unsigned char *cur, unsigned char *ref, int curStride, int refStride, int blockSize, int bsShift, int searchRange, short *mv, int mvCost) {

	int i, j ;
	int sad, cost ;
	int minSad ;
	int minCost ;

	minSad = 65536 ;
	minCost = 262144 ;
	mv[0] = 0 ;
	mv[1] = 0 ;
	for(j = -searchRange; j <= searchRange; j++) {
		for(i = -searchRange; i <= searchRange; i++) {
			sad = memSAD(cur, ref + i + j * refStride,  curStride, refStride, blockSize, bsShift) ;
			if(mvCost) {
				int ai, aj ;

				ai = i >= 0 ? i : -i ;
				aj = j >= 0 ? j : -j ;
				cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
			} else {
				cost = sad ;
			}
			if(cost <= minCost) {
				if(cost == minCost) {
					if(mv[0] * mv[0] + mv[1] * mv[1] > i * i + j * j) {
						minCost = cost ;
						minSad = sad ;
						mv[0] = i ;
						mv[1] = j ;
					}
				} else {
					minCost = cost ;
					minSad = sad ;
					mv[0] = i ;
					mv[1] = j ;
				}
			}
		}
	}

	return minSad ;
}

int meMbFullSearch(unsigned char *cur, unsigned char *ref, int curStride, int refStride, int blockSize, int searchRange, short *mv, int mvCost) {

	int i, j ;
	int sad, cost ;
	int minSad ;
	int minCost ;

	minSad = 65536 ;
	minCost = 262144 ;
	mv[0] = 0 ;
	mv[1] = 0 ;
	for(j = -searchRange; j <= searchRange; j++) {
		for(i = -searchRange; i <= searchRange; i++) {
			sad = meSAD(cur, ref + i + j * refStride,  curStride, refStride, blockSize) ;
			if(mvCost) {
				int ai, aj ;

				ai = i >= 0 ? i : -i ;
				aj = j >= 0 ? j : -j ;
				cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
			} else {
				cost = sad ;
			}
			if(cost <= minCost) {
				if(cost == minCost) {
					if(mv[0] * mv[0] + mv[1] * mv[1] > i * i + j * j) {
						minCost = cost ;
						minSad = sad ;
						mv[0] = i ;
						mv[1] = j ;
					}
				} else {
					minCost = cost ;
					minSad = sad ;
					mv[0] = i ;
					mv[1] = j ;
				}
			}
		}
	}

	return minSad ;
}

void meSetReference(MotionEstimationHandle *hME, FRAME_YUV420 *pRef) {

	copyFrame420(pRef, &hME->ref) ;
	borderExtendFrame420(&hME->ref) ;
}

void mcCopyBlk(unsigned char *dest, unsigned char *src, int destStride, int srcStride, int blockSize) {

	int i, j ;

	for(j = 0; j < blockSize; j++) {
		for(i = 0; i < blockSize; i++) {
			dest[i + j * destStride] = src[i + j * srcStride] ;
		}
	}
}

void motionCompensation(MotionEstimationHandle *hME, FRAME_YUV420 *pCon) {

	int i, j ;
	unsigned char *pRefA ;
	unsigned char *pConA ;
	int nMBx ;
	int nMBy ;

	nMBx = hME->nMBx ;
	nMBy = hME->nMBy ;

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {
			pRefA = getActiveFrame420_Y(&hME->ref) + (i << 4) + (j << 4) * hME->ref.strideY ;
			pConA = getActiveFrame420_Y(pCon) + (i << 4) + (j << 4) * pCon->strideY ;

			pRefA += hME->pMVs[i + j * nMBx][0] + hME->pMVs[i + j * nMBx][1] * hME->ref.strideY ;
			mcCopyBlk(pConA, pRefA, pCon->strideY, hME->ref.strideY, 16) ;

			pRefA = getActiveFrame420_U(&hME->ref) + (i << 3) + (j << 3) * hME->ref.strideUV ;
			pConA = getActiveFrame420_U(pCon) + (i << 3) + (j << 3) * pCon->strideUV ;

			pRefA += (hME->pMVs[i + j * nMBx][0] / 2) + (hME->pMVs[i + j * nMBx][1] / 2) * hME->ref.strideUV ;
			mcCopyBlk(pConA, pRefA, pCon->strideUV, hME->ref.strideUV, 8) ;

			pRefA = getActiveFrame420_V(&hME->ref) + (i << 3) + (j << 3) * hME->ref.strideUV ;
			pConA = getActiveFrame420_V(pCon) + (i << 3) + (j << 3) * pCon->strideUV ;

			pRefA += (hME->pMVs[i + j * nMBx][0] / 2) + (hME->pMVs[i + j * nMBx][1] / 2) * hME->ref.strideUV ;
			mcCopyBlk(pConA, pRefA, pCon->strideUV, hME->ref.strideUV, 8) ;
		}
	}
}

void meFSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) {

	int i, j ;
	int nMBx, nMBy ;
	unsigned char *curBlk, *refBlk ;

	nMBx = hME->nMBx ;
	nMBy = hME->nMBy ;

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {
			curBlk = getActiveFrame420_Y(pCur) + (i << 4) + (j << 4) * pCur->strideY ;
			refBlk = getActiveFrame420_Y(&hME->ref) + (i << 4) + (j << 4) * hME->ref.strideY ;
			meMbFullSearch(curBlk, refBlk, pCur->strideY, hME->ref.strideY, 16, hME->searchRange, hME->pMVs[i + j * nMBx], mvCost) ;
		}
	}
}

void meHierFullSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) {

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MED(a, b, c) MIN(MIN(MAX(a, b), MAX(b, c)), MAX(a, c))

	int i, j ;
	int nMBx, nMBy ;
	unsigned char *curBlk, *refBlk ;

	nMBx = hME->nMBx ;
	nMBy = hME->nMBy ;

	downSamplingFrameByTwo(&hME->ref, &hME->hRef, hME->hRef.xOff, hME->hRef.yOff) ;
	downSamplingFrameByTwo(&hME->hRef, &hME->qRef, hME->qRef.xOff, hME->qRef.yOff) ;
	downSamplingFrameByTwo(pCur, &hME->hCur, hME->hCur.xOff, hME->hCur.yOff) ;
	downSamplingFrameByTwo(&hME->hCur, &hME->qCur, hME->qCur.xOff, hME->qCur.yOff) ;

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {
			curBlk = getActiveFrame420_Y(&hME->qCur) + (i << 2) + (j << 2) * hME->qCur.strideY ;
			refBlk = getActiveFrame420_Y(&hME->qRef) + (i << 2) + (j << 2) * hME->qRef.strideY ;
			meMbFullSearch(curBlk, refBlk, hME->qCur.strideY, hME->qRef.strideY, 4, (hME->searchRange >> 2) - 1, hME->qMVs[i + j * nMBx], mvCost) ;

//			hME->qMVs[i + j * nMBx][0] = 0 ;
//			hME->qMVs[i + j * nMBx][1] = 0 ;
		}
	}

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {

			curBlk = getActiveFrame420_Y(&hME->hCur) + (i << 3) + (j << 3) * hME->hCur.strideY ;
			refBlk = getActiveFrame420_Y(&hME->hRef) + ((i << 3) + (hME->qMVs[i + j * nMBx][0] << 1)) + ((j << 3) + (hME->qMVs[i + j * nMBx][1] << 1)) * hME->hRef.strideY ;
			meMbFullSearch(curBlk, refBlk, hME->hCur.strideY, hME->hRef.strideY, 8, 4, hME->hMVs[i + j * nMBx], mvCost) ;
			hME->hMVs[i + j * nMBx][0] += (hME->qMVs[i + j * nMBx][0] << 1) ;
			hME->hMVs[i + j * nMBx][1] += (hME->qMVs[i + j * nMBx][1] << 1) ;
/*
			{
				int sad0, sad ;
				short mv[2] ;
				short mv0[2] ;

				refBlk = getActiveFrame420_Y(&hME->hRef) + ((i << 3) + (hME->qMVs[i + j * nMBx][0] << 1)) + ((j << 3) + (hME->qMVs[i + j * nMBx][1] << 1)) * hME->hRef.strideY ;
				sad = meMbFullSearch(curBlk, refBlk, hME->hCur.strideY, hME->hRef.strideY, 8, 2, mv, mvCost) ;
				refBlk = getActiveFrame420_Y(&hME->hRef) + (i << 3) + (j << 3) * hME->hRef.strideY ;
				sad0 = meMbFullSearch(curBlk, refBlk, hME->hCur.strideY, hME->hRef.strideY, 8, (hME->searchRange >> 1) - 1, mv0, mvCost) ;

				if(0 && sad0 > sad) {
					hME->hMVs[i + j * nMBx][0] = mv[0] + (hME->qMVs[i + j * nMBx][0] << 1) ;
					hME->hMVs[i + j * nMBx][1] = mv[1] + (hME->qMVs[i + j * nMBx][1] << 1) ;
				} else {
					hME->hMVs[i + j * nMBx][0] = mv0[0] ;
					hME->hMVs[i + j * nMBx][1] = mv0[1] ;
				}
			}
*/
/*
			{
				int lmvX, lmvY ;
				int rmvX, rmvY ;
				int tmvX, tmvY ;
				int pmvX, pmvY ;

				lmvX = rmvX = tmvX = hME->qMVs[i + j * nMBx][0] << 1 ;
				lmvY = rmvY = tmvY = hME->qMVs[i + j * nMBx][1] << 1 ;

				if(i > 0) {
					lmvX = hME->qMVs[i - 1 + j * nMBx][0] << 1 ;
					lmvY = hME->qMVs[i - 1 + j * nMBx][1] << 1 ;
				}

				if(j > 0) {
					tmvX = hME->qMVs[i + (j - 1) * nMBx][0] << 1 ;
					tmvY = hME->qMVs[i + (j - 1) * nMBx][1] << 1 ;
				}

				if(i < (nMBx - 1) && j > 0) {
					rmvX = hME->qMVs[i + 1 + (j - 1) * nMBx][0] << 1 ;
					rmvY = hME->qMVs[i + 1 + (j - 1) * nMBx][1] << 1 ;
				}

				pmvX = MED(lmvX, rmvX, tmvX) ;
				pmvY = MED(lmvY, rmvY, tmvY) ;

				refBlk = getActiveFrame420_Y(&hME->hRef) + ((i << 3) + (pmvX)) + ((j << 3) + (pmvY)) * hME->hRef.strideY ;
				meMbFullSearch(curBlk, refBlk, hME->hCur.strideY, hME->hRef.strideY, 8, 4, hME->hMVs[i + j * nMBx], mvCost) ;
				hME->hMVs[i + j * nMBx][0] += (pmvX) ;
				hME->hMVs[i + j * nMBx][1] += (pmvY) ;
			}
*/
		}
	}

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {
			curBlk = getActiveFrame420_Y(pCur) + (i << 4) + (j << 4) * pCur->strideY ;
			refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + (hME->hMVs[i + j * nMBx][0] << 1)) + ((j << 4) + (hME->hMVs[i + j * nMBx][1] << 1)) * hME->ref.strideY ;
			meMbFullSearch(curBlk, refBlk, pCur->strideY, hME->ref.strideY, 16, 1, hME->pMVs[i + j * nMBx], mvCost) ;
			hME->pMVs[i + j * nMBx][0] += (hME->hMVs[i + j * nMBx][0] << 1) ;
			hME->pMVs[i + j * nMBx][1] += (hME->hMVs[i + j * nMBx][1] << 1) ;
		}
	}

#undef MIN
#undef MAX
#undef MED
}

void meRCSearch(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int mvCost) {

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MED(a, b, c) MIN(MIN(MAX(a, b), MAX(b, c)), MAX(a, c))

	int i, j ;
	int pmvX, pmvY ;
	int lmvX, lmvY ;
	int tmvX, tmvY ;
	int rmvX, rmvY ;
	int centerX, centerY ;
	int nMBx, nMBy ;
	int searchRange ;
	int curStride ;
	int refStride ;
	int sad, minSad ;
	int cost, minCost ;
	int blockSize ;
	unsigned char *curBlk, *refBlk ;

	nMBx = hME->nMBx ;
	nMBy = hME->nMBy ;
	curStride = pCur->strideY ;
	refStride = hME->ref.strideY ;
	searchRange = hME->searchRange ;
	blockSize = 16 ;

	for(j = 0; j < nMBy; j++) {
		for(i = 0; i < nMBx; i++) {
			curBlk = getActiveFrame420_Y(pCur) + (i << 4) + (j << 4) * pCur->strideY ;

			lmvX = rmvX = tmvX = 0 ;
			lmvY = rmvY = tmvY = 0 ;
			centerX = pmvX = 0 ;
			centerY = pmvY = 0 ;
			refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
			sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

			if(mvCost) {
				int ai, aj ;

				ai = pmvX >= 0 ? pmvX : -pmvX ;
				aj = pmvY >= 0 ? pmvY : -pmvY ;
				cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
			} else {
				cost = sad ;
			}

			minSad = sad ;
			minCost = cost ;

			if(i > 0) {
				lmvX = pmvX = hME->pMVs[i - 1 + j * nMBx][0] ;
				lmvY = pmvY = hME->pMVs[i - 1 + j * nMBx][1] ;
				refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
				sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

				if(mvCost) {
					int ai, aj ;

					ai = pmvX >= 0 ? pmvX : -pmvX ;
					aj = pmvY >= 0 ? pmvY : -pmvY ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
				
				if(cost < minCost) {
					centerX = pmvX ;
					centerY = pmvY ;
					minCost = cost ;
					minSad = sad ;
				}
			}

			if(j > 0) {
				tmvX = pmvX = hME->pMVs[i + (j - 1) * nMBx][0] ;
				tmvY = pmvY = hME->pMVs[i + (j - 1) * nMBx][1] ;
				refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
				sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

				if(mvCost) {
					int ai, aj ;

					ai = pmvX >= 0 ? pmvX : -pmvX ;
					aj = pmvY >= 0 ? pmvY : -pmvY ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
				
				if(cost < minCost) {
					centerX = pmvX ;
					centerY = pmvY ;
					minCost = cost ;
					minSad = sad ;
				}
			}

			if(i > 0 && j > 0) {
				pmvX = hME->pMVs[i - 1 + j * nMBx][0] ;
				pmvY = hME->pMVs[i + (j - 1) * nMBx][1] ;
				refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
				sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

				if(mvCost) {
					int ai, aj ;

					ai = pmvX >= 0 ? pmvX : -pmvX ;
					aj = pmvY >= 0 ? pmvY : -pmvY ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
				
				if(cost < minCost) {
					centerX = pmvX ;
					centerY = pmvY ;
					minCost = cost ;
					minSad = sad ;
				}
			}

			if(j > 0 && i < (nMBx - 1)) {
				rmvX = pmvX = hME->pMVs[(i + 1) + (j - 1) * nMBx][0] ;
				rmvY = pmvY = hME->pMVs[(i + 1) + (j - 1) * nMBx][1] ;
				refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
				sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

				if(mvCost) {
					int ai, aj ;

					ai = pmvX >= 0 ? pmvX : -pmvX ;
					aj = pmvY >= 0 ? pmvY : -pmvY ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
				
				if(cost < minCost) {
					centerX = pmvX ;
					centerY = pmvY ;
					minCost = cost ;
					minSad = sad ;
				}
			}

			if(i > 0 && j > 0 && i < (nMBx - 1)) {
				pmvX = MED(lmvX, rmvX, tmvX) ;
				pmvY = MED(lmvY, rmvY, tmvY) ;
				refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
				sad = meSAD(curBlk, refBlk, curStride, refStride, blockSize) ;

				if(mvCost) {
					int ai, aj ;

					ai = pmvX >= 0 ? pmvX : -pmvX ;
					aj = pmvY >= 0 ? pmvY : -pmvY ;
					cost = sad + (blockSize * 16 - blockSize * blockSize) * (ai * ai + aj * aj) / 4 ;
				} else {
					cost = sad ;
				}
				
				if(cost < minCost) {
					centerX = pmvX ;
					centerY = pmvY ;
					minCost = cost ;
					minSad = sad ;
				}
			}
			
			pmvX = centerX ;
			pmvY = centerY ;

			refBlk = getActiveFrame420_Y(&hME->ref) + ((i << 4) + pmvX) + ((j << 4) + pmvY) * hME->ref.strideY ;
			meMbRCSearch(curBlk, refBlk, curStride, refStride, blockSize, -searchRange - pmvX, searchRange - pmvX, -searchRange - pmvY, searchRange - pmvY, hME->pMVs[i + j * nMBx], mvCost) ;
//			meMbRCSearch(curBlk, refBlk, curStride, refStride, blockSize, -(searchRange/2) - pmvX, (searchRange/2) - pmvX, -(searchRange/2) - pmvY, (searchRange/2) - pmvY, hME->pMVs[i + j * nMBx], mvCost) ;
			hME->pMVs[i + j * nMBx][0] += centerX ;
			hME->pMVs[i + j * nMBx][1] += centerY ;
		}
	}

#undef MIN
#undef MAX
#undef MED

}

void motionEstimation(MotionEstimationHandle *hME, FRAME_YUV420 *pCur, int meAlg, int mvCost) {

	switch(meAlg) {
	case ME_FS:
		meFSearch(hME, pCur, mvCost) ;
		break ;
	case ME_HFS:
		meHierFullSearch(hME, pCur, mvCost) ;
		break ;
	case ME_RCS:
		meRCSearch(hME, pCur, mvCost) ;
		break ;
	}
}

#ifdef __cplusplus
}
#endif
