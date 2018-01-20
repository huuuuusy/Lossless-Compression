#pragma once
#include "../Tools/videoDeinterlace.h"
#include "../processing.h"
#include "../AviWriter.h"

//#define DI_AVI_OUTPUT

class CAppDeinterlacing :
	public CProcessing
{
public:
//	FILE *dumpFP ;
	unsigned char *frameCurYUV ;
	unsigned char *frameBtmYUV ;
	unsigned char *frameRefYUV ;
	unsigned char *frameOutYUV ;
	VideoDeinterlaceHandle *hVideoDeinterlace ;
	unsigned char refReady ;

#ifdef DI_AVI_OUTPUT
	CAviWriter aviWriter ;
#endif

public:
	CAppDeinterlacing(void);
	void CustomInit(CView *pView) ;
	void Perform() ;
	void CustomFinal() ;
	~CAppDeinterlacing(void);
};
