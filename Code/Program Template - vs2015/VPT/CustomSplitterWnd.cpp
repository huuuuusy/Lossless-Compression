#include "StdAfx.h"
#include "CustomSplitterWnd.h"
#include "Resource.h"

IMPLEMENT_DYNCREATE(CCustomSplitterWnd, CSplitterWnd)

CCustomSplitterWnd::CCustomSplitterWnd(void)
{
	borderSizeX = this->m_cxBorder ;
	borderSizeY = this->m_cyBorder ;
	splitterSizeX = this->m_cxSplitter ;
	splitterSizeY = this->m_cxSplitter ;
}

CCustomSplitterWnd::~CCustomSplitterWnd(void)
{
}

void CCustomSplitterWnd::SetBorderSize(int sx, int sy) {
	borderSizeX = this->m_cxBorder = sx ;
	borderSizeY = this->m_cyBorder = sy ;
}

void CCustomSplitterWnd::SetSplitterSize(int sx, int sy) {
	splitterSizeX = this->m_cxSplitter = sx ;
	splitterSizeY = this->m_cxSplitter = sy ;
}

void CCustomSplitterWnd::OnMouseMove(UINT nFlags, CPoint pt) {

	if (m_bTracking && GetCapture() == this) {
		return ;
	} else {
		CSplitterWnd::OnMouseMove(nFlags, pt) ;
	}
}

BEGIN_MESSAGE_MAP(CCustomSplitterWnd, CSplitterWnd)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
