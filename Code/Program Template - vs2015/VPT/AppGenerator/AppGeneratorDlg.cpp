// AppGeneratorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppGenerator.h"
#include "AppGeneratorDlg.h"
#include "TCHAR.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAppGeneratorDlg dialog




CAppGeneratorDlg::CAppGeneratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppGeneratorDlg::IDD, pParent)
	, appClassName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_APP_CLASS_NAME, appClassName);
	DDV_MaxChars(pDX, appClassName, 256);
}

BEGIN_MESSAGE_MAP(CAppGeneratorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAppGeneratorDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAppGeneratorDlg message handlers

BOOL CAppGeneratorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAppGeneratorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAppGeneratorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAppGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ProcessFile(FILE *ifp, FILE *ofp, TCHAR *appClassName) {

#define APP_CLASS_NAME _T("APP_CLASS_NAME")

	TCHAR lineBuf[1024] ;
	TCHAR *line ;
	TCHAR *ptr ;

	line = lineBuf ;
	_fgetts(line, 1024, ifp) ;
	while(!feof(ifp)) {
		ptr = _tcsstr(line, APP_CLASS_NAME) ;
		if(ptr) {
			if(ptr != line) {
				*ptr = 0 ;
				_ftprintf(ofp, line) ;
			}
			_ftprintf(ofp, appClassName) ;
			line = (ptr + _tcslen(APP_CLASS_NAME)) ;
			ptr = _tcsstr(line, APP_CLASS_NAME) ;
			while(ptr) {
				*ptr = 0 ;
				_ftprintf(ofp, line) ;
				_ftprintf(ofp, appClassName) ;
				line = (ptr + _tcslen(APP_CLASS_NAME)) ;
				ptr = _tcsstr(line, APP_CLASS_NAME) ;
			} ;
		}
		_ftprintf(ofp, line) ;
		line = lineBuf ;
		_fgetts(line, 1024, ifp) ;
	}

#undef APP_CLASS_NAME
}

void CAppGeneratorDlg::OnBnClickedOk()
{
	FILE *ifp ;
	FILE *ofp ;
	TCHAR oFileName[256] ;
	TCHAR tAppClassName[256] ;

	// TODO: Add your control notification handler code here
	OnOK();

	_tcscpy_s(tAppClassName, appClassName) ;

	_tfopen_s(&ifp, _T("AppTemplate.h"), _T("rt")) ;
	_stprintf_s(oFileName, _T("../App/%s.h"), tAppClassName) ;
	_tfopen_s(&ofp, oFileName, _T("wt")) ;

	if(!ifp) {
		AfxMessageBox(_T("Cannot open template file!")) ;
		exit(0) ;
	}
	if(!ofp) {
		AfxMessageBox(_T("Failed to create output file!")) ;
		fclose(ifp) ;
		exit(0) ;
	}

	ProcessFile(ifp, ofp, tAppClassName) ;

	fclose(ofp) ;
	fclose(ifp) ;

	_tfopen_s(&ifp, _T("AppTemplate.cpp"), _T("rt")) ;
	_stprintf_s(oFileName, _T("../App/%s.cpp"), tAppClassName) ;
	_tfopen_s(&ofp, oFileName, _T("wt")) ;

	if(!ifp) {
		AfxMessageBox(_T("Cannot open template file!")) ;
		exit(0) ;
	}
	if(!ofp) {
		AfxMessageBox(_T("Failed to create output file!")) ;
		fclose(ifp) ;
		exit(0) ;
	}

	ProcessFile(ifp, ofp, tAppClassName) ;

	fclose(ofp) ;
	fclose(ifp) ;

	AfxMessageBox(_T("Application Create Successfully!")) ;
}
