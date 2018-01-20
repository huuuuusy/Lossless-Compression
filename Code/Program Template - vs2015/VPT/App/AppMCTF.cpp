#include "StdAfx.h"
#include "AppMCTF.h"

CAppMCTF::CAppMCTF(void)
{
	// Class Constructor
}

CAppMCTF::~CAppMCTF(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

unsigned char *CAppMCTF::NewFrame(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) {

	int frameWidth ;
	int frameHeight ;

	frameWidth = hPad * 2 + width ;
	frameHeight = vPad * 2 + height ;

	pFrame->strideY = frameWidth ;
	pFrame->strideUV = pFrame->strideY >> 1 ;

	pFrame->Y = new unsigned char [frameWidth * frameHeight + frameWidth * frameHeight / 2] ;
	pFrame->U = pFrame->Y + frameWidth * frameHeight ;
	pFrame->V = pFrame->U + frameWidth * frameHeight / 4 ;

	pFrame->xOff = hPad ;
	pFrame->yOff = vPad ;

	pFrame->width = width ;
	pFrame->height = height ;

	return pFrame->Y ;
}

void CAppMCTF::FreeFrame(FRAME_YUV420 *pFrame) {

	if(pFrame->Y) {
		delete [] pFrame->Y ;
		pFrame->Y = NULL ;
		pFrame->U = NULL ;
		pFrame->V = NULL ;
	}
}

void CAppMCTF::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists

	NewFrame(&cur, width, height, 0, 0) ;
	NewFrame(&filtered, width, height, 0, 0) ;
	NewFrame(&noisy, width, height, 0, 0) ;

	pNoiseGen = new CNoiseGenerator ;
	pNoisyInput = (unsigned char *) new unsigned char[width * height * 3] ;

	{
		int i ;

		for(i = 0; i < 3; i++) {
			NewFrame(&buf[i], width, height, 0, 0) ;

			pBuf[i] = &buf[i] ;
		}
	}

	hMCTF = mcTemporalFilterInit(width, height, 16, 32) ;

	CreateTunnel(width, height, &pFilteredRGB) ;

	frameNum = 0 ;
}

void CAppMCTF::CustomFinal(void) {
	// Add custom finalization code here

	FreeFrame(&cur) ;
	FreeFrame(&filtered) ;
	FreeFrame(&noisy) ;

	delete [] pNoisyInput ;
	delete pNoiseGen ;

	{
		int i ;

		for(i = 0; i < 3; i++) {
			FreeFrame(&buf[i]) ;
		}
	}

	mcTemporalFilterFinal(hMCTF) ;

	RemoveTunnel(pFilteredRGB) ;
}

