#include "StdAfx.h"
#include "AppFieldOrder.h"

CAppFieldOrder::CAppFieldOrder(void)
{
	// Class Constructor
}

CAppFieldOrder::~CAppFieldOrder(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppFieldOrder::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
}

void CAppFieldOrder::CustomFinal(void) {
	// Add custom finalization code here
}

void CAppFieldOrder::Process(void) {

	// The template generates a dummy copy frame applications
	int j ;

//	memcpy(pOutput, pInput, width * height * 3) ;

	/*
	for(j = 0; j < height / 2; j++) {
		memcpy(pOutput + (j * 2) * width * 3, pInput + j * width * 3, width * 3) ;
		if(j > 0)
			memcpy(pOutput + (j * 2 - 1) * width * 3, pInput + (height / 2 + j) * width * 3, width * 3) ;
	}
	*/
	for(j = 0; j < height / 2; j++) {
		memcpy(pOutput + 2 * j * width * 3, pInput + (2 * j + 1) * width * 3, width * 3) ;
		memcpy(pOutput + (j * 2 + 1) * width * 3, pInput + (2 * j) * width * 3, width * 3) ;
	}

	PutDC(pOutput) ;
}
