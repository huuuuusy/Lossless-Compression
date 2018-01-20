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

#include "frame.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *getActiveFrame420_Y(FRAME_YUV420 *pCur) {

	return pCur->Y + (pCur->xOff) + (pCur->yOff) * pCur->strideY ;
}

unsigned char *getActiveFrame420_U(FRAME_YUV420 *pCur) {

	return pCur->U + (pCur->xOff >> 1) + (pCur->yOff >> 1) * pCur->strideUV ;
}

unsigned char *getActiveFrame420_V(FRAME_YUV420 *pCur) {

	return pCur->V + (pCur->xOff >> 1) + (pCur->yOff >> 1) * pCur->strideUV ;
}

unsigned char *newFrame420(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) {

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

void freeFrame420(FRAME_YUV420 *pFrame) {

	if(pFrame->Y) {
		delete [] pFrame->Y ;
		pFrame->Y = NULL ;
		pFrame->U = NULL ;
		pFrame->V = NULL ;
	}
}

void getPSNRFrame420(FRAME_YUV420 *pCur, FRAME_YUV420 *pRef, double *psnrY, double *psnrU, double *psnrV) {

	int i, j ;
	int diff ;
	double sumDiff ;
	unsigned char *pCurA ;
	unsigned char *pRefA ;
	int size ;

	size = pCur->height * pCur->width ;

	if(psnrY) {
		pCurA = getActiveFrame420_Y(pCur) ;
		pRefA = getActiveFrame420_Y(pRef) ;
		sumDiff = 0.0 ;
		for(j = 0; j < pCur->height; j++) {
			for(i = 0; i < pCur->width; i++) {
				diff = pCurA[i + j * pCur->strideY] - pRefA[i + j * pRef->strideY] ;
				sumDiff += (double) (diff * diff) ;
			}
		}

		sumDiff /= ((double) size) ;

		*psnrY = (double) 10.0 * log10(255.0 * 255.0 / (sumDiff + 0.00001)) ;
	}

	size >>= 2 ;

	if(psnrU) {
		pCurA = getActiveFrame420_U(pCur) ;
		pRefA = getActiveFrame420_U(pRef) ;
		sumDiff = 0.0 ;
		for(j = 0; j < pCur->height / 2; j++) {
			for(i = 0; i < pCur->width / 2; i++) {
				diff = pCurA[i + j * pCur->strideUV] - pRefA[i + j * pRef->strideUV] ;
				sumDiff += (double) (diff * diff) ;
			}
		}

		sumDiff /= ((double) size) ;
		*psnrU = (double) 10 * log10(255.0 * 255.0 / (sumDiff * sumDiff + 0.00001)) ;
	}

	if(psnrV) {
		pCurA = getActiveFrame420_V(pCur) ;
		pRefA = getActiveFrame420_V(pRef) ;
		sumDiff = 0.0 ;
		for(j = 0; j < pCur->height / 2; j++) {
			for(i = 0; i < pCur->width / 2; i++) {
				diff = pCurA[i + j * pCur->strideUV] - pRefA[i + j * pRef->strideUV] ;
				sumDiff += (double) (diff * diff) ;
			}
		}

		sumDiff /= ((double) size) ;
		*psnrV = (double) 10.0 * log10(255.0 * 255.0 / (sumDiff * sumDiff + 0.00001)) ;
	}
}

void borderExtend(unsigned char *pCur, int width, int height, int stride, int xOff, int yOff) {

	unsigned char *pCurA ;


	int j ;

	pCurA = pCur + xOff + yOff * stride ;
	for(j = 0; j < height; j++) {
		memset(pCurA - xOff + j * stride, pCurA[j * stride], xOff) ;
		memset(pCurA + width + j * stride, pCurA[width - 1 + j * stride], xOff) ;
	}

	for(j = 0; j < yOff; j++) {
		memset(pCur + j * stride, pCurA[0], xOff) ;
		memset(pCur + xOff + width + j * stride, pCurA[width - 1], xOff) ;
		memset(pCur + (j + height + yOff) * stride, pCurA[(height - 1) * stride], xOff) ;
		memset(pCur + xOff + width + (j + height + yOff) * stride, pCurA[width - 1 + (height - 1) * stride], xOff) ;
		memcpy(pCur + xOff + j * stride, pCurA, width) ;
		memcpy(pCur + xOff + (j + height + yOff) * stride, pCurA + (height - 1) * stride, width) ;
	}
}

void borderExtendFrame420(FRAME_YUV420 *pCur) {

	if(pCur->Y) {
		borderExtend(pCur->Y, pCur->width, pCur->height, pCur->strideY, pCur->xOff, pCur->yOff) ;
	}
	if(pCur->U) {
		borderExtend(pCur->U, pCur->width >> 1, pCur->height >> 1, pCur->strideUV, pCur->xOff >> 1, pCur->yOff >> 1) ;
	}
	if(pCur->V) {
		borderExtend(pCur->V, pCur->width >> 1, pCur->height >> 1, pCur->strideUV, pCur->xOff >> 1, pCur->yOff >> 1) ;
	}
}

void blur3x3Frame420_Y(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest) {

	int i, j, k ;
	int sum, x, y ;
	unsigned char *pDestY ;
	unsigned char *pDestU ;
	unsigned char *pDestV ;
	unsigned char *pSrcY ;
	unsigned char *pSrcU ;
	unsigned char *pSrcV ;

	if(!pSrc->Y || !pDest->Y) return ;

	pSrcY = getActiveFrame420_Y(pSrc) ;
	pDestY = getActiveFrame420_Y(pDest) ;

	for(j = 0; j < pSrc->height; j++) {
		for(i = 0; i < pSrc->width; i++) {
			if(i < 1 || j < 1 || (i > pSrc->width - 2) || (j > pSrc->height - 2)) {
				pDestY[i + j * pDest->strideY] = pSrcY[i + j * pSrc->strideY] ;
			} else {
				sum = 0 ;
				for(k = 0; k < 9; k++) {
					x = k % 3 - 1 ;
					y = k / 3 - 1 ;
					sum += pSrcY[(i + x) + (j + y) * pSrc->strideY] ;
				}
				pDestY[i + j * pDest->strideY] = sum / 9 ;
			}
		}
	}
	if(pSrc->U && pDest->U) {
		pSrcU = getActiveFrame420_U(pSrc) ;
		pDestU = getActiveFrame420_U(pDest) ;
		for(j = 0; j < pSrc->height / 2; j++) {
			memcpy(pDestU + j * pDest->strideUV, pSrcU + j * pSrc->strideUV, pSrc->width / 2) ;
		}
	}
	if(pSrc->V && pDest->V) {
		pSrcV = getActiveFrame420_V(pSrc) ;
		pDestV = getActiveFrame420_V(pDest) ;
		for(j = 0; j < pSrc->height / 2; j++) {
			memcpy(pDestV + j * pDest->strideUV, pSrcV + j * pSrc->strideUV, pSrc->width / 2) ;
		}
	}
}

void copyFrame420(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest) {

	int j ;
	unsigned char *pDestY ;
	unsigned char *pDestU ;
	unsigned char *pDestV ;
	unsigned char *pSrcY ;
	unsigned char *pSrcU ;
	unsigned char *pSrcV ;

	if(pSrc->Y && pDest->Y) {
		pSrcY = getActiveFrame420_Y(pSrc) ;
		pDestY = getActiveFrame420_Y(pDest) ;
		for(j = 0; j < pSrc->height; j++) {
			memcpy(pDestY + j * pDest->strideY, pSrcY + j * pSrc->strideY, pSrc->width) ;
		}
	}
	if(pSrc->U && pDest->U) {
		pSrcU = getActiveFrame420_U(pSrc) ;
		pDestU = getActiveFrame420_U(pDest) ;
		for(j = 0; j < pSrc->height / 2; j++) {
			memcpy(pDestU + j * pDest->strideUV, pSrcU + j * pSrc->strideUV, pSrc->width / 2) ;
		}
	}
	if(pSrc->V && pDest->V) {
		pSrcV = getActiveFrame420_V(pSrc) ;
		pDestV = getActiveFrame420_V(pDest) ;
		for(j = 0; j < pSrc->height / 2; j++) {
			memcpy(pDestV + j * pDest->strideUV, pSrcV + j * pSrc->strideUV, pSrc->width / 2) ;
		}
	}
}

void downSamplingByTwo(unsigned char *pSrc, unsigned char *pDest, int width, int height, int srcStride, int destStride, int hPad, int vPad) {

	int i, j ;
	int sum ;
	unsigned char *pDestA ;

	pDestA = pDest + hPad + vPad * destStride ;
	for(j = 0; j < height / 2; j++) {
		for(i = 0; i < width / 2; i++) {
			sum = pSrc[(i << 1) + (j << 1) * srcStride] ;
			sum += pSrc[((i << 1) + 1) + (j << 1) * srcStride] ;
			sum += pSrc[(i << 1) + ((j << 1) + 1) * srcStride] ;
			sum += pSrc[((i << 1) + 1) + ((j << 1) + 1) * srcStride] ;
			pDestA[i + j * destStride] = (sum >> 2) ;
		}
		memset(pDestA - hPad + j * destStride, pDestA[j * destStride], hPad) ;
		memset(pDestA + width / 2 + j * destStride, pDestA[width / 2 - 1 + j * destStride], hPad) ;
	}

	for(j = 0; j < vPad; j++) {
		memset(pDest + j * destStride, pDestA[0], hPad) ;
		memset(pDest + hPad + (width / 2) + j * destStride, pDestA[width / 2 - 1], hPad) ;
		memset(pDest + (j + (height / 2) + vPad) * destStride, pDestA[(height / 2 - 1) * destStride], hPad) ;
		memset(pDest + hPad + (width / 2) + (j + (height / 2) + vPad) * destStride, pDestA[width / 2 - 1 + (height / 2 - 1) * destStride], hPad) ;
		memcpy(pDest + hPad + j * destStride, pDestA, width / 2) ;
		memcpy(pDest + hPad + (j + (height / 2) + vPad) * destStride, pDestA + (height / 2 - 1) * destStride, width / 2) ;
	}
}

void downSamplingFrameByTwo(FRAME_YUV420 *pSrc, FRAME_YUV420 *pDest, int hPad, int vPad) {

	unsigned char *pSrcY ;
	unsigned char *pSrcU ;
	unsigned char *pSrcV ;

	if(!pDest->Y) return ;

	pDest->xOff = hPad ;
	pDest->yOff = vPad ;
	pDest->width = pSrc->width >> 1 ;
	pDest->height = pSrc->height >> 1 ;
	pDest->strideY = pDest->width + (hPad << 1) ;
	pDest->strideUV = (pDest->strideY >> 1) ;

	pSrcY = getActiveFrame420_Y(pSrc) ;

	downSamplingByTwo(pSrcY, pDest->Y, pSrc->width, pSrc->height, pSrc->strideY, pDest->strideY, hPad, vPad) ;

	if(pDest->U) {
		pSrcU = getActiveFrame420_U(pSrc) ;
		downSamplingByTwo(pSrcU, pDest->U, pSrc->width >> 1, pSrc->height >> 1, pSrc->strideUV, pDest->strideUV, hPad >> 1, vPad >> 1) ;
	}

	if(pDest->V) {
		pSrcV = getActiveFrame420_V(pSrc) ;
		downSamplingByTwo(pSrcV, pDest->V, pSrc->width >> 1, pSrc->height >> 1, pSrc->strideUV, pDest->strideUV, hPad >> 1, vPad >> 1) ;
	}
}

#ifdef __cplusplus
}
#endif


