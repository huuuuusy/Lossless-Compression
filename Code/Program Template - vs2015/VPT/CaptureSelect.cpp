// CaptureSelect.cpp : implementation file
//

#include "stdafx.h"
#include "VPT.h"
#include "CaptureSelect.h"


// CCaptureSelect dialog

IMPLEMENT_DYNAMIC(CCaptureSelect, CDialog)
CCaptureSelect::CCaptureSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureSelect::IDD, pParent)
	, capChoice(0)
{
	count = 0 ;
}

CCaptureSelect::~CCaptureSelect()
{
}

void CCaptureSelect::addDevice(TCHAR *dev) {

	_tcscpy_s(device[count], dev) ;
	count++ ;
}

BOOL CCaptureSelect::OnInitDialog() {

	int i ;

	for(i = 0; i < count; i++)
		SendDlgItemMessage(IDC_CAP_LIST, CB_ADDSTRING, 0, (LPARAM) device[i]) ;
	SendDlgItemMessage(IDC_CAP_LIST, CB_SETCURSEL, 0, 0) ;
	return TRUE ;
}

void CCaptureSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_CAP_LIST, capChoice);
}


BEGIN_MESSAGE_MAP(CCaptureSelect, CDialog)
END_MESSAGE_MAP()


// CCaptureSelect message handlers
