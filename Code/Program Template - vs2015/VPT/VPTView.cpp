// VPTView.cpp : implementation of the CVPTView class
//

#include "stdafx.h"
#include "VPT.h"

#include "VPTDoc.h"
#include "VPTView.h"
#include "ChildFrm.h"
#include "VideoCtrlFrm.h"
#include "Tunnel.h"
#include "App/AppEncodeDecode.h"
#include "App/AppConvert.h"
#include "App/AppCompress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

// CVPTView

IMPLEMENT_DYNCREATE(CVPTView, CScrollView)

BEGIN_MESSAGE_MAP(CVPTView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
//	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_EDIT_COPY, &CVPTView::OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, &CVPTView::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CVPTView::OnFileSave)
	ON_COMMAND(ID_EDIT_YUV, &CVPTView::OnEditYUV)
	ON_COMMAND(ID_EDIT_COMPRESS, &CVPTView::OnEditCompress)
	ON_COMMAND(ID_EDIT_YUV_Y, &CVPTView::OnEditYUV_Y)
	ON_COMMAND(ID_EDIT_YUV_U, &CVPTView::OnEditYUV_U)
	ON_COMMAND(ID_EDIT_YUV_V, &CVPTView::OnEditYUV_V)
END_MESSAGE_MAP()

// CVPTView construction/destruction

CVPTView::CVPTView()
{
	// TODO: add construction code here
	tunnel.pView = (CView *) this ;
	hDC = 0 ;
	hGLRC = 0 ;
	hBitmap = 0 ;
}

CVPTView::~CVPTView()
{
	CVPTDoc* pDoc = GetDocument();
	int i = 0 ;
	
	if(hGLRC) {
		wglMakeCurrent(NULL, NULL) ;
		wglDeleteContext(hGLRC) ;
	}
	if(hBitmap) DeleteObject(hBitmap) ;
	if(hDC) DeleteDC(hDC) ;

	if(ProcessingList.Size()) {
		ProcessingList.SeekHead() ;
		while(ProcessingList.Size()) {
			ProcessingList.Data()->Final() ;
			delete ProcessingList.Data() ;
			ProcessingList.Delete() ;
		}
	}
	tunnel.RemoveAll() ;
/*
	if(pDoc->playable) {
		pDoc->playable = FALSE ;
	}
	if(ProcessingList.Size()) {
		ProcessingList.SeekHead() ;
		while(ProcessingList.Size()) {
			ProcessingList.Data()->Final() ;
			delete ProcessingList.Data() ;
			ProcessingList.Delete() ;
		}
	}
	tunnel.RemoveAll() ;
*/
}

BOOL CVPTView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

BOOL CVPTView::OnEraseBkgnd(CDC *pDC) {

	CVPTDoc *pDoc = GetDocument() ;
	RECT clientRect ;
	RECT top, left, right, bottom ;
	int imageWidth ;
	int imageHeight ;

	imageWidth = pDoc->ImageWidth() ;
	imageHeight = pDoc->ImageHeight() ;

	GetClientRect(&clientRect) ;
	bottom.left = top.left = clientRect.left ;
	bottom.right = top.right = clientRect.right ;
	top.top = 0 ;
	top.bottom = clientRect.bottom / 2 - imageHeight / 2 ;
	top.bottom = top.bottom < 0 ? 0 : top.bottom ;
	bottom.top = top.bottom + imageHeight ;
	bottom.bottom = clientRect.bottom ;
	bottom.top = bottom.top > clientRect.bottom ? clientRect.bottom : bottom.top ;
	right.top = left.top = top.bottom ;
	right.bottom = left.bottom = bottom.top ;
	left.left = 0 ;
	left.right = clientRect.right / 2 - imageWidth / 2 ;
	left.right = left.right < 0 ? 0 : left.right ;
	right.right = clientRect.right ;
	right.left = left.right + imageWidth ;
	right.left = right.left > clientRect.right ? clientRect.right : right.left ;
	
	pDC->FillSolidRect(&top, 0x00FFFFFF) ;
	pDC->FillSolidRect(&bottom, 0x00FFFFFF) ;
	pDC->FillSolidRect(&left, 0x00FFFFFF) ;
	pDC->FillSolidRect(&right, 0x00FFFFFF) ;

	return TRUE ;
}

// CVPTView drawing

void CVPTView::OnDraw(CDC* pDC)
{
	CVPTDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here

	if(pDoc->imageLoader.bits) {
		RECT clientRect ;

		GetClientRect(&clientRect) ;

		BitBlt(pDC->GetSafeHdc(),
			(clientRect.right / 2 - pDoc->imageLoader.width / 2) > 0 ? (clientRect.right / 2 - pDoc->imageLoader.width / 2) : 0,
			(clientRect.bottom / 2 - pDoc->imageLoader.height / 2) > 0 ? (clientRect.bottom / 2 - pDoc->imageLoader.height / 2) : 0,
			pDoc->imageLoader.width, pDoc->imageLoader.height,
			hDC, 0, 0, SRCCOPY) ;
/*
		GLStart() ;
			glBegin(GL_LINES) ;
				glVertex2i(0, 0) ;
				glVertex2i(pDoc->imageLoader.width-1, pDoc->imageLoader.height-1) ;
			glEnd() ;
		GLFinish() ;
*/
	}
}

void CVPTView::ResizeToImage() {

	CVPTDoc* pDoc = GetDocument();
//	CSplitterWnd *splitter;

	if(pDoc->IsVideo()) {
//		splitter = (CSplitterWnd *) GetParentSplitter(this, TRUE) ;
//		splitter->SetColumnInfo(0, pDoc->imageLoader.width, 0) ;
//		splitter->SetRowInfo(0, pDoc->imageLoader.height, 0) ;
//		splitter->RecalcLayout() ;
		ResizeParentToFit(FALSE) ;
	} else {
		ResizeParentToFit(FALSE) ;
	}
}

int CVPTView::ImageWidth() {

	CVPTDoc* pDoc = GetDocument();

	return pDoc->ImageWidth() ;
}

int CVPTView::ImageHeight() {

	CVPTDoc* pDoc = GetDocument() ;
	return pDoc->ImageHeight() ;
}

void CVPTView::OnInitialUpdate()
{
	PIXELFORMATDESCRIPTOR pfd ;
	int nPixelFormat ;
	BITMAPINFOHEADER BIH ;
	unsigned char *memDCBits ;
	CVPTDoc* pDoc = GetDocument();

	CScrollView::OnInitialUpdate();

	/*
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
	*/

	// Fill in the header info. 
	if(!(pDoc->imageLoader.width && pDoc->imageLoader.height)) {
//		SetScrollSizes(MM_TEXT, CSize(pDoc->imageLoader.width, pDoc->imageLoader.height)) ;
//		ResizeToImage() ;
		return ;
	}

//	printf(_T("%d, %d\r\n"), pDoc->imageLoader.width, pDoc->imageLoader.height) ;

	memset(&BIH, 0, sizeof(BITMAPINFOHEADER));
	BIH.biSize = sizeof(BITMAPINFOHEADER);
	BIH.biWidth = (pDoc->imageLoader.width + 3) / 4 * 4;
	BIH.biHeight = pDoc->imageLoader.height;
	BIH.biPlanes = 1;
	BIH.biBitCount = 24;
	BIH.biCompression = BI_RGB;

	hDC = CreateCompatibleDC(NULL) ;

	hBitmap = CreateDIBSection(GetDC()->GetSafeHdc(), (BITMAPINFO *) &BIH, DIB_RGB_COLORS, (void **) &memDCBits, NULL, 0) ;
//	hBitmap = CreateCompatibleBitmap(GetDC()->GetSafeHdc(), (pDoc->imageLoader.width + 3) / 4 * 4, pDoc->imageLoader.height) ;
	SelectObject(hDC, hBitmap) ;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)) ;
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR); 
	pfd.nVersion   = 1 ; 
	pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_BITMAP | PFD_SUPPORT_GDI ;
	pfd.iPixelType = PFD_TYPE_RGBA ;
	pfd.cColorBits = 24 ;
	pfd.cDepthBits = 32 ;
	pfd.iLayerType = PFD_MAIN_PLANE ;
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	if(!SetPixelFormat (hDC, nPixelFormat, &pfd)) exit(0) ;

	SetPixelFormat (hDC, nPixelFormat, &pfd) ;

	hGLRC = wglCreateContext(hDC) ;
	if(!hGLRC) exit(0) ;


	pDoc->imageLoader.PutDC(hDC) ;
	SetScrollSizes(MM_TEXT, CSize(pDoc->imageLoader.width, pDoc->imageLoader.height)) ;

	if(GetParentFrame()->IsKindOf(RUNTIME_CLASS(CVideoCtrlFrame))) {
		if(!pDoc->IsVideo())
			((CVideoCtrlFrame *) GetParentFrame())->HideToolBar() ;
	}
	ResizeToImage() ;
}

