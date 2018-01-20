#include "StdAfx.h"
#include "DSGraphBuilder.h"
#include "Psapi.h"

#define WM_GRAPHNOTIFY WM_APP+13

CDSGraphBuilder::CDSGraphBuilder(void)
{
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );  

	pGraphBuilder = NULL ;
	pMediaControl = NULL ;
	pMediaSeeking = NULL ;
	pMediaEventEx = NULL ;
	pMediaFilter = NULL ;
	pBasicVideo = NULL ;
	pVideoWindow = NULL ;

	Init(NULL) ;
}

CDSGraphBuilder::CDSGraphBuilder(HWND owner)
{
	pGraphBuilder = NULL ;
	pMediaControl = NULL ;
	pMediaSeeking = NULL ;
	pMediaEventEx = NULL ;
	pMediaFilter = NULL ;
	pBasicVideo = NULL ;
	pVideoWindow = NULL ;

	Init(owner) ;
}

CDSGraphBuilder::~CDSGraphBuilder(void)
{
	CleanUp() ;
	CoUninitialize() ;
//	EmptyWorkingSet(GetCurrentProcess()) ;
}

int CDSGraphBuilder::Init(HWND owner) {

	HRESULT hr;

	if(pGraphBuilder) CleanUp() ;

	// Get an instance of the graph builder
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
						(void**)&pGraphBuilder);

	if (!pGraphBuilder)
	{
		MessageBox(NULL, _T("Critical error, unable to create graph builder"), _T("Error"), MB_OK);
		return -1 ;
	}

	// Get the references to interfaces
	hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
	hr = pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void**)&pMediaSeeking);
	hr = pGraphBuilder->QueryInterface(IID_IMediaEventEx, (void**)&pMediaEventEx);
	hr = pGraphBuilder->QueryInterface(IID_IMediaFilter, (void **) &pMediaFilter) ;
	hr = pGraphBuilder->QueryInterface(IID_IBasicVideo, (void**)&pBasicVideo);
	hr = pGraphBuilder->QueryInterface(IID_IVideoWindow, (void**)&pVideoWindow);

	if (!pMediaControl || !pMediaSeeking || !pMediaEventEx || !pVideoWindow || !pBasicVideo)
	{
		MessageBox(NULL, _T("Critical error, error getting references"), _T("Error"), MB_OK);
		return -1 ;
	}

	// hOwner stores the handle to the dialog
	hOwner = owner;
	seekCaps = 0 ;
	frameCount = 0 ;
	frameRate = 0 ;
	frameTime = 0 ;
	width = height = 0 ;
	videoDuration = 0 ;

	// set our dialog to receive events from graph manager
	pMediaEventEx->SetNotifyWindow((OAHWND)hOwner, WM_GRAPHNOTIFY, 0);
	return 0 ;
}

int CDSGraphBuilder::SetOwner(HWND owner) {
	// hOwner stores the handle to the dialog
	hOwner = owner;

	// set our dialog to receive events from graph manager
	pMediaEventEx->SetNotifyWindow((OAHWND)hOwner, WM_GRAPHNOTIFY, 0);
	return 0 ;
}

int CDSGraphBuilder::GetVideoWidth(void) {
	return width ;
}

int CDSGraphBuilder::GetVideoHeight(void) {
	return height ;
}

long long CDSGraphBuilder::GetFrameCount(void) {
	return frameCount ;
}

DWORD CDSGraphBuilder::GetSeekingCapabilities(void) {

	DWORD dwCaps = 0 ;

	if(pMediaSeeking) {
		pMediaSeeking->GetCapabilities(&dwCaps) ;
		seekCaps = dwCaps ;
		return dwCaps ;
	} else {
		seekCaps = 0 ;
	}
	return 0 ;
}

bool CDSGraphBuilder::HasSeekingCapability(DWORD capability) {

	return (seekCaps & capability) != 0 ;
}

IBaseFilter *CDSGraphBuilder::GetVideoRenderer(void) {

	HRESULT hr ;
	IBaseFilter *pVideoRenderer ;

	hr = pGraphBuilder->FindFilterByName(_T("Video Renderer"), &pVideoRenderer) ;
	if(FAILED(hr)) return NULL ;

	return pVideoRenderer ;
}

