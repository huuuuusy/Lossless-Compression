#pragma once
#include <dshow.h>
#include <uuids.h>
#include "qedit.h"
#include "Tools/DSGraphBuilder.h"

class CFrameBuffer {
public:
	int width, height ;
	int frameSize ;
	unsigned char *pData ;
public:
	CFrameBuffer(void) {
		width = 0;
		height = 0;
		frameSize = 0 ;
		pData = NULL ;
	}
	CFrameBuffer(int w, int h, int fs) {
		Init(w, h, fs) ;
	}
	~CFrameBuffer(void) {
		CleanUp() ;
	}
	unsigned char *Init(int w, int h, int fs) {
		width = w ;
		height = h ;
		frameSize = fs ;
		pData = new unsigned char[fs] ;

		return pData ;
	}
	void CleanUp(void) {
		if(pData) {
			delete [] pData ;
			pData = NULL ;
		}
		width = height = frameSize = 0 ;
	}
	void UpdateBuffer(unsigned char *buf) {
		memcpy(pData, buf, frameSize) ;
	}
	void CopyBuffer(unsigned char *buf) {
		memcpy(buf, pData, frameSize) ;
	}
	void UpdateBuffer(unsigned char *buf, int w, int h, int fs) {
		if(w != width || height != h || frameSize != fs) {
			CleanUp() ;
			Init(w, h, fs) ;
		}
		UpdateBuffer(buf) ;
	}
} ;

class CFrameBufferQueue {
public:
	int queueSize ;
	int headPos ;
	int tailPos ;
	int numFrame ;
	CFrameBuffer *pBufferList ;
public:
	CFrameBufferQueue(void) {
		queueSize = 0 ;
		pBufferList = 0 ;
		numFrame = 0 ;
		headPos = tailPos = 0 ;
	}
	CFrameBufferQueue(int size, int width, int height, int frameSize) {
		Init(size, width, height, frameSize) ;
	}
	~CFrameBufferQueue(void) {
		CleanUp() ;
	}
	CFrameBuffer *Init(int size, int width, int height, int frameSize) {
		queueSize = 0 ;
		pBufferList = 0 ;
		numFrame = 0 ;
		headPos = tailPos = 0 ;
		queueSize = size ;
		if(pBufferList) {
			delete [] pBufferList ;
		}
		pBufferList = new CFrameBuffer[size] ;
		headPos = tailPos = 0 ;
		numFrame = 0 ;
		if(pBufferList && width && height && frameSize) {
			int i ;

			for(i = 0; i < size; i++) {
				pBufferList[i].Init(width, height, frameSize) ;
			}
		}
		return pBufferList ;
	}
	void CleanUp(void) {
		if(pBufferList) {
			delete [] pBufferList ;
			pBufferList = NULL ;
		}
		queueSize = 0 ;
		headPos = tailPos = 0 ;
		numFrame = 0 ;
	}
	void Flush(void) {
		headPos = tailPos = 0 ;
		numFrame = 0 ;
	}
	HRESULT Enqueue(unsigned char *pBuf) { // Update buffer data while keeping width, height, and frameSize unchanged
		if(pBuf) {
			if(numFrame < queueSize) {
				pBufferList[tailPos].UpdateBuffer(pBuf) ;
				tailPos++ ;
				tailPos %= queueSize ;
				numFrame++ ;
				return S_OK ;
			}
		}
		return E_FAIL ;
	}
	HRESULT Dequeue(unsigned char *pBuf) {
		if(pBuf) {
			if(numFrame > 0) {
				pBufferList[headPos].CopyBuffer(pBuf) ;
				headPos++ ;
				headPos %= queueSize ;
				numFrame-- ;
				return S_OK ;
			}
		}
		return E_FAIL ;
	}
	int QueueSize(void) {
		return queueSize ;
	}
	int FreeSlotsCount(void) {
		return queueSize - numFrame ;
	}
	int UsedSlotsCount(void) {
		return numFrame ;
	}
} ;

//
// Callback class for Sample Grabber filter.
//
class CSampleGrabberCB : public ISampleGrabberCB
{
public :

	// Fake out any COM ref counting
	//
	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		if (NULL == ppv)
			return E_POINTER;

		*ppv = NULL;
		if (IID_IUnknown == iid)
		{
			*ppv = (IUnknown*)this;
			AddRef();
			return S_OK;
		}
		else if (IID_ISampleGrabberCB == iid)
		{
			*ppv = (ISampleGrabberCB*)this;
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	// Constructor/destructor
	CSampleGrabberCB() 
	{
		hOwner = 0 ;
		pCDSGraphBuilder = NULL ;
		framenum = 0 ;
		pFrameBufferQueue = NULL ;
	}

	~CSampleGrabberCB()
	{
		hOwner = 0 ;
		framenum = 0 ;
		pFrameBufferQueue = NULL ;
	}

	void SetGraphBuilder(CDSGraphBuilder *pGraphBuilder) {
		pCDSGraphBuilder = pGraphBuilder ;
	}

public:
	// These will get set by the main thread below. We need to
	// know this in order to write out the bmp
	long framenum;
	CFrameBufferQueue *pFrameBufferQueue ;
	CString strPath;
	HWND hOwner ;
	CDSGraphBuilder *pCDSGraphBuilder ;

	STDMETHODIMP SampleCB(double n,IMediaSample *pms)
	{
		return 0;
	}


	// The sample grabber is calling us back on its deliver thread.
	// This is NOT the main app thread!
	//
	STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize ) ;
};

class CDSVideoGrabber : public CDSGraphBuilder
{
private:
	IBaseFilter *pSampleGrabber ;
	IBaseFilter *pFFDShow ;
	IBaseFilter *pVideoRender ;
	IBaseFilter *pNullRenderer ;
	ISampleGrabber *pGrabber ;
	CSampleGrabberCB grabberCB ;

public:
	CFrameBufferQueue *pFrameBufferQueue ;
	unsigned char *pFrameData ;
/*
  AM_SEEKING_CanSeekAbsolute    = 0x1,
  AM_SEEKING_CanSeekForwards    = 0x2,
  AM_SEEKING_CanSeekBackwards   = 0x4,
  AM_SEEKING_CanGetCurrentPos   = 0x8,
  AM_SEEKING_CanGetStopPos      = 0x10,
  AM_SEEKING_CanGetDuration     = 0x20,
  AM_SEEKING_CanPlayBackwards   = 0x40,
  AM_SEEKING_CanDoSegments      = 0x80,
  AM_SEEKING_Source             = 0x100 
*/

public:
	CDSVideoGrabber(void) ;
	void CleanUp(void) ;
	int AddNullRenderer(void) ;
	int AddSampleGrabber(void) ;
	int LoadFile(const TCHAR *filename) ;
	HRESULT StepNextFrame(unsigned char *bits) ;
	HRESULT NextFrame(unsigned char *bits) ;
	HRESULT StepPrevFrame(unsigned char *bits) ;
	void WaitForFrame(unsigned char *bits) ;
	void GetRGB24(unsigned char *bits) ;
	~CDSVideoGrabber(void) ;
};
