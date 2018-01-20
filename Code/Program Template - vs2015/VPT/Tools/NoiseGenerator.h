#pragma once


class CNoiseGenerator
{
private:
	double *invGaussianTab ;
	int G_MAX_ENTRY ;
private:
	void Init(int precision) ;
public:
	CNoiseGenerator(void) ;
	CNoiseGenerator(int precision) ;
	double gaussianNoise(double mean, double sd) ;
	int gaussianNoise(int mean, int sd) ;
	double uniformNoise(double min, double max) ;
	int uniformNoise(int min, int max) ;
	~CNoiseGenerator(void) ;
};
