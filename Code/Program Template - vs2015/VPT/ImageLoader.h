#pragma once
#include "atlimage.h"
#include "Gdiplusimaging.h"
#include ".\aviloader.h"
#include ".\VideoCapture.h"

//#define H264_FILE_SUPPORT
#define DIRECTSHOW_SUPPORT

#ifdef H264_FILE_SUPPORT
#include "decoder.h"
#endif

#ifdef DIRECTSHOW_SUPPORT
#include ".\DSVideoGrabber.h"
#endif

#include "TCHAR.H"

#define IMG_TYPE_UDF -1
#define IMG_TYPE_AVI 0
#define IMG_TYPE_CAP 1
#define IMG_TYPE_BMP 2
#define IMG_TYPE_PPM 3
#define IMG_TYPE_JPG 4
#define IMG_TYPE_TIF 5
#define IMG_TYPE_PNG 6
#define IMG_TYPE_GIF 7
#define IMG_TYPE_420 8
#define IMG_TYPE_420_V 9
#define IMG_TYPE_422 10
#define IMG_TYPE_422_V 11
#define IMG_TYPE_VYUY 12
#define IMG_TYPE_VYUY_V 13
#define IMG_TYPE_Y_U_V 14
#define IMG_TYPE_IPT 15
#define IMG_TYPE_VPT 16
#define IMG_TYPE_DSV 17				// Direct show openable video
#define IMG_TYPE_264 18				// H264 Elementary Stream

class CImageLoader
{
public:
	int width, height ;
	unsigned char *bits ;
	int type ;
	int index ;
	int startIndex, endIndex ;
	CAviLoader aviLoader ;
#ifdef DIRECTSHOW_SUPPORT
	CDSVideoGrabber dsVideoGrabber ;
#endif
	CVideoCapture videoCapture ;
#ifdef H264_FILE_SUPPORT
	H264File *pH264File ;
#endif
public:
	CImageLoader(void);
	~CImageLoader(void);
	HRESULT Load(const TCHAR *path, BOOL init = TRUE) ;
	HRESULT Create(int width, int height, unsigned char *copy = NULL) ;
	HRESULT CapInit(HWND hwnd) ;
	HRESULT Save(const TCHAR *path) ;
	HRESULT Next() ;
	HRESULT Previous() ;
	HRESULT StepNext() ;
	HRESULT StepPrevious() ;
	HRESULT Rewind() ;
	HRESULT LoadFrame(int idx) ;
	BOOL IsVideo(void) {
		return (startIndex < endIndex) || (type == IMG_TYPE_CAP) ;
	}
	BOOL IsVideoCapture(void) {
		return type == IMG_TYPE_CAP ;
	}
	int GetVideoPos(int scale) {
		if(startIndex < endIndex) {
			return index * scale / (endIndex - startIndex) ;
		}
		return 0 ;
	}
	int SeekVideoPos(int pos, int scale) {
		if(startIndex < endIndex) {
			index = pos * (endIndex - startIndex) / scale ;
			return LoadFrame(index) ;
		}
		return E_ACCESSDENIED ;
	}
	unsigned char *GetBits(void) {
		return bits ;
	}
	void PutDC(HDC hDC) ;
private:
	char pbm_getc(FILE *fp) ;
	int pbm_getint(FILE *fp) ;
	unsigned char *pbm_read(FILE *fp) ;

	struct IPTFormat {
		TCHAR format[256] ;
		int start ;
		int end ;
		int current ;
		int type ;
	} *FMT ;

	int fmtIndex, fmtCount ;
};
