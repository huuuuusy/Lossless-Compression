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
// videoDeinterlace.c/videoDeinterlace.h - Module that provides functions for Deinterlacing

#include "videoDeinterlace.h"
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//	videoDeinterlaceInit:	Allocates a handle structure and initalizes the associated resources for video deinterlacing
//	Input:
//		width:				width of the video (must be a multiple of 16)
//		height:				height of the video (must be a multiple of 16)
//		strength:			The strength for Deinterlacing
//							Range [0, 1023]
//								0 =>	Almost no interpolation, no matter how high is the motion level
//								63 =>	Always perform interpolation, no matter how low is the motion level
//							Recommended value: 32
//	Return:
//		Pointer to an allocated VideoDeinterlaceHandle structure if succeed
//		Return NULL if failed
VideoDeinterlaceHandle *videoDeinterlaceInit(int width, int height, int strength) {

	VideoDeinterlaceHandle *hVideoDeinterlace ;

	hVideoDeinterlace= (VideoDeinterlaceHandle *) malloc(sizeof(VideoDeinterlaceHandle)) ;
	if(!hVideoDeinterlace) return NULL ;

	hVideoDeinterlace->width = width ;
	hVideoDeinterlace->height = height ;

	if(strength < 0) {
		strength = 0 ;
	}
	if(strength > 63) {
		strength = 63 ;
	}
	hVideoDeinterlace->strength = strength ;

	hVideoDeinterlace->diffFrame = (unsigned char *) malloc(width * height) ;
	hVideoDeinterlace->edgeFrame[0] = (unsigned char *) malloc(width * height / 2) ;
	hVideoDeinterlace->edgeFrame[1] = (unsigned char *) malloc(width * height / 2) ;
	hVideoDeinterlace->edgeFrame[2] = (unsigned char *) malloc(width * height / 2) ;

	return hVideoDeinterlace ;
}

//	videoDeinterlaceFinal:	Release all the resources associated with the videoDeinterlaceHandle
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//	Return:						NIL
void videoDeinterlaceFinal(VideoDeinterlaceHandle *hVideoDeinterlace) {

	if(hVideoDeinterlace) {
		if(hVideoDeinterlace->diffFrame) free(hVideoDeinterlace->diffFrame) ;
		if(hVideoDeinterlace->edgeFrame[0]) free(hVideoDeinterlace->edgeFrame[0]) ;
		if(hVideoDeinterlace->edgeFrame[1]) free(hVideoDeinterlace->edgeFrame[1]) ;
		if(hVideoDeinterlace->edgeFrame[2]) free(hVideoDeinterlace->edgeFrame[2]) ;
		free(hVideoDeinterlace) ;
	}
}

//	vdiFrameDifference:			Calculate pixel differences between reference frame and current frame
//								The pixel differences are essential for estimating motion level for
//								each pixel
//								This function is internal to Video Deinterlacing Module
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//		pRef:					Pointer to the reference frame in YUV420 format
//		pCur:					Pointer to the current frame in YUV420 format
//	Output:
//		Pixel differences are stored in the the difference frame buffer pointed to by
//		hVideoDeinterlace->diffFrame
//	Return:						NIL
void vdiFrameDifference(VideoDeinterlaceHandle *hVideoDeinterlace, FRAME_Y420 *pRef, FRAME_Y420 *pCur) {

	unsigned char *diffFrame = hVideoDeinterlace->diffFrame ;
	int width = hVideoDeinterlace->width ;
	int height = hVideoDeinterlace->height ;
	int strideCur = pCur->strideY ;
	int strideRef = pRef->strideY ;
	short diff ;
	int i, j ;

	//	hVideoDeinterlace->diffFrame holds previous pixel difference values

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			diff = pCur->Y[i + j * strideCur] - pRef->Y[i + j * strideRef] ;
			diff = diff > 0 ? diff : -diff ;
			if(diff > diffFrame[i + j * width]) {
				// Current pixel difference is greater than previous pixel difference
				// That may indicate a sudden increase in motion level
				diffFrame[i + j * width] = (unsigned char) diff ;
			}
			else {
				// Curretn pixel difference is NOT greater than previous pixel difference
				// But to play safe, we adopt an averaging scheme here to smooth out
				// the motion level to assure a more stable deinterlacing effect along
				// moving objects' edge
				diffFrame[i + j * width] = (diffFrame[i + j * width] + diff) >> 1 ;
			}
		}
	}
}

