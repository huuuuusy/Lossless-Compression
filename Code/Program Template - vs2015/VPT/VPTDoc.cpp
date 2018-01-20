// VPTDoc.cpp : implementation of the CVPTDoc class
//

#include "stdafx.h"
#include "VPT.h"

#include "VPTDoc.h"
#include "ConsoleView.h"
#include "VPTView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVPTDoc

IMPLEMENT_DYNCREATE(CVPTDoc, CDocument)

BEGIN_MESSAGE_MAP(CVPTDoc, CDocument)
END_MESSAGE_MAP()


// CVPTDoc construction/destruction

CVPTDoc::CVPTDoc()
{
	// TODO: add one-time construction code here
	inTunnelView = NULL ;
}

CVPTDoc::~CVPTDoc()
{
	CVPTView *pView = (CVPTView *) inTunnelView ;
	if(pView) {
		pView->tunnel.Remove(this) ;
	}
}

void CVPTDoc::printf(const TCHAR *fmt, ...) {

	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	CDocument *consoleDoc ;

	if(pApp->consoleExist) {
		va_list ap ;
		TCHAR str[512] ;

		va_start(ap, fmt) ;

		_vstprintf_s(str, fmt, ap) ;
		consoleDoc = pApp->consoleDoc ;
		POSITION pos = consoleDoc->GetFirstViewPosition() ;
		((CConsoleView *) consoleDoc->GetNextView(pos))->printf(str) ;
		va_end(ap) ;
	}
}

void CVPTDoc::PutDC(HDC hDC) {

	if(hDC) imageLoader.PutDC(hDC) ;
	else {
		POSITION pos ;
		CView *pView ;

		pos = GetFirstViewPosition() ;
		do {
			pView = (CView *) GetNextView(pos) ;
			if(pView->IsKindOf(RUNTIME_CLASS(CVPTView))) {
				imageLoader.PutDC(((CVPTView *) pView)->hDC) ;
			}
		} while(pos != NULL) ;
	}
}

BOOL CVPTDoc::OnNewDocument()
{
	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;

	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	if(pApp->newCapture) {
		imageLoader.type = IMG_TYPE_CAP ;
		pApp->newCapture = FALSE ;
		if(imageLoader.CapInit(pApp->m_pMainWnd->m_hWnd) != S_OK || !imageLoader.bits) {
			AfxMessageBox(_T("Cannot open capture device!")) ;
			return FALSE ;
		}
		return TRUE;
	}
	if(pApp->newConsole) {
		/*
		if(pApp->newTunnel) {
			pApp->newTunnel = FALSE ;
			return TRUE;
		}
		*/
		pApp->newConsole = FALSE ;
		return TRUE;
	}
	if(pApp->newTunnel) {
		pApp->newTunnel = FALSE ;
		return TRUE;
	}
	return FALSE;
}

BOOL CVPTDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
/*
	CImgParam ParamDlg ;

	ParamDlg.DoModal() ;
*/

	imageLoader.type = IMG_TYPE_UDF ;
	if(imageLoader.Load(lpszPathName) != S_OK || !imageLoader.bits) {
		AfxMessageBox(_T("File cannot be opened!"));
		return FALSE ;
	}

	return TRUE;
}


// CVPTDoc serialization

void CVPTDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CVPTDoc diagnostics

#ifdef _DEBUG
void CVPTDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CVPTDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CVPTDoc commands