void CVPTView::GLStart(void) {

	CVPTDoc *pDoc = GetDocument() ;

	wglMakeCurrent(hDC, hGLRC) ;
	glViewport(0, 0, pDoc->imageLoader.width, pDoc->imageLoader.height) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	glOrtho(-0.5, (double) pDoc->imageLoader.width+0.5, -0.5, (double) pDoc->imageLoader.height+0.5, -1000.0, 1000.0) ;
	glMatrixMode( GL_MODELVIEW) ;
	glLoadIdentity() ;
}

void CVPTView::GLFinish(void) {

	wglMakeCurrent(hDC, NULL) ;
}

void CVPTView::printf(const TCHAR *fmt, ...) {

	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	CVPTDoc *pDoc = GetDocument() ;

	if(pApp->consoleExist) {
		va_list ap ;
		TCHAR str[512] ;

		va_start(ap, fmt) ;

		_vstprintf_s(str, fmt, ap) ;

		va_start(ap, fmt) ;

		pDoc->printf(str) ;

		va_end(ap) ;
	}
}

void CVPTView::AddProcessing(CProcessing *pProc) {

	pProc->Init(this) ;
	ProcessingList.Append(pProc) ;
}

void CVPTView::RemoveProcessing(CProcessing *pProc) {
	ProcessingList.Delete(pProc) ;
}