//	vdiEdgeDifference:			Calculate pixel differences between two consecutive even scan lines
//								These pixel differences are essential for estimating edge orientation
//								for each pixel
//								This function is internal to Video Deinterlacing Module
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//		pCur:					Pointer to the current frame in YUV420 format
//	Output:
//		Pixel differences are stored in the the difference frame buffers pointed to by
//		hVideoDeinterlace->edgeFrame[0..3]
//	Return:						NIL
void vdiEdgeDifference(VideoDeinterlaceHandle *hVideoDeinterlace, FRAME_Y420 *pCur) {

	unsigned char *edgeFrame[3] ;
	int width = hVideoDeinterlace->width ;
	int width2 ;
	int height = hVideoDeinterlace->height ;
	int strideCur = pCur->strideY ;
	int strideCur2 ;
	short diff ;
	int i, j ;

	width2 = width << 1 ;
	strideCur2 = strideCur << 1 ;

	edgeFrame[0] = hVideoDeinterlace->edgeFrame[0] ;
	edgeFrame[1] = hVideoDeinterlace->edgeFrame[1] ;
	edgeFrame[2] = hVideoDeinterlace->edgeFrame[2] ;

	// edgeFrame[0] holds the pixel difference between the consecutive even scanlines as follows
	// Current Even Scanline		*** *** cur *** ***
	// Next Odd Scanline			*** *** *** *** ***
	// Next Even Scanline			*** *** *** *** nxt

	// edgeFrame[1] holds the pixel difference between the consecutive even scanlines as follows
	// Current Even Scanline		*** *** cur *** ***
	// Next Odd Scanline			*** *** *** *** ***
	// Next Even Scanline			*** *** nxt *** ***

	// edgeFrame[2] holds the pixel difference between the consecutive even scanlines as follows
	// Current Even Scanline		*** *** cur *** ***
	// Next Odd Scanline			*** *** *** *** ***
	// Next Even Scanline			nxt *** *** *** ***

	for(j = 0; j < height / 2; j++) {

		// To ensure in each scanline we can read successfully the (x - 2, y + 2) and (x + 2, y + 2) pixels
		// we need to loop for the x coordinate in each scanline only from 2 to width - 2
		// As such, the left most 2 and right most 2 pixels' difference has to be initialized differently
		// we mandates them to be 255
		edgeFrame[0][0 + j * width] = edgeFrame[1][0 + j * width] = edgeFrame[2][0 + j * width] = 255 ;
		edgeFrame[0][1 + j * width] = edgeFrame[1][1 + j * width] = edgeFrame[2][1 + j * width] = 255 ;
		edgeFrame[0][width - 2 + j * width] = edgeFrame[1][width - 2 + j * width] = edgeFrame[2][width - 2 + j * width] = 255 ;
		edgeFrame[0][width - 1 + j * width] = edgeFrame[1][width - 1 + j * width] = edgeFrame[2][width - 1 + j * width] = 255 ;
		for(i = 2; i < width - 2; i++) {
			diff = pCur->Y[i + j * strideCur2] - pCur->Y[i + 2 + (j + 1) * strideCur2] ;
			diff = diff > 0 ? diff : -diff ;
			edgeFrame[0][i + j * width] = (unsigned char) diff ;
			diff = pCur->Y[i + j * strideCur2] - pCur->Y[i + (j + 1) * strideCur2] ;
			diff = diff > 0 ? diff : -diff ;
			edgeFrame[1][i + j * width] = (unsigned char) diff ;
			diff = pCur->Y[i + j * strideCur2] - pCur->Y[i - 2 + (j + 1) * strideCur2] ;
			diff = diff > 0 ? diff : -diff ;
			edgeFrame[2][i + j * width] = (unsigned char) diff ;
		}
	}
}

