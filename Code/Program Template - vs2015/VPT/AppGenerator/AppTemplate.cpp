#include "StdAfx.h"
#include "APP_CLASS_NAME.h"

CAPP_CLASS_NAME::CAPP_CLASS_NAME(void)
{
	// Class Constructor
}

CAPP_CLASS_NAME::~CAPP_CLASS_NAME(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAPP_CLASS_NAME::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
}

void CAPP_CLASS_NAME::CustomFinal(void) {
	// Add custom finalization code here
}

void CAPP_CLASS_NAME::Process(void) {

	// The template generates a dummy copy frame applications
	memcpy(pOutput, pInput, width * height * 3) ;
	PutDC(pOutput) ;
}
