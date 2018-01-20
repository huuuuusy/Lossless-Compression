#include "StdAfx.h"
#include ".\imageconvert.h"
#include <emmintrin.h>
#include "cpuid.h"

#define byte_align_16(a) (a + (((((long long) a)+15)&(~15)) - ((long long) a)))

int dimensionExtend(int width, int height, int &width32, int &height16) {

	if((width % 32) || (height % 32)) {
		width32 = (width + 31) / 32 * 32 ;
		height16 = (height + 15) / 16 * 16 ;

		return 1 ;
	}
	width32 = width ;
	height16 = height ;
	return 0 ;
}

void frameCopyRGB24(unsigned char *dest, unsigned char *src, int width_d, int height_d, int width_s, int height_s) {

	int j ;
	int height, width ;

	height = height_d < height_s ? height_d : height_s ;
	width = width_d < width_s ? width_d : width_s ;

	for(j = 0; j < height; j++) {
		memcpy(dest + j * width_d * 3, src + j * width_s * 3, width * 3) ;
	}
}

void frameCopyYUV420(unsigned char *dest, unsigned char *src, int width_d, int height_d, int width_s, int height_s) {

	int j ;
	int height, width ;

	width_d = (width_d + 1) / 2 * 2 ;
	width_s = (width_s + 1) / 2 * 2 ;
	height_d = (height_d + 1) / 2 * 2 ;
	height_s = (height_s + 1) / 2 * 2 ;

	height = height_d < height_s ? height_d : height_s ;
	width = width_d < width_s ? width_d : width_s ;

	for(j = 0; j < height; j++) {
		memcpy(dest + j * width_d, src + j * width_s, width) ;
	}

	dest += width_d * height_d ;
	src += width_s * height_s ;

	width_d = (width_d + 1) >> 1 ;
	width_s = (width_s + 1) >> 1 ;
	height_d = (height_d + 1) >> 1 ;
	height_s = (height_s + 1) >> 1 ;
	width = (width + 1) >> 1 ;
	height = (height + 1) >> 1 ;
	for(j = 0; j < height; j++) {
		memcpy(dest + j * width_d, src + j * width_s, width) ;
	}

	dest += width_d * height_d ;
	src += width_s * height_s ;
	for(j = 0; j < height; j++) {
		memcpy(dest + j * width_d, src + j * width_s, width) ;
	}
}

void frameCopyVYUY(unsigned char *dest, unsigned char *src, int width_d, int height_d, int width_s, int height_s) {

	int j ;
	int height, width ;

	width_d = (width_d + 1) / 2 * 2 ;
	width_s = (width_s + 1) / 2 * 2 ;
	height = height_d < height_s ? height_d : height_s ;
	width = width_d < width_s ? width_d : width_s ;

	for(j = 0; j < height; j++) {
		memcpy(dest + j * width_d * 2, src + j * width_s * 2, width * 2) ;
	}
}

CImageConvert::CImageConvert(void)
{
	int i ;

	for(i = 0; i < 512; i++) {
		if((i - 128) < 0) {
			clipval[i] = 0 ;
		} else if((i - 128) > 255) {
			clipval[i] = 255 ;
		} else {
			clipval[i] = (unsigned char) (i - 128) ;
		}
	}
	clip = clipval + 128 ;

	if(cpuSSE2()) {
		SSE2 = 1 ;
		C128_32 = _mm_set1_epi32(128) ;
		C128 = _mm_set1_epi16(128) ;
		C16 = _mm_set1_epi16(16) ;
		C1 = _mm_set1_epi16(1) ;
		C0 = _mm_setzero_si128() ;
		YCOEF_R = _mm_set1_epi16(66) ;
		YCOEF_G = _mm_set1_epi16(129) ;
		YCOEF_B = _mm_set1_epi16(25) ;
		UCOEF_R = _mm_set1_epi16(-38) ;
		UCOEF_G = _mm_set1_epi16(-74) ;
		UCOEF_B = _mm_set1_epi16(112) ;
		VCOEF_R = _mm_set1_epi16(112) ;
		VCOEF_G = _mm_set1_epi16(-94) ;
		VCOEF_B = _mm_set1_epi16(-18) ;
		RCOEF_Y = _mm_set1_epi16(298) ;
		RCOEF_U = _mm_set1_epi16(0) ;
		RCOEF_V = _mm_set1_epi16(409) ;
		GCOEF_Y = _mm_set1_epi16(298) ;
		GCOEF_U = _mm_set1_epi16(-100) ;
		GCOEF_V = _mm_set1_epi16(-208) ;
		BCOEF_Y = _mm_set1_epi16(298) ;
		BCOEF_U = _mm_set1_epi16(516) ;
		BCOEF_V = _mm_set1_epi16(0) ;
	} else {
		SSE2 = 0 ;
	}
}

CImageConvert::~CImageConvert(void)
{
}

#define RGB_8_8_8_TO_RGB_888(R0_15, G0_15, B0_15, RGB0, RGB1, RGB2) {\
\
		__m128i MASK, TMP ;\
		MASK = _mm_setr_epi32(0x000000FF, 0x00000000, 0x00000000, 0x00000000) ; /* MASK = 00000000|00000000|00000000|000000FF */ \
		RGB0 = _mm_and_si128(B0_15, MASK) ;	/* RGB0 = 00000000|00000000|00000000|000000B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 1) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|0000G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|00R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|0000FF00 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 3) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|000000G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|0000R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|00FF0000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 5) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 6) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|000000R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|FF000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 6) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|0000B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 7) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 8) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|000000FF|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 8) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 000000B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 9) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 0000G4B4|00G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 10) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|0000FF00|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 10) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = B5R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		RGB1 = _mm_srli_si128(TMP, 5) ;		/* RGB1 = 00000000|00000000|00000000|000000G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|0000R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00FF0000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|00B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 3) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|000000R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|FF000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|0000B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 1) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|000000FF|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|000000B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 1) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|0000G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|0000FF00|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 3) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 000000G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 0000R9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00FF0000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00BAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 5) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = GABAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB2 = _mm_srli_si128(TMP, 10) ;	/* RGB2 = 00000000|00000000|00000000|000000RA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|FF000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 10) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|0000BBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 9) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|00GBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 8) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 000000FF|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 8) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|000000BC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 7) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|0000GCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 6) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00RCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 0000FF00|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 6) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 5) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|000000GD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|0000RDGD|BDRCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00FF0000|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00BERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 3) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 000000RE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = FF000000|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 0000BFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 1) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00GFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = RFGFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
}

#define RGB_8_8_8_TO_RGB_888_2(R0_15, G0_15, B0_15, RGB0, RGB1, RGB2) {\
\
		__m128i MASK, TMP ;\
		MASK = _mm_setr_epi32(0x000000FF, 0x00000000, 0x00000000, 0x00000000) ; /* MASK = 00000000|00000000|00000000|000000FF */ \
		RGB0 = _mm_and_si128(B0_15, MASK) ;	/* RGB0 = 00000000|00000000|00000000|000000B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 1) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|0000G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|00R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|0000FF00 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00000000|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 3) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|000000G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|0000R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|00FF0000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|00B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 5) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00000000|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 6) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|000000R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00000000|FF000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 6) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|0000B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 7) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|00G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 8) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00000000|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|000000FF|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 8) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 000000B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 9) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 0000G4B4|00G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 10) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = 00R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|0000FF00|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 10) ; \
		RGB0 = _mm_or_si128(TMP, RGB0) ;	/* RGB0 = B5R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		RGB1 = _mm_srli_si128(TMP, 5) ;		/* RGB1 = 00000000|00000000|00000000|000000G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|0000R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|00FF0000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|00B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 3) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00000000|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|000000R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00000000|FF000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|0000B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 1) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|00G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00000000|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|000000FF|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|000000B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 1) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|0000G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|00R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|0000FF00|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 2) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00000000|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 3) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 000000G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 0000R9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|00FF0000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 4) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = 00BAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_slli_si128(TMP, 5) ; \
		RGB1 = _mm_or_si128(TMP, RGB1) ;	/* RGB1 = GABAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB2 = _mm_srli_si128(TMP, 10) ;	/* RGB2 = 00000000|00000000|00000000|000000RA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00000000|FF000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 10) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|0000BBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 9) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|00GBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 8) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00000000|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 000000FF|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 8) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|000000BC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 7) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|0000GCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 6) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|00RCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 0000FF00|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 6) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00000000|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 5) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|000000GD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|0000RDGD|BDRCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = 00FF0000|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 4) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|00BERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 3) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00000000|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 000000RE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		MASK = _mm_slli_si128(MASK, 1) ;	/* MASK = FF000000|00000000|00000000|00000000 */ \
		TMP = _mm_and_si128(B0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 2) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 0000BFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(G0_15, MASK) ; \
		TMP = _mm_srli_si128(TMP, 1) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = 00GFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
		TMP = _mm_and_si128(R0_15, MASK) ; \
		RGB2 = _mm_or_si128(TMP, RGB2) ;	/* RGB2 = RFGFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
} ;

#define SET_RGB_888_MASK { \
	MASK_0 = _mm_setr_epi32(0x000000FF, 0x00000000, 0x00000000, 0x00000000) ; \
	MASK_1 = _mm_setr_epi32(0x0000FF00, 0x00000000, 0x00000000, 0x00000000) ; \
	MASK_2 = _mm_setr_epi32(0x00FF0000, 0x00000000, 0x00000000, 0x00000000) ; \
	MASK_3 = _mm_setr_epi32(0xFF000000, 0x00000000, 0x00000000, 0x00000000) ; \
	MASK_4 = _mm_setr_epi32(0x00000000, 0x000000FF, 0x00000000, 0x00000000) ; \
	MASK_5 = _mm_setr_epi32(0x00000000, 0x0000FF00, 0x00000000, 0x00000000) ; \
	MASK_6 = _mm_setr_epi32(0x00000000, 0x00FF0000, 0x00000000, 0x00000000) ; \
	MASK_7 = _mm_setr_epi32(0x00000000, 0xFF000000, 0x00000000, 0x00000000) ; \
	MASK_8 = _mm_setr_epi32(0x00000000, 0x00000000, 0x000000FF, 0x00000000) ; \
	MASK_9 = _mm_setr_epi32(0x00000000, 0x00000000, 0x0000FF00, 0x00000000) ; \
	MASK_A = _mm_setr_epi32(0x00000000, 0x00000000, 0x00FF0000, 0x00000000) ; \
	MASK_B = _mm_setr_epi32(0x00000000, 0x00000000, 0xFF000000, 0x00000000) ; \
	MASK_C = _mm_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x000000FF) ; \
	MASK_D = _mm_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x0000FF00) ; \
	MASK_E = _mm_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x00FF0000) ; \
	MASK_F = _mm_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0xFF000000) ; \
}

#define RGB_888_TO_RGB_16_16_16_EXTRACT(RGB_X, C, MASK_X, L_R, S_N) { \
	C = _mm_or_si128(C, _mm_s##L_R##li_si128(_mm_and_si128(RGB_X, MASK_##MASK_X##), S_N)) ; \
}

#define RGB_888_TO_RGB_16_16_16(RGB0, RGB1, RGB2, R0_7, R8_15, G0_7, G8_15, B0_7, B8_15) { \
\
	/* RGB0 = B5R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
	/* RGB1 = GABAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
	/* RGB2 = RFGFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
\
	R0_7 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
	G0_7 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
	B0_7 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
	R8_15 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
	G8_15 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
	B8_15 = _mm_setr_epi32(0x0, 0x0, 0x0, 0x0) ; \
\
																	/* RGB0		= B5R4G4B4|R3G3B3R2|G2B2R1G1|B1R0G0B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, 0, l, 0) ;			/* B0_7		= 00000000|00000000|00000000|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, 3, r, 1) ;			/* B0_7		= 00000000|00000000|00000000|00B100B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, 6, r, 2) ;			/* B0_7		= 00000000|00000000|000000B2|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, 9, r, 3) ;			/* B0_7		= 00000000|00000000|00B300B2|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, C, r, 4) ;			/* B0_7		= 00000000|000000B4|00B300B2|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, B0_7, F, r, 5) ;			/* B0_7		= 00000000|00B500B4|00B300B2|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, G0_7, 1, r, 1) ;			/* G0_7		= 00000000|00000000|00000000|000000G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, G0_7, 4, r, 2) ;			/* G0_7		= 00000000|00000000|00000000|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, G0_7, 7, r, 3) ;			/* G0_7		= 00000000|00000000|000000G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, G0_7, A, r, 4) ;			/* G0_7		= 00000000|00000000|00G300G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, G0_7, D, r, 5) ;			/* G0_7		= 00000000|000000G4|00G300G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, R0_7, 2, r, 2) ;			/* R0_7		= 00000000|00000000|00000000|000000R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, R0_7, 5, r, 3) ;			/* R0_7		= 00000000|00000000|00000000|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, R0_7, 8, r, 4) ;			/* R0_7		= 00000000|00000000|000000R2|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, R0_7, B, r, 5) ;			/* R0_7		= 00000000|00000000|00R300R2|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB0, R0_7, E, r, 6) ;			/* R0_7		= 00000000|000000R4|00R300R2|00R100R0 */ \
