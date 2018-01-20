#include "StdAfx.h"
#include ".\imagefilter.h"

double CImageFilter::sobel_h[9] = {-1.0,  0.0,  1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0} ;
double CImageFilter::sobel_v[9] = {-1.0, -2.0, -1.0,  0.0, 0.0,  0.0,  1.0, 2.0, 1.0} ;

CImageFilter::CImageFilter(void)
{
}

CImageFilter::~CImageFilter(void)
{
}

void CImageFilter::Convolution(double *img, double *out, int width, int height, double *kernel, double div, int kwidth, int kheight) {

	int i, j, ki, kj, kx, ky ;
	double v ;

	kx = kwidth / 2 ;
	ky = kheight / 2 ;
	for(j = ky; j < (height - ky); j++) {
		for(i = kx; i < (width - kx); i++) {
			v = 0.0 ;
			for(kj = 0; kj < kheight; kj++) {
				for(ki = 0; ki < kwidth; ki++) {
					v += img[(i + ki - kx) + (j + kj - ky) * width] * kernel[ki + kj * kwidth] ;
				}
			}
			out[i + j * width] = v / div ;
		}
	}
	for(j = 0; j < height; j++) {
		for(i = 0; i < kx; i++)
			out[width - 1 - i + j * width] = out[i + j * width] = 0.0 ;
	}
	for(i = kx; i < (width - kx); i++) {
		for(j = 0; j < ky; j++)
			out[i + (height - 1 - j) * width] = out[i + j * width] = 0.0 ;
	}
}

void CImageFilter::Sobel_H(double *img, double *sob_out, int width, int height) {

	Convolution(img, sob_out, width, height, sobel_h, 4.0, 3, 3) ;
}

void CImageFilter::Sobel_V(double *img, double *sob_out, int width, int height) {
	Convolution(img, sob_out, width, height, sobel_v, 4.0, 3, 3) ;
}
