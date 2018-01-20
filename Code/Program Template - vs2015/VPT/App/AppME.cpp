#include "StdAfx.h"
#include "AppME.h"

CAppME::CAppME(void)
{
	// Class Constructor
}

CAppME::~CAppME(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

unsigned char *CAppME::NewFrame(FRAME_YUV420 *pFrame, int width, int height, int hPad, int vPad) {

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

void CAppME::FreeFrame(FRAME_YUV420 *pFrame) {

	if(pFrame->Y) {
		delete [] pFrame->Y ;
		pFrame->Y = NULL ;
		pFrame->U = NULL ;
		pFrame->V = NULL ;
	}
}

void CAppME::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists

	NewFrame(&cur, width, height, 0, 0) ;
	NewFrame(&recon, width, height, 0, 0) ;

	hME[0] = motionEstimationInit(width, height, 32, 1) ;
	hME[1] = motionEstimationInit(width, height, 32, 1) ;

	CreateTunnel(width + hME[0]->ref.xOff * 2, height + hME[0]->ref.yOff * 2, &pRefRGB) ;
	SetTitle(pRefRGB, _T("Reference Frame")) ;

	CreateTunnel(width / 2 + hME[0]->hRef.xOff * 2, height / 2 + hME[0]->hRef.yOff * 2, &pHRefRGB) ;
	SetTitle(pHRefRGB, _T("Half Reference Frame")) ;

	CreateTunnel(width / 4 + hME[0]->qRef.xOff * 2, height / 4 + hME[0]->qRef.yOff * 2, &pQRefRGB) ;
	SetTitle(pQRefRGB, _T("Quarter Reference Frame")) ;

	frameNum = 0 ;
}

void CAppME::CustomFinal(void) {
	// Add custom finalization code here

	FreeFrame(&cur) ;
	FreeFrame(&recon) ;

	motionEstimationFinal(hME[0]) ;
	motionEstimationFinal(hME[1]) ;
	hME[0] = NULL ;
	hME[1] = NULL ;

	RemoveTunnel(pRefRGB) ;
	RemoveTunnel(pHRefRGB) ;
	RemoveTunnel(pQRefRGB) ;
}

void CAppME::Process(void) {

	// The template generates a dummy copy frame applications
	CImageConvert imgConvert ;
	int meAlg[2] ;

	imgConvert.RGB24_to_YUV420(pInput, cur.Y, width, height) ;

	if(!frameNum) {
		meSetReference(hME[0], &cur) ;
		meSetReference(hME[1], &cur) ;
	}

	meAlg[0] = ME_HFS ;
	meAlg[1] = ME_RCS ;
	motionEstimation(hME[0], &cur, meAlg[0], 1) ;
	motionEstimation(hME[1], &cur, meAlg[1], 1) ;

	if(pRefRGB) {
		imgConvert.YUV420_to_RGB24(hME[0]->ref.Y, pRefRGB, width + hME[0]->ref.xOff * 2, height + hME[0]->ref.yOff * 2) ;
		PutDC(pRefRGB) ;
		GLStart(pRefRGB) ;
		{
			int i, j ;
			int x, y ;

			glColor3d(1.0, 0.0, 0.0) ;
			glBegin(GL_LINES) ;
			for(j = 0; j < hME[0]->nMBy; j++) {
				y = height - (j * 16 + 8) + hME[0]->ref.yOff ;
				for(i = 0; i < hME[0]->nMBx; i++) {
					x = i * 16 + 8 + hME[0]->ref.xOff ;
					glVertex2i(x, y) ;
					glVertex2i(x + hME[0]->pMVs[i + j * hME[0]->nMBx][0], y - hME[0]->pMVs[i + j * hME[0]->nMBx][1]) ;
//					glVertex2i(x + hME[0]->qMVs[i + j * hME[0]->nMBx][0] * 4, y - hME[0]->qMVs[i + j * hME[0]->nMBx][1] * 4) ;
				}
			}
			glEnd() ;

			glColor3d(0.0, 0.0, 1.0) ;
			glBegin(GL_LINES) ;
			for(j = 0; j < hME[1]->nMBy; j++) {
				y = height - (j * 16 + 8) + hME[1]->ref.yOff ;
				for(i = 0; i < hME[1]->nMBx; i++) {
					x = i * 16 + 8 + hME[1]->ref.xOff ;
					glVertex2i(x, y) ;
					glVertex2i(x + hME[1]->pMVs[i + j * hME[1]->nMBx][0], y - hME[1]->pMVs[i + j * hME[1]->nMBx][1]) ;
				}
			}
			glEnd() ;
		}
		GLFinish(pRefRGB) ;
	}

	if(pHRefRGB) {
		imgConvert.YUV420_to_RGB24(hME[0]->hRef.Y, pHRefRGB, width / 2 + hME[0]->hRef.xOff * 2, height / 2 + hME[0]->hRef.yOff * 2) ;
		PutDC(pHRefRGB) ;

		GLStart(pHRefRGB) ;
		{
			int i, j ;
			int x, y ;

			glColor3d(1.0, 0.0, 0.0) ;
			glBegin(GL_LINES) ;
			for(j = 0; j < hME[0]->nMBy; j++) {
				y = hME[0]->hRef.height - (j * 8 + 2) + hME[0]->hRef.yOff ;
				for(i = 0; i < hME[0]->nMBx; i++) {
					x = i * 8 + 2 + hME[0]->hRef.xOff ;
					glVertex2i(x, y) ;
					glVertex2i(x + hME[0]->hMVs[i + j * hME[0]->nMBx][0], y - hME[0]->hMVs[i + j * hME[0]->nMBx][1]) ;
				}
			}
			glEnd() ;
		}
		GLFinish(pHRefRGB) ;
	}

	if(pQRefRGB) {
		imgConvert.YUV420_to_RGB24(hME[0]->qRef.Y, pQRefRGB, width / 4 + hME[0]->qRef.xOff * 2, height / 4 + hME[0]->qRef.yOff * 2) ;
		PutDC(pQRefRGB) ;

		GLStart(pQRefRGB) ;
		{
			int i, j ;
			int x, y ;

			glColor3d(1.0, 0.0, 0.0) ;
			glBegin(GL_LINES) ;
			for(j = 0; j < hME[0]->nMBy; j++) {
				y = hME[0]->qRef.height - (j * 4 + 2) + hME[0]->qRef.yOff ;
				for(i = 0; i < hME[0]->nMBx; i++) {
					x = i * 4 + 2 + hME[0]->qRef.xOff ;
					glVertex2i(x, y) ;
					glVertex2i(x + hME[0]->qMVs[i + j * hME[0]->nMBx][0], y - hME[0]->qMVs[i + j * hME[0]->nMBx][1]) ;
				}
			}
			glEnd() ;
		}
		GLFinish(pQRefRGB) ;
	}

	motionCompensation(hME[0], &recon) ;

	{
		double psnrY, psnrU, psnrV ;

		getPSNRFrame420(&recon, &cur, &psnrY, &psnrU, &psnrV) ;

		switch(meAlg[0]) {
		case ME_FS:
			printf(_T("Full Search PSNR: ")) ;
			break ;
		case ME_HFS:
			printf(_T("Hier Search PSNR: ")) ;
			break ;
		case ME_RCS:
			printf(_T("RC   Search PSNR: ")) ;
			break ;
		}

		printf(_T("(Y) %3.3f, (U) %3.3f, (V) %3.3f\r\n"), psnrY, psnrU, psnrV) ;
	}

	imgConvert.YUV420_to_RGB24(recon.Y, pOutput, width, height) ;

	motionCompensation(hME[1], &recon) ;

	{
		double psnrY, psnrU, psnrV ;

		getPSNRFrame420(&recon, &cur, &psnrY, &psnrU, &psnrV) ;

		switch(meAlg[1]) {
		case ME_FS:
			printf(_T("Full Search PSNR: ")) ;
			break ;
		case ME_HFS:
			printf(_T("Hier Search PSNR: ")) ;
			break ;
		case ME_RCS:
			printf(_T("RC   Search PSNR: ")) ;
			break ;
		}

		printf(_T("(Y) %3.3f, (U) %3.3f, (V) %3.3f\r\n"), psnrY, psnrU, psnrV) ;
	}

	imgConvert.YUV420_to_RGB24(recon.Y, pInput, width, height) ;

	meSetReference(hME[0], &cur) ;
	meSetReference(hME[1], &cur) ;

//	imgConvert.YUV420_to_RGB24(recon.Y, pOutput, width, height) ;
	frameNum++ ;

//	memcpy(pOutput, pInput, width * height * 3) ;
	PutDC(pOutput) ;
	PutDC(pInput) ;
}