\
																	/* RGB1		= GABAR9G9|B9R8G8B8|R7G7B7R6|G6B6R5G5 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G0_7, 0, l, 10) ;			/* G0_7		= 00000000|00G500G4|00G300G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G0_7, 3, l, 9) ;			/* G0_7		= 000000G6|00G500G4|00G300G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G0_7, 6, l, 8) ;			/* G0_7		= 00G700G6|00G500G4|00G300G2|00G100G0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, R0_7, 1, l, 9) ;			/* R0_7		= 00000000|00R500R4|00R300R2|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, R0_7, 4, l, 8) ;			/* R0_7		= 000000R6|00R500R4|00R300R2|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, R0_7, 7, l, 7) ;			/* R0_7		= 00R700R6|00R500R4|00R300R2|00R100R0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, B0_7, 2, l, 10) ;			/* B0_7		= 000000B6|00B500B4|00B300B2|000000B0 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, B0_7, 5, l, 9) ;			/* B0_7		= 000000B6|00B500B4|00B300B2|000000B0 */ \
\
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, B8_15, 8, r, 8) ;			/* B8_15	= 00000000|00000000|00000000|000000B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, B8_15, B, r, 9) ;			/* B8_15	= 00000000|00000000|00000000|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, B8_15, E, r, 10) ;		/* B8_15	= 00000000|00000000|000000BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G8_15, 9, r, 9) ;			/* G8_15	= 00000000|00000000|00000000|000000G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G8_15, C, r, 10) ;		/* G8_15	= 00000000|00000000|00000000|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, G8_15, F, r, 11) ;		/* G8_15	= 00000000|00000000|000000GA|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, R8_15, A, r, 10) ;		/* R8_15	= 00000000|00000000|00000000|000000R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB1, R8_15, D, r, 11) ;		/* R8_15	= 00000000|00000000|00000000|00R900R8 */ \
\
																	/* RGB2		= RFGFBFRE|GEBERDGD|BDRCGCBC|RBGBBBRA */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, 0, l, 4) ;			/* R8_15	= 00000000|00000000|000000RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, 3, l, 3) ;			/* R8_15	= 00000000|00000000|00RB00RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, 6, l, 2) ;			/* R8_15	= 00000000|000000RC|00RB00RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, 9, l, 1) ;			/* R8_15	= 00000000|00RD00RC|00RB00RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, C, l, 0) ;			/* R8_15	= 000000RE|00RD00RC|00RB00RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, R8_15, F, r, 1) ;			/* R8_15	= 00RF00RE|00RD00RC|00RB00RA|00R900R8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, B8_15, 1, l, 5) ;			/* B8_15	= 00000000|00000000|00BB00BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, B8_15, 4, l, 4) ;			/* B8_15	= 00000000|000000BC|00BB00BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, B8_15, 7, l, 3) ;			/* B8_15	= 00000000|00BD00BC|00BB00BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, B8_15, A, l, 2) ;			/* B8_15	= 000000BE|00BD00BC|00BB00BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, B8_15, D, l, 1) ;			/* B8_15	= 00BF00BE|00BD00BC|00BB00BA|00B900B8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, G8_15, 2, l, 4) ;			/* G8_15	= 00000000|00000000|00GB00GA|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, G8_15, 5, l, 3) ;			/* G8_15	= 00000000|000000GC|00GB00GA|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, G8_15, 8, l, 2) ;			/* G8_15	= 00000000|00GD00GC|00GB00GA|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, G8_15, B, l, 1) ;			/* G8_15	= 000000GE|00GD00GC|00GB00GA|00G900G8 */ \
	RGB_888_TO_RGB_16_16_16_EXTRACT(RGB2, G8_15, E, l, 0) ;			/* G8_15	= 00GF00GE|00GD00GC|00GB00GA|00G900G8 */ \
\
}

#define RGB_16_16_16_TO_YUV_16_16_16(R0_7, G0_7, B0_7, Y0_7, U0_7, V0_7) { \
\
		__m128i YMUL_R, YMUL_G, YMUL_B ; \
		__m128i UMUL_R, UMUL_G, UMUL_B ; \
		__m128i VMUL_R, VMUL_G, VMUL_B ; \
\
		YMUL_R = _mm_mullo_epi16(R0_7, YCOEF_R) ; \
		YMUL_G = _mm_mullo_epi16(G0_7, YCOEF_G) ; \
		YMUL_B = _mm_mullo_epi16(B0_7, YCOEF_B) ; \
\
		Y0_7 = _mm_add_epi16(YMUL_R, YMUL_G) ; \
		Y0_7 = _mm_add_epi16(Y0_7, YMUL_B) ; \
		Y0_7 = _mm_add_epi16(Y0_7, C128) ; \
		Y0_7 = _mm_srli_epi16(Y0_7, 8) ; \
		Y0_7 = _mm_add_epi16(Y0_7, C16) ; \
\
		UMUL_R = _mm_mullo_epi16(R0_7, UCOEF_R) ; \
		UMUL_G = _mm_mullo_epi16(G0_7, UCOEF_G) ; \
		UMUL_B = _mm_mullo_epi16(B0_7, UCOEF_B) ; \
\
		U0_7 = _mm_adds_epi16(UMUL_R, UMUL_G) ; \
		U0_7 = _mm_adds_epi16(U0_7, UMUL_B) ; \
		U0_7 = _mm_adds_epi16(U0_7, C128) ; \
		U0_7 = _mm_srai_epi16(U0_7, 8) ; \
		U0_7 = _mm_adds_epi16(U0_7, C128) ; \
