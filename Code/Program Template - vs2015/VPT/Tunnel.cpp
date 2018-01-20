#include "StdAfx.h"
#include ".\tunnel.h"
#include "VPTView.h"


CTunnel::CTunnel(void)
{
	count = 0 ;
}

CTunnel::~CTunnel(void)
{
	for(int i = 0; i < count; i++) {
		docList[i]->inTunnelView = NULL ;
	}
}

CVPTDoc *CTunnel::Create(int width, int height, unsigned char **ptr) {

	CVPTDoc *pDoc = (CVPTDoc *) pView->GetDocument() ;
	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	CVPTDoc *pNewDoc ;
	CDocTemplate *pTemplate = pDoc->GetDocTemplate() ;

	if(count >= MaxTunnelCount) return NULL ;
	pApp->newTunnel = TRUE ;
//	pNewDoc = (CIPTDoc *) pTemplate->OpenDocumentFile(NULL) ;
	pNewDoc = (CVPTDoc *) ((CVPTApp *) AfxGetApp())->pImageTemplate->OpenDocumentFile(NULL) ;
	
	if(pNewDoc) {
		pNewDoc->imageLoader.Create(width, height, NULL) ;
		POSITION pos = pNewDoc->GetFirstViewPosition() ;

		do {
			CView *pView = (CView *) pNewDoc->GetNextView(pos) ;
			pView->OnInitialUpdate() ;
		} while(pos != NULL) ;
		docList[count] = pNewDoc ;
		framePtr[count] = ptr ;
		count++ ;
		pNewDoc->inTunnelView = (CView *) pView ;
		*ptr = pNewDoc->imageLoader.bits ;
		return pNewDoc ;
	}
	*ptr = NULL ;
	return NULL ;
}

void CTunnel::PutDC(CVPTDoc *pDoc) {

	pDoc->PutDC() ;
}

void CTunnel::SetTitle(CVPTDoc *pDoc, TCHAR *fmt, ...) {

	va_list ap ;
	TCHAR str[512] ;

	va_start(ap, fmt) ;

	_vstprintf_s(str, fmt, ap) ;
	pDoc->SetTitle(str) ;

	va_end(ap) ;
}

void CTunnel::SetTitle(unsigned char *bits, TCHAR *fmt, ...) {

	int i ;
	va_list ap ;
	TCHAR str[512] ;

	va_start(ap, fmt) ;

	_vstprintf_s(str, fmt, ap) ;
	va_end(ap) ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			docList[i]->SetTitle(str) ;
		}
	}
}

void CTunnel::PutDC(unsigned char *bits) {

	int i ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			PutDC(docList[i]) ;
		}
	}
}

HDC CTunnel::GetDC(CVPTDoc *pDoc) {

	POSITION pos ;
	CVPTView *pView ;

	pos = pDoc->GetFirstViewPosition() ;
	do {
		pView = (CVPTView *) pDoc->GetNextView(pos) ;
		if(pView->IsKindOf(RUNTIME_CLASS(CVPTView))) {
			break ;
		}
	} while(pos != NULL) ;
	return pView->hDC ;
}

HDC CTunnel::GetDC(unsigned char *bits) {

	int i ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			return GetDC(docList[i]) ;
		}
	}
	return NULL ;
}

void CTunnel::GLStart(CVPTDoc *pDoc) {
	
	POSITION pos ;
	CVPTView *pView ;

	pos = pDoc->GetFirstViewPosition() ;
	do {
		pView = (CVPTView *) pDoc->GetNextView(pos) ;
		if(pView->IsKindOf(RUNTIME_CLASS(CVPTView))) {
			break ;
		}
	} while(pos != NULL) ;
	pView->GLStart() ;
}

void CTunnel::GLStart(unsigned char *bits) {

	int i ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			return GLStart(docList[i]) ;
		}
	}
}
	
void CTunnel::GLFinish(CVPTDoc *pDoc) {
	
	POSITION pos ;
	CVPTView *pView ;

	pos = pDoc->GetFirstViewPosition() ;
	do {
		pView = (CVPTView *) pDoc->GetNextView(pos) ;
		if(pView->IsKindOf(RUNTIME_CLASS(CVPTView))) {
			break ;
		}
	} while(pos != NULL) ;
	pView->GLFinish() ;
}

void CTunnel::GLFinish(unsigned char *bits) {

	int i ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			return GLFinish(docList[i]) ;
		}
	}
}

int CTunnel::RemoveAll(void) {

	int i = 0 ;

	while(count > 0) {
		Remove(docList[0]) ;
		i++ ;
	}

	return i > 0 ? 1 : 0 ;
}

int CTunnel::Remove(CVPTDoc *pDoc) {
	
	int i, j ;

	i = 0 ;
	while(pDoc != docList[i] && i < count) i++ ;

	if(i < count) {
		pDoc->inTunnelView = NULL ;
		*framePtr[i] = NULL ;
		for(j = i; j < (count - 1) ; j++) {
			docList[j] = docList[j+1] ;
			framePtr[j] = framePtr[j+1] ;
		}
		count-- ;
		return 1 ;
	}
	return 0 ;
}

int CTunnel::Remove(unsigned char *bits) {
	int i ;

	for(i = 0; i < count; i++) {
		if(*(framePtr[i]) == bits) {
			return Remove(docList[i]) ;
		}
	}
	return 0 ;
}

void CTunnel::Update(void) {

	int i ;

	for(i = 0; i < count; i++) {
//		docList[i]->UpdateAllViews(NULL) ;
		POSITION pos = docList[i]->GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = docList[i]->GetNextView(pos);
			InvalidateRect(pView->m_hWnd, 0, TRUE) ;
		}
	}
}