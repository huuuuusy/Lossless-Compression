#pragma once
#include <emmintrin.h>

class CImageConvert
{
public:
	CImageConvert(void);
	~CImageConvert(void);
	void RGB24_to_YUV420(unsigned char *rgb, unsigned char *yuv, int width, int height) ;
	void RGB24_to_YUV420_C(unsigned char *rgb, unsigned char *yuv, int width, int height) ;
	void RGB24_to_YUV420_SSE2(unsigned char *rgb, unsigned char *yuv, int width, int height) ;
	void YUV420_to_RGB24(unsigned char *yuv, unsigned char *rgb, int width, int height) ;
	void YUV420_to_RGB24_C(unsigned char *yuv, unsigned char *rgb, int width, int height) ;
	void YUV420_to_RGB24_SSE2(unsigned char *yuv, unsigned char *rgb, int width, int height) ;
	void RGB24_to_VYUY(unsigned char *rgb, unsigned char *vyuy, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void RGB24_to_VYUY_C(unsigned char *rgb, unsigned char *vyuy, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void RGB24_to_VYUY_SSE2(unsigned char *rgb, unsigned char *vyuy, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void VYUY_to_RGB24(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void VYUY_to_RGB24_C(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void VYUY_to_RGB24_SSE2(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset = 1, int u_offset = 2, int v_offset = 0) ;
	void RGB24_to_YUYV(unsigned char *rgb, unsigned char *yuyv, int width, int height) ;
	void YUYV_to_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height) ;
	void RGB24_to_RGBpDouble(unsigned char *rgb, double *rgbdouble, int width, int height) ;
	void Char8_to_Double(unsigned char *c, double *f, int width, int height) ;
	void Double_to_Char8(double *f, unsigned char *c, int width, int height, BOOL normalize = FALSE) ;
	void Char8_to_RGB24(unsigned char *c, unsigned char *rgb, int width, int height) ;
	void RGB24_to_Char8(unsigned char *rgb, unsigned char *c, int width, int height) ;
	void RGB24_to_Double(unsigned char *rgb, double *f, int width, int height) ;
	void Double_to_RGB24(double *f, unsigned char *rgb, int width, int height, BOOL normalize = FALSE) ;
	void RGB24_to_RGB24p(unsigned char *rgb, unsigned char *rgbp, int width, int height) ;
	void RGB24p_to_RGB24(unsigned char *rgbp, unsigned char *rgb, int width, int height) ;
	void RGB24_to_RGBA(unsigned char *rgb, unsigned char *rgba, int width, int height) ;
	void RGBA_to_RGB24(unsigned char *rgba, unsigned char *rgb, int width, int height) ;
	int SizeRGB24(int width, int height) {
		return width * height * 3 ;
	}
	int SizeYUYV(int width, int height) {
		return ((width + 1) / 2 * 2) * height * 2 ;
	}
	int SizeYUV420(int width, int height) {
		return ((width + 1) / 2 * 2) * ((height + 1) / 2 * 2) + ((width + 1) / 2) * ((height + 1) / 2) * 2 ;
	}
private:
	__m128i RCOEF_Y, RCOEF_U, RCOEF_V ;
	__m128i GCOEF_Y, GCOEF_U, GCOEF_V ;
	__m128i BCOEF_Y, BCOEF_U, BCOEF_V ;
	__m128i YCOEF_R, YCOEF_G, YCOEF_B ;
	__m128i UCOEF_R, UCOEF_G, UCOEF_B ;
	__m128i VCOEF_R, VCOEF_G, VCOEF_B ;
	__m128i C128, C16, C1, C0, C128_32 ;
	unsigned char clipval[512] ;
	unsigned char *clip ;
	unsigned SSE2 ;
};