\
		VMUL_R = _mm_mullo_epi16(R0_7, VCOEF_R) ; \
		VMUL_G = _mm_mullo_epi16(G0_7, VCOEF_G) ; \
		VMUL_B = _mm_mullo_epi16(B0_7, VCOEF_B) ; \
\
		V0_7 = _mm_adds_epi16(VMUL_R, VMUL_G) ; \
		V0_7 = _mm_adds_epi16(V0_7, VMUL_B) ; \
		V0_7 = _mm_adds_epi16(V0_7, C128) ; \
		V0_7 = _mm_srai_epi16(V0_7, 8) ; \
		V0_7 = _mm_adds_epi16(V0_7, C128) ; \
};

#define YUV_16_16_16_TO_RGB_16_16_16(Y0_7, U0_7, V0_7, R0_7, G0_7, B0_7) {\
\
		__m128i RMUL_LO, RMUL_HI ; \
		__m128i GMUL_LO, GMUL_HI ; \
		__m128i BMUL_LO, BMUL_HI ; \
		__m128i RMUL_Y0, RMUL_Y1 ; \
		__m128i RMUL_U0, RMUL_U1 ; \
		__m128i RMUL_V0, RMUL_V1 ; \
		__m128i GMUL_Y0, GMUL_Y1 ; \
		__m128i GMUL_U0, GMUL_U1 ; \
		__m128i GMUL_V0, GMUL_V1 ; \
		__m128i BMUL_Y0, BMUL_Y1 ; \
		__m128i BMUL_U0, BMUL_U1 ; \
		__m128i BMUL_V0, BMUL_V1 ; \
		__m128i R0_3, R4_7 ; \
		__m128i G0_3, G4_7 ; \
		__m128i B0_3, B4_7 ; \
		__m128i lY0_7, lU0_7, lV0_7 ; \
\
		/* Calculation for Red Component */ \
\
		lY0_7 = _mm_subs_epu16(Y0_7, C16) ; \
		lU0_7 = _mm_sub_epi16(U0_7, C128) ; \
		lV0_7 = _mm_sub_epi16(V0_7, C128) ; \
		RMUL_LO = _mm_mullo_epi16(lY0_7, RCOEF_Y) ; \
		RMUL_HI = _mm_mulhi_epi16(lY0_7, RCOEF_Y) ; \
		RMUL_Y0 = _mm_unpacklo_epi16(RMUL_LO, RMUL_HI) ; \
		RMUL_Y1 = _mm_unpackhi_epi16(RMUL_LO, RMUL_HI) ; \
\
		RMUL_LO = _mm_mullo_epi16(lU0_7, RCOEF_U) ; \
		RMUL_HI = _mm_mulhi_epi16(lU0_7, RCOEF_U) ; \
		RMUL_U0 = _mm_unpacklo_epi16(RMUL_LO, RMUL_HI) ; \
		RMUL_U1 = _mm_unpackhi_epi16(RMUL_LO, RMUL_HI) ; \
\
		RMUL_LO = _mm_mullo_epi16(lV0_7, RCOEF_V) ; \
		RMUL_HI = _mm_mulhi_epi16(lV0_7, RCOEF_V) ; \
		RMUL_V0 = _mm_unpacklo_epi16(RMUL_LO, RMUL_HI) ; \
		RMUL_V1 = _mm_unpackhi_epi16(RMUL_LO, RMUL_HI) ; \
\
		R0_3 = _mm_add_epi32(RMUL_Y0, RMUL_U0) ; \
		R0_3 = _mm_add_epi32(R0_3, RMUL_V0) ; \
		R0_3 = _mm_add_epi32(R0_3, C128_32) ; \
		R0_3 = _mm_srai_epi32(R0_3, 8) ; \
\
		R4_7 = _mm_add_epi32(RMUL_Y1, RMUL_U1) ; \
		R4_7 = _mm_add_epi32(R4_7, RMUL_V1) ; \
		R4_7 = _mm_add_epi32(R4_7, C128_32) ; \
		R4_7 = _mm_srai_epi32(R4_7, 8) ; \
\
		/* Calculation for Green Component */ \
\
		GMUL_LO = _mm_mullo_epi16(lY0_7, GCOEF_Y) ; \
		GMUL_HI = _mm_mulhi_epi16(lY0_7, GCOEF_Y) ; \
		GMUL_Y0 = _mm_unpacklo_epi16(GMUL_LO, GMUL_HI) ; \
		GMUL_Y1 = _mm_unpackhi_epi16(GMUL_LO, GMUL_HI) ; \
\
		GMUL_LO = _mm_mullo_epi16(lU0_7, GCOEF_U) ; \
		GMUL_HI = _mm_mulhi_epi16(lU0_7, GCOEF_U) ; \
		GMUL_U0 = _mm_unpacklo_epi16(GMUL_LO, GMUL_HI) ; \
		GMUL_U1 = _mm_unpackhi_epi16(GMUL_LO, GMUL_HI) ; \
\
		GMUL_LO = _mm_mullo_epi16(lV0_7, GCOEF_V) ; \
		GMUL_HI = _mm_mulhi_epi16(lV0_7, GCOEF_V) ; \
		GMUL_V0 = _mm_unpacklo_epi16(GMUL_LO, GMUL_HI) ; \
		GMUL_V1 = _mm_unpackhi_epi16(GMUL_LO, GMUL_HI) ; \
\
		G0_3 = _mm_add_epi32(GMUL_Y0, GMUL_U0) ; \
		G0_3 = _mm_add_epi32(G0_3, GMUL_V0) ; \
		G0_3 = _mm_add_epi32(G0_3, C128_32) ; \
		G0_3 = _mm_srai_epi32(G0_3, 8) ; \
\
		G4_7 = _mm_add_epi32(GMUL_Y1, GMUL_U1) ; \
		G4_7 = _mm_add_epi32(G4_7, GMUL_V1) ; \
		G4_7 = _mm_add_epi32(G4_7, C128_32) ; \
		G4_7 = _mm_srai_epi32(G4_7, 8) ; \
\
		/* Calculation for Blue Component */ \
\
		BMUL_LO = _mm_mullo_epi16(lY0_7, BCOEF_Y) ; \
		BMUL_HI = _mm_mulhi_epi16(lY0_7, BCOEF_Y) ; \
		BMUL_Y0 = _mm_unpacklo_epi16(BMUL_LO, BMUL_HI) ; \
		BMUL_Y1 = _mm_unpackhi_epi16(BMUL_LO, BMUL_HI) ; \
\
		BMUL_LO = _mm_mullo_epi16(lU0_7, BCOEF_U) ; \
		BMUL_HI = _mm_mulhi_epi16(lU0_7, BCOEF_U) ; \
		BMUL_U0 = _mm_unpacklo_epi16(BMUL_LO, BMUL_HI) ; \
		BMUL_U1 = _mm_unpackhi_epi16(BMUL_LO, BMUL_HI) ; \
\
		BMUL_LO = _mm_mullo_epi16(lV0_7, BCOEF_V) ; \
		BMUL_HI = _mm_mulhi_epi16(lV0_7, BCOEF_V) ; \
		BMUL_V0 = _mm_unpacklo_epi16(BMUL_LO, BMUL_HI) ; \
		BMUL_V1 = _mm_unpackhi_epi16(BMUL_LO, BMUL_HI) ; \
\
		B0_3 = _mm_add_epi32(BMUL_Y0, BMUL_U0) ; \
		B0_3 = _mm_add_epi32(B0_3, BMUL_V0) ; \
		B0_3 = _mm_add_epi32(B0_3, C128_32) ; \
		B0_3 = _mm_srai_epi32(B0_3, 8) ; \
\
		B4_7 = _mm_add_epi32(BMUL_Y1, BMUL_U1) ; \
		B4_7 = _mm_add_epi32(B4_7, BMUL_V1) ; \
		B4_7 = _mm_add_epi32(B4_7, C128_32) ; \
		B4_7 = _mm_srai_epi32(B4_7, 8) ; \
\
		R0_7 = _mm_packs_epi32(R0_3, R4_7) ; \
		G0_7 = _mm_packs_epi32(G0_3, G4_7) ; \
		B0_7 = _mm_packs_epi32(B0_3, B4_7) ; \
\
} ;