//	videoDeinterlace:	Deinterlacing the current frame based on a reference frame
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//		pRef:					Pointer to the reference frame in YUV420 format
//		pCur:					Pointer to the current frame in YUV420 format
//		pOut:					Pointer to a pre-allocated output frame in YUV420 format
//	Output:
//		pOut:					Deinterlaced frame data are stored in the location addressed by pOut
//	Return:						NIL
void videoDeinterlace(VideoDeinterlaceHandle *hVideoDeinterlace, FRAME_Y420 *pRef, FRAME_Y420 *pCur, FRAME_Y420 *pOut) {

#define MAX2(a, b) ((a) >= (b) ? (a) : (b))
#define MAX3(a, b, c) MAX2(MAX2(a, b), c)

	unsigned char *edgeFrame[3] ;
	unsigned char *diffFrame = hVideoDeinterlace->diffFrame ;
	int width = hVideoDeinterlace->width ;
	int width2 ;
	int height = hVideoDeinterlace->height ;
	int strideCur = pCur->strideY ;
	int strideRef = pRef->strideY ;
	int strideOut = pOut->strideY ;
	int strideCur2 ;
	int strideRef2 ;
	int strideOut2 ;
	int T ;
	int i, j ;
	int edgeOri[3] ;		// Intermediate values for edge orientation estimates
	int diff[3] ;			// Intermediate values for motion level estimates
	int i_off ;				// Offset for x-coordinate of each pixel for us to perform edge
							// oriented interpolation
							// For each pixel (x, y) in an odd scanline, we interpolate pixel (x, y)
							// based on the pixels (x + i_off, y) and (x - i_off, y) in the neighboring
							// even scanlines
	int motionLevel ;		
	int alpha ;				// Weighting coefficients for interpolation

	edgeFrame[0] = hVideoDeinterlace->edgeFrame[0] ;
	edgeFrame[1] = hVideoDeinterlace->edgeFrame[1] ;
	edgeFrame[2] = hVideoDeinterlace->edgeFrame[2] ;
	strideCur2 = strideCur << 1 ;
	strideRef2 = strideRef << 1 ;
	strideOut2 = strideOut << 1 ;
	width2 = width << 1 ;

	T = 64 - hVideoDeinterlace->strength ;
	T = T * T ;

	// Calculate pixel differences for subsequent motion level and edge orientation estimations
	vdiFrameDifference(hVideoDeinterlace, pRef, pCur) ;
	vdiEdgeDifference(hVideoDeinterlace, pCur) ;

	// We leaves chrominance values untouched, because under YUV420 format the deinterlacing for
	// chrominance values are not necessary
	// So we simply copy all the chrominance values from current frame to deinterlaced output frame
	for(j = 0; j < height / 2; j++) {
		memcpy(pOut->U + j * pOut->strideUV, pCur->U + j * pCur->strideUV, width / 2) ;
		memcpy(pOut->V + j * pOut->strideUV, pCur->V + j * pCur->strideUV, width / 2) ;
	}

	// Copy the even scanlines from current frame to output frames, because deinterlacing leaves
	// even scanlines untouched. In otherwords we only interpolate odd scanlines from even scanlines
	for(j = 0; j < height / 2; j++) {
		memcpy(pOut->Y + j * strideOut2, pCur->Y + j * strideCur2, width) ;
	}

	// The last odd scanline cannot be interpolated, so we copy it as it is as well
	memcpy(pOut->Y + (height - 1) * strideOut, pCur->Y + (height - 1) * strideCur, width) ;

	// The loop for interpolating odd scanline pixels
	for(j = 0; j < height / 2 - 1; j++) {

		// As we can only estimate the edge orientation for pixels with x-coordinate from 2 to width - 2
		// Therefore we deinterlace the left most and right most 2 pixels by line doubling
		pOut->Y[(2 * j + 1) * strideOut] = pCur->Y[(2 * j + 1) * strideCur] ;
		pOut->Y[1 + (2 * j + 1) * strideOut] = pCur->Y[1 + (2 * j + 1) * strideCur] ;
		pOut->Y[width - 2 + (2 * j + 1) * strideOut] = pCur->Y[width - 2 + (2 * j + 1) * strideCur] ;
		pOut->Y[width - 1 + (2 * j + 1) * strideOut] = pCur->Y[width - 1 + (2 * j + 1) * strideCur] ;
		for(i = 2; i < width - 2; i++) {

			// edgeOri[0] for pixel (x,y) is the sum of pixel differences as follows
			// Current Even Scanline		+++ +++ +++ *** ***
			// Next Odd Scanline			*** *** x,y *** ***
			// Next Even Scanline			*** *** --- --- ---
			// If edgeOri[0] results in small value for pixel (x, y), that means
			// most likely pixel (x, y) is lying on an edge with orientation '\'

			// edgeOri[1] for pixel (x,y) is the sum of pixel differences as follows
			// Current Even Scanline		*** +++ +++ +++ ***
			// Next Odd Scanline			*** *** x,y *** ***
			// Next Even Scanline			*** --- --- --- ***
			// If edgeOri[1] results in small value for pixel (x, y), that means
			// most likely pixel (x, y) is lying on an edge with orientation '|'

			// edgeOri[2] for pixel (x,y) is the sum of pixel differences as follows
			// Current Even Scanline		+++ +++ +++ *** ***
			// Next Odd Scanline			*** *** x,y *** ***
			// Next Even Scanline			*** *** --- --- ---
			// If edgeOri[2] results in small value for pixel (x, y), that means
			// most likely pixel (x, y) is lying on an edge with orientation '/'

			edgeOri[0] = edgeFrame[0][i - 2 + j * width] + edgeFrame[0][i - 1 + j * width] * 2 + edgeFrame[0][i + j * width] ;
			edgeOri[1] = edgeFrame[1][i - 1 + j * width] + edgeFrame[1][i + j * width] * 2 + edgeFrame[1][i + 1 + j * width] ;
			edgeOri[2] = edgeFrame[2][i + j * width] + edgeFrame[2][i + 1 + j * width] * 2 + edgeFrame[2][i + 2 + j * width] ;

			if(edgeOri[1] <= edgeOri[0] && edgeOri[1] <= edgeOri[2]) {
				// winning edge orientation is '|'
				// so the interpolating pixels are (x + 0, y - 1) and (x + 0, y + 1)
				i_off = 0 ;
			} else if(edgeOri[0] <= edgeOri[1] && edgeOri[0] <= edgeOri[2]) {
				// winning edge orientation is '\'
				// so the interpolating pixels are (x - 1, y - 1) and (x + 1, y + 1)
				i_off = -1 ;
			} else {
				// winning edge orientation is '\'
				// so the interpolating pixels are (x - 1, y - 1) and (x + 1, y + 1)
				i_off = 1 ;
			}

			// Motion level is estimated to be the max pixel differences in a 3x3 window
			// around the current pixel
			diff[0] = MAX3(diffFrame[i - 1 + j * width2], diffFrame[i + j * width2], diffFrame[i + 1 + j * width2]) ;
			diff[1] = MAX3(diffFrame[i - 1 + (2 * j + 1) * width], diffFrame[i + (2 * j + 1) * width], diffFrame[i + 1 + (2 * j + 1) * width]) ;
			diff[2] = MAX3(diffFrame[i - 1 + (j + 1) * width2], diffFrame[i + (j + 1) * width2], diffFrame[i + 1 + (j + 1) * width2]) ;

			motionLevel = MAX3(diff[0], diff[1], diff[2]) ;

			// Interpolating (weighting) coefficients for even scanlines based on the estimated motion level
			// The higher the motion level, the higher will be the weightings for even scanlines
			// The lower the motion level, the higher will be the weighting coefficient for odd scanline,
			// which essentially results in preserving vertical resolution for relatively stable pixels
			// The T here controls the deinterlacing strength
			// A higher T, corresponds to a lower strength because it suppress the weighting coefficients
			// for even scanlines
			// In contrast, a lower T corresponds to a higher strength in deinterlacing because it increase
			// the weigthing coefficients for even scanlines
			alpha = motionLevel * motionLevel * 512 / (motionLevel * motionLevel + T) ;

			// Interpolating the current odd scanline pixel
			pOut->Y[i + (2 * j + 1) * strideOut] = (alpha * (pCur->Y[i + i_off + j * strideCur2] + pCur->Y[i - i_off + (j + 1) * strideCur2])
													+ (1024 - 2 * alpha) * pCur->Y[i + (2 * j + 1) * strideCur]) >> 10 ;
		}
	}

#undef MAX2
#undef MAX3
}

#ifdef __cplusplus
}
#endif