int CDSGraphBuilder::RenderFile(const TCHAR *pFilename) {

	HRESULT hr ;
	REFTIME avgRefTimePerFrame ;
	IBaseFilter *pVideoRenderer = NULL ;

	hr = pGraphBuilder->RenderFile(pFilename, NULL);
	if(FAILED(hr)) {
		return -1 ;
	}
	pVideoWindow->put_Owner((OAHWND) hOwner) ;
	pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME) ;
	GetSeekingCapabilities() ;
	if(FAILED(hr) || !HasSeekingCapability(AM_SEEKING_CanSeekAbsolute)) {
		return -1 ;
	}
	hr = pMediaSeeking->GetDuration(&videoDuration) ;
	if(FAILED(hr)) {
		return -1 ;
	}

	hr = pBasicVideo->get_AvgTimePerFrame(&avgRefTimePerFrame) ;

	if(FAILED(hr)) {
		return -1 ;
	}
	//	hr = pMediaFilter->SetSyncSource(NULL) ;

	pBasicVideo->get_VideoWidth(&width) ;
	pBasicVideo->get_VideoHeight(&height) ;
/*
	pBasicVideo->put_SourceWidth(width / 2) ;
	pBasicVideo->put_SourceHeight(height / 2) ;
*/
	frameRate = (long long) (1.0 / avgRefTimePerFrame) ;
	frameTime = 10000000 / frameRate ;
	frameCount = (long long) (videoDuration / ((long long) (avgRefTimePerFrame * 10000000))) ;
	SeekToFrameIdx(0) ;

	return 0 ;
}

int CDSGraphBuilder::SeekToFrameIdx(long long frameNum) {

	HRESULT hr ;

	long long currentTime ;
	long long stopTime ;

	currentTime = frameNum * frameTime ;
	stopTime = frameNum * frameTime + (frameTime >> 1) ;

	hr = pMediaSeeking->SetPositions(&currentTime, AM_SEEKING_AbsolutePositioning, &stopTime, AM_SEEKING_AbsolutePositioning) ;

	if(FAILED(hr)) return -1 ;

	return 0 ;
}

HRESULT CDSGraphBuilder::GetConnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, the pin we want
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
            else  // Unconnected, not the pin we want.
            {
                pTmp->Release();
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}

HRESULT CDSGraphBuilder::GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return hr;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
            if (SUCCEEDED(hr))  // Already connected, not the pin we want.
            {
                pTmp->Release();
            }
            else  // Unconnected, this is the pin we want.
            {
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return E_FAIL;
}

IPin *CDSGraphBuilder::GetPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir    // Direction of the pin to find.
	)
{
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
        return NULL;
    }
    while (pEnum->Next(1, &pPin, NULL) == S_OK)
    {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir)
        {
            pEnum->Release();
			return pPin ;
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching pin.
    return NULL;
}

HRESULT CDSGraphBuilder::ConnectFilters(
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest)    // Downstream filter.
{
	IGraphBuilder *pGraph = pGraphBuilder ;
    if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }
#ifdef debug
        PIN_DIRECTION PinDir;
        pOut->QueryDirection(&PinDir);
        _ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

    // Find an input pin on the downstream filter.
    IPin *pIn = 0;
    HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr))
    {
        return hr ;
    }
    // Try to connect them.
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
    return hr;
}

HRESULT CDSGraphBuilder::ConnectFilters(
    IBaseFilter *pSrc, 
    IBaseFilter *pDest)
{
	IGraphBuilder *pGraph = pGraphBuilder ;
    if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
    {
        return E_POINTER;
    }

    // Find an output pin on the first filter.
    IPin *pOut = 0;
    HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) 
    {
        return hr;
    }
    hr = ConnectFilters(pOut, pDest);
    pOut->Release();
    return hr;
}

HRESULT CDSGraphBuilder::Pause(void) {

	return pMediaControl->Pause() ;
}

HRESULT CDSGraphBuilder::Run(void) {

	return pMediaControl->Run() ;
}

int CDSGraphBuilder::SetStopFrameIdx(long long frameNum) {

	HRESULT hr ;

	long long currentTime ;
	long long stopTime ;

	stopTime = frameNum * frameTime + (frameTime >> 1) ;
	currentTime = 0 ;

	hr = pMediaSeeking->GetCurrentPosition(&currentTime) ;
	hr = pMediaSeeking->SetPositions(&currentTime, AM_SEEKING_AbsolutePositioning, &stopTime, AM_SEEKING_AbsolutePositioning) ;
//	hr = pMediaSeeking->SetPositions(NULL, AM_SEEKING_NoPositioning, &stopTime, AM_SEEKING_AbsolutePositioning) ;

	if(FAILED(hr)) return -1 ;

	return 0 ;
}