void CImageConvert::RGB24_to_YUV420_SSE2(unsigned char *rgb, unsigned char *yuv, int width, int height) {

	int i, j ;
	int size ;

	size = width * height ;

	__m128i *pRGB, *pnRGB, *pYUV_Y, *pnYUV_Y, *pYUV_U, *pYUV_V ;
	__m128i RGB0, RGB1, RGB2, RGB3, RGB4, RGB5 ;
	__m128i nRGB0, nRGB1, nRGB2, nRGB3, nRGB4, nRGB5 ;
	__m128i R0_7, R8_15, R16_23, R24_31 ;
	__m128i G0_7, G8_15, G16_23, G24_31 ;
	__m128i B0_7, B8_15, B16_23, B24_31 ;
	__m128i nR0_7, nR8_15, nR16_23, nR24_31 ;
	__m128i nG0_7, nG8_15, nG16_23, nG24_31 ;
	__m128i nB0_7, nB8_15, nB16_23, nB24_31 ;
	__m128i Y0_7, Y8_15, Y16_23, Y24_31 ;
	__m128i U0_7, U8_15, U16_23, U24_31, U0_15 ;
	__m128i V0_7, V8_15, V16_23, V24_31, V0_15 ;
	__m128i nY0_7, nY8_15, nY16_23, nY24_31 ;
	__m128i nU0_7, nU8_15, nU16_23, nU24_31 ;
	__m128i nV0_7, nV8_15, nV16_23, nV24_31 ;
	__m128i MASK_0, MASK_1, MASK_2, MASK_3, MASK_4, MASK_5, MASK_6, MASK_7 ;
	__m128i MASK_8, MASK_9, MASK_A, MASK_B, MASK_C, MASK_D, MASK_E, MASK_F ;

/* Microsoft's RGB to YUV conversion
Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/

	pRGB = (__m128i *) rgb ;
	pnRGB = (__m128i *) (rgb + width * 3) ;
	pYUV_Y = (__m128i *) yuv ;
	pnYUV_Y = (__m128i *) (yuv + width) ;
	pYUV_U = (__m128i *) (yuv + size) ;
	pYUV_V = (__m128i *) (yuv + size + size / 4) ;

	SET_RGB_888_MASK ;

	for(j = 0; j < height; j += 2) {
		for(i = 0; i < width; i += 32) {
			RGB0 = *pRGB ;
			pRGB++ ;
			RGB1 = *pRGB ;
			pRGB++ ;
			RGB2 = *pRGB ;
			pRGB++ ;
			RGB3 = *pRGB ;
			pRGB++ ;
			RGB4 = *pRGB ;
			pRGB++ ;
			RGB5 = *pRGB ;
			pRGB++ ;

			nRGB0 = *pnRGB ;
			pnRGB++ ;
			nRGB1 = *pnRGB ;
			pnRGB++ ;
			nRGB2 = *pnRGB ;
			pnRGB++ ;
			nRGB3 = *pnRGB ;
			pnRGB++ ;
			nRGB4 = *pnRGB ;
			pnRGB++ ;
			nRGB5 = *pnRGB ;
			pnRGB++ ;

			RGB_888_TO_RGB_16_16_16(RGB0, RGB1, RGB2, R0_7, R8_15, G0_7, G8_15, B0_7, B8_15) ;
			RGB_888_TO_RGB_16_16_16(RGB3, RGB4, RGB5, R16_23, R24_31, G16_23, G24_31, B16_23, B24_31) ;
			RGB_888_TO_RGB_16_16_16(nRGB0, nRGB1, nRGB2, nR0_7, nR8_15, nG0_7, nG8_15, nB0_7, nB8_15) ;
			RGB_888_TO_RGB_16_16_16(nRGB3, nRGB4, nRGB5, nR16_23, nR24_31, nG16_23, nG24_31, nB16_23, nB24_31) ;

			RGB_16_16_16_TO_YUV_16_16_16(R0_7, G0_7, B0_7, Y0_7, U0_7, V0_7) ;
			RGB_16_16_16_TO_YUV_16_16_16(R8_15, G8_15, B8_15, Y8_15, U8_15, V8_15) ;
			RGB_16_16_16_TO_YUV_16_16_16(R16_23, G16_23, B16_23, Y16_23, U16_23, V16_23) ;
			RGB_16_16_16_TO_YUV_16_16_16(R24_31, G24_31, B24_31, Y24_31, U24_31, V24_31) ;
			RGB_16_16_16_TO_YUV_16_16_16(nR0_7, nG0_7, nB0_7, nY0_7, nU0_7, nV0_7) ;
			RGB_16_16_16_TO_YUV_16_16_16(nR8_15, nG8_15, nB8_15, nY8_15, nU8_15, nV8_15) ;
			RGB_16_16_16_TO_YUV_16_16_16(nR16_23, nG16_23, nB16_23, nY16_23, nU16_23, nV16_23) ;
			RGB_16_16_16_TO_YUV_16_16_16(nR24_31, nG24_31, nB24_31, nY24_31, nU24_31, nV24_31) ;

			*pYUV_Y = _mm_packus_epi16(Y0_7, Y8_15) ;
			pYUV_Y++ ;
			*pYUV_Y = _mm_packus_epi16(Y16_23, Y24_31) ;
			pYUV_Y++ ;
			*pnYUV_Y = _mm_packus_epi16(nY0_7, nY8_15) ;
			pnYUV_Y++ ;
			*pnYUV_Y = _mm_packus_epi16(nY16_23, nY24_31) ;
			pnYUV_Y++ ;

			U0_7 = _mm_adds_epi16(U0_7, nU0_7) ;
			U0_7 = _mm_madd_epi16(U0_7, C1) ;
			U0_7 = _mm_srai_epi32(U0_7, 2) ;
			U8_15 = _mm_adds_epi16(U8_15, nU8_15) ;
			U8_15 = _mm_madd_epi16(U8_15, C1) ;
			U8_15 = _mm_srai_epi32(U8_15, 2) ;
			U16_23 = _mm_adds_epi16(U16_23, nU16_23) ;
			U16_23 = _mm_madd_epi16(U16_23, C1) ;
			U16_23 = _mm_srai_epi32(U16_23, 2) ;
			U24_31 = _mm_adds_epi16(U24_31, nU24_31) ;
			U24_31 = _mm_madd_epi16(U24_31, C1) ;
			U24_31 = _mm_srai_epi32(U24_31, 2) ;

			U0_7 = _mm_packs_epi32(U0_7, U8_15) ;
			U8_15 = _mm_packs_epi32(U16_23, U24_31) ;

			U0_15 = _mm_packus_epi16(U0_7, U8_15) ;

			V0_7 = _mm_adds_epi16(V0_7, nV0_7) ;
			V0_7 = _mm_madd_epi16(V0_7, C1) ;
			V0_7 = _mm_srai_epi32(V0_7, 2) ;
			V8_15 = _mm_adds_epi16(V8_15, nV8_15) ;
			V8_15 = _mm_madd_epi16(V8_15, C1) ;
			V8_15 = _mm_srai_epi32(V8_15, 2) ;
			V16_23 = _mm_adds_epi16(V16_23, nV16_23) ;
			V16_23 = _mm_madd_epi16(V16_23, C1) ;
			V16_23 = _mm_srai_epi32(V16_23, 2) ;
			V24_31 = _mm_adds_epi16(V24_31, nV24_31) ;
			V24_31 = _mm_madd_epi16(V24_31, C1) ;
			V24_31 = _mm_srai_epi32(V24_31, 2) ;

			V0_7 = _mm_packs_epi32(V0_7, V8_15) ;
			V8_15 = _mm_packs_epi32(V16_23, V24_31) ;

			V0_15 = _mm_packus_epi16(V0_7, V8_15) ;
			*pYUV_U = U0_15 ;
			pYUV_U++ ;
			*pYUV_V = V0_15 ;
			pYUV_V++ ;
		}
		pRGB += (3 * width / 16) ;
		pnRGB += (3 * width / 16) ;
		pYUV_Y += (width / 16) ;
		pnYUV_Y += (width / 16) ;
	}
}
/*
void CImageConvert::YUV420_to_RGB24_SSE2(unsigned char *yuv, unsigned char *rgb, int width, int height) {

	int i, j ;
	int size, hwidth, hheight ;

	size = width * height ;
	hwidth = width / 2 ;
	hheight = height / 2 ;

	unsigned char *pu ;
	unsigned char *pv ;
	unsigned char *py ;
	unsigned char *pr ;
	unsigned char *pg ;
	unsigned char *pb ;
	__m128i COEFS_1, COEFS_2, CDE_1, CDE_2, RGB_32, RGB_16, RGB ;
	__m128i C298 ;

	pu = yuv + size ;
	pv = pu + hwidth * hheight ;
	py = yuv ;
	pb = rgb ;
	pg = rgb+1 ;
	pr = rgb+2 ;
	
	COEFS_1 = _mm_set_epi16(298, 1, 298, 1, 298, 1, 0, 0) ;
	COEFS_2 = _mm_set_epi16(0, 409, -100, -208, 516, 0, 0, 0) ;

	for(j = 0; j < height; j++) {
		int c, d, e ;

		for(i = 0; i < width; i++) {
			c = *py - 16 ;
			d = *pu - 128 ;
			e = *pv - 128 ;

			CDE_1 = _mm_set_epi16(c, 128, c, 128, c, 128, 0, 0) ;
			C298 = _mm_madd_epi16(COEFS_1, CDE_1) ;
			CDE_2 = _mm_set_epi16(d, e, d, e, d, e, 0, 0) ;
			RGB_32 = _mm_madd_epi16(COEFS_2, CDE_2) ;

			RGB_32 = _mm_add_epi32(RGB_32, C298) ;
			RGB_32 = _mm_srai_epi32(RGB_32, 8) ;
			RGB_16 = _mm_packs_epi32(RGB_32, RGB_32) ;
			RGB = _mm_packus_epi16(RGB_16, RGB_16) ;

//			*pr = clip[(298 * c + 409 * e + 128) >> 8] ;
//			*pg = clip[(298 * c - 100 * d - 208 * e + 128) >> 8] ;
//			*pb = clip[(298 * c + 516 * d + 128) >> 8] ;

			*pr = ((unsigned char *) (&RGB))[3] ;
			*pg = ((unsigned char *) (&RGB))[2] ;
			*pb = ((unsigned char *) (&RGB))[1] ;

			py++ ;
			pu += (i % 2) ;
			pv += (i % 2) ;
			pr += 3 ;
			pg += 3 ;
			pb += 3 ;
		}
		pu -= (1 - (j % 2)) * hwidth ;
		pv -= (1 - (j % 2)) * hwidth ;
	}
}
*/

void CImageConvert::YUV420_to_RGB24_SSE2(unsigned char *yuv, unsigned char *rgb, int width, int height) {
	int i, j ;
	int size ;

	size = width * height ;

	__m128i *pRGB, *pnRGB, *pYUV_Y, *pnYUV_Y, *pYUV_U, *pYUV_V ;
	__m128i RGB0, RGB1, RGB2, RGB3, RGB4, RGB5 ;
	__m128i nRGB0, nRGB1, nRGB2, nRGB3, nRGB4, nRGB5 ;
	__m128i R0_15, R16_31, R0_7, R8_15, R16_23, R24_31 ;
	__m128i G0_15, G16_31, G0_7, G8_15, G16_23, G24_31 ;
	__m128i B0_15, B16_31, B0_7, B8_15, B16_23, B24_31 ;
	__m128i nR0_15, nR16_31, nR0_7, nR8_15, nR16_23, nR24_31 ;
	__m128i nG0_15, nG16_31, nG0_7, nG8_15, nG16_23, nG24_31 ;
	__m128i nB0_15, nB16_31, nB0_7, nB8_15, nB16_23, nB24_31 ;
	__m128i Y0_15, Y16_31, Y0_7, Y8_15, Y16_23, Y24_31 ;
	__m128i U0_15, U0_7, U8_15, U16_23, U24_31, U_TMP ;
	__m128i V0_15, V0_7, V8_15, V16_23, V24_31, V_TMP ;
	__m128i nY0_15, nY16_31, nY0_7, nY8_15, nY16_23, nY24_31 ;


	pRGB = (__m128i *) rgb ;
	pnRGB = (__m128i *) (rgb + width * 3) ;
	pYUV_Y = (__m128i *) yuv ;
	pnYUV_Y = (__m128i *) (yuv + width) ;
	pYUV_U = (__m128i *) (yuv + size) ;
	pYUV_V = (__m128i *) (yuv + size + size / 4) ;

	for(j = 0; j < height; j += 2) {
		for(i = 0; i < width; i += 32) {
			Y0_15 = *pYUV_Y ;
			pYUV_Y++ ;
			Y16_31 = *pYUV_Y ;
			pYUV_Y++ ;
			nY0_15 = *pnYUV_Y ;
			pnYUV_Y++ ;
			nY16_31 = *pnYUV_Y ;
			pnYUV_Y++ ;
			U0_15 = *pYUV_U ;
			pYUV_U++ ;
			V0_15 = *pYUV_V ;
			pYUV_V++ ;

			Y0_7 = _mm_unpacklo_epi8(Y0_15, C0) ;
			Y8_15 = _mm_unpackhi_epi8(Y0_15, C0) ;
			Y16_23 = _mm_unpacklo_epi8(Y16_31, C0) ;
			Y24_31 = _mm_unpackhi_epi8(Y16_31, C0) ;

			nY0_7 = _mm_unpacklo_epi8(nY0_15, C0) ;
			nY8_15 = _mm_unpackhi_epi8(nY0_15, C0) ;
			nY16_23 = _mm_unpacklo_epi8(nY16_31, C0) ;
			nY24_31 = _mm_unpackhi_epi8(nY16_31, C0) ;

			U0_7 = _mm_unpacklo_epi8(U0_15, C0) ;
			U_TMP = _mm_slli_si128(U0_7, 1) ;
			U_TMP = _mm_or_si128(U0_7, U_TMP) ;
			U0_7 = _mm_unpacklo_epi8(U_TMP, C0) ;
			U8_15 = _mm_unpackhi_epi8(U_TMP, C0) ;
			U16_23 = _mm_unpackhi_epi8(U0_15, C0) ;
			U_TMP = _mm_slli_si128(U16_23, 1) ;
			U_TMP = _mm_or_si128(U16_23, U_TMP) ;
			U16_23 = _mm_unpacklo_epi8(U_TMP, C0) ;
			U24_31 = _mm_unpackhi_epi8(U_TMP, C0) ;

			V0_7 = _mm_unpacklo_epi8(V0_15, C0) ;
			V_TMP = _mm_slli_si128(V0_7, 1) ;
			V_TMP = _mm_or_si128(V0_7, V_TMP) ;
			V0_7 = _mm_unpacklo_epi8(V_TMP, C0) ;
			V8_15 = _mm_unpackhi_epi8(V_TMP, C0) ;
			V16_23 = _mm_unpackhi_epi8(V0_15, C0) ;
			V_TMP = _mm_slli_si128(V16_23, 1) ;
			V_TMP = _mm_or_si128(V16_23, V_TMP) ;
			V16_23 = _mm_unpacklo_epi8(V_TMP, C0) ;
			V24_31 = _mm_unpackhi_epi8(V_TMP, C0) ;

			YUV_16_16_16_TO_RGB_16_16_16(Y0_7, U0_7, V0_7, R0_7, G0_7, B0_7) ;
			YUV_16_16_16_TO_RGB_16_16_16(Y8_15, U8_15, V8_15, R8_15, G8_15, B8_15) ;
			YUV_16_16_16_TO_RGB_16_16_16(Y16_23, U16_23, V16_23, R16_23, G16_23, B16_23) ;
			YUV_16_16_16_TO_RGB_16_16_16(Y24_31, U24_31, V24_31, R24_31, G24_31, B24_31) ;

			R0_15 = _mm_packus_epi16(R0_7, R8_15) ;
			R16_31 = _mm_packus_epi16(R16_23, R24_31) ;
			G0_15 = _mm_packus_epi16(G0_7, G8_15) ;
			G16_31 = _mm_packus_epi16(G16_23, G24_31) ;
			B0_15 = _mm_packus_epi16(B0_7, B8_15) ;
			B16_31 = _mm_packus_epi16(B16_23, B24_31) ;

			RGB_8_8_8_TO_RGB_888(R0_15, G0_15, B0_15, RGB0, RGB1, RGB2) ;
			RGB_8_8_8_TO_RGB_888(R16_31, G16_31, B16_31, RGB3, RGB4, RGB5) ;

			YUV_16_16_16_TO_RGB_16_16_16(nY0_7, U0_7, V0_7, nR0_7, nG0_7, nB0_7) ;
			YUV_16_16_16_TO_RGB_16_16_16(nY8_15, U8_15, V8_15, nR8_15, nG8_15, nB8_15) ;
			YUV_16_16_16_TO_RGB_16_16_16(nY16_23, U16_23, V16_23, nR16_23, nG16_23, nB16_23) ;
			YUV_16_16_16_TO_RGB_16_16_16(nY24_31, U24_31, V24_31, nR24_31, nG24_31, nB24_31) ;

			nR0_15 = _mm_packus_epi16(nR0_7, nR8_15) ;
			nR16_31 = _mm_packus_epi16(nR16_23, nR24_31) ;
			nG0_15 = _mm_packus_epi16(nG0_7, nG8_15) ;
			nG16_31 = _mm_packus_epi16(nG16_23, nG24_31) ;
			nB0_15 = _mm_packus_epi16(nB0_7, nB8_15) ;
			nB16_31 = _mm_packus_epi16(nB16_23, nB24_31) ;

			RGB_8_8_8_TO_RGB_888(nR0_15, nG0_15, nB0_15, nRGB0, nRGB1, nRGB2) ;
			RGB_8_8_8_TO_RGB_888(nR16_31, nG16_31, nB16_31, nRGB3, nRGB4, nRGB5) ;

			*pRGB = RGB0 ;
			pRGB++ ;
			*pRGB = RGB1 ;
			pRGB++ ;
			*pRGB = RGB2 ;
			pRGB++ ;
			*pRGB = RGB3 ;
			pRGB++ ;
			*pRGB = RGB4 ;
			pRGB++ ;
			*pRGB = RGB5 ;
			pRGB++ ;
			*pnRGB = nRGB0 ;
			pnRGB++ ;
			*pnRGB = nRGB1 ;
			pnRGB++ ;
			*pnRGB = nRGB2 ;
			pnRGB++ ;
			*pnRGB = nRGB3 ;
			pnRGB++ ;
			*pnRGB = nRGB4 ;
			pnRGB++ ;
			*pnRGB = nRGB5 ;
			pnRGB++ ;
		}
		pRGB += (3 * width / 16) ;
		pnRGB += (3 * width / 16) ;
		pYUV_Y += (width / 16) ;
		pnYUV_Y += (width / 16) ;
	}
}

void CImageConvert::RGB24_to_YUV420(unsigned char *rgb, unsigned char *yuv, int width, int height) {

	if(SSE2) {
		unsigned char *in, *out ;
		int width32, height16 ;
		int extend ;

		extend = dimensionExtend(width, height, width32, height16) ;
		in = new unsigned char[SizeRGB24(width32, height16)+16] ;
		out = new unsigned char[SizeYUV420(width32, height16)+16] ;
		frameCopyRGB24(byte_align_16(in), rgb, width32, height16, width, height) ;

		RGB24_to_YUV420_SSE2(byte_align_16(in), byte_align_16(out), width32, height16) ;

		frameCopyYUV420(yuv, byte_align_16(out), width, height, width32, height16) ;
		delete [] in ;
		delete [] out ;
	} else {
		RGB24_to_YUV420_C(rgb, yuv, width, height) ;
	}
}

void CImageConvert::RGB24_to_YUV420_C(unsigned char *rgb, unsigned char *yuv, int width, int height) {

	int i, j ;
	int size, hwidth, hheight ;
	unsigned char *r ;
	unsigned char *g ;
	unsigned char *b ;
	unsigned char *u ;
	unsigned char *v ;

	size = width * height ;
	hwidth = width / 2 ;
	hheight = height / 2 ;

/* Microsoft's RGB to YUV conversion
Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/
	b = rgb ;
	g = rgb+1 ;
	r = rgb+2 ;

	for(i = 0; i < size; i++) {
		yuv[i] = (( -38 * (*r) -74 * (*g) + 112 * (*b) + 128) >> 8) + 128 ;
		r+=3 ;
		g+=3 ;
		b+=3 ;
	}

	u = yuv + size ;
	for(j = 0; j < hheight; j++) {
		unsigned char *p ;
		unsigned char *p1 ;

		p = yuv + 2 * j * width ;
		p1 = yuv + (2 * j + 1) * width ;
		for(i = 0; i < hwidth; i++) {
			*u = ((*p) + (*(p + 1)) + (*p1) + (*(p1 + 1))) >> 2 ;
			u++ ;
			p += 2 ;
			p1 += 2 ;
		}
	}

	b = rgb ;
	g = rgb+1 ;
	r = rgb+2 ;

	for(i = 0; i < size; i++) {
		yuv[i] = (( 112 * (*r) -94 * (*g) -18 * (*b) + 128) >> 8) + 128 ;
		r+=3 ;
		g+=3 ;
		b+=3 ;
	}

	v = yuv + size + hwidth * hheight ;
	for(j = 0; j < hheight; j++) {
		unsigned char *p ;
		unsigned char *p1 ;

		p = yuv + 2 * j * width ;
		p1 = yuv + (2 * j + 1) * width ;
		for(i = 0; i < hwidth; i++) {
			*v = ((*p) + (*(p + 1)) + (*p1) + (*(p1 + 1))) >> 2 ;
			v++ ;
			p += 2 ;
			p1 += 2 ;
		}
	}

	b = rgb ;
	g = rgb+1 ;
	r = rgb+2 ;
	for(i = 0; i < size; i++) {
		yuv[i] = (( 66 * (*r) + 129 * (*g) + 25 * (*b) + 128) >> 8) + 16 ;
		r+=3 ;
		g+=3 ;
		b+=3 ;
	}
}

void CImageConvert::YUV420_to_RGB24(unsigned char *yuv, unsigned char *rgb, int width, int height) {

	YUV420_to_RGB24_SSE2(yuv, rgb, width, height) ;

	return ;

	if(SSE2) {
		unsigned char *in, *out ;
		int width32, height16 ;
		int extend ;

		extend = dimensionExtend(width, height, width32, height16) ;
		out = new unsigned char[SizeRGB24(width32, height16)+16] ;
		in = new unsigned char[SizeYUV420(width32, height16)+16] ;
		frameCopyYUV420(byte_align_16(in), yuv, width32, height16, width, height) ;

		YUV420_to_RGB24_SSE2(byte_align_16(in), byte_align_16(out), width32, height16) ;

		frameCopyRGB24(rgb, byte_align_16(out), width, height, width32, height16) ;
		delete [] in ;
		delete [] out ;
	} else {
		YUV420_to_RGB24_C(yuv, rgb, width, height) ;
	}
}

void CImageConvert::YUV420_to_RGB24_C(unsigned char *yuv, unsigned char *rgb, int width, int height) {

	int i, j ;
	int size, hwidth, hheight ;
	unsigned char *u ;
	unsigned char *v ;

	size = width * height ;
	hwidth = width / 2 ;
	hheight = height / 2 ;

	u = new unsigned char [width * height] ;
	v = new unsigned char [width * height] ;

	for(j = 0; j < hheight; j++) {
		unsigned char *pdu, *pu ;
		unsigned char *pdu1 ;
		unsigned char *pdv, *pv ;
		unsigned char *pdv1 ;

		pdu = u + 2 * j * width ;
		pdu1 = u + (2 * j + 1) * width ;
		pdv = v + 2 * j * width ;
		pdv1 = v + (2 * j + 1) * width ;
		pu = yuv + size + j * hwidth ;
		pv = yuv + size + hwidth * hheight + j * hwidth ;
		for(i = 0; i < hwidth; i++) {
			*pdu = *pu ;
			*(pdu + 1) = *pu ;
			*pdu1 = *pu ;
			*(pdu1 + 1) = *pu ;
			*pdv = *pv ;
			*(pdv + 1) = *pv ;
			*pdv1 = *pv ;
			*(pdv1 + 1) = *pv ;
			pdu += 2 ;
			pdu1 += 2 ;
			pdv += 2 ;
			pdv1 += 2 ;
			pu++ ;
			pv++ ;
		}
	}

/*
C = Y - 16
D = U - 128
E = V - 128
Using the previous coefficients and noting that clip() denotes clipping a value to the range of 0 to 255, the following formulas provide the conversion from YUV to RGB:

R = clip(( 298 * C           + 409 * E + 128) >> 8)
G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
B = clip(( 298 * C + 516 * D           + 128) >> 8)
*/
	for(j = 0; j < height; j++) {
		unsigned char *pu ;
		unsigned char *pv ;
		unsigned char *py ;
		unsigned char *pr ;
		unsigned char *pg ;
		unsigned char *pb ;
		int c, d, e ;

		pu = u + j * width ;
		pv = v + j * width ;
		py = yuv + j * width ;
		pb = rgb + j * width * 3 ;
		pg = rgb + j * width * 3 + 1 ;
		pr = rgb + j * width * 3 + 2 ;
		for(i = 0; i < width; i++) {
			c = *py - 16 ;
			d = *pu - 128 ;
			e = *pv - 128 ;

			*pr = clip[(298 * c + 409 * e + 128) >> 8] ;
			*pg = clip[(298 * c - 100 * d - 208 * e + 128) >> 8] ;
			*pb = clip[(298 * c + 516 * d + 128) >> 8] ;
			
			py++ ;
			pu++ ;
			pv++ ;
			pr += 3 ;
			pg += 3 ;
			pb += 3 ;
		}
	}

	delete [] u ;
	delete [] v ;
}

void CImageConvert::RGB24_to_VYUY(unsigned char *rgb, unsigned char *yuyv, int width, int height, int y_offset, int u_offset, int v_offset) {
	
	if(SSE2) {
		unsigned char *in, *out ;
		int width32, height16 ;
		int extend ;

		extend = dimensionExtend(width, height, width32, height16) ;
		in = new unsigned char[SizeRGB24(width32, height16)+16] ;
		out = new unsigned char[SizeYUYV(width32, height16)+16] ;
		frameCopyRGB24(byte_align_16(in), rgb, width32, height16, width, height) ;

		RGB24_to_VYUY_SSE2(byte_align_16(in), byte_align_16(out), width32, height16, y_offset, u_offset, v_offset) ;

		frameCopyVYUY(yuyv, byte_align_16(out), width, height, width32, height16) ;
		delete [] in ;
		delete [] out ;
	} else {
		RGB24_to_VYUY_C(rgb, yuyv, width, height, y_offset, u_offset, v_offset) ;
	}
}

void CImageConvert::RGB24_to_VYUY_C(unsigned char *rgb, unsigned char *vyuy, int width, int height, int y_offset, int u_offset, int v_offset) {

	int i, j ;
	int size, hwidth, hheight ;
	unsigned char *r ;
	unsigned char *g ;
	unsigned char *b ;
	unsigned char *u, *u_buf ;
	unsigned char *v, *v_buf ;
	unsigned char *y ;

	size = width * height ;
	hwidth = width / 2 ;
	hheight = height / 2 ;

/* Microsoft's RGB to YUV conversion
Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/
	b = rgb ;
	g = rgb+1 ;
	r = rgb+2 ;

	u = u_buf = new unsigned char [width * height] ;
	v = v_buf = new unsigned char [width * height] ;
	y = vyuy + y_offset ;

	for(i = 0; i < size; i++) {
		*y = (( 66 * (*r) + 129 * (*g) + 25 * (*b) + 128) >> 8) + 16 ;
//		*y = (*y - 16) * 255 / 220 ;
		*u = (( -38 * (*r) -74 * (*g) + 112 * (*b) + 128) >> 8) + 128 ;
//		*u = (*u - 128) * 128 / 112 + 128 ;
		*v = (( 112 * (*r) -94 * (*g) -18 * (*b) + 128) >> 8) + 128 ;
//		*v = (*v - 128) * 128 / 112 + 128 ;
		r+=3 ;
		g+=3 ;
		b+=3 ;
		y+=2 ;
		u++ ;
		v++ ;
	}

	for(j = 0; j < height; j++) {
		unsigned char *pu ;
		unsigned char *pv ;

		pu = vyuy + j * width * 2 + u_offset ;
		pv = vyuy + j * width * 2 + v_offset ;
		u = u_buf + j * width ;
		v = v_buf + j * width ;
		for(i = 0; i < hwidth; i++) {
			*pu = ((*u) + (*(u + 1))) >> 1 ;
			*pv = ((*v) + (*(v + 1))) >> 1 ;
			u+=2 ;
			v+=2 ;
			pu += 4 ;
			pv += 4 ;
		}
	}

	delete [] u_buf ;
	delete [] v_buf ;
}

void CImageConvert::RGB24_to_VYUY_SSE2(unsigned char *rgb, unsigned char *vyuy, int width, int height, int y_offset, int u_offset, int v_offset) {

	int i ;
	int size ;

	size = width * height ;

	__m128i *pRGB, *pVYUY ;
	__m128i RGB0, RGB1, RGB2 ;
	__m128i R0_7, R8_15 ;
	__m128i G0_7, G8_15 ;
	__m128i B0_7, B8_15 ;
	__m128i Y0_7, Y8_15 ;
	__m128i U0_7, U8_15 ;
	__m128i V0_7, V8_15 ;
	__m128i SHIFTED_Y, SHIFTED_U, SHIFTED_V ;
	__m128i VYUY0, VYUY1 ;
	__m128i MASK_0, MASK_1, MASK_2, MASK_3, MASK_4, MASK_5, MASK_6, MASK_7 ;
	__m128i MASK_8, MASK_9, MASK_A, MASK_B, MASK_C, MASK_D, MASK_E, MASK_F ;

/* Microsoft's RGB to YUV conversion
Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/

	pRGB = (__m128i *) rgb ;
	pVYUY = (__m128i *) vyuy ;

	y_offset *= 8 ;
	u_offset *= 8 ;
	v_offset *= 8 ;

	SET_RGB_888_MASK ;

	for(i = 0; i < size; i+=16) {
		RGB0 = *pRGB ;
		pRGB++ ;
		RGB1 = *pRGB ;
		pRGB++ ;
		RGB2 = *pRGB ;
		pRGB++ ;

		RGB_888_TO_RGB_16_16_16(RGB0, RGB1, RGB2, R0_7, R8_15, G0_7, G8_15, B0_7, B8_15) ;

		RGB_16_16_16_TO_YUV_16_16_16(R0_7, G0_7, B0_7, Y0_7, U0_7, V0_7) ;
		RGB_16_16_16_TO_YUV_16_16_16(R8_15, G8_15, B8_15, Y8_15, U8_15, V8_15) ;

		U0_7 = _mm_madd_epi16(U0_7, C1) ;
		U0_7 = _mm_srai_epi32(U0_7, 1) ;
		U8_15 = _mm_madd_epi16(U8_15, C1) ;
		U8_15 = _mm_srai_epi32(U8_15, 1) ;
		V0_7 = _mm_madd_epi16(V0_7, C1) ;
		V0_7 = _mm_srai_epi32(V0_7, 1) ;
		V8_15 = _mm_madd_epi16(V8_15, C1) ;
		V8_15 = _mm_srai_epi32(V8_15, 1) ;

		SHIFTED_Y = _mm_slli_epi32(Y0_7, y_offset) ;
		SHIFTED_U = _mm_slli_epi32(U0_7, u_offset) ;
		SHIFTED_V = _mm_slli_epi32(V0_7, v_offset) ;

		VYUY0 = _mm_or_si128(SHIFTED_Y, SHIFTED_U) ;
		VYUY0 = _mm_or_si128(VYUY0, SHIFTED_V) ;

		SHIFTED_Y = _mm_slli_epi32(Y8_15, y_offset) ;
		SHIFTED_U = _mm_slli_epi32(U8_15, u_offset) ;
		SHIFTED_V = _mm_slli_epi32(V8_15, v_offset) ;

		VYUY1 = _mm_or_si128(SHIFTED_Y, SHIFTED_U) ;
		VYUY1 = _mm_or_si128(VYUY1, SHIFTED_V) ;

		*pVYUY = VYUY0 ;
		pVYUY++ ;
		*pVYUY = VYUY1 ;
		pVYUY++ ;
	}

}

void CImageConvert::VYUY_to_RGB24(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset, int u_offset, int v_offset) {

	if(SSE2) {
		unsigned char *in, *out ;
		int width32, height16 ;
		int extend ;

		extend = dimensionExtend(width, height, width32, height16) ;
		out = new unsigned char[SizeRGB24(width32, height16)+16] ;
		in = new unsigned char[SizeYUYV(width32, height16)+16] ;
		frameCopyVYUY(byte_align_16(in), vyuy, width32, height16, width, height) ;

		VYUY_to_RGB24_SSE2(byte_align_16(in), byte_align_16(out), width32, height16, y_offset, u_offset, v_offset) ;

		frameCopyRGB24(rgb, byte_align_16(out), width, height, width32, height16) ;
		delete [] in ;
		delete [] out ;
	} else {
		VYUY_to_RGB24_C(vyuy, rgb, width, height, y_offset, u_offset, v_offset) ;
	}
}

void CImageConvert::VYUY_to_RGB24_C(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset, int u_offset, int v_offset) {

	int i, j ;
	int size, hwidth, hheight ;
	unsigned char *u ;
	unsigned char *v ;

	size = width * height ;
	hwidth = width / 2 ;
	hheight = height / 2 ;

	u = new unsigned char [width * height] ;
	v = new unsigned char [width * height] ;

	for(j = 0; j < height; j++) {
		unsigned char *pdu, *pu ;
		unsigned char *pdv, *pv ;

		pdu = u + j * width ;
		pdv = v + j * width ;
		pu = vyuy + j * width * 2 + u_offset ;
		pv = vyuy + j * width * 2 + v_offset ;
		for(i = 0; i < hwidth; i++) {
			*pdu = *pu ;
			*(pdu + 1) = *pu ;
			*pdv = *pv ;
			*(pdv + 1) = *pv ;
			pdu += 2 ;
			pu+=4 ;
			pdv += 2 ;
			pv+=4 ;
		}
	}

/*
C = Y - 16
D = U - 128
E = V - 128
Using the previous coefficients and noting that clip() denotes clipping a value to the range of 0 to 255, the following formulas provide the conversion from YUV to RGB:

R = clip(( 298 * C           + 409 * E + 128) >> 8)
G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
B = clip(( 298 * C + 516 * D           + 128) >> 8)
*/
	for(j = 0; j < height; j++) {
		unsigned char *pu ;
		unsigned char *pv ;
		unsigned char *py ;
		unsigned char *pr ;
		unsigned char *pg ;
		unsigned char *pb ;
		int c, d, e ;

		pu = u + j * width ;
		pv = v + j * width ;
		py = vyuy + j * width * 2 + y_offset ;
		pb = rgb + j * width * 3 ;
		pg = rgb + j * width * 3 + 1 ;
		pr = rgb + j * width * 3 + 2 ;
		for(i = 0; i < width; i++) {
			c = *py - 16 ;
			d = *pu - 128 ;
			e = *pv - 128 ;

			*pr = clip[(298 * c + 409 * e + 128) / 256] ;
			*pg = clip[(298 * c - 100 * d - 208 * e + 128) / 256] ;
			*pb = clip[(298 * c + 516 * d + 128) / 256] ;
			
			py+=2 ;
			pu++ ;
			pv++ ;
			pr += 3 ;
			pg += 3 ;
			pb += 3 ;
		}
	}

	delete [] u ;
	delete [] v ;
}

void CImageConvert::VYUY_to_RGB24_SSE2(unsigned char *vyuy, unsigned char *rgb, int width, int height, int y_offset, int u_offset, int v_offset) {

	int i ;
	int size ;

	__m128i *pRGB, *pVYUY ;
	__m128i RGB0, RGB1, RGB2 ;
	__m128i R0_7, R8_15 ;
	__m128i G0_7, G8_15 ;
	__m128i B0_7, B8_15 ;
	__m128i R0_15, G0_15, B0_15 ;
	__m128i Y0_7, Y8_15 ;
	__m128i U0_7, U8_15, U_TMP ;
	__m128i V0_7, V8_15, V_TMP ;
	__m128i YMASK, UVMASK ;
	__m128i VYUY0, VYUY1 ;

	size = width * height ;

	pRGB = (__m128i *) rgb ;
	pVYUY = (__m128i *) vyuy ;
	YMASK = _mm_set_epi32(0x00FF00FF, 0x00FF00FF, 0x00FF00FF, 0x00FF00FF) ;
	UVMASK = _mm_set_epi32(0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF) ;
/*
C = Y - 16
D = U - 128
E = V - 128
Using the previous coefficients and noting that clip() denotes clipping a value to the range of 0 to 255, the following formulas provide the conversion from YUV to RGB:

R = clip(( 298 * C           + 409 * E + 128) >> 8)
G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
B = clip(( 298 * C + 516 * D           + 128) >> 8)
*/
	y_offset *= 8 ;
	u_offset *= 8 ;
	v_offset *= 8 ;

	for(i = 0; i < size; i += 16) {
		VYUY0 = *pVYUY ;
		pVYUY++ ;
		VYUY1 = *pVYUY ;
		pVYUY++ ;

//		VYUY0 = _mm_setr_pi32(0x30201000, 0x70605040) ;
//		VYUY1 = _mm_setr_pi32(0xB0A09080, 0xF0E0D0C0) ;

		// VYUY0 = Y3U1Y2V1Y1U0Y0V0
		// VYUY1 = Y7U3Y6V3Y5U2Y4V2
		Y0_7 = _mm_srli_epi32(VYUY0, y_offset) ;
		Y0_7 = _mm_and_si128(Y0_7, YMASK) ;
		Y8_15 = _mm_srli_epi32(VYUY1, y_offset) ;
		Y8_15 = _mm_and_si128(Y8_15, YMASK) ;

		U0_7 = _mm_srli_epi32(VYUY0, u_offset) ;
		U0_7 = _mm_and_si128(U0_7, UVMASK) ;
		U_TMP = _mm_slli_epi32(U0_7, 16) ;	// Upsamping 000000U1000000U0 -> 00U100U100U000U0
		U0_7 = _mm_or_si128(U_TMP, U0_7) ;
		U8_15 = _mm_srli_epi32(VYUY1, u_offset) ;
		U8_15 = _mm_and_si128(U8_15, UVMASK) ;
		U_TMP = _mm_slli_epi32(U8_15, 16) ;	// Upsamping 000000U3000000U2 -> 00U300U300U200U2
		U8_15 = _mm_or_si128(U_TMP, U8_15) ;

		V0_7 = _mm_srli_epi32(VYUY0, v_offset) ;
		V0_7 = _mm_and_si128(V0_7, UVMASK) ;
		V_TMP = _mm_slli_epi32(V0_7, 16) ;	// Upsamping 000000V1000000V0 -> 00V100V100V000V0
		V0_7 = _mm_or_si128(V_TMP, V0_7) ;
		V8_15 = _mm_srli_epi32(VYUY1, v_offset) ;
		V8_15 = _mm_and_si128(V8_15, UVMASK) ;
		V_TMP = _mm_slli_epi32(V8_15, 16) ;	// Upsamping 000000V3000000V2 -> 00V300V300V200V2
		V8_15 = _mm_or_si128(V_TMP, V8_15) ;

		YUV_16_16_16_TO_RGB_16_16_16(Y0_7, U0_7, V0_7, R0_7, G0_7, B0_7) ;
		YUV_16_16_16_TO_RGB_16_16_16(Y8_15, U8_15, V8_15, R8_15, G8_15, B8_15) ;

		R0_15 = _mm_packus_epi16(R0_7, R8_15) ;
		G0_15 = _mm_packus_epi16(G0_7, G8_15) ;
		B0_15 = _mm_packus_epi16(B0_7, B8_15) ;

		RGB_8_8_8_TO_RGB_888(R0_15, G0_15, B0_15, RGB0, RGB1, RGB2) ;

		*pRGB = RGB0 ;
		pRGB++ ;
		*pRGB = RGB1 ;
		pRGB++ ;
		*pRGB = RGB2 ;
		pRGB++ ;
	}
}

void CImageConvert::RGB24_to_YUYV(unsigned char *rgb, unsigned char *yuyv, int width, int height) {
	RGB24_to_VYUY(rgb, yuyv, width, height, 0, 1, 3) ;
}

void CImageConvert::YUYV_to_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height) {
	VYUY_to_RGB24(yuyv, rgb, width, height, 0, 1, 3) ;
}

