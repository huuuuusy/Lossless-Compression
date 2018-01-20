#pragma once

#include "TCHAR.H"
#include "afxwin.h"

// CConsoleView form view

class CConsoleView : public CFormView
{
	DECLARE_DYNCREATE(CConsoleView)
public:
	BOOL showState ;

public:
	void printf(const TCHAR *fmt, ...) ;
	void ToggleShowState() ;

protected:
	CConsoleView();           // protected constructor used by dynamic creation
	virtual ~CConsoleView();

public:
	enum { IDD = IDD_FORMVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	void AlignWindowPos(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditCopy();
	afx_msg void OnEditSelectall();
	afx_msg void OnEditSelectnone();
};


