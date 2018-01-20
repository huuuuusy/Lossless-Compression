#include "StdAfx.h"
#include "AppTest.h"
//#include "cabac.h"
#include "../Tools/ImageConvert.h"

CAppTest::CAppTest(void)
{
	// Class Constructor
}

CAppTest::~CAppTest(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppTest::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
	CreateTunnel(256, 256, &rgb) ;
	yuv = new unsigned char[256 * 256 * 3 / 2] ;
}

void CAppTest::CustomFinal(void) {
	// Add custom finalization code here

	RemoveTunnel(rgb) ;

	delete [] yuv ;
}

void CAppTest::Process(void) {

	// The template generates a dummy copy frame applications
	CImageConvert imgConvert ;

	memcpy(pOutput, pInput, width * height * 3) ;
	PutDC(pOutput) ;

	{
		int i, j ;

		for(i = 0; i < 256; i++) {
			for(j = 0; j < 256; j++) {
				yuv[(i + j * 256)] = 128 ;
				yuv[256 * 256 + (i >> 1) + (j >> 1) * 128] = i ;
				yuv[256 * 256 + 128 * 128 + (i >> 1) + (j >> 1) * 128] = 255 - j ;
			}
		}

		imgConvert.YUV420_to_RGB24(yuv, rgb, 256, 256) ;
		PutDC(rgb) ;
	}
/*
 {

	CABAC_CTX eCtx, dCtx ;
	BITSTREAM bs ;
	unsigned char bsBuf[1024] ;
	int data, rData, bit ;
	int i, j ;

	cabacInitContext(&eCtx, 0, 30, 0) ;
	cabacInitContext(&dCtx, 0, 30, 0) ;
	cabacInitEncode(&eCtx) ;

	eCtx.ctxIdx = dCtx.ctxIdx = 27 ;
	bsInit(&bs, bsBuf, 1024) ;

	srand(time(NULL)) ;

	printf(_T("Encode Start\r\n")) ;
	for(j = 0; j < 5; j++) {
		data = rand() ;
		data |= 1 ;
		for(i = 0; i < 32; i++) {
			printf(_T("%d"), (data >> (31 - i)) & 1) ;
			if(i == 31 && j == 4) {
				cabacEncodeTerminate(&eCtx, &bs, (data >> (31 - i)) & 1) ;
			}
			else cabacEncodeBin(&eCtx, &bs, 0, (data >> (31 - i)) & 1) ;
		}
	}

	printf(_T("\r\nBitstream Size = %d bits\r\n"), bsFilledSizeInBits(&bs)) ;

	cabacInitDecode(&dCtx, &bs) ;

	printf(_T("Decode Start\r\n")) ;
	for(j = 0; j < 5; j++) {
		for(i = 0; i < 32; i++) {
			if(i == 31 && j == 4) {
				cabacDecodeTerminate(&dCtx, &bs, &bit) ;
			} else cabacDecodeBin(&dCtx, &bs, 0, &bit) ;
			printf(_T("%d"), bit) ;
			rData = (rData << 1) | bit ;
		}
	}
	printf(_T("\r\nDecode Terminate\r\n")) ;

	printf(_T("%d, %d\r\n"), data, rData) ;
}
*/
}
