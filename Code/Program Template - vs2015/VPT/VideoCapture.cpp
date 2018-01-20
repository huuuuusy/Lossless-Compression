#include "StdAfx.h"
#include ".\videocapture.h"
#include ".\CaptureSelect.h"
#include ".\Tools\ImageConvert.h"
#include "TCHAR.H"

BOOL CVideoCapture::listInited = false ;
CVideoCapture *CVideoCapture::vCapList[CAP_DEV_COUNT] ;
HWND CVideoCapture::hCapList[CAP_DEV_COUNT] ;

CVideoCapture::CVideoCapture(void)
{
	if(!CVideoCapture::listInited) {
		int i ;
		for(i = 0; i < CAP_DEV_COUNT; i++) {
			vCapList[i] = NULL ;
		}
		CVideoCapture::listInited = TRUE ;
	}
	captureBuffer = NULL ;
	frameReady = TRUE ;
	bitCount = 0 ;
	hWndC = 0 ;
	width = height = 0 ;
	initialized = FALSE ;
}

CVideoCapture::~CVideoCapture(void)
{
	if(initialized) stop() ;
}

LRESULT PASCAL VideoCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr) 
{
	int i ;
	CVideoCapture *vcap ;
	CImageConvert imgConvert ;

    if (!hWnd) 
        return FALSE;
 
	vcap = NULL ;
	for(i = 0; i < CAP_DEV_COUNT; i++) {
		if(CVideoCapture::vCapList[i] && CVideoCapture::hCapList[i] == hWnd) {
			vcap = CVideoCapture::vCapList[i] ;
			break ;
		}
	}
	if(!vcap) return TRUE;

	vcap->frameReady = FALSE ;
	if(vcap->bitCount == 16) {
		imgConvert.YUYV_to_RGB24(lpVHdr->lpData, vcap->captureBuffer, vcap->width, vcap->height) ;
	} else {
		for(i = 0; i < vcap->height; i++) {
			memcpy(vcap->captureBuffer + (vcap->height - 1 - i) * vcap->width * 3, lpVHdr->lpData + i * vcap->width * 3, vcap->width * 3) ;
		}
	}

	vcap->frameReady = TRUE ;
    return (LRESULT) TRUE ; 
}

LRESULT PASCAL ErrorCallbackProc(HWND hWnd, int nID, LPCSTR lpsz) 
{
	int i ;
	CVideoCapture *vcap ;

    if (!hWnd) 
        return FALSE;
 
	for(i = 0; i < CAP_DEV_COUNT; i++) {
		if(CVideoCapture::vCapList[i] && CVideoCapture::hCapList[i] == hWnd) {
			vcap = CVideoCapture::vCapList[i] ;
			break ;
		}
	}

    return (LRESULT) 0 ; 
}

unsigned char *CVideoCapture::getFrame(void) {

	if(initialized && frameReady)
		return captureBuffer ;
	else
		return NULL ;
}

