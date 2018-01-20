#include "StdAfx.h"
#include "AppCompress.h"
#include"iostream"  
#include "map"  
#include "string"  
#include "iterator"  
#include "vector"  
using namespace std;  
CAppCompress::CAppCompress(void)
{
	// Class Constructor
}

CAppCompress::~CAppCompress(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppCompress::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
}


void CAppCompress::CustomFinal(void) {
	// Add custom finalization code here
}
unsigned char *CAppCompress::Compress(int &cDataSize) {
	unsigned char *compressedData ;
	cDataSize = width*height*3 ;    
	compressedData=new unsigned char[cDataSize*2];
	int cSize=0;
	unsigned short currentB = pInput[0],nextB,repeat = 1,currentG = pInput[1],nextG,currentR=pInput[2],nextR;
	for (int i = 1; i<cDataSize/3;i++)
	{
		nextB=pInput[i*3];
		if (nextB == currentB&&repeat<=127)
		{
			repeat=repeat+1;
			if (i==(cDataSize/3-1))
			{ 
				compressedData[cSize]=repeat;
				compressedData[cSize+1]=currentB;
				cSize=cSize+2;
			}
		}
		else
		{
			compressedData[cSize] = repeat;
			compressedData[cSize+1]=currentB;
			cSize=cSize+2;
			currentB = nextB;
			repeat = 1;
			if (i==(cDataSize/3-1))
			{
				compressedData[cSize] = 1;
				compressedData[cSize+1]=currentB;
				cSize=cSize+2;
			}
		}
	}
	GPosition=cSize;
	repeat=1;
	for (int i=1;i<cDataSize/3;i++)
	{
		nextG = pInput[i*3+1];
		if (nextG == currentG && repeat<=127)
		{
			repeat=repeat+1;
			if (i==(cDataSize/3-1))
			{
				compressedData[cSize] = repeat;
				compressedData[cSize + 1] = currentG;
				cSize=cSize+2;
			}
		}
		else
		{
			compressedData[cSize] = repeat;
			compressedData[cSize + 1] = currentG;
			cSize=cSize+2;
            currentG = nextG;
            repeat = 1;
            if (i==(cDataSize/3-1))
            {
                compressedData[cSize] = 1;
                compressedData[cSize+1]=currentG;
                cSize=cSize+2;
            }
        }
    }
    repeat = 1;
	RPosition=cSize;
    for (int i=1;i<cDataSize/3;i++)
    {
        nextR=pInput[i*3+2];
        if (nextR == currentR && repeat<=127)
        {
            repeat=repeat+1;
            if (i ==(cDataSize/3-1))
            {
                compressedData[cSize]=repeat;
                compressedData[cSize+1]=currentR;
                cSize=cSize+2;
            }
        }
        else
        {
            compressedData[cSize]=repeat;
            compressedData[cSize+1]=currentR;
            cSize=cSize+2;
            currentR=nextR;
            repeat=1;
            if (i==(cDataSize/3-1))
            {
                compressedData[cSize]=1;
                compressedData[cSize+1]=currentR;
                cSize=cSize+2;
            }
        }
    }
    cDataSize = cSize;
    return compressedData;
	}
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {
	int repeat;
	unsigned int b, g, r;
	int i=0,j=0,p=0;
    for (i=0,j=0;i<width*height,j<GPosition;j=j+2)
    {
        repeat = compressedData[j];
        for (p=0;p<repeat;p++)
        {
            int d = compressedData[j+1];
            uncompressedData[i*3+p*3+0]=compressedData[j+1];
        }
         i=i+repeat;
    }
    for (i=0,j=GPosition;i<width*height,j<RPosition;j=j+2)
    {
        repeat = compressedData[j];
        for (p=0; p<repeat;p++)
        {
            int d = compressedData[j+1];
            uncompressedData[i*3+p*3+1]=compressedData[j+1];
        }
        i=i+repeat;
    }
    for (i=0,j=RPosition;i<width*height, j<cDataSize;j=j+2)
    {
        repeat = compressedData[j];
        for (int p=0;p<repeat;p++)
        {
            int d=compressedData[j+1];
            uncompressedData[i*3+p*3+2]=compressedData[j+1];
        }
        i=i+repeat;
    }

}


void CAppCompress::Process(void) {

	// Don't change anything within this function.
     int i, cDataSize ;
     unsigned char *compressedData ;
	 unsigned char *verifyCompressedData ;
	SetTitle(pOutput, _T("Lossless Decompressed Image")) ;
	compressedData = Compress(cDataSize) ;
	Decompress(compressedData, cDataSize, pOutput) ;
	for(i = 0; i < width * height * 3; i++) {
		if(pInput[i] != pOutput[i]) {
			printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n")) ;
			break ;
		}
	}

	printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n  "), width * height * 3, cDataSize, (double) width * height * 3 / cDataSize) ;

	PutDC(pOutput) ;
}