int CDSGraphBuilder::SetCurrentFrameIdx(long long frameNum) {

	HRESULT hr ;

	long long currentTime ;
	long long stopTime ;

	hr = pMediaSeeking->GetPositions(&currentTime, &stopTime) ;

	currentTime = frameNum * frameTime ;
//	stopTime = frameNum * frameTime ;

	hr = pMediaSeeking->SetPositions(&currentTime, AM_SEEKING_AbsolutePositioning, &stopTime, AM_SEEKING_AbsolutePositioning) ;

	if(FAILED(hr)) return -1 ;

	return 0 ;
}

int CDSGraphBuilder::GetCurrentFrameIdx(long long &frameNum) {

	HRESULT hr ;
	long long currentTime ;
	long long stopTime ;

	hr = pMediaSeeking->GetPositions(&currentTime, &stopTime) ;

	if(FAILED(hr)) return -1 ;
	frameNum = currentTime / frameTime ;

	return 0 ;
}

int CDSGraphBuilder::GetStopFrameIdx(long long &frameNum) {

	HRESULT hr ;
	long long currentTime ;
	long long stopTime ;

	hr = pMediaSeeking->GetPositions(&currentTime, &stopTime) ;

	if(FAILED(hr)) return -1 ;
	frameNum = stopTime / frameTime ;

	return 0 ;
}

int CDSGraphBuilder::SetFramePositions(long long start, long long stop) {

	HRESULT hr ;

	long long currentTime ;
	long long stopTime ;

	currentTime = start * frameTime ;
	stopTime = stop * frameTime + (frameTime >> 1) ;

	hr = pMediaSeeking->SetPositions(&currentTime, AM_SEEKING_AbsolutePositioning, &stopTime, AM_SEEKING_AbsolutePositioning) ;

	if(FAILED(hr)) return -1 ;

	return 0 ;
}

HRESULT CDSGraphBuilder::AddFilter(IBaseFilter *pFilter, const TCHAR *pFilterName) {

	return pGraphBuilder->AddFilter(pFilter, pFilterName) ;
}

HRESULT CDSGraphBuilder::AddFilter(const CLSID &clsid, IBaseFilter **ppFilter, const TCHAR *pFilterName) {

	HRESULT hr ;
	IBaseFilter *pFilter ;

	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pFilter);
	if(FAILED(hr)) {
		*ppFilter = NULL ;
		return hr ;
	}
	*ppFilter = pFilter ;
	return AddFilter(pFilter, pFilterName) ;
}

HRESULT CDSGraphBuilder::RemoveFilter(IBaseFilter *pFilter) {

	HRESULT hr ;

	if(pGraphBuilder) {
		hr = pGraphBuilder->RemoveFilter(pFilter) ;
		return hr ;
	} else {
		return E_POINTER ;
	}
}

HRESULT CDSGraphBuilder::RemoveFilter(const TCHAR *pFilterName) {

	HRESULT hr ;
	IBaseFilter *pFilter ;

	hr = pGraphBuilder->FindFilterByName(pFilterName, &pFilter) ;
	if(FAILED(hr)) return hr ;
	hr = RemoveFilter(pFilter) ;

	if(pFilter) pFilter->Release() ;

	return hr ;
}

void CDSGraphBuilder::CleanUp(void) {
	if (pMediaControl)
	{
		pMediaControl->Release();
		pMediaControl = NULL;
	}
	if (pMediaSeeking)
	{
		pMediaSeeking->Release();
		pMediaSeeking = NULL;
	}
	if (pMediaEventEx)
	{
		// unset the notify window
		pMediaEventEx->SetNotifyWindow(NULL, 0, 0);

		//release resources
		pMediaEventEx->Release();
		pMediaEventEx = NULL;
	}
	if (pMediaFilter)
	{
		pMediaFilter->Release();
		pMediaFilter = NULL;
	}
	if (pBasicVideo)
	{
		pBasicVideo->Release();
		pBasicVideo = NULL ;
	}
	if (pVideoWindow)
	{
		pVideoWindow->put_Owner(NULL) ;
		pVideoWindow->Release();
		pVideoWindow = NULL ;
	}
	if (pGraphBuilder)
	{
		pGraphBuilder->Release();
		pGraphBuilder = NULL;
	}

	seekCaps = 0 ;
	frameCount = 0 ;
	frameRate = 0 ;
	frameTime = 0 ;
	width = height = 0 ;
	videoDuration = 0 ;
}
