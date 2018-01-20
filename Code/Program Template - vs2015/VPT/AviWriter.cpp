#include "StdAfx.h"
#include "AviWriter.h"

#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.

CAviWriter::CAviWriter(void)
{
	width = height = 0 ;
	pAVIFile = NULL ;	
	pVideoStream = NULL ;
	pCompressedStream = NULL ;
	wpbih = NULL ;
	index = 0 ;
	ready = 0 ;
	buffer = NULL ;
	aopts[0] = &opts;
}

HRESULT CAviWriter::init(const TCHAR *aviFile, int imgWidth, int imgHeight) {

	HRESULT hr ;

	width = imgWidth ;
	height = imgHeight ;

	AVIFileInit();

	_tremove(aviFile) ;

	hr = AVIFileOpen(&pAVIFile,		    // returned file pointer
			       aviFile,							// file name
				   OF_WRITE | OF_CREATE,		    // mode to open file with
				   NULL);

	if( FAILED( hr ) )
	{		
		return hr;
	}

	memset(&streamInfo, 0, sizeof(streamInfo));
	streamInfo.fccType                = streamtypeVIDEO;// stream type
	streamInfo.fccHandler             = 0;
	streamInfo.dwScale                = 1;
	streamInfo.dwRate                 = 25;		    // 25 fps
	streamInfo.dwSuggestedBufferSize  = width * height * 3 ;
	SetRect(&streamInfo.rcFrame, 0, 0,		    // rectangle for stream
			(int) width,
			(int) height);

	hr = AVIFileCreateStream(pAVIFile,		    // file pointer
					         &pVideoStream,		    // returned stream pointer
							 &streamInfo);	    // stream header
	if ( FAILED( hr) )
	{
		return hr;
	}

	memset(&opts, 0, sizeof(opts));
	
	if (!AVISaveOptions(NULL, 0, 1, &pVideoStream, (LPAVICOMPRESSOPTIONS FAR *) &aopts))
	{
		return -1;
	}

	wpbih = new BITMAPINFOHEADER ;
	memset(wpbih, 0, sizeof(BITMAPINFOHEADER)) ;
	wpbih->biSize = sizeof(BITMAPINFOHEADER) ;
	wpbih->biCompression = BI_RGB ;
	wpbih->biWidth = width ;
	wpbih->biHeight = height ;
	wpbih->biPlanes = 1 ;
	wpbih->biBitCount = 24 ;
	wpbih->biSizeImage = width * height * 3 ;

	hr = AVIMakeCompressedStream(&pCompressedStream, pVideoStream, &opts, NULL);
	if ( FAILED( hr) )
	{
		return hr ;
	}

	hr = AVIStreamSetFormat(pCompressedStream, 0,
					   wpbih,	    // stream format
				       wpbih->biSize);

	if ( FAILED( hr ) )
	{
		return hr ;
	}

	buffer = new unsigned char[width * height * 3] ;

	index = 0 ;
	ready = 1 ;

	return hr ;
}

int CAviWriter::write(unsigned char *rgb) {

	HRESULT hr ;
	int i ;

	if(ready) {
		for(i = 0; i < height; i++) {
			memcpy(buffer + i * width * 3, rgb + (height - 1 - i) * width * 3, width * 3) ;
		}
		hr = AVIStreamWrite(pCompressedStream,	// stream pointer
			index,				// time of this frame
			1,				// number to write
			buffer,
			width * height * 3,	// size of this frame
			AVIIF_KEYFRAME,			 // flags....
			NULL,
			NULL);
			index++ ;
	} else {
		return -1 ;
	}

	return hr ;
}

void CAviWriter::close(void) {

	if (pVideoStream) {
		AVIStreamClose(pVideoStream);
		pVideoStream = NULL ;
	}

	if (pCompressedStream) {
		AVIStreamClose(pCompressedStream);
		pCompressedStream = NULL ;
	}

	if (pAVIFile) {
		AVIFileClose(pAVIFile);
		pAVIFile = NULL ;
	}

	if (wpbih) {
		delete wpbih ;
		wpbih = NULL ;
	}

	if (buffer) {
		delete [] buffer ;
		buffer = NULL ;
	}

	ready = index = 0 ;

	AVIFileExit() ;
}

CAviWriter::~CAviWriter(void)
{
	close() ;
}
