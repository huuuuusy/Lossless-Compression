#pragma once
#include "../processing.h"

class CAppConvert :
	public CProcessing
{
public:
	// Add variables here
	unsigned char *pOutput420 ;
	int mode ;

public:
	CAppConvert(void);
	~CAppConvert(void);
	// Add methods here

	void RGBtoYUV(unsigned char *pRGB, unsigned char *pYUV) ;
	void YUVtoRGB(unsigned char *pYUV, unsigned char *pRGB) ;
	void YUVtoYUV420(unsigned char *pYUV, unsigned char *pYUV420) ;

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
