// VideoCtrlFrm.cpp : implementation file
//

#include "stdafx.h"
#include "VPT.h"
#include "VideoCtrlFrm.h"
#include "VPTDoc.h"
#include "VPTView.h"

// CVideoCtrlFrame

//#define DIRECTSHOW_SUPPORT

IMPLEMENT_DYNCREATE(CVideoCtrlFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CVideoCtrlFrame, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIDEO_NEXT, &CVideoCtrlFrame::OnVideoNext)
	ON_COMMAND(ID_VIDEO_PLAY, &CVideoCtrlFrame::OnVideoPlay)
	ON_COMMAND(ID_VIDEO_STOP, &CVideoCtrlFrame::OnVideoStop)
	ON_COMMAND(ID_VIDEO_PREVIOUS, &CVideoCtrlFrame::OnVideoPrevious)
	ON_COMMAND(ID_VIDEO_REWIND, &CVideoCtrlFrame::OnVideoRewind)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_PLAY, &CVideoCtrlFrame::OnUpdateVideoPlay)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_STOP, &CVideoCtrlFrame::OnUpdateVideoStop)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_NEXT, &CVideoCtrlFrame::OnUpdateVideoNext)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_PREVIOUS, &CVideoCtrlFrame::OnUpdateVideoPrevious)
	ON_UPDATE_COMMAND_UI(ID_VIDEO_REWIND, &CVideoCtrlFrame::OnUpdateVideoRewind)
	ON_UPDATE_COMMAND_UI(IDC_VIDEO_SLIDER, &CVideoCtrlFrame::OnUpdateVideoSlider)
END_MESSAGE_MAP()


CVideoCtrlFrame::CVideoCtrlFrame()
{
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	timer = NULL ;
}

CVideoCtrlFrame::~CVideoCtrlFrame()
{
}



// CVideoCtrlFrame message handlers

BOOL CVideoCtrlFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	BOOL br ;

	br =  m_wndSplitter.Create(this,
		2, 1,       // TODO: adjust the number of rows, columns
		CSize(10, 10),  // TODO: adjust the minimum pane size
		pContext, WS_CHILD | WS_VISIBLE |  SPLS_DYNAMIC_SPLIT);

	if(!br)
		return FALSE ;
	
	return TRUE ;
}

int CVideoCtrlFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_VPTTYPE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.Init() ;

//	m_wndToolBar.SetHeight(90) ;

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);

	return 0;
}

void CVideoCtrlFrame::OnDestroy(void) {

	if(timer) {
		KillTimer(timer) ;
		timer = NULL ;
	}

	CMDIChildWnd::OnDestroy() ;
}

void CVideoCtrlFrame::ResizeSlider(void) {

	RECT rect ;
	RECT tRect ;
	int i ;

	if(!GetActiveDocument()) return ;

	i = m_wndToolBar.CommandToIndex(IDC_VIDEO_SLIDER) ;

	m_wndToolBar.GetItemRect(i, &rect) ;
	m_wndToolBar.GetClientRect(&tRect) ;

	m_wndToolBar.SetButtonInfo(i, IDC_VIDEO_SLIDER, TBBS_SEPARATOR, tRect.right - tRect.left) ;

	m_wndToolBar.sliderCtrl.MoveWindow(rect.left, rect.top, (tRect.right - tRect.left) - rect.left, rect.bottom - rect.top) ;
	
}

void CVideoCtrlFrame::OnSizing(UINT nSide, LPRECT lpRect) {

	CMDIChildWnd::OnSizing(nSide, lpRect) ;

	ResizeSlider() ;
}

void CVideoCtrlFrame::OnSize(UINT nType, int cx, int cy) {

	CMDIChildWnd::OnSize(nType, cx, cy) ;

	ResizeSlider() ;
}

void CVideoCtrlFrame::UpdateSlider(void) {

	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	m_wndToolBar.SetSliderPos(pDoc->GetVideoPos(m_wndToolBar.sliderScale)) ;
}

void CVideoCtrlFrame::HideToolBar(void) {

	ShowControlBar(&m_wndToolBar, 0, 0) ;
}

void CVideoCtrlFrame::ShowToolBar(void) {

	ShowControlBar(&m_wndToolBar, 1, 0) ;
}

void CVideoCtrlFrame::OnVideoNext()
{
	// TODO: Add your command handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;

//	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_PREVIOUS, 0) ;
	pDoc->StepNext() ;
	UpdateSlider() ;
	pDoc->UpdateAllViews(NULL) ;
	pView->DoProcessing() ;
}