int CVideoCapture::init(HWND hwndParent, int fps) {

	int fOK, i ;
	TCHAR szDeviceName[80];
	TCHAR szDeviceVersion[80];
	int wIndex ;
	CAPDRIVERCAPS CapDrvCaps;
	CCaptureSelect capSelect ;

	hWndC = capCreateCaptureWindow (
	    _T("My Capture Window"), // window name if pop-up 
		WS_CHILD /*| WS_VISIBLE*/,       // window style 
		0, 0, 352, 288,              // window position and dimensions
		(HWND) hwndParent, 
		(int) -1/* child ID */);

	for(i = 0; i < CAP_DEV_COUNT; i++) {
		if(!vCapList[i]) {
			hCapList[i] = hWndC ;
			vCapList[i] = this ;
			break ;
		}
	}

//	capSetCallbackOnFrame(hWndC, VideoCallbackProc);

	for (wIndex = 0; wIndex < 10; wIndex++) 
	{
		if (capGetDriverDescription (wIndex, szDeviceName, 
			sizeof (szDeviceName), szDeviceVersion, 
			sizeof (szDeviceVersion))) 
		{
			capSelect.addDevice(szDeviceName) ;
			// Append name to list of installed capture drivers
			// and then let the user select a driver to use.
		}
	}
	capSelect.DoModal() ;

	fOK = capDriverConnect(hWndC, capSelect.capChoice) ;
	if(!fOK) return fOK ;

//	capSetCallbackOnError(hWndC, ErrorCallbackProc) ;

	CAPTUREPARMS CaptureParms;

	capCaptureGetSetup(hWndC, &CaptureParms, sizeof(CAPTUREPARMS));

	CaptureParms.dwRequestMicroSecPerFrame = (DWORD) (1.0e6 / (double) fps);
	CaptureParms.fCaptureAudio = FALSE ;
	CaptureParms.fLimitEnabled = FALSE ;
	CaptureParms.fAbortLeftMouse = FALSE ;
	CaptureParms.fAbortRightMouse = FALSE ;
//	CaptureParms.wPercentDropForError = 100 ;
	CaptureParms.fYield = TRUE ;
	capCaptureSetSetup(hWndC, &CaptureParms, sizeof (CAPTUREPARMS));
	
	// Use the macro to retrieve the driver capabilities. 
	capDriverGetCaps(hWndC, &CapDrvCaps, sizeof (CAPDRIVERCAPS));

	if (CapDrvCaps.fHasDlgVideoSource)
		capDlgVideoSource(hWndC); 

	// Video format dialog box. 
	if (CapDrvCaps.fHasDlgVideoFormat) 
	{
		capDlgVideoFormat(hWndC); 

		// Are there new image dimensions?
//		capGetStatus(hWndC, &CapStatus, sizeof (CAPSTATUS));

		// If so, notify the parent of a size change.
	} 
 
	// Video display dialog box. 
	
	if (CapDrvCaps.fHasDlgVideoDisplay)
		capDlgVideoDisplay(hWndC);

/*
	if (CapDrvCaps.fHasDlgVideoSource)
		capDlgVideoSource(hWndC) ;
*/	
	LPBITMAPINFO lpbi;
	DWORD dwSize;

	dwSize = capGetVideoFormatSize(hWndC);
	lpbi = (LPBITMAPINFO) malloc(dwSize);
	capGetVideoFormat(hWndC, lpbi, dwSize); 

	// Access the video format and then free the allocated memory.
	memcpy(&bmiHeader, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER)) ;
	width = bmiHeader.biWidth ;
	height = bmiHeader.biHeight ;
	bitCount = bmiHeader.biBitCount ;

	free(lpbi) ;
	if(captureBuffer)
		delete [] captureBuffer ;

	captureBuffer = new unsigned char [width * height * 3] ;

	// Preview Settings

//	capPreviewRate(hWndC, 1);     // rate, in milliseconds
//	capPreview(hWndC, FALSE);        // disables preview 

	// Overlay Settings
/*
	if (CapDrvCaps.fHasOverlay) 
		capOverlay(hWndC, TRUE);
*/
	// Capture to a file
/*
	char szCaptureFile[] = "MYCAP.AVI";

	capFileSetCaptureFile( hWndC, szCaptureFile); 
	capFileAlloc( hWndC, (1024L * 1024L * 5));
*/
	capSetCallbackOnVideoStream(hWndC, VideoCallbackProc);
	fOK = capCaptureSequenceNoFile(hWndC);

	initialized = TRUE ;
	return fOK ;
}

int CVideoCapture::stop(void) {

	int i ;

	capCaptureStop(hWndC) ;
	capSetCallbackOnVideoStream(hWndC, NULL);
	capDriverDisconnect (hWndC);
	for(i = 0; i < CAP_DEV_COUNT; i++) {
		if(vCapList[i] && hCapList[i] == hWndC) {
			vCapList[i] = NULL ;
		}
	}

	delete [] captureBuffer ;
	captureBuffer = NULL ;
	bitCount = 0 ;
	DestroyWindow(hWndC) ;
	hWndC = 0 ;
	width = height = 0 ;
	initialized = FALSE ;
	return 0 ;
}