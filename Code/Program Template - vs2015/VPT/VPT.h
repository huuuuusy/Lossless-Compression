// VPT.h : main header file for the VPT application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CVPTApp:
// See VPT.cpp for the implementation of this class
//

class CVPTApp : public CWinApp
{
public:
	CMultiDocTemplate* pConsoleTemplate ;
	CMultiDocTemplate* pImageTemplate ;
	BOOL consoleExist ;
	BOOL newTunnel ;
	CDocument *consoleDoc ;
	BOOL newCapture ;
	BOOL newConsole ;

public:
	CVPTApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual CDocument *OpenDocumentFile(LPCTSTR lpszFileName) ;

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen() ;
	afx_msg void OnFileNew() ;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileLog();
	afx_msg void OnFileCapture();
};

extern CVPTApp theApp;