// ImgParam.cpp : implementation file
//

#include "stdafx.h"
#include "VPT.h"
#include "ImgParam.h"
#include "ImageLoader.h"


// CImgParam dialog

IMPLEMENT_DYNAMIC(CImgParam, CDialog)
CImgParam::CImgParam(CWnd* pParent /*=NULL*/)
	: CDialog(CImgParam::IDD, pParent)
	, width(352)
	, height(288)
	, format(IMG_TYPE_420)
	, cbYUV420(FALSE)
	, cbYUV422(TRUE)
	, cbVYUY(TRUE)
	, cbYUV420_v(TRUE)
	, cbYUV422_v(TRUE)
	, cbVYUY_v(TRUE)
{
}

CImgParam::~CImgParam()
{
}

void CImgParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_IMG_WIDTH, width);
	DDV_MinMaxInt(pDX, width, 0, 65536);
	DDX_Text(pDX, IDC_IMG_HEIGHT, height);
	DDV_MinMaxInt(pDX, height, 0, 65536);
	switch(GetCheckedRadioButton(IDC_YUV420, IDC_VYUY_V)) {
		case IDC_YUV420: format = IMG_TYPE_420 ; break ;
		case IDC_YUV422: format = IMG_TYPE_422 ; break ;
		case IDC_VYUY: format = IMG_TYPE_VYUY ; break ;
		case IDC_YUV420_V: format = IMG_TYPE_420_V ; break ;
		case IDC_YUV422_V: format = IMG_TYPE_422_V ; break ;
		case IDC_VYUY_V: format = IMG_TYPE_VYUY_V ; break ;
	}
	DDX_Radio(pDX, IDC_YUV420, cbYUV420);
	DDX_Radio(pDX, IDC_YUV422, cbYUV422);
	DDX_Radio(pDX, IDC_VYUY, cbVYUY);
	DDX_Radio(pDX, IDC_YUV420_V, cbYUV420_v);
	DDX_Radio(pDX, IDC_YUV422_V, cbYUV422_v);
	DDX_Radio(pDX, IDC_VYUY_V, cbVYUY_v);
}


BEGIN_MESSAGE_MAP(CImgParam, CDialog)
	ON_BN_CLICKED(IDC_YUV420, &CImgParam::OnBnClickedYuv420)
	ON_BN_CLICKED(IDC_YUV422, &CImgParam::OnBnClickedYuv422)
	ON_BN_CLICKED(IDC_VYUY, &CImgParam::OnBnClickedVyuy)
	ON_BN_CLICKED(IDC_YUV420_V, &CImgParam::OnBnClickedYuv420V)
	ON_BN_CLICKED(IDC_YUV422_V, &CImgParam::OnBnClickedYuv422V)
	ON_BN_CLICKED(IDC_VYUY_V, &CImgParam::OnBnClickedVyuyV)
END_MESSAGE_MAP()


// CImgParam message handlers

void CImgParam::OnBnClickedYuv420()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_YUV420) ;
	// TODO: Add your control notification handler code here
}

void CImgParam::OnBnClickedYuv422()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_YUV422) ;
	// TODO: Add your control notification handler code here
}

void CImgParam::OnBnClickedVyuy()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_VYUY) ;
	// TODO: Add your control notification handler code here
}

void CImgParam::OnBnClickedYuv420V()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_YUV420_V) ;
	// TODO: Add your control notification handler code here
}

void CImgParam::OnBnClickedYuv422V()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_YUV422_V) ;
	// TODO: Add your control notification handler code here
}

void CImgParam::OnBnClickedVyuyV()
{
	CheckRadioButton(IDC_YUV420, IDC_VYUY_V, IDC_VYUY_V) ;
	// TODO: Add your control notification handler code here
}
