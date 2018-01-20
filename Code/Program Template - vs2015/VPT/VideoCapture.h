#pragma once
#include "vfw.h"

#define CAP_DEV_COUNT 16

class CVideoCapture
{
public:
	unsigned char *captureBuffer ;
	int width, height ;
	int bitCount ;
	BOOL frameReady ;
	static HWND hCapList[CAP_DEV_COUNT] ;
	static CVideoCapture *vCapList[CAP_DEV_COUNT] ;
	static BOOL listInited ;
private:
	BOOL initialized ;
	BITMAPINFOHEADER bmiHeader ;
	HWND hWndC ;
public:
	CVideoCapture(void);
	int init(HWND hwndParent, int fps = 25) ;
	unsigned char *getFrame(void) ;
	int stop(void) ;
	~CVideoCapture(void);
};
