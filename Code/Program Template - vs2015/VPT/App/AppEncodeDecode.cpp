#include "StdAfx.h"
#include "AppEncodeDecode.h"
#include "../Tools/imageConvert.h"
//#include "../Tools/mp4quant.h"
//#include "../Tools/mp4dct.h"

CAppEncodeDecode::CAppEncodeDecode(void)
{
	csize = 0 ;
}

CAppEncodeDecode::~CAppEncodeDecode(void)
{
	Final() ;
}

void CAppEncodeDecode::CustomInit(CView *pView) {

	if(!compCodec.Ready()) {
		compCodec.Init('ASK', TRUE, width, height, 24, TRUE) ;
//		compCodec.Init('X264', TRUE, width, height, 24, TRUE) ;
//		compCodec.Init('MVVD', TRUE, width, height, 24, FALSE) ;
		//		compCodec.Config(this->GetSafeHwnd()) ;
		if(compCodec.Ready() && !decompCodec.Ready()) {
//			decompCodec.Init('X264', FALSE, width, height, 24, FALSE) ;
			decompCodec.Init(compCodec.GetFCC(), FALSE, width, height, 24, FALSE) ;			
//			decompCodec.Init('MVVD', FALSE, width, height, 24, FALSE) ;
		}
//		decompCodec.Config(this->GetSafeHwnd()) ;
		csize = compCodec.Compress(((CVPTDoc *) pDoc)->imageLoader.bits, &cdata) ;

		dumpFile = fopen("c:\\temp\\dump.h264", "wb") ;
		fwrite(cdata, csize, 1, dumpFile) ;

		if(pOutput)
			decompCodec.Decompress(cdata, csize, pOutput) ;
	}
}

void CAppEncodeDecode::Process(void) {

	if(compCodec.Ready()) {
		int size ;

		GetSrc() ;
/*
		{
			int i ;

			for(i = 0; i < width * height * 3; i++) {
				pInput[i] = (pInput[i] >> 2) << 2 ;
			}

			PutDC(pInput) ;
		}
*/
		size = compCodec.Compress(pInput, &cdata) ;

		fwrite(cdata, size, 1, dumpFile) ;

		csize += size ;
		if(pOutput && decompCodec.Ready()) {
			if(decompCodec.Decompress(cdata, size, pOutput) > 0) {
				PutDC() ;
			}
		}
		printf(_T("size = %d\r\n"), csize) ;
	}
}

void CAppEncodeDecode::CustomFinal(void) {

	fclose(dumpFile) ;

	if(decompCodec.Ready()) {
		decompCodec.Close() ;
	}
	if(compCodec.Ready()) {
		compCodec.Close() ;
	}
}