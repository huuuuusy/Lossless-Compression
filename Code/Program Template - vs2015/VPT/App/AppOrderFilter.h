#pragma once
#include "../processing.h"
#include "../Tools/ImageConvert.h"

class CAppOrderFilter :
	public CProcessing
{
public:
	// Add variables here
	unsigned char *pInputYUV ;
	unsigned char *pOutputYUV ;

public:
	CAppOrderFilter(void);
	~CAppOrderFilter(void);
	// Add methods here

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