void CImageConvert::RGB24_to_RGBpDouble(unsigned char *rgb, double *rgbdouble, int width, int height) {

	int i, j ;
	unsigned char *prgb ;
	double *prf, *pgf, *pbf ;

	pbf = rgbdouble ;
	pgf = rgbdouble + width * height ;
	prf = pgf + width * height ;
	prgb = rgb ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			*pbf = (double) (*prgb) ;
			*pgf = (double) (*(prgb + 1)) ;
			*prf = (double) (*(prgb + 2)) ;
			pbf++ ;
			pgf++ ;
			prf++ ;
			prgb += 3 ;
		}
	}
}

void CImageConvert::Char8_to_Double(unsigned char *c, double *f, int width, int height) {

	int size = width * height ;

	for(int i = 0; i < size; i++) {
		*f = (double) (*c) ;
		f++ ;
		c++ ;
	}
}

void CImageConvert::Double_to_Char8(double *f, unsigned char *c, int width, int height, BOOL normalize) {

	int i ;
	int size = width * height ;
	double scale = 1.0, offset = 0.0, v ;

	if(normalize) {
		double min, max ;
		min = 1000000.0 ;
		max = -1000000.0 ;
		for(i = 0; i < size; i++) {
			if(f[i] < min) min = f[i] ;
			if(f[i] > max) max = f[i] ;
		}
		if(max - min < 0.001) {
			scale = 1.0 ;
			offset = 0.0 ;
		}
		else {
			scale = 255.0 / (max - min) ;
			offset = min ;
		}
	}

	for(i = 0; i < size; i++) {
		v = ((*f) - offset) * scale ;
		*c = clip[(int) v] ;
		f++ ;
		c++ ;
	}
}