void CVideoCtrlFrame::OnVideoPlay()
{
	// TODO: Add your command handler code here

	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;

	if(pDoc->imageLoader.type != IMG_TYPE_DSV)
		timer = SetTimer(1, 40, NULL);
	else {
		if(pDoc && pDoc->imageLoader.type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
			pDoc->imageLoader.dsVideoGrabber.SetCurrentFrameIdx(pDoc->imageLoader.index) ;
			pDoc->imageLoader.dsVideoGrabber.SetStopFrameIdx(pDoc->imageLoader.dsVideoGrabber.frameCount - 1) ;
			pDoc->imageLoader.dsVideoGrabber.Run() ;
			timer = SetTimer(1, 40, NULL) ;
#endif
		}
	}
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_PLAY, pDoc->IsVideo() && !timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_STOP, pDoc->IsVideo() && timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_NEXT, pDoc->IsVideo() && !timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_PREVIOUS, pDoc->IsVideo() && !timer && !pDoc->IsVideoCapture()) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_REWIND, pDoc->IsVideo() && !pDoc->IsVideoCapture() && !timer) ;

//	timer = SetTimer(1, 40, NULL);
}

void CVideoCtrlFrame::OnTimer(UINT_PTR nTimer) {

	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;
	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	HRESULT hr ;

	if(pApp->newCapture) return ;
	if((hr = pDoc->Next()) != S_OK) {
		if(hr != E_PENDING) {
			if(timer) {
				KillTimer(timer);
				timer = NULL ;
			}
		}
	} else {
		UpdateSlider() ;
		pDoc->UpdateAllViews(NULL) ;
		pView->DoProcessing() ;
		if(pDoc && pDoc->imageLoader.type == IMG_TYPE_DSV) {
#ifdef DIRECTSHOW_SUPPORT
			pDoc->imageLoader.dsVideoGrabber.Run() ;
#endif
		}
	}

	CMDIChildWnd::OnTimer(nTimer);
}
void CVideoCtrlFrame::OnVideoStop()
{
	// TODO: Add your command handler code here

	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	if(timer) {
#ifdef DIRECTSHOW_SUPPORT
		if(pDoc->imageLoader.type == IMG_TYPE_DSV) {
			pDoc->imageLoader.dsVideoGrabber.pFrameBufferQueue->Flush() ;
			pDoc->imageLoader.dsVideoGrabber.Pause() ;
		}
#endif
		KillTimer(timer);
		timer = NULL ;
	}

	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_PLAY, pDoc->IsVideo() && !timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_STOP, pDoc->IsVideo() && timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_NEXT, pDoc->IsVideo() && !timer) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_PREVIOUS, pDoc->IsVideo() && !timer && !pDoc->IsVideoCapture()) ;
	m_wndToolBar.GetToolBarCtrl().EnableButton(ID_VIDEO_REWIND, pDoc->IsVideo() && !pDoc->IsVideoCapture() && !timer) ;
}

void CVideoCtrlFrame::OnVideoPrevious()
{
	// TODO: Add your command handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;

	pDoc->Previous() ;
	UpdateSlider() ;
	pDoc->UpdateAllViews(NULL) ;
	pView->DoProcessing() ;
}

void CVideoCtrlFrame::OnVideoRewind()
{
	// TODO: Add your command handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;

	pDoc->Rewind() ;
	UpdateSlider() ;
	pDoc->UpdateAllViews(NULL) ;
	pView->DoProcessing() ;
}

void CVideoCtrlFrame::OnVideoSeek(void) {

	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	CVPTView* pView = (CVPTView *) GetActiveView() ;

	int pos ;

	pos = m_wndToolBar.sliderCtrl.GetPos() ;

	pDoc->SeekVideoPos(m_wndToolBar.sliderCtrl.GetPos(), m_wndToolBar.sliderScale) ;
	UpdateSlider() ;
	pDoc->UpdateAllViews(NULL) ;
	pView->DoProcessing() ;
}

void CVideoCtrlFrame::OnUpdateVideoPlay(CCmdUI *pCmdUI)
{
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable(pDoc->IsVideo() && !timer) ;
//	pCmdUI->SetCheck(timer != NULL) ;
}

void CVideoCtrlFrame::OnUpdateVideoStop(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	pCmdUI->Enable(pDoc->IsVideo() && timer) ;
}

void CVideoCtrlFrame::OnUpdateVideoNext(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	pCmdUI->Enable(pDoc->IsVideo() && !timer) ;
}

void CVideoCtrlFrame::OnUpdateVideoPrevious(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	pCmdUI->Enable(pDoc->IsVideo() && !timer && !pDoc->IsVideoCapture()) ;
}


void CVideoCtrlFrame::OnUpdateVideoRewind(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	pCmdUI->Enable(pDoc->IsVideo() && !pDoc->IsVideoCapture() && !timer) ;
}

void CVideoCtrlFrame::OnUpdateVideoSlider(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CVPTDoc* pDoc = (CVPTDoc *) GetActiveDocument() ;

	pCmdUI->Enable(pDoc->IsVideo() && !pDoc->IsVideoCapture() && !timer) ;
}