// VPT.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VPT.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "VideoCtrlFrm.h"
#include "VPTDoc.h"
#include "VPTView.h"
#include "ConsoleView.h"

#include "ImageLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVPTApp

BEGIN_MESSAGE_MAP(CVPTApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CVPTApp::OnAppAbout)
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_NEW, &CVPTApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN, &CVPTApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_LOG, &CVPTApp::OnFileLog)
	ON_COMMAND(ID_FILE_CAPTURE, &CVPTApp::OnFileCapture)
END_MESSAGE_MAP()


// CVPTApp construction

CVPTApp::CVPTApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	consoleExist = FALSE ;
	newCapture = FALSE ;
	newConsole = FALSE ;
	newTunnel = FALSE ;
}


// The one and only CVPTApp object

CVPTApp theApp;


// CVPTApp initialization

BOOL CVPTApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_VPTTYPE,
		RUNTIME_CLASS(CVPTDoc),
//		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CVideoCtrlFrame), // custom MDI child frame
		RUNTIME_CLASS(CVPTView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	pImageTemplate = pDocTemplate ;

	{
		CMultiDocTemplate* pNewDocTemplate = new CMultiDocTemplate(
			IDR_CONSOLETYPE,
			RUNTIME_CLASS(CVPTDoc),
			RUNTIME_CLASS(CChildFrame),
			RUNTIME_CLASS(CConsoleView));
		AddDocTemplate(pNewDocTemplate);
		pConsoleTemplate = pNewDocTemplate ;
	}

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}



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

// App command to run the dialog
void CVPTApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CVPTApp::OnFileNew()
{
	pImageTemplate->OpenDocumentFile(NULL) ;
}

void CVPTApp::OnFileCapture()
{
	// TODO: Add your command handler code here
	newCapture = TRUE ;
	OnFileNew() ;
}

void CVPTApp::OnFileOpen() {

	CFileDialog FileDlg(TRUE, NULL, NULL, NULL, _T("ALL(*.*)|*.*|VPT Files(*.VPT)|*.vpt|IPT Files(*.IPT)|*.ipt|BMP(*.bmp)|*.bmp|PPM(*.ppm)|*.ppm|GIF(*.gif)|*.gif|JPG(*.jpg)|*.jpg||"));

   if( FileDlg.DoModal () == IDOK )
   {
	   CString pathName = FileDlg.GetPathName();

	   pImageTemplate->OpenDocumentFile((LPCTSTR) pathName) ;
   }
}
// CVPTApp message handlers


void CVPTApp::OnFileLog()
{
	// TODO: Add your command handler code here
	if(!consoleExist) {
		newConsole = TRUE ;
		consoleDoc = pConsoleTemplate->OpenDocumentFile(NULL);
		consoleExist = TRUE ;
	} else {
		POSITION pos = consoleDoc->GetFirstViewPosition() ;
		CConsoleView *pView = (CConsoleView *) consoleDoc->GetNextView(pos) ;

		pView->ToggleShowState() ;
	}
}

CDocument *CVPTApp::OpenDocumentFile(LPCTSTR pathName) {

   return pImageTemplate->OpenDocumentFile((LPCTSTR) pathName) ;
}