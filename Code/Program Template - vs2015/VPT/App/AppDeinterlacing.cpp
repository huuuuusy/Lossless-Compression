#include "StdAfx.h"
#include "AppDeinterlacing.h"
#include "../Tools/videoDeinterlace.h"
#include "../Tools/imageConvert.h"
#include "../processing.h"
#include "../AviWriter.h"

CAppDeinterlacing::CAppDeinterlacing(void)
{
}

CAppDeinterlacing::~CAppDeinterlacing(void)
{
}

void CAppDeinterlacing::CustomInit(CView *pView)
{
	frameCurYUV = (unsigned char *) malloc(width * height * 2) ;
	frameBtmYUV = (unsigned char *) malloc(width * height * 2) ;
	frameRefYUV = (unsigned char *) malloc(width * height * 2) ;
	frameOutYUV = (unsigned char *) malloc(width * height * 2) ;

	hVideoDeinterlace = videoDeinterlaceInit(width, height, 40) ;

	SetTitle(pOutput, _T("Deinterlaced Video")) ;

	refReady = 0 ;
#ifdef STAB_AVI_OUTPUT
	aviWriter.init(_T("\\Temp\\videoDeinterlace.avi"), width, height) ;
#endif
//	dumpFP = fopen("C:\\TEMP\\vdi.yuv", "wb") ;
}

void CAppDeinterlacing::CustomFinal(void)
{
	free(frameCurYUV) ;
	free(frameBtmYUV) ;
	free(frameRefYUV) ;
	free(frameOutYUV) ;

	if(hVideoDeinterlace) videoDeinterlaceFinal(hVideoDeinterlace) ;
//	fclose(dumpFP) ;
#ifdef STAB_AVI_OUTPUT
	aviWriter.close() ;
#endif
}

void CAppDeinterlacing::Perform()
{
	static CImageConvert imgConvert ;
	FRAME_Y420 CUR, REF, DI ;


	CUR.Y = frameCurYUV ;
	CUR.U = CUR.Y + width * height ;
	CUR.V = CUR.U + width * height / 4 ;
	REF.Y = frameRefYUV ;
	REF.U = REF.Y + width * height ;
	REF.V = REF.U + width * height / 4 ;
	DI.Y = frameOutYUV ;
	DI.U = DI.Y + width * height ;
	DI.V = DI.U + width * height / 4 ;
	CUR.strideY = REF.strideY = DI.strideY = width ;
	CUR.strideUV = REF.strideUV = DI.strideUV = width / 2 ;
	
	GetSrc() ;
	imgConvert.RGB24_to_YUV420(pInput, frameCurYUV, width, height) ;

	if(height < 480) {
		((CVPTDoc *) pDoc)->imageLoader.Next() ;
		GetSrc() ;
		imgConvert.RGB24_to_YUV420(pInput, frameBtmYUV, width, height) ;
		{
			int j ;

			for(j = 1; j < height; j += 2) {
				memcpy(frameCurYUV + j * width, frameBtmYUV + j * width, width) ;
			}
		}
	}

//	fwrite(frameCurYUV, width * height * 3 / 2, 1, dumpFP) ;
	int ticks ;
	ticks = GetTickCount() ;
	imgConvert.YUV420_to_RGB24(frameCurYUV, pInput, width, height) ;
	imgConvert.YUV420_to_RGB24(frameCurYUV, pInput, width, height) ;
	imgConvert.YUV420_to_RGB24(frameCurYUV, pInput, width, height) ;
	imgConvert.YUV420_to_RGB24(frameCurYUV, pInput, width, height) ;
	imgConvert.YUV420_to_RGB24(frameCurYUV, pInput, width, height) ;
	ticks = GetTickCount() - ticks ;
	printf(_T("SSE Version = %d ticks\r\n"), ticks) ;

	PutDC(pInput) ;

	if(!refReady) {
		memset(REF.Y, 0, width * height) ;
		memset(REF.U, 0, width * height / 2) ;
		refReady = 1 ;
	}

	videoDeinterlace(hVideoDeinterlace, &REF, &CUR, &DI) ;

//	memset(frameOutYUV + width * height, 128, width * height / 2) ;
	imgConvert.YUV420_to_RGB24(frameOutYUV, pOutput, width, height) ;
	PutDC(pOutput) ;

	memcpy(frameRefYUV, frameCurYUV, width * height * 3 / 2) ;
#ifdef DI_AVI_OUTPUT
	aviWriter.write(pInput) ;
#endif

	UpdateTunnel() ;
}