void CImageConvert::Char8_to_RGB24(unsigned char *c, unsigned char *rgb, int width, int height) {

	int size = width * height ;
	unsigned char *pc, *prgb ;

	pc = c ;
	prgb = rgb ;
	for(int i = 0; i < size; i++) {
		*prgb = *(prgb + 1) = *(prgb + 2) = *c ;
		c++ ;
		prgb += 3 ;
	}
}

void CImageConvert::RGB24_to_Char8(unsigned char *rgb, unsigned char *c, int width, int height) {

	int size = width * height ;
	unsigned char *pc, *prgb ;

	pc = c ;
	prgb = rgb ;
	for(int i = 0; i < size; i++) {
		*pc = (( 66 * (*(prgb+2)) + 129 * (*(prgb + 1)) + 25 * (*(prgb)) + 128) >> 8) + 16 ;
		pc++ ;
		prgb += 3 ;
	}
}

void CImageConvert::RGB24_to_Double(unsigned char *rgb, double *f, int width, int height) {

	int size = width * height ;
	unsigned char *prgb ;
	double *pf ;

	pf = f ;
	prgb = rgb ;
	for(int i = 0; i < size; i++) {
		*pf = (double) ((( 66 * (*(prgb+2)) + 129 * (*(prgb + 1)) + 25 * (*(prgb)) + 128) >> 8) + 16) ;
		pf++ ;
		prgb += 3 ;
	}
}

