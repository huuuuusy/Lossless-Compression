// AppGeneratorDlg.h : header file
//

#pragma once


// CAppGeneratorDlg dialog
class CAppGeneratorDlg : public CDialog
{
// Construction
public:
	CAppGeneratorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_APPGENERATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString appClassName;
	afx_msg void OnEnChangeAppClassName();
	afx_msg void OnBnClickedOk();
};