void CAppMCTF::Process(void) {

#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MED(a, b, c) MIN(MIN(MAX(a, b), MAX(b, c)), MAX(a, c))

	// The template generates a dummy copy frame applications
	CImageConvert imgConvert ;

	MotionEstimationHandle *hME ;

	memcpy(pNoisyInput, pInput, width * height * 3) ;

	{
		int i, j ;
		int y ;
		int noiseLevel ;

		noiseLevel = 16 ;

		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				y = pInput[(i + j * width) * 3] + pNoiseGen->gaussianNoise(0, noiseLevel) ;
				if(y < 0) y = 0 ;
				if(y > 255) y = 255 ;
				pNoisyInput[(i + j * width) * 3] = y ;

				y = pInput[(i + j * width) * 3 + 1] + pNoiseGen->gaussianNoise(0, noiseLevel) ;
				if(y < 0) y = 0 ;
				if(y > 255) y = 255 ;
				pNoisyInput[(i + j * width) * 3 + 1] = y ;

				y = pInput[(i + j * width) * 3 + 2] + pNoiseGen->gaussianNoise(0, noiseLevel) ;
				if(y < 0) y = 0 ;
				if(y > 255) y = 255 ;
				pNoisyInput[(i + j * width) * 3 + 2] = y ;
			}
		}
	}

	imgConvert.RGB24_to_YUV420(pInput, cur.Y, width, height) ;
	imgConvert.RGB24_to_YUV420(pNoisyInput, noisy.Y, width, height) ;

	hME = hMCTF->hME ;

	/*
	{
		int i, j ;
		unsigned char *pCurY ;
		int y ;

		pCurY = getActiveFrame420_Y(&cur) ;

		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				y = pCurY[i + j * cur.strideY] + noiseGen.gaussianNoise(0, 4) ;
				if(y < 16) y = 16 ;
				if(y > 240) y = 240 ;
				pCurY[i + j * cur.strideY] = y ;
			}
		}
	}
	*/

	imgConvert.YUV420_to_RGB24(noisy.Y, pInput, width, height) ;
	PutDC(pInput) ;


	if(!frameNum) {
		copyFrame420(&cur, &buf[0]) ;
		copyFrame420(&cur, &buf[1]) ;
		copyFrame420(&cur, &buf[2]) ;
		copyFrame420(&cur, &filtered) ;
	}

	mcTemporalFilter(hMCTF, &noisy, &filtered) ;
	
	{
		double psnrY, psnrU, psnrV ;

		getPSNRFrame420(&noisy, &cur, &psnrY, &psnrU, &psnrV) ;

		printf(_T("Noisy Image - PSNR: (Y) %3.3f, (U) %3.3f, (V) %3.3f\r\n"), psnrY, psnrU, psnrV) ;

		getPSNRFrame420(&hMCTF->filtered, &cur, &psnrY, &psnrU, &psnrV) ;
		printf(_T("Denoised Image - PSNR: (Y) %3.3f, (U) %3.3f, (V) %3.3f\r\n"), psnrY, psnrU, psnrV) ;

		printf(_T("Frame %04d: Noise Level = %d\r\n"), frameNum, hMCTF->noiseLevel) ;
	}
	
	// printf(_T("Noise Level = %d\r\n"), hMCTF->noiseLevel) ;

	imgConvert.YUV420_to_RGB24(hMCTF->recon.Y, pOutput, width, height) ;
/*
	{
		int i, j ;
		unsigned char *pFilteredY ;
		int p0, p1, p2 ;

		pFilteredY = getActiveFrame420_Y(&filtered) ;

		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				p0 = pBuf[0]->Y[i + j * pBuf[0]->strideY] ;
				p1 = pBuf[1]->Y[i + j * pBuf[1]->strideY] ;
				p2 = pBuf[2]->Y[i + j * pBuf[2]->strideY] ;

				pFilteredY[i + j * filtered.strideY] = MED(p0, p1, p2) ;
//				pFilteredY[i + j * filtered.strideY] = p2 ;
				pFilteredY[i + j * filtered.strideY] = (p0 + p1 + p2) / 3 ;
			}
		}
	}
*/
	imgConvert.YUV420_to_RGB24(filtered.Y, pFilteredRGB, width, height) ;

	if(pOutput) {
		PutDC(pOutput) ;
		GLStart(pOutput) ;
		{
			int i, j ;
			int x, y ;

			glColor3d(1.0, 0.0, 0.0) ;
			glBegin(GL_LINES) ;
			for(j = 0; j < hME->nMBy; j++) {
				y = height - (j * 16 + 8) ;
				for(i = 0; i < hME->nMBx; i++) {
					x = i * 16 + 8 ;
					glVertex2i(x, y) ;
					glVertex2i(x + hME->pMVs[i + j * hME->nMBx][0], y - hME->pMVs[i + j * hME->nMBx][1]) ;
				}
			}
			glEnd() ;

		}
		GLFinish(pOutput) ;
	}

	{
		FRAME_YUV420 *pTemp ;

		pTemp = pBuf[2] ;
		pBuf[2] = pBuf[1] ;
		pBuf[1] = pBuf[0] ;
		pBuf[0] = pBuf[2] ;
	}
	copyFrame420(&cur, pBuf[0]) ;

	frameNum++ ;

//	PutDC(pOutput) ;
	PutDC(pFilteredRGB) ;

//	imgConvert.YUV420_to_RGB24(blurred.Y, pInput, width, height) ;
//	PutDC(pInput) ;

#undef MIN
#undef MAX
#undef MED
}
