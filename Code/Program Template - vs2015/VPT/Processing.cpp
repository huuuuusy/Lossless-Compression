#include "StdAfx.h"
#include "VPTView.h"
#include "Processing.h"

CProcessing::CProcessing(void)
{
	pView = NULL ;
	pDoc = NULL ;
	pOutput = NULL ;
	pInput = NULL ;
	width = height = 0 ;
	tunnelCount = 0 ;
}

CProcessing::~CProcessing(void)
{
	Final() ;
}

void CProcessing::printf(const TCHAR *fmt, ...) {

	va_list ap ;
	TCHAR str[512] ;

	va_start(ap, fmt) ;

	_vstprintf_s(str, fmt, ap) ;
	((CVPTDoc *) (pDoc))->printf(str) ;

	va_end(ap) ;
}

void CProcessing::GLStart(unsigned char *bits) {

	if(!bits) {
		((CVPTView *) pView)->tunnel.GLStart(pOutput) ;
	} else if(bits == pInput) {
		((CVPTView *) pView)->GLStart() ;
	} else {
		((CVPTView *) pView)->tunnel.GLStart(bits) ;
	}
}

void CProcessing::GLFinish(unsigned char *bits) {

	if(!bits) {
		((CVPTView *) pView)->tunnel.GLFinish(pOutput) ;
	} else if(bits == pInput) {
		((CVPTView *) pView)->GLFinish() ;
		((CVPTDoc *) pDoc)->UpdateAllViews(NULL) ;
	} else {
		((CVPTView *) pView)->tunnel.GLFinish(bits) ;
		((CVPTView *) pView)->tunnel.Update() ;
	}
}

int CProcessing::GetFrameNum(void) {

	return ((CVPTDoc *) pDoc)->imageLoader.index ;
}

void CProcessing::PutDC(unsigned char *bits) {

	if(!bits) {
		((CVPTView *) pView)->tunnel.PutDC(pOutput) ;
	} else if(bits == pInput) {
		((CVPTView *) pView)->GetDocument()->PutDC(((CVPTView *) pView)->hDC) ;
	} else {
		((CVPTView *) pView)->tunnel.PutDC(bits) ;
	}
}

HDC CProcessing::GetDC(unsigned char *bits) {
	if(!bits) {
		return ((CVPTView *) pView)->tunnel.GetDC(pOutput) ;
	} else if(bits == pInput) {
		return ((CVPTView *) pView)->hDC ;
	} else {
		return ((CVPTView *) pView)->tunnel.GetDC(bits) ;
	}
}

unsigned char *CProcessing::GetBits(void) {

	return pInput = ((CVPTDoc *) pDoc)->GetBits() ;
}

unsigned char *CProcessing::GetSrc(void) {

	return pInput = ((CVPTDoc *) pDoc)->GetBits() ;
}

BOOL CProcessing::TunnelValid(void) {

	return pOutput != NULL ;
}

void CProcessing::Init(CView *pView) {

	CVPTDoc *pVPTDoc ;

	this->pView = pView ;
	pVPTDoc = ((CVPTView *) pView)->GetDocument() ;
	pDoc = (CDocument *) pVPTDoc ;
	width = pVPTDoc->ImageWidth() ;
	height = pVPTDoc->ImageHeight() ;
	pInput = pVPTDoc->GetBits() ;

	CreateTunnel(width, height, &pOutput) ;

	CustomInit(pView) ;
}

void CProcessing::Final(void) {

	CVPTView *pVPTView ;

	CustomFinal() ;

	if(pView) {
		pVPTView = (CVPTView *) pView ;
		pVPTView->tunnel.Remove(pOutput) ;
//		pVPTView->tunnel.RemoveAll() ;
	}
}

void CProcessing::CreateTunnel(int w, int h, unsigned char **bits) {

	CVPTView *pVPTView ;
	CVPTDoc *pVPTDoc ;

	pVPTView = (CVPTView *) pView ;
	pVPTDoc = pVPTView->GetDocument() ;
	pVPTView->tunnel.Create(w, h, bits) ;
	memset(*bits, 0, w * h * 3) ;
	if(*bits) {
		tunnelBits[tunnelCount] = *bits ;
		pVPTView->tunnel.PutDC(tunnelBits[tunnelCount]) ;
		pVPTView->tunnel.Update() ;
		tunnelCount++ ;
	}
}

void CProcessing::RemoveTunnel(unsigned char *bits) {

	CVPTView *pVPTView ;
	int i, j ;

	pVPTView = (CVPTView *) pView ;
	pVPTView->tunnel.Remove(bits) ;

	for(i = 0; i < tunnelCount; i++) {
		if(bits == tunnelBits[i])
			break ;
	}

	if(i < tunnelCount) {
		tunnelCount-- ;
		for(j = i; j < tunnelCount; j++) {
			tunnelBits[j] = tunnelBits[j+1] ;
		}
	}
}

void CProcessing::UpdateTunnel(void) {
	((CVPTView *) pView)->tunnel.Update() ;
}

void CProcessing::SetTitle(unsigned char *bits, TCHAR *fmt, ...) {

	va_list ap ;
	TCHAR str[512] ;

	va_start(ap, fmt) ;

	_vstprintf_s(str, fmt, ap) ;
	((CVPTView *) pView)->tunnel.SetTitle(bits, str) ;
	
	va_end(ap) ;
}

void CProcessing::CustomInit(CView *pView) {
}

void CProcessing::CustomFinal(void) {
}

void CProcessing::Perform(void) {

	GetBits() ;
	Process() ;
}

void CProcessing::Process(void) {

	memcpy(pOutput, pInput, width * height * 3) ;
	((CVPTView *) pView)->tunnel.PutDC(pOutput) ;
}