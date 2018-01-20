// VPTDoc.h : interface of the CVPTDoc class
//


#pragma once
#include "ImageLoader.h"

class CVPTDoc : public CDocument
{
protected: // create from serialization only
	CVPTDoc();
	DECLARE_DYNCREATE(CVPTDoc)

// Attributes
public:
	CImageLoader imageLoader ;
	CView *inTunnelView ;

// Operations
public:
	void printf(const TCHAR *fmt, ...) ;
	int ImageWidth(void) {
		return imageLoader.width ;
	}
	int ImageHeight(void) {
		return imageLoader.height ;
	}
	BOOL IsVideo(void) {
		return imageLoader.IsVideo() ;
//		return ((imageLoader.startIndex < imageLoader.endIndex) || (imageLoader.type == IMG_TYPE_CAP)) ;
	}
	BOOL IsVideoCapture(void) {
		return imageLoader.IsVideoCapture() ;
//		return imageLoader.type == IMG_TYPE_CAP ;
	}
	int GetVideoPos(int scale) {
		return imageLoader.GetVideoPos(scale) ;
	}
	HRESULT SeekVideoPos(int pos, int scale, HDC hDC = 0) {
		HRESULT hr ;
		
		hr = imageLoader.SeekVideoPos(pos, scale) ;
		if(hr == S_OK) PutDC(hDC) ;

		return hr ;
	}
	unsigned char *GetBits(void) {
		return imageLoader.GetBits() ;
	}
	void PutDC(HDC hDC = 0) ;

	HRESULT Next(HDC hDC = 0) {
		HRESULT hr ;

		hr = imageLoader.Next() ;
		if(hr == S_OK) PutDC(hDC) ;

		return hr ;
	}

	HRESULT StepNext(HDC hDC = 0) {
		HRESULT hr ;

		hr = imageLoader.StepNext() ;
		if(hr == S_OK) PutDC(hDC) ;

		return hr ;
	}

	HRESULT Previous(HDC hDC = 0) {
		HRESULT hr ;

		hr = imageLoader.Previous() ;
		if(hr == S_OK) PutDC(hDC) ;

		return hr ;
	}
	HRESULT Rewind(HDC hDC = 0) {
		HRESULT hr ;

		hr = imageLoader.Rewind() ;
		if(hr == S_OK) PutDC(hDC) ;

		return hr ;
	}

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CVPTDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