void CVPTView::DoProcessing(void) {

	CProcessing *head, *pProc ;

	if(ProcessingList.Size()) {
		head = ProcessingList.SeekHead() ;
		while(head && !head->TunnelValid()) {
			head->Final() ;
			delete ProcessingList.Data() ;
			ProcessingList.Delete() ;
			if(ProcessingList.Size())
				head = ProcessingList.Data() ;
			else head = NULL ;
		}
		if(ProcessingList.Size()) {
			pProc = head ;
			do {
				if(!pProc->TunnelValid()) {
					pProc->Final() ;
					delete ProcessingList.Data() ;
					ProcessingList.Delete() ;
				} else {
					pProc = ProcessingList.Next() ;
				}
			} while(!ProcessingList.Sol()) ;
		}
	}

	if(ProcessingList.Size()) {
		pProc = head = ProcessingList.SeekHead() ;
		do {
			if(pProc->TunnelValid()) {
				pProc->Perform() ;
				pProc = ProcessingList.Next() ;
			}
		} while(!ProcessingList.Sol()) ;
		tunnel.Update() ;
	}
}

// CVPTView printing

BOOL CVPTView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CVPTView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CVPTView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CVPTView::OnTimer(UINT nTimer) {

	CScrollView::OnTimer(nTimer);
}

// CVPTView diagnostics

