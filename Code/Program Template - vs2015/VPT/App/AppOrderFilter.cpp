#include "StdAfx.h"
#include "AppOrderFilter.h"

CAppOrderFilter::CAppOrderFilter(void)
{
	// Class Constructor
}

CAppOrderFilter::~CAppOrderFilter(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppOrderFilter::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists

	pInputYUV = new unsigned char[width * height * 2] ;
	pOutputYUV = new unsigned char[width * height * 2] ;
}

void CAppOrderFilter::CustomFinal(void) {
	// Add custom finalization code here
}

void SortBuf(unsigned char *buf, int size) {

	int i, j ;
	int temp ;

	for(i = 0; i < size - 1; i++) {
		for(j = i + 1; j < size; j++) {
			if(buf[i] > buf[j]) {
				temp = buf[i] ;
				buf[i] = buf[j] ;
				buf[j] = temp ;
			}
		}
	}
}

int Rank(unsigned char *buf, int size, int val) {

	int r ;

	r = 0 ;
	while(buf[r] < val && r < size) {
		r++ ;
	}

	return r ;
}

int RankCheck(unsigned char *buf, int size, int oVal, int nVal) {

	int r ;

	r = 0 ;
	while(buf[r] < oVal && r < size) {
		r++ ;
	}

	return r ;
}

void OrderFilter(unsigned char *iY, unsigned char *oY, int width, int height) {

	int i, j, k ;
	unsigned char buf5x5[25] ;
	unsigned char buf3x3[9] ;
	int rank3x3 ;
	int rank5x5 ;
	int curY ;

	for(j = 2; j < height - 2; j++) {
		for(i = 2; i < width - 2; i++) {
			curY = iY[i + j * width] ;
			for(k = 0; k < 25; k++) {
				buf5x5[(k % 5) + (k / 5) * 5] = iY[i + (k % 5) - 2 + (j + (k / 5) - 2) * width] ;
			}
			for(k = 0; k < 9; k++) {
				buf3x3[(k % 3) + (k / 3) * 3] = iY[i + (k % 3) - 1 + (j + (k / 3) - 1) * width] ;
			}
			SortBuf(buf5x5, 25) ;
			SortBuf(buf3x3, 9) ;
			rank3x3 = Rank(buf3x3, 9, curY) ;
			rank5x5 = Rank(buf5x5, 25, curY) ;
			if(Rank(buf3x3, 9, buf3x3[4]) != rank3x3 || Rank(buf5x5, 25, buf3x3[4]) != rank5x5) {
				oY[i + j * width] = curY ;
			} else {
				oY[i + j * width] = buf3x3[4] ;
			}
		}
	}
}

void CAppOrderFilter::Process(void) {

	// The template generates a dummy copy frame applications
	CImageConvert imgConvert ;

	imgConvert.RGB24_to_YUV420(pInput, pInputYUV, width, height) ;
	memcpy(pOutputYUV, pInputYUV, width * height * 3 / 2) ;
	memset(pOutputYUV + width * height, 128, width * height / 2) ;
	OrderFilter(pInputYUV, pOutputYUV, width, height) ;
	imgConvert.YUV420_to_RGB24(pOutputYUV, pOutput, width, height) ;

//	memcpy(pOutput, pInput, width * height * 3) ;
	PutDC(pOutput) ;
}
