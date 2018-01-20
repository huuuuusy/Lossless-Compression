// ConsoleView.cpp : implementation file
//

#include "stdafx.h"
#include "VPT.h"
#include "ConsoleView.h"
#include "Tools/cpuid.h"
#include "TCHAR.H"

// CConsoleView

IMPLEMENT_DYNCREATE(CConsoleView, CFormView)

CConsoleView::CConsoleView()
	: CFormView(CConsoleView::IDD)
{
	showState = TRUE ;
}

CConsoleView::~CConsoleView()
{
	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	pApp->consoleExist = FALSE ;
}

void CConsoleView::ToggleShowState() {
	if(showState) {
		GetParentFrame()->ShowWindow(SW_HIDE) ;
		showState = FALSE ;
	} else {
		GetParentFrame()->ShowWindow(SW_SHOWNOACTIVATE) ;
		showState = TRUE ;
	}
}

void CConsoleView::printf(const TCHAR *fmt, ...) {

	va_list ap ;
	TCHAR str[512] ;

	va_start(ap, fmt) ;

	_vstprintf_s(str, fmt, ap) ;
	SendDlgItemMessage(IDC_CONSOLE_LOG, (UINT) EM_REPLACESEL, TRUE, (LPARAM) str) ;
	
	va_end(ap) ;
}

void CConsoleView::OnDraw(CDC* pDC) {

	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	CWnd *wlog ;
	int wdiff ;
	int hdiff ;

	RECT WindowRect, ClientRect;
	RECT AppRect;
	pApp->GetMainWnd()->GetClientRect(&AppRect );
	GetParentFrame()->GetWindowRect(&WindowRect) ;
	GetParentFrame()->GetClientRect(&ClientRect) ;
	wdiff = WindowRect.right - WindowRect.left - ClientRect.right + ClientRect.left;
	hdiff = WindowRect.bottom - WindowRect.top - ClientRect.bottom + ClientRect.top ;
	wlog = GetDlgItem(IDC_CONSOLE_LOG) ;
	wlog->SetWindowPos(wlog, ClientRect.left, ClientRect.top, ClientRect.right - ClientRect.left - 1, ClientRect.bottom - ClientRect.top - 1, SWP_NOZORDER) ;
	pApp->consoleDoc->SetTitle(_T("Console Log")) ;
}

void CConsoleView::OnInitialUpdate() {

	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	int wdiff ;
	int hdiff ;
	CWnd *wlog ;
	SIZE ViewSize ;

	RECT WindowRect, ClientRect;
	RECT AppRect;
	pApp->GetMainWnd()->GetClientRect(&AppRect );
	GetParentFrame()->GetWindowRect(&WindowRect) ;
	GetParentFrame()->GetClientRect(&ClientRect) ;
	wdiff = WindowRect.right - WindowRect.left - ClientRect.right + ClientRect.left;
	hdiff = WindowRect.bottom - WindowRect.top - ClientRect.bottom + ClientRect.top ;

	SendDlgItemMessage(IDC_CONSOLE_LOG, (UINT) EM_SETLIMITTEXT, 4 * 1024 * 1024, (LPARAM) NULL) ;

	GetParentFrame()->SetWindowPos(GetParentFrame(), 0, AppRect.bottom - 168 - 45, AppRect.right - AppRect.left, 168, SWP_NOZORDER) ;
	wlog = GetDlgItem(IDC_CONSOLE_LOG) ;
	wlog->SetWindowPos(wlog, ClientRect.left, ClientRect.top, ClientRect.right - ClientRect.left - 1, ClientRect.bottom - ClientRect.top - 1, SWP_NOZORDER) ;
	ViewSize.cx = ClientRect.right - ClientRect.left;
	ViewSize.cy = ClientRect.bottom - ClientRect.top ;
	SetScaleToFitSize(ViewSize) ;
//	printf("CPU %s MMX\r\n", cpuMMX() ? "supports" : "not support") ;
//	printf("CPU %s SSE\r\n", cpuSSE() ? "supports" : "not support") ;
//	printf("CPU %s SSE2\r\n", cpuSSE2() ? "supports" : "not support") ;
//	printf("CPU %s 3DNOW\r\n", cpu3DNOW() ? "supports" : "not support") ;
}

void CConsoleView::AlignWindowPos(void) {
	CVPTApp *pApp = (CVPTApp *) AfxGetApp() ;
	CWnd *wlog ;
	int wdiff ;
	int hdiff ;

	RECT WindowRect, ClientRect;
	RECT AppRect;

	pApp->GetMainWnd()->GetClientRect(&AppRect);
	GetParentFrame()->GetWindowRect(&WindowRect) ;
	GetParentFrame()->GetClientRect(&ClientRect) ;
	wdiff = WindowRect.right - WindowRect.left - ClientRect.right + ClientRect.left;
	hdiff = WindowRect.bottom - WindowRect.top - ClientRect.bottom + ClientRect.top ;

	GetParentFrame()->SetWindowPos(GetParentFrame(), 0, AppRect.bottom - 168 - 45, AppRect.right - AppRect.left, 168, SWP_NOZORDER) ;
	wlog = GetDlgItem(IDC_CONSOLE_LOG) ;
	wlog->SetWindowPos(wlog, ClientRect.left, ClientRect.top, ClientRect.right - ClientRect.left - 1, ClientRect.bottom - ClientRect.top - 1, SWP_NOZORDER) ;
}

void CConsoleView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CConsoleView, CFormView)
	ON_COMMAND(ID_EDIT_COPY, &CConsoleView::OnEditCopy)
	ON_COMMAND(ID_EDIT_SELECTALL, &CConsoleView::OnEditSelectall)
	ON_COMMAND(ID_EDIT_SELECTNONE, &CConsoleView::OnEditSelectnone)
END_MESSAGE_MAP()


// CConsoleView diagnostics

#ifdef _DEBUG
void CConsoleView::AssertValid() const
{
	CFormView::AssertValid();
}

void CConsoleView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


// CConsoleView message handlers

void CConsoleView::OnEditCopy()
{
	// TODO: Add your command handler code here

	CEdit *pConsoleLog ;

	pConsoleLog = (CEdit *) GetDlgItem(IDC_CONSOLE_LOG) ;
	pConsoleLog->Copy() ;
}

void CConsoleView::OnEditSelectall()
{
	// TODO: Add your command handler code here

	CEdit *pConsoleLog ;

	pConsoleLog = (CEdit *) GetDlgItem(IDC_CONSOLE_LOG) ;
	pConsoleLog->SetSel(0, -1); ;
}

void CConsoleView::OnEditSelectnone()
{
	// TODO: Add your command handler code here

	CEdit *pConsoleLog ;

	pConsoleLog = (CEdit *) GetDlgItem(IDC_CONSOLE_LOG) ;
	pConsoleLog->SetSel(-1, 0); ;
}