#ifdef _DEBUG
void CVPTView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CVPTView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CVPTDoc* CVPTView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVPTDoc)));
	return (CVPTDoc*)m_pDocument;
}
#endif //_DEBUG

// CVPTView message handlers

void CVPTView::OnEditCopy()
{
	// TODO: Add your command handler code here
	CBitmap bitmap;
	CBitmap *pOldBitmap ;
	CDC memDC;
	CVPTDoc *pDoc = GetDocument() ;
	int imageWidth ;
	int imageHeight ;

	imageWidth = pDoc->ImageWidth() ;
	imageHeight = pDoc->ImageHeight() ;

	memDC.CreateCompatibleDC(GetDC());

	bitmap.CreateCompatibleBitmap(GetDC(), imageWidth, imageHeight) ;

	pOldBitmap = memDC.SelectObject(&bitmap);
	memDC.BitBlt(0, 0, imageWidth, imageHeight, CDC::FromHandle(hDC), 0, 0, SRCCOPY) ;

	OpenClipboard() ;
	EmptyClipboard() ;
	SetClipboardData (CF_BITMAP, bitmap.GetSafeHandle()) ;
	CloseClipboard () ;

	memDC.SelectObject(pOldBitmap);
	bitmap.Detach();
}

void CVPTView::OnFileSave()
{
	// TODO: Add your command handler code here
	CImage image ;
	CVPTDoc *pDoc = GetDocument() ;
	TCHAR title[256] ;
	TCHAR *dotPtr, *ndotPtr ;

	_tcscpy_s(title, pDoc->GetTitle()) ;
	dotPtr = ndotPtr = _tcschr(title, '.') ;
	while(ndotPtr) {
		dotPtr = ndotPtr ;
		ndotPtr = _tcschr(ndotPtr + 1, '.') ;
	}
	if(dotPtr) {
		*dotPtr = NULL ;
	}

	CFileDialog FileDlg(TRUE, _T("bmp"), title, OFN_OVERWRITEPROMPT, _T("BMP(*.bmp)|*.bmp||"));

	if( FileDlg.DoModal () == IDOK ) {
		CString pathName = FileDlg.GetPathName();
		
		image.Attach(hBitmap) ;
		image.Save(pathName, ImageFormatBMP) ;
		image.Detach() ;
	}
}

void CVPTView::OnEditYUV()
{
	// TODO: Add your command handler code here
//	unsigned char *rgb ;
	CProcessing *pProc ;

	pProc = (CProcessing *) new CAppConvert ;
	
	AddProcessing(pProc) ;
	DoProcessing() ;
}

void CVPTView::OnEditYUV_Y()
{
	// TODO: Add your command handler code here
//	unsigned char *rgb ;
	CProcessing *pProc ;

	pProc = (CProcessing *) new CAppConvert ;
	((CAppConvert *) pProc)->mode = 1 ;

	AddProcessing(pProc) ;
	DoProcessing() ;
}

void CVPTView::OnEditYUV_U()
{
	// TODO: Add your command handler code here
//	unsigned char *rgb ;
	CProcessing *pProc ;

	pProc = (CProcessing *) new CAppConvert ;
	((CAppConvert *) pProc)->mode = 2 ;

	AddProcessing(pProc) ;
	DoProcessing() ;
}

void CVPTView::OnEditYUV_V()
{
	// TODO: Add your command handler code here
//	unsigned char *rgb ;
	CProcessing *pProc ;

	pProc = (CProcessing *) new CAppConvert ;
	((CAppConvert *) pProc)->mode = 3 ;

	AddProcessing(pProc) ;
	DoProcessing() ;
}

void CVPTView::OnEditCompress()
{
	// TODO: Add your command handler code here
//	unsigned char *rgb ;
	CProcessing *pProc ;

	pProc = (CProcessing *) new CAppCompress ;

	AddProcessing(pProc) ;
	DoProcessing() ;
}
