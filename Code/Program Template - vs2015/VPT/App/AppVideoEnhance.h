#pragma once
#include "../processing.h"

class CAppVideoEnhance :
	public CProcessing
{
public:
	// Add variables here
	unsigned char *frameCurYUV ;
	unsigned char *frameOutYUV ;
	unsigned char *frameCurRGBA ;
	unsigned char *frameOutRGBA ;

public:
	CAppVideoEnhance(void);
	~CAppVideoEnhance(void);
	// Add methods here

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
