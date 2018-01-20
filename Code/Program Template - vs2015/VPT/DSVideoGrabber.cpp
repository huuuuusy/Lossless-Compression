#include "StdAfx.h"
#include "DSVideoGrabber.h"

#define WM_GRAPHNOTIFY WM_APP+13

//#define MEDIASUBTYPE MEDIASUBTYPE_YV12
//#define MEDIASUBTYPE MEDIASUBTYPE_NV12
#define MEDIASUBTYPE MEDIASUBTYPE_RGB24
//#define MEDIASUBTYPE MEDIASUBTYPE_RGB32

// The sample grabber is calling us back on its deliver thread.
// This is NOT the main app thread!
//

STDMETHODIMP CSampleGrabberCB::BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize )
{
	framenum++;

	/*_tprintf(TEXT("Found a sample at time %ld ms\t[%s]\r\n"), 
	long(SampleTime*1000), szFilename );*/
	/*
	// Write out the file header
	//
	BITMAPFILEHEADER bfh;
	memset( &bfh, 0, sizeof( bfh ) );
	bfh.bfType = 'MB';
	bfh.bfSize = sizeof( bfh ) + BufferSize + sizeof( BITMAPINFOHEADER );
	bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

	// Write the bitmap format
	//
	BITMAPINFOHEADER bih;
	memset( &bih, 0, sizeof( bih ) );
	bih.biSize = sizeof( bih );
	bih.biWidth = Width;
	bih.biHeight = Height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	*/
	if(pFrameBufferQueue) {
//		memcpy(pCopyBuf, pBuffer, BufferSize) ;
		pFrameBufferQueue->Enqueue(pBuffer) ;
		if(pFrameBufferQueue->FreeSlotsCount() <= 0) {
			pCDSGraphBuilder->Pause() ;
		}
//		if(hOwner) {
//			::SendMessage(hOwner, WM_TIMER, 0, 0) ;
//		}
	}

	return 0;
}


CDSVideoGrabber::CDSVideoGrabber(void)
{
	pSampleGrabber = NULL ;
	pNullRenderer = NULL ;
	pGrabber = NULL ;

	pFrameBufferQueue = NULL ;

	pFrameData = NULL ;

	frameCount = 0 ;
}

void CDSVideoGrabber::CleanUp(void) {
	if (pSampleGrabber)
	{
		pSampleGrabber->Release() ;
		pSampleGrabber = NULL ;
	}
	if (pGrabber) {
		pGrabber->Release() ;
		pGrabber = NULL ;
	}
	if (pNullRenderer)
	{
		pNullRenderer->Release() ;
		pSampleGrabber = NULL ;
	}
	if (pFrameBufferQueue)
	{
		delete pFrameBufferQueue ;
		pFrameBufferQueue = NULL ;
	}
	if (pFrameData) {
		delete [] pFrameData ;
	}

	frameCount = 0 ;
}

int CDSVideoGrabber::AddSampleGrabber(void) {

	HRESULT hr ;
	AM_MEDIA_TYPE mt;

	hr = AddFilter(CLSID_SampleGrabber, &pSampleGrabber, _T("SampleGrabber")) ;

	if(FAILED(hr)) return -1 ;

	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE ;
//	mt.subtype = MEDIASUBTYPE_NV12 ;
//	mt.formattype = GUID_NULL;
	mt.formattype = FORMAT_VideoInfo ;

	if(pGrabber) {
		pGrabber->Release() ;
		pGrabber = NULL ;
	}
	hr = pSampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber) ;
	hr = pGrabber->SetMediaType(&mt) ;
	pGrabber->SetOneShot(FALSE);   
	pGrabber->SetBufferSamples(FALSE);
	pGrabber->SetCallback(&grabberCB, 1) ;
	grabberCB.SetGraphBuilder((CDSGraphBuilder *) this) ;

	if(FAILED(hr)) {
		return -1 ;
	}

	return 0 ;
}

int CDSVideoGrabber::AddNullRenderer(void) {

	HRESULT hr ;

	hr = AddFilter(CLSID_NullRenderer, &pNullRenderer, _T("NullRenderer")) ;
	if(FAILED(hr)) return -1 ;
	return 0 ;
}

