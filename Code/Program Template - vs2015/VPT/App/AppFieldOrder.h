#pragma once
#include "../processing.h"

class CAppFieldOrder :
	public CProcessing
{
public:
	// Add variables here

public:
	CAppFieldOrder(void);
	~CAppFieldOrder(void);
	// Add methods here

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
