#include "StdAfx.h"
#include "math.h"
#include "stdlib.h"
#include "NoiseGenerator.h"

CNoiseGenerator::CNoiseGenerator(void) {

	invGaussianTab = NULL ;

	Init(2048) ;
}

CNoiseGenerator::CNoiseGenerator(int precision) {

	invGaussianTab = NULL ;

	Init(precision) ;
}

void CNoiseGenerator::Init(int precision)
{
	int i, j ;
	double di ;
	double pi ;
	double doubleMax ;
	double max ;
	double *gaussianTab ;

	G_MAX_ENTRY = precision ;

	gaussianTab = new double[G_MAX_ENTRY] ;
	if(invGaussianTab) {
		delete [] invGaussianTab ;
	}
	invGaussianTab = new double[G_MAX_ENTRY] ;
	max = (double) G_MAX_ENTRY ;
	doubleMax = 2.0 * max ;

	gaussianTab[0] = 0.0 ;
	for(i = 0; i < G_MAX_ENTRY; i++) {
		di = 8.0/doubleMax + 8.0 * ((double) i) / ((double) max) ;
		pi = 1.0 / sqrt(2.0 * 3.1415926536) * exp(- (di - 4.0) * (di - 4.0) / 2.0) ;
		if(i == 0) gaussianTab[i] = 8.0 * pi / max ;
		else gaussianTab[i] = gaussianTab[i - 1] + pi * 8.0/max ;
	}

	for(i = 0; i < G_MAX_ENTRY; i++) {
		di = ((double) i) / max + 1.0 / doubleMax ;
		for(j = 0; j < G_MAX_ENTRY; j++) {
			if(gaussianTab[j] > di) {
				invGaussianTab[i] = 8.0 * ((double) j) / max - 4.0 + 8.0 / doubleMax ;
				break ;
			}
		}
		if(j == G_MAX_ENTRY) {
			invGaussianTab[i] = 8.0 * (max - 1.0) / max - 4.0 + 8.0 / doubleMax ;
		}
	}

	delete [] gaussianTab ;
}

double CNoiseGenerator::gaussianNoise(double mean, double sd) {

	int r ;
	double gVal ;

	r = rand() ;
	gVal = invGaussianTab[r * G_MAX_ENTRY / RAND_MAX] ;

	return sd * gVal + mean ;
}

int CNoiseGenerator::gaussianNoise(int mean, int sd) {

	double gVal ;
	double absVal ;
	
	gVal = gaussianNoise((double) mean, (double) sd) ;

	if(gVal < 0.0) {
		absVal = -gVal ;
		return (int) -(absVal + 0.5) ;
	}

	return (int) (gVal + 0.5) ;
}

double CNoiseGenerator::uniformNoise(double min, double max) {

	int r ;
	double uVal ;

	r = rand() ;
	uVal = (max - min) * ((double) r) / ((double) RAND_MAX) + min ;

	return uVal ;
}

int CNoiseGenerator::uniformNoise(int min, int max) {

	return (max - min + 1) * rand() / RAND_MAX + min ;
}

CNoiseGenerator::~CNoiseGenerator(void)
{
	if(invGaussianTab)
		delete [] invGaussianTab ;
	invGaussianTab = NULL ;
}