int CDSVideoGrabber::LoadFile(const TCHAR *filename) {

	IBaseFilter *pVideoRenderer = NULL ;
	HRESULT hr ;

	hr = AddFilter(CLSID_FFDSHOW, &pFFDShow, _T("FFDShow")) ;

	if(RenderFile(filename)) {
		return -1 ;
	}

	if(pFrameBufferQueue) {
		delete pFrameBufferQueue ;
	}

	AddNullRenderer() ;
	AddSampleGrabber() ;

	pVideoRenderer = GetVideoRenderer() ;

	if(!pVideoRenderer) {
		return -1 ;
	}

	if(pFrameData) {
		delete [] pFrameData ;
	}

	if(MEDIASUBTYPE == MEDIASUBTYPE_NV12 || MEDIASUBTYPE == MEDIASUBTYPE_YV12) {
		pFrameBufferQueue = new CFrameBufferQueue(4, width, height, width * height * 3 / 2) ;
		pFrameData = new unsigned char[width * height * 3 / 2] ;
	} else if(MEDIASUBTYPE == MEDIASUBTYPE_RGB24) {
		pFrameBufferQueue = new CFrameBufferQueue(4, width, height, width * height * 3) ;
		pFrameData = new unsigned char[width * height * 3] ;
	} else {
		pFrameBufferQueue = new CFrameBufferQueue(4, width, height, width * height * 4) ;
		pFrameData = new unsigned char[width * height * 4] ;
	}
	grabberCB.pFrameBufferQueue = pFrameBufferQueue ;

	/*
	{
		if(pVideoRender)
			pVideoRenderer->Release() ;
		pVideoRenderer = NULL ;
	}
	*/

	if(pVideoRenderer) {
		//get input pin of video renderer
		IPin* ipin = GetPin(pVideoRenderer, PINDIR_INPUT);
		IPin* opin = NULL;
		//find out who the renderer is connected to and disconnect from them
		ipin->ConnectedTo(&opin);
		ipin->Disconnect();
		opin->Disconnect();

		if(pNullRenderer && pSampleGrabber) {
			hr = ConnectFilters(opin, pSampleGrabber) ;

			opin->Release() ;
			if(hr != S_OK) {
				ipin->Release() ;
				return -1 ;
			}
			opin = GetPin(pSampleGrabber, PINDIR_OUTPUT) ;

			hr = ConnectFilters(opin, pNullRenderer) ;
			if(hr != S_OK) {
				ipin->Release() ;
				opin->Release() ;
				return -1 ;
			}
		}

		ipin->Release() ;
		opin->Release() ;

		//remove the default renderer from the graph		
		RemoveFilter(pVideoRenderer);

		pVideoRenderer->Release() ;

		/*
		//see if the video renderer was originally connected to 
		//a color space converter
		IBaseFilter* pColorConverter = NULL;
		m_pGraph->FindFilterByName(L"Color Space Converter", &pColorConverter);
		if(pColorConverter)
		{
			opin->Release() ;

			//remove the converter from the graph as well
			ipin = GetPin(pColorConverter, PINDIR_INPUT);

			ipin->ConnectedTo(&opin);
			ipin->Disconnect();
			opin->Disconnect();

			ipin->Release() ;
			
			m_pGraph->RemoveFilter(pColorConverter);
			pColorConverter->Release() ;
		}
		*/
	}

	SeekToFrameIdx(0) ;
	Run() ;

	return 0 ;
}

void CDSVideoGrabber::GetRGB24(unsigned char *bits) {

	int i, j ;

	if(MEDIASUBTYPE == MEDIASUBTYPE_NV12 || MEDIASUBTYPE == MEDIASUBTYPE_YV12) {
		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				bits[i * 3 + j * width * 3] = pFrameData[i + j * width] ;
				bits[i * 3 + 1 + j * width * 3] = pFrameData[i + j * width] ;
				bits[i * 3 + 2 + j * width * 3] = pFrameData[i + j * width] ;
			}
		}
	} else if(MEDIASUBTYPE == MEDIASUBTYPE_RGB24) {
		for(i = 0; i < height; i++) {
			memcpy(bits + i * width * 3, pFrameData + (height - 1 - i) * width * 3, width * 3) ;
		}
	} else if(MEDIASUBTYPE == MEDIASUBTYPE_RGB32) {
		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				bits[i * 3 + j * width * 3] = pFrameData[4 * i + (height - 1 - j) * width * 4] ;
				bits[i * 3 + 1 + j * width * 3] = pFrameData[4 * i + 1 + (height - 1 - j) * width * 4] ;
				bits[i * 3 + 2 + j * width * 3] = pFrameData[4 * i + 2 + (height - 1 - j) * width * 4] ;
			}
		}
	}
}

HRESULT CDSVideoGrabber::NextFrame(unsigned char *bits) {

	long long currentFrameNumber ;

	GetCurrentFrameIdx(currentFrameNumber) ;
	if(pFrameBufferQueue->UsedSlotsCount() > 0) {
		pFrameBufferQueue->Dequeue(pFrameData) ;
		GetRGB24(bits) ;
		Run() ;
	} else {
		if(currentFrameNumber < (frameCount - 1))
			return E_PENDING ;
		else
			return E_POINTER ;
	}
	return S_OK ;
}

HRESULT CDSVideoGrabber::StepNextFrame(unsigned char *bits) {

	long long currentFrameNumber ;

	GetCurrentFrameIdx(currentFrameNumber) ;
	currentFrameNumber++ ;
	if(currentFrameNumber < frameCount) {
		SeekToFrameIdx(currentFrameNumber) ;
		WaitForFrame(bits) ;
	} else {
		return E_POINTER ;
	}
	return S_OK ;
}

HRESULT CDSVideoGrabber::StepPrevFrame(unsigned char *bits) {

	long long currentFrameNumber ;

	GetCurrentFrameIdx(currentFrameNumber) ;
	currentFrameNumber-- ;
	if(currentFrameNumber >= 0) {
		SeekToFrameIdx(currentFrameNumber) ;
		WaitForFrame(bits) ;
	} else {
		return E_POINTER ;
	}
	return S_OK ;
}

void CDSVideoGrabber::WaitForFrame(unsigned char *bits) {

	int i = 0 ;
	while(pFrameBufferQueue->Dequeue(pFrameData) && i < 25) {
		i++ ;
		Sleep(40) ;
	}
	GetRGB24(bits) ;
}

CDSVideoGrabber::~CDSVideoGrabber(void)
{
	CleanUp() ;
}
