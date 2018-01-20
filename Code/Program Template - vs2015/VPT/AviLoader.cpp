#include "StdAfx.h"
#include ".\aviloader.h"

CAviLoader::CAviLoader(void) {

	pAVIFile = NULL ;	
	pVideoStream = NULL ;
	pgf = NULL ;
	index_end = 0 ;
	index = 0 ;		
	width = 0 ;
	height = 0 ;
	size = 0 ;
	buffer = NULL ;
}

HRESULT CAviLoader::init(const TCHAR *avifile)
{
	BITMAPINFOHEADER *pbih;
	BITMAPINFOHEADER *wpbih ;
	HRESULT hr ;
	AVIFILEINFO FileInfo ;

	AVIFileInit();
	
//	hr = AVIStreamOpenFromFile(&pVideoStream,avifile,streamtypeVIDEO, 0,OF_SHARE_DENY_WRITE,NULL) ;

	hr = AVIFileOpen( &pAVIFile, avifile, OF_SHARE_DENY_NONE, NULL );
	if( FAILED( hr ) )
	{		
		return hr;
	}

	AVIFileInfo(pAVIFile, &FileInfo, sizeof(AVIFILEINFO)) ;

	hr = AVIFileGetStream( pAVIFile, &pVideoStream, streamtypeVIDEO, 0 );
	if( FAILED( hr ) )
	{	
		return hr;
	}

	wpbih = new BITMAPINFOHEADER ;
	memset(wpbih, 0, sizeof(BITMAPINFOHEADER)) ;
	wpbih->biSize = sizeof(BITMAPINFOHEADER) ;
	wpbih->biCompression = BI_RGB ;
	wpbih->biWidth = FileInfo.dwWidth ;
	wpbih->biHeight = FileInfo.dwHeight ;
	wpbih->biPlanes = 1 ;
	wpbih->biBitCount = 24 ;
	wpbih->biSizeImage = wpbih->biWidth * wpbih->biHeight * 3 ;

	index_end = AVIStreamEnd(pVideoStream);
	index = AVIStreamStart( pVideoStream);

	pgf = AVIStreamGetFrameOpen(pVideoStream, wpbih);

	delete wpbih ;

	if(pgf == NULL)
	{	
		return AVIERR_NODATA;
	}

	pbih = (BITMAPINFOHEADER *) AVIStreamGetFrame(pgf, index);

	width=pbih->biWidth;
	height=pbih->biHeight;
	size=pbih->biSizeImage;
	buffer = new unsigned char [pbih->biSizeImage] ;
	return S_OK ;
}

int CAviLoader::fetch(int frame_index) {

	BITMAPINFOHEADER *pbih;


	if(frame_index < 0 || frame_index > index_end) {
		return 0 ;
	}

	pbih = (BITMAPINFOHEADER *) AVIStreamGetFrame(pgf, frame_index);

	if(pbih) {
		for(int i = 0; i < height; i++) {
			memcpy(buffer + (height - 1 - i) * width * 3, ((unsigned char *) &pbih[1]) + i * width * 3, width * 3);
		}
		index = frame_index ;
		return 1 ;
	}
	return 0 ;
}

CAviLoader::~CAviLoader(void)
{
	if(buffer) delete buffer ;
	AVIStreamGetFrameClose(pgf);	
	AVIFileExit();
}
