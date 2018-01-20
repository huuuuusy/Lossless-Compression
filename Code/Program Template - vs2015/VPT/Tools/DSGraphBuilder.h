#pragma once
#include <dshow.h>
#include <uuids.h>
#include "../qedit.h"

class CDSGraphBuilder
{
private:
	IGraphBuilder *pGraphBuilder ;
	IMediaControl *pMediaControl ;
	IMediaSeeking *pMediaSeeking ;
	IMediaEventEx *pMediaEventEx ;
	IMediaFilter *pMediaFilter ;
	IBasicVideo *pBasicVideo ;
	IVideoWindow *pVideoWindow ;
//	IBaseFilter *pSampleGrabber ;
	HWND hOwner ;

public:
	DWORD seekCaps ;
	long long videoDuration ; // in 100ns
	long long frameRate ;
	long long frameCount ;
	long long frameTime ;
	long width, height ;

public:
	CDSGraphBuilder(void);
	CDSGraphBuilder(HWND owner) ;
	int Init(HWND owner = NULL) ;
	void CleanUp(void) ;
	int SetOwner(HWND owner);	// Set the owner window handler of this graph builder
	int GetVideoWidth(void) ;
	int GetVideoHeight(void) ;
	long long GetFrameCount(void) ;
	IBaseFilter *GetVideoRenderer(void) ;
	int RenderFile(const TCHAR *pFilename) ;
	HRESULT RemoveFilter(IBaseFilter *pFilter) ;
	HRESULT RemoveFilter(const TCHAR *pFilterName) ;
	HRESULT AddFilter(IBaseFilter *pFilter, const TCHAR *pFilterName = NULL) ;
	HRESULT AddFilter(const CLSID &clsid, IBaseFilter **ppFilter, const TCHAR *pFilterName = NULL) ;
	HRESULT GetConnectedPin(
		IBaseFilter *pFilter,   // Pointer to the filter.
		PIN_DIRECTION PinDir,   // Direction of the pin to find.
		IPin **ppPin) ;         // Receives a pointer to the pin.
	HRESULT GetUnconnectedPin(
		IBaseFilter *pFilter,   // Pointer to the filter.
		PIN_DIRECTION PinDir,   // Direction of the pin to find.
		IPin **ppPin) ;         // Receives a pointer to the pin.
	IPin *GetPin(
		IBaseFilter *pFilter,   // Pointer to the filter.
		PIN_DIRECTION PinDir   // Direction of the pin to find.
		) ;
	HRESULT ConnectFilters(
		IPin *pOut,            // Output pin on the upstream filter.
		IBaseFilter *pDest) ;  // Downstream filter.
	HRESULT ConnectFilters(
		IBaseFilter *pSrc, 
		IBaseFilter *pDest) ;
	HRESULT Pause() ;
	HRESULT Run() ;
	int SeekToFrameIdx(long long frameNum) ;
	int SetStopFrameIdx(long long frameNum) ;
	int SetCurrentFrameIdx(long long frameNum) ;
	int GetStopFrameIdx(long long &frameNum) ;
	int GetCurrentFrameIdx(long long &frameNum) ;
	int SetFramePositions(long long start, long long stop) ;
	DWORD GetSeekingCapabilities(void) ;
	bool HasSeekingCapability(DWORD capability) ;
	~CDSGraphBuilder(void);
};
