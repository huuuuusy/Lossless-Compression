#pragma once
#include "../processing.h"

class CAppTest :
	public CProcessing
{
public:
	// Add variables here
	unsigned char *rgb ;
	unsigned char *yuv ;

public:
	CAppTest(void);
	~CAppTest(void);
	// Add methods here

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
