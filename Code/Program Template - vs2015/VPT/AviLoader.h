#pragma once
#include <TCHAR.H>
//#include "wmsdk.h"
#include "vfw.h"

class CAviLoader
{
public:
	IAVIFile *      pAVIFile;	
	IAVIStream *    pVideoStream;
	PGETFRAME pgf;
	int index_end;
	int index;		
	int width;
	int height;
	int size;
	unsigned char *buffer;
public:
	CAviLoader(void);
	HRESULT init(const TCHAR *file);
	int fetch(int frame_index) ;
	~CAviLoader(void);
};
