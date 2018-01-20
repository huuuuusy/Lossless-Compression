#pragma once

#include "VideoCtrlBar.h"

#ifdef _WIN32_WCE
#error "CMDIChildWnd is not supported for Windows CE."
#endif 

// CVideoCtrlFrame frame with splitter

class CVideoCtrlFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CVideoCtrlFrame)
public:
	CVideoCtrlBar m_wndToolBar ;
	UINT_PTR timer ;

public:
	void HideToolBar(void) ;
	void ShowToolBar(void) ;
	void OnVideoSeek() ;

protected:
	CVideoCtrlFrame();           // protected constructor used by dynamic creation
	virtual ~CVideoCtrlFrame();
	virtual void OnSizing(UINT nSide, LPRECT lpRect) ;
	virtual void OnSize(UINT nType, int cx, int cy) ;
	void ResizeSlider(void) ;
	void UpdateSlider(void) ;

	CSplitterWnd m_wndSplitter;

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void) ;
	afx_msg void OnTimer(UINT_PTR) ;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnVideoNext();
	afx_msg void OnVideoPlay();
	afx_msg void OnVideoStop();
	afx_msg void OnVideoPrevious();
	afx_msg void OnVideoRewind();
	afx_msg void OnUpdateVideoPlay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVideoStop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVideoNext(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVideoPrevious(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVideoRewind(CCmdUI *pCmdUI);
	afx_msg void OnUpdateVideoSlider(CCmdUI *pCmdUI);
};


