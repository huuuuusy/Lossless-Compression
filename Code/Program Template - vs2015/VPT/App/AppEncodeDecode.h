#pragma once
#include "../Processing.h"
#include "../Tools/VFWInterface.h"

class CAppEncodeDecode : public CProcessing
{
public:
	CVFWInterface compCodec ;
	CVFWInterface decompCodec ;
	unsigned char *cdata ;
	long csize ;
	FILE *dumpFile ;
public:
	CAppEncodeDecode(void);
	void CustomInit(CView *pView) ;
	void Process() ;
	void CustomFinal() ;
public:
	~CAppEncodeDecode(void);
};
