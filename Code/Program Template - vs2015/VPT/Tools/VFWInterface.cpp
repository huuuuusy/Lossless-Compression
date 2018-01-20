#include "StdAfx.h"
#include ".\vfwinterface.h"

CVFWInterface::CVFWInterface(void)
{
	hic = 0 ;
	data = NULL ;
	prevFrame = NULL ;
	pbih = NULL ;
	ready = FALSE ;
}

CVFWInterface::~CVFWInterface(void)
{
	Close() ;
	ready = FALSE ;
}

HIC CVFWInterface::Init(DWORD fcc, BOOL modeComp, int video_width, int video_height, int bit, BOOL config) {

	int FmtLength ;
	union {
		char c[4] ;
		DWORD dw ;
	} ofcc, nfcc ;
	COMPVARS pc ;

	this->fcc = fcc ;
	ofcc.dw = fcc ;
	nfcc.c[0] = ofcc.c[3] ;
	nfcc.c[1] = ofcc.c[2] ;
	nfcc.c[2] = ofcc.c[1] ;
	nfcc.c[3] = ofcc.c[0] ;

	//Construct BitmapInfoHeader(RGB24)
	compMode = modeComp ;
	ifcc = nfcc.dw ;

	hic = 0 ;
	pbih = NULL ;
	if(compMode) {
		ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = video_width;
		bih.biHeight = video_height;
		bih.biPlanes = 1;
		bih.biBitCount = bit;
	
		bih.biCompression = BI_RGB;
		bih.biSizeImage = (video_width * video_height * bit)/8;

		if(!fcc || fcc == 'ASK') {
			memset(&pc, 0, sizeof(COMPVARS)) ;
			pc.cbSize = sizeof(COMPVARS);
			ICCompressorChoose(NULL, ICMF_CHOOSE_ALLCOMPRESSORS, NULL, NULL, &pc, "Select Compressor") ;
			ifcc = pc.fccHandler ;
			nfcc.dw = ifcc ;
			ofcc.c[0] = nfcc.c[3] ;
			ofcc.c[1] = nfcc.c[2] ;
			ofcc.c[2] = nfcc.c[1] ;
			ofcc.c[3] = nfcc.c[0] ;
			this->fcc = ofcc.dw ;
			ICCompressorFree(&pc) ;
		}
		hic = ICOpen(ICTYPE_VIDEO, ifcc, ICMODE_COMPRESS) ;
		if(hic) {
			FmtLength = (int) ICCompressGetFormat(hic, &bih,  NULL);

			if(FmtLength > 0)
				pbih = (LPBITMAPINFOHEADER) malloc(FmtLength) ;

			if(pbih) {
				ICCompressGetFormat(hic, &bih, pbih) ;
				if(config && fcc && fcc != 'ASK') Config(NULL) ;
				ICCompressBegin(hic, &bih, pbih) ;
				data = new unsigned char[bih.biSizeImage * 3] ;
				prevFrame = new unsigned char[bih.biSizeImage] ;
			} else {
				ICClose(hic) ;
				hic = 0 ;
			}
		}
	} else {
		ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = video_width;
		bih.biHeight = video_height;
		bih.biPlanes = 1;
		bih.biBitCount = bit;
		bih.biCompression = BI_RGB ;	// For some codec, this must be set to BI_RGB :(
		bih.biSizeImage = (video_width * video_height * bit)/8;

		if(!fcc || fcc == 'ASK') {
			memset(&pc, 0, sizeof(COMPVARS)) ;
			pc.cbSize = sizeof(COMPVARS);
			ICCompressorChoose(NULL, ICMF_CHOOSE_ALLCOMPRESSORS, NULL, NULL, &pc, "Select DeCompressor") ;
			ifcc = pc.fccHandler ;
			ICCompressorFree(&pc) ;
		}
		

		if(fcc == 'X264' || fcc == 'x264' || fcc == 'h264' || fcc == 'MP42' || fcc == 'MJPG' || fcc == 'H264') {
			hic = ICOpen(ICTYPE_VIDEO, 'SDFF', ICMODE_DECOMPRESS) ;
		} else {
			hic = ICOpen(ICTYPE_VIDEO, ifcc, ICMODE_DECOMPRESS) ;
		}
		if(hic) {
			FmtLength = (int) ICDecompressGetFormat(hic, &bih,  NULL);
			if(config && fcc && fcc != 'ASK') Config(NULL) ;

			if(FmtLength > 0)
				pbih = (LPBITMAPINFOHEADER) malloc(FmtLength) ;

			if(pbih) {
				ICDecompressGetFormat(hic, &bih, pbih) ;
				{
					HIC hicTemp ;
					LPBITMAPINFOHEADER pbihTemp ;
					BITMAPINFOHEADER bihTemp ;

					pbihTemp = NULL ;
					ZeroMemory(&bihTemp, sizeof(BITMAPINFOHEADER));
					bihTemp.biSize = sizeof(BITMAPINFOHEADER);
					bihTemp.biWidth = video_width;
					bihTemp.biHeight = video_height;
					bihTemp.biPlanes = 1;
					bihTemp.biBitCount = bit;
	
					bihTemp.biCompression = BI_RGB;
					bihTemp.biSizeImage = (video_width * video_height * bit)/8;

					hicTemp = ICOpen(ICTYPE_VIDEO, ifcc, ICMODE_COMPRESS) ;
					if(hicTemp)
						FmtLength = (int) ICCompressGetFormat(hicTemp, &bihTemp,  NULL);
					else
						FmtLength = 0 ;
					if(FmtLength > 0) {
						pbihTemp = (LPBITMAPINFOHEADER) malloc(FmtLength) ;
						ICCompressGetFormat(hicTemp, &bihTemp, pbihTemp) ;
						bih.biCompression = pbihTemp->biCompression ;
					} else {
						bih.biCompression = ifcc ;
					}
					if(hicTemp) ICClose(hicTemp) ;
					if(pbihTemp) free(pbihTemp) ;
				}

//				bih.biCompression = ifcc ;
				ICDecompressBegin(hic, &bih, pbih) ;
			} else {
				ICClose(hic) ;
				hic = 0 ;
			}
		}

/*
		{
			char str[256] ;

			sprintf(str, "%d, %d, %d, %d"
				, pbih->biWidth, pbih->biHeight, pbih->biBitCount,
				pbih->biSizeImage) ;
			AfxMessageBox(str) ;
		}
*/
	}
	fnum = 0 ;
	if(hic) ready = TRUE ;
	return hic ;
}

