#pragma once
#include <TCHAR.H>
//#include "wmsdk.h"
#include "vfw.h"

class CAviWriter
{
public:
	AVISTREAMINFO streamInfo ;
	int index ;		
	int width ;
	int height ;
	int size ;
	int ready ;
	unsigned char *buffer ;
	IAVIFile *      pAVIFile ;	
	IAVIStream *    pVideoStream ;
	IAVIStream *	pCompressedStream ;
	BITMAPINFOHEADER *wpbih ;
	AVICOMPRESSOPTIONS opts ;
	AVICOMPRESSOPTIONS FAR * aopts[1] ;
public:
	CAviWriter(void);
	HRESULT init(const TCHAR *aviFile, int imgWidth, int imgHeight);
	void close(void) ;
	int write(unsigned char *rgb) ;
	~CAviWriter(void);
};
