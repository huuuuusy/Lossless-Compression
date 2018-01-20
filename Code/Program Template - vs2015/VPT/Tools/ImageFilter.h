#pragma once

class CImageFilter
{
private:
	static double sobel_h[9] ;
	static double sobel_v[9] ;
public:
	CImageFilter(void);
	~CImageFilter(void);
	void Convolution(double *img, double *out, int width, int height, double *kernel, double div, int kwidth, int kheight) ;
	void Sobel_H(double *img, double *sob_out, int width, int height) ;
	void Sobel_V(double *img, double *sob_out, int width, int height) ;
};
