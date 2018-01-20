#include "StdAfx.h"
#include "VideoCtrlBar.h"
#include "Resource.h"
#include "VideoCtrlFrm.h"

IMPLEMENT_DYNCREATE(CVideoCtrlBar, CToolBar)

BEGIN_MESSAGE_MAP(CVideoCtrlBar, CToolBar)
//	ON_WM_CREATE()
//	ON_WM_SIZING()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CVideoCtrlBar::CVideoCtrlBar(void) :
	sliderScale(1000)
{
}

CVideoCtrlBar::~CVideoCtrlBar(void)
{
}

void CVideoCtrlBar::Init(void) {

	RECT rect ;
	RECT tRect ;
	int i ;

	i = CommandToIndex(IDC_VIDEO_SLIDER) ;

	SetButtonInfo(i, IDC_VIDEO_SLIDER, TBBS_SEPARATOR, 200) ;
//	SetButtonStyle(i-1, GetButtonStyle(i - 1) | TBBS_WRAPPED) ;
	GetItemRect(i, &rect) ;
	GetParent()->GetClientRect(&tRect) ;

	rect.right += 200 ;

	sliderCtrl.Create(WS_CHILD|WS_VISIBLE, rect, this, IDC_VIDEO_SLIDER) ;
	sliderCtrl.SetRange(0, 1000) ;

}

void CVideoCtrlBar::SetSliderPos(int pos) {

	sliderCtrl.SetPos(pos) ;
}

int CVideoCtrlBar::GetSliderPos(int pos) {

	return sliderCtrl.GetPos() ;
}

void CVideoCtrlBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {

	((CVideoCtrlFrame *)GetParentFrame())->OnVideoSeek() ;
	CToolBar::OnHScroll(nSBCode, nPos, pScrollBar);
	return ;
}