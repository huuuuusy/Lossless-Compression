#pragma once
#include "afxwin.h"
#include "resource.h"
#include "TCHAR.H"

// CCaptureSelect dialog

class CCaptureSelect : public CDialog
{
	DECLARE_DYNAMIC(CCaptureSelect)
public:
	TCHAR device[10][80] ;
	int count ;

public:
	CCaptureSelect(CWnd* pParent = NULL);   // standard constructor
	void addDevice(TCHAR *dev) ;
	virtual ~CCaptureSelect();
	virtual BOOL OnInitDialog() ;

// Dialog Data
	enum { IDD = IDD_CAP_SOURCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int capChoice;
};
