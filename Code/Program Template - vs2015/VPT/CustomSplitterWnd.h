#pragma once
#include "afxext.h"

class CCustomSplitterWnd :
	public CSplitterWnd
{
protected:
	DECLARE_DYNCREATE(CCustomSplitterWnd)
public:
	int borderSizeX ;
	int borderSizeY ;
	int splitterSizeX ;
	int splitterSizeY ;

public:
	CCustomSplitterWnd(void);
	void SetBorderSize(int sx, int sy) ;
	void SetSplitterSize(int sx, int sy) ;

	afx_msg void OnMouseMove(UINT nFlags, CPoint pt) ;

public:
	~CCustomSplitterWnd(void);

	DECLARE_MESSAGE_MAP()
};