DWORD CVFWInterface::GetFCC(void) {

	return this->fcc ;
}

void CVFWInterface::Config(HWND hwnd) {
	if(ICQueryConfigure(hic)) ICConfigure(hic, hwnd) ;
}

int CVFWInterface::Compress(unsigned char *src, unsigned char **cdata) {

	int r ;

	pbih->biSizeImage = bih.biSizeImage ;
	if(fnum == 0) {
		r = ICCompress(hic, ICCOMPRESS_KEYFRAME, pbih, data, &bih, src, &dwCkID, &dwFlags, fnum, 0, ICGetDefaultQuality(hic), NULL, NULL) ;
	} else {
		r = ICCompress(hic, 0, pbih, data, &bih, src, &dwCkID, &dwFlags, fnum, 0, ICGetDefaultQuality(hic), &bih, prevFrame) ;
//		r = ICCompress(hic, 0, pbih, data, &bih, src, &dwCkID, &dwFlags, fnum, 0, ICGetDefaultQuality(hic), NULL, NULL) ;
	}
	memcpy(prevFrame, src, bih.biSizeImage) ;
	fnum++ ;
	*cdata = data ;
	if(r == ICERR_OK)
		return pbih->biSizeImage ;
	else return -1 ;
}

int CVFWInterface::Decompress(unsigned char *cdata, int size, unsigned char *out) {

	int r ;
	bih.biSizeImage = size ;
	r = ICDecompress(hic, 0, &bih, cdata, pbih, out) ;
	if(r == ICERR_OK) {
		fnum++ ;
		return pbih->biSizeImage ;
	}
	else return -1 ;
}

BOOL CVFWInterface::Ready(void) {

	return ready ;
}

void CVFWInterface::Close(void) {

	if(hic > 0) {
		if(compMode) {
			ICCompressEnd(hic) ;
		} else {
			ICDecompressEnd(hic) ;
		}
		ICClose(hic) ;
		if(compMode) {
			delete [] data ;
			data = NULL ;
			delete [] prevFrame ;
			prevFrame = NULL ;
		}
		hic = 0 ;
		free(pbih) ;
		pbih = NULL ;
		ready = FALSE ;
	}
}