void CImageConvert::Double_to_RGB24(double *f, unsigned char *rgb, int width, int height, BOOL normalize) {

	int i ;
	int size = width * height ;
	double scale = 1.0, offset = 0.0, v ;

	if(normalize) {
		double min, max ;
		min = 1000000.0 ;
		max = -1000000.0 ;
		for(i = 0; i < size; i++) {
			if(f[i] < min) min = f[i] ;
			if(f[i] > max) max = f[i] ;
		}
		if(max - min < 0.001) {
			scale = 1.0 ;
			offset = 0.0 ;
		}
		else {
			scale = 255.0 / (max - min) ;
			offset = min ;
		}
	}

	for(i = 0; i < size; i++) {
		v = ((*f) - offset) * scale ;
		*rgb = *(rgb + 1) = *(rgb + 2) = clip[(int) v] ;
		f++ ;
		rgb+=3 ;
	}
}

void CImageConvert::RGB24_to_RGB24p(unsigned char *rgb, unsigned char *rgbp, int width, int height) {

	int i, j ;
	unsigned char *rgbcopy, *pr, *pg, *pb ;
	unsigned char *r, *g, *b ;

	if(rgbp) {
		rgbcopy = rgbp ;
	} else {
		rgbcopy = new unsigned char [width * height * 3] ;
	}

	b = rgb ;
	g = rgb + 1 ;
	r = rgb + 2 ;
	pb = rgbcopy ;
	pg = rgbcopy + width * height ;
	pr = pg + width * height ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			*pb = *b ;
			*pg = *g ;
			*pr = *r ;
			b += 3 ;
			g += 3 ;
			r += 3 ;
			pb++ ;
			pg++ ;
			pr++ ;
		}
	}

	if(!rgbp) {
		memcpy(rgb, rgbcopy, width * height * 3) ;
		delete [] rgbcopy ;
	}
}

