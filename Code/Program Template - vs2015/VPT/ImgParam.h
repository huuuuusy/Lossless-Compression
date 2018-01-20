#pragma once
#include "afxwin.h"
#include "resource.h"

// CImgParam dialog

class CImgParam : public CDialog
{
	DECLARE_DYNAMIC(CImgParam)

public:
	CImgParam(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImgParam();

// Dialog Data
	enum { IDD = IDD_IMG_PARAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Image Width
	int width;
	// Image Height
	int height;
	// Image Format
	int format;
public:
	afx_msg void OnBnClickedYuv420();
public:
	afx_msg void OnBnClickedYuv422();
public:
	afx_msg void OnBnClickedVyuy();
public:
	afx_msg void OnBnClickedYuv420V();
public:
	afx_msg void OnBnClickedYuv422V();
public:
	afx_msg void OnBnClickedVyuyV();
public:
	BOOL cbYUV420;
public:
	BOOL cbYUV422;
public:
	BOOL cbVYUY;
public:
	BOOL cbYUV420_v;
public:
	BOOL cbYUV422_v;
public:
	BOOL cbVYUY_v;
};
