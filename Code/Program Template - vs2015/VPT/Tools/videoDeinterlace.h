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

#ifndef VIDEODEINTERLACE_H
#define VIDEODEINTERLACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TYPE_FRAME_Y420

#define TYPE_FRAME_Y420

// Video Frame Structure for use with camMotion, videoStab and videoDeinterlace modules
// Notes: videoDeinterlace modules assume the frame data is organized in YUV420 format
typedef struct {
	unsigned char *Y ;					// Y-Plane pointer
	unsigned char *U ;					// U-Plane pointer
	unsigned char *V ;					// V-Plane pointer
	int strideY ;						// Stride for Y-Plane
	int strideUV ;						// Stride for UV-Plane
} FRAME_Y420 ;

#endif

// VideoDeinterlaceHandle:	A handle structure for Video Deinterlacing
typedef struct {
	unsigned char *edgeFrame[3]	;		//	The frame buffers for holding edge orientation estimates, which are internal to Video Deinterlacing Module
	unsigned char *diffFrame ;			//	The frame buffer for storing motion level estimates, which are internal to Video Deinterlacing Module
	int width, height ;					//	Width and Height of the input image
	int strength ;						//	The strength for Deinterlacing
										//		Range [0, 1023]
										//		0 =>	Almost no interpolation, no matter how high is the motion level
										//		63 =>	Always perform interpolation, no matter how low is the motion level
} VideoDeinterlaceHandle ;

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
VideoDeinterlaceHandle *videoDeinterlaceInit(int width, int height, int strength) ;

//	videoDeinterlace:	Deinterlacing the current frame based on a reference frame
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//		pRef:					Pointer to the reference frame in YUV420 format
//		pCur:					Pointer to the current frame in YUV420 format
//		pOut:					Pointer to a pre-allocated output frame in YUV420 format
//	Output:
//		pOut:					Deinterlaced frame data are stored in the location addressed by pOut
//	Return:						NIL
void videoDeinterlace(VideoDeinterlaceHandle *hVideoDeinterlace, FRAME_Y420 *pRef, FRAME_Y420 *pCur, FRAME_Y420 *pOut) ;

//	videoDeinterlaceFinal:	Release all the resources associated with the videoDeinterlaceHandle
//	Input:
//		hVideoDeinterlace:		Pointer to an initialized VideoDeinterlaceHandle
//	Return:						NIL
void videoDeinterlaceFinal(VideoDeinterlaceHandle *hVideoDeinterlace) ;

#ifdef __cplusplus
}
#endif

#endif
