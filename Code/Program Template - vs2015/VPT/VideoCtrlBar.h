#pragma once
#include "afxext.h"

class CVideoCtrlBar :
	public CToolBar
{
protected:
	DECLARE_DYNCREATE(CVideoCtrlBar)
	CSliderCtrl sliderCtrl ;
	int sliderScale ;

public:
	CVideoCtrlBar(void);
	~CVideoCtrlBar(void);
	void Init(void) ;
	void SetSliderPos(int pos) ;
	int GetSliderPos(int pos) ;

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

protected:
	DECLARE_MESSAGE_MAP()
public:
};
