#pragma once
#include "../processing.h"
#include<vector>
#include<list>
using namespace std; 

class CAppCompress :
	public CProcessing
{
public:
	int RPosition,GPosition;
	// Add variables here
public:
	CAppCompress(void);
	~CAppCompress(void);
	// Add methods here
	unsigned char *Compress(int &cDataSize) ;
void Decompress(unsigned char *compressedData, int cDataSize, unsigned char *deCompressedData) ;
	
public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};
