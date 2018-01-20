// VPTView.h : interface of the CVPTView class
//


#pragma once
#include "GL/gl.h"
#include "GL/glu.h"
#include "Tunnel.h"
#include "Tools/linklist.h"
#include "Processing.h"

class CVPTView : public CScrollView
{
protected: // create from serialization only
	CVPTView();
	DECLARE_DYNCREATE(CVPTView)

// Attributes
public:
	CVPTDoc* GetDocument() const;
	HDC hDC ;
	HBITMAP hBitmap ;
	HGLRC hGLRC ;
	CTunnel tunnel ;
	LinkList<CProcessing *> ProcessingList ;

// Operations
public:
	void GLStart(void) ;
	void GLFinish(void) ;
	void printf(const TCHAR *fmt, ...) ;
	void ResizeToImage() ;
	int	ImageWidth() ;
	int ImageHeight() ;
	void AddProcessing(CProcessing *pProc) ;
	void RemoveProcessing(CProcessing *pProc) ;
	void DoProcessing(void) ;
	unsigned char *GetBits(void) {
		return GetDocument()->GetBits() ;
	}

// Overrides
public:
	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnTimer(UINT) ;

// Implementation
public:
	virtual ~CVPTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditCopy();
	afx_msg void OnFileSave();
	afx_msg void OnEditYUV();
	afx_msg void OnEditYUV_Y();
	afx_msg void OnEditYUV_U();
	afx_msg void OnEditYUV_V();
	afx_msg void OnEditCompress();
};

#ifndef _DEBUG  // debug version in VPTView.cpp
inline CVPTDoc* CVPTView::GetDocument() const
   { return reinterpret_cast<CVPTDoc*>(m_pDocument); }
#endif