void CImageConvert::RGB24p_to_RGB24(unsigned char *rgbp, unsigned char *rgb, int width, int height) {

	int i, j ;
	unsigned char *rgbcopy, *pr, *pg, *pb ;
	unsigned char *r, *g, *b ;

	if(rgb) {
		rgbcopy = rgb ;
	} else {
		rgbcopy = new unsigned char [width * height * 3] ;
	}

	b = rgbcopy ;
	g = rgbcopy + 1 ;
	r = rgbcopy + 2 ;
	pb = rgbp ;
	pg = rgbp + width * height ;
	pr = pg + width * height ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			*b = *pb ;
			*g = *pg ;
			*r = *pr ;
			b += 3 ;
			g += 3 ;
			r += 3 ;
			pb++ ;
			pg++ ;
			pr++ ;
		}
	}

	if(!rgb) {
		memcpy(rgbp, rgbcopy, width * height * 3) ;
		delete [] rgbcopy ;
	}
}

void CImageConvert::RGB24_to_RGBA(unsigned char *rgb, unsigned char *rgba, int width, int height) {

	int i, j ;
	int width3 = width * 3 ;
	int width4 = width * 4 ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			rgba[i * 4 + j * width4 + 0] = rgb[i * 3 + j * width3 + 0] ;
			rgba[i * 4 + j * width4 + 1] = rgb[i * 3 + j * width3 + 1] ;
			rgba[i * 4 + j * width4 + 2] = rgb[i * 3 + j * width3 + 2] ;
			rgba[i * 4 + j * width4 + 3] = 0 ;
		}
	}
}

void CImageConvert::RGBA_to_RGB24(unsigned char *rgba, unsigned char *rgb, int width, int height) {

	int i, j ;
	int width3 = width * 3 ;
	int width4 = width * 4 ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			rgb[i * 3 + j * width3 + 0] = rgba[i * 4 + j * width4 + 0] ;
			rgb[i * 3 + j * width3 + 1] = rgba[i * 4 + j * width4 + 1] ;
			rgb[i * 3 + j * width3 + 2]  = rgba[i * 4 + j * width4 + 2];
		}
	}
}
