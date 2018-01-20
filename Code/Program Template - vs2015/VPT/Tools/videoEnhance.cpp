#include "stdafx.h"
#include "videoEnhance.h"
#include "math.h"
#include "tables.h"

void buildLogTable(void) {

	int i ;

	for(i = 0; i < 256; i++) {
//		logTable[i] = (int) (45.986 * log((double) (i + 1)) * 256.0) ;
	}
}

int histogramEqualize(int *histOrg, int *weight, int n, int *map) {

	int i ;
	long cf[4096] ;
	int hist[4096] ;
	long totalFreq ;
	int sum ;

	memset(cf, 0, sizeof(int) * 4096) ;
	memcpy(hist, histOrg, sizeof(int) * n) ;

	if(weight) {
		cf[0] = hist[0] * weight[0] ;
		for(i = 1; i < n; i++) {
			cf[i] = cf[i-1] + hist[i] * weight[i] ;
		}
	} else {
		cf[0] = hist[0]  ;
		for(i = 1; i < n; i++) {
			cf[i] = cf[i-1] + hist[i]  ;
		}
	}

	totalFreq = cf[n - 1] ;

	if(totalFreq) {

		for(i = 0; i < n; i++) {
			{
				double dtf ;
				double dcf ;

				dcf = (double) cf[i];
				dtf = (double) totalFreq ;
				map[i] = (int) (dcf * ((double) (n - 1)) / dtf) ;
			}
		}
		memset(hist, 0, sizeof(int) * 4096) ;
		for(i = 0; i < n; i++) {
			hist[map[i]] += histOrg[i] ;
		}
		sum = 0 ;
		for(i = 0; i < n; i++) {
			if(hist[i]) sum++ ;
		}
		/*
		for(i = 0; i < n; i++) {
			map[i] = (4 * i + map[i])  / 5 ;
		}
		*/
	} else {
		for(i = 0; i < n; i++) {
			map[i] = i ;
		}
	}
	/*
	{
		FILE *fp ;

		fp = fopen("D:\\TEMP\\verify.txt", "wb") ;
		for(i = 0; i < n; i++) {
			fprintf(fp, "%d\t%d\t%d\t%d\r\n", histOrg[i], hist[i], i, map[i]) ;
		}
		fclose(fp) ;
	}
	*/
	return sum ;
}

void histogramSpecification(int *histOrg, int *histSpec, int n, int *map) {

	int mapOrg[4096] ;
	int mapSpec[4096] ;
	int i, j ;
	int s ;

	histogramEqualize(histOrg, NULL, n, mapOrg) ;
	histogramEqualize(histSpec, NULL, n, mapSpec) ;

	for(i = 0; i < n; i++) {
		s = mapOrg[i] ;
		for(j = 0; j < n; j++) {
			if(mapSpec[j] >= s) break ;
		}
		map[i] = j ;
	}
}

void averagingFilter(unsigned char *in, unsigned char *out, int halfFilterStep, int width, int height) {

	int i, j, k ;
	int sum ;
	unsigned char *cpy ;

	cpy = (unsigned char *) malloc(width * height) ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			sum = 0 ;
			for(k = -halfFilterStep; k <= halfFilterStep; k++) {
				sum += in[((i + k) < 0 ? 0 : ((i + k) >= width ? width - 1 : (i + k))) + j * width] ;
			}
			sum /= (halfFilterStep * 2 + 1) ;
			cpy[i + j * width] = sum ;
		}
	}

	for(i = 0; i < width; i++) {
		for(j = 0; j < height; j++) {
			sum = 0 ;
			for(k = -halfFilterStep; k <= halfFilterStep; k++) {
				sum += cpy[i + ((j + k) < 0 ? 0 : ((j + k) >= height ? height - 1 : (j + k))) * width] ;
			}
			sum /= (halfFilterStep * 2 + 1) ;
			out[i + j * width] = sum ;
		}
	}

	free(cpy) ;
}

void exponentialFilter(unsigned char *in, unsigned char *out, int filterSize, int width, int height) {

	int i, j ;
	unsigned char *cpy ;
	unsigned char *linecpy[2] ;
	int meany ;

	cpy = (unsigned char *) malloc(width * height) ;
	linecpy[0] = (unsigned char *) malloc(width > height ? width : height) ;
	linecpy[1] = (unsigned char *) malloc(width > height ? width : height) ;
	
	for(j = 0; j < height; j++) {
//		linecpy[0][0] = in[j * width] ;
		meany = 0 ;
		for(i = 0; i < filterSize; i++) {
			meany += in[i + j * width] ;
		}
		linecpy[0][0] = meany / filterSize ;
		for(i = 1; i < width; i++) {
			linecpy[0][i] = ((filterSize - 1) * linecpy[0][i - 1] + in[i + j * width]) / filterSize ;
		}
//		linecpy[1][width - 1] = in[j * width + width - 1] ;
		meany = 0 ;
		for(i = width - filterSize; i < width; i++) {
			meany += in[i + j * width] ;
		}
		linecpy[1][width - 1] = meany / filterSize ;
		for(i = width - 2; i >= 0; i--) {
			linecpy[1][i] = ((filterSize - 1) * linecpy[1][i + 1] + in[i + 1 + j * width]) / filterSize ;
		}
		for(i = 0; i < width; i++) {
			cpy[i + j * width] = (linecpy[0][i] + linecpy[1][i]) / 2 ;
		}
	}

	for(i = 0; i < width; i++) {
//		linecpy[0][0] = cpy[i] ;
		meany = 0 ;
		for(j = 0; j < filterSize; j++) {
			meany += cpy[i + j * width] ;
		}
		linecpy[0][0] = meany / filterSize ;
		for(j = 1; j < height; j++) {
			linecpy[0][j] = ((filterSize - 1) * linecpy[0][j - 1] + cpy[i + j * width]) / filterSize ;
		}
//		linecpy[1][height - 1] = cpy[i + (height - 1) * width] ;
		linecpy[1][height - 1] = (127 * (filterSize - 1) + cpy[i + (height - 1) * width]) / filterSize ;
		meany = 0 ;
		for(j = height - filterSize; j < height; j++) {
			meany += cpy[i + j * width] ;
		}
		linecpy[1][height - 1] = meany / filterSize ;
		for(j = height - 2; j >= 0; j--) {
			linecpy[1][j] = ((filterSize - 1) * linecpy[1][j + 1] + cpy[i + (j + 1) * width]) / filterSize ;
			out[i + j * width] = (linecpy[0][j] + linecpy[1][j]) / 2 ;
		}
		for(j = 0; j < height; j++) {
			out[i + j * width] = (linecpy[0][j] + linecpy[1][j]) / 2 ;
		}
	}
	
	free(cpy) ;
	free(linecpy[0]) ;
	free(linecpy[1]) ;
}

void approxAverageFilter(unsigned char *srcBuf, int bytesPerRow, int nextPixelIncr) {
}

void videoEnhance_yuv(unsigned char *srcYUV, unsigned char *destYUV, int width, int height) {

	unsigned char *level[3] ;
	short *reflectance ;
	unsigned char *luminance ;
	int meanR ;
	int i, j ;
	int y, u, v ;
	int gammaIndex ;
	int yHistogram[256] ;
	int yLow, yHigh ;
	int cfLow, cfHigh ;

	level[0] = (unsigned char *) malloc(width * height) ;
	level[1] = (unsigned char *) malloc(width * height) ;
	level[2] = (unsigned char *) malloc(width * height) ;

	reflectance = (short *) malloc(width * height * 2) ;
	luminance = (unsigned char *) malloc(width * height * 2) ;
	
	memset(yHistogram, 0, 256 * sizeof(int)) ;

	exponentialFilter(srcYUV, level[0], width / 22, width, height) ;
	exponentialFilter(srcYUV, level[1], width / 44, width, height) ;
	exponentialFilter(srcYUV, level[2], width / 88, width, height) ;

	meanR = 0 ;
	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			luminance[i + j * width] = expTable[(logTable[level[0][i + j * width]] + logTable[level[1][i + j * width]] + logTable[level[2][i + j * width]]) / 3] / 4 ;
			reflectance[i + j * width] = (srcYUV[i + j * width] + 1) * 256 / (luminance[i + j * width] + 1) ;
			meanR += reflectance[i + j * width] ;
			yHistogram[srcYUV[i + j * width]]++ ;
		}
	}
	meanR /= (width * height) ;

	cfLow = cfHigh = 0 ;
	yLow = yHigh = -1 ;
	for(i = 0; i < 256; i++) {
		if(100 * cfLow > 5 * width * height) {
			if(yLow < 0)
				yLow = i ;
		} else {
			cfLow += yHistogram[i] ;
		}
		if(100 * cfHigh > 5 * width * height) {
			if(yHigh < 0)
				yHigh = 255 - i ;
		} else {
			cfHigh += yHistogram[255 - i] ;
		}
	}

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
//			gammaIndex = luminance[i + j * width] + yLow - (255 - yHigh) ;
			gammaIndex = luminance[i + j * width] ;
			y = srcYUV[i + j * width] ;
//			y = (yLow + yHigh) / 2 + (y - ((yLow + yHigh) / 2)) * 256 / (yHigh - yLow + 192) ;
//			if(y < 0) y = 0 ;
//			if(y > 255) y = 255 ;
			if(gammaIndex < 0) gammaIndex = 0 ;
			if(gammaIndex > 255) gammaIndex = 255 ;
			y = reflectance[i + j * width] * yTable[gammaIndex][srcYUV[i + j * width]] / (meanR + 1) ;
//			y = reflectance[i + j * width] * srcYUV[i + j * width] / (meanR + 1) ;

//			y = reflectance[i + j * width] * yTable[gammaIndex][gammaIndex] / 255 ;
//			y = yTable[gammaIndex][srcYUV[i + j * width]] ;
//			y = reflectance[i + j * width] * yTable[gammaIndex][y] / (meanR + 1) ;
			if(y < 0) y = 0 ;
			if(y > 255) y = 255 ;
			destYUV[i + j * width] = y ;
		}
	}

/*
	{
		for(j = 0; j < height; j++) {
			for(i = 0; i < width; i++) {
				gammaIndex = luminance[i + j * width] ;
				if(gammaIndex < 0) gammaIndex = 0 ;
				if(gammaIndex > 255) gammaIndex = 255 ;
				y = reflectance[i + j * width] * yTable2[gammaIndex][srcYUV[i + j * width]] / (meanR + 1) ;
				if(y < 0) y = 0 ;
				if(y > 255) y = 255 ;
				destYUV[i + j * width] = y ;
			}
		}
	}
*/
	{
		for(j = 0; j < height / 2; j++) {
			for(i = 0; i < width / 2; i++) {
				u = srcYUV[width * height + i + j * width / 2] ;
				u -= 128 ;
				u = (5 * srcYUV[2 * i + 2 * j * width] + 2 * destYUV[2 * i + 2 * j * width] + 1) * u / (5 * srcYUV[2 * i + 2 * j * width] + 1) ;
				u += 128 ;
				if(u < 0) u = 0 ;
				if(u > 255) u = 255 ;
				destYUV[width * height + i + j * width / 2] = u ;
				v = srcYUV[width * height + width * height / 4 + i + j * width / 2] ;
				v -= 128 ;
				v = (5 * srcYUV[2 * i + 2 * j * width] + 2 * destYUV[2 * i + 2 * j * width] + 1) * v / (5 * srcYUV[2 * i + 2 * j * width] + 1) ;
				v += 128 ;
				if(v < 0) v = 0 ;
				if(v > 255) v = 255 ;
				destYUV[width * height + width * height / 4 + i + j * width / 2] = v ;
			}
		}
	}

	free(luminance) ;
	free(reflectance) ;
	free(level[0]) ;
	free(level[1]) ;
	free(level[2]) ;
}

void videoEnhance_rgba(unsigned char *srcRGBA, unsigned char *destRGBA, int width, int height) {

	double sum_g ;
	double sum_r, sum_r2 ;
	double sum_b, sum_b2 ;
	int gHist[256] ;
	int gMap[256] ;
	int max_r ;
	int max_g ;
	int max_b ;
	int size4 = width * height * 4 ;

	int i ;

	double u_r, v_r ;
	double u_b, v_b ;
	double d ;
	int r, g, b ;
	int eg, ig ;

	memset(gHist, 0, 256 * sizeof(int)) ;

	for(i = 0; i < size4; i+=4) {
		gHist[(srcRGBA[i+1] + srcRGBA[i+2] + srcRGBA[i+3])/3]++ ;
	}

	histogramEqualize(gHist, NULL, 256, gMap) ;

	sum_r = 0.0 ;
	sum_g = 0.0 ;
	sum_b = 0.0 ;
	sum_r2 = 0.0 ;
	sum_b2 = 0.0 ;
	max_r = -1 ;
	max_g = -1 ;
	max_b = -1 ;
	for(i = 0; i < size4; i+=4) {
		ig = (srcRGBA[i+1] + srcRGBA[i+2] + srcRGBA[i+3])/3 ;
		eg = gMap[ig] ;

		g = srcRGBA[i + 1] * (eg + 256) / (ig + 256) ;
		r = srcRGBA[i] * (eg + 256) / (ig + 256) ;
		b = srcRGBA[i + 2] * (eg + 256) / (ig + 256) ;

		g = eg ;
		r = eg ;
		b = eg ;

		if(r < 0) r = 0 ;
		if(r > 255) r = 255 ;
		if(g < 0) g = 0 ;
		if(g > 255) g = 255 ;
		if(b < 0) b = 0 ;
		if(b > 255) b = 255 ;

		srcRGBA[i] = r ;
		srcRGBA[i + 1] = g ;
		srcRGBA[i + 2] = b ;

		destRGBA[i] = r ;
		destRGBA[i + 1] = g ;
		destRGBA[i + 2] = b ;
	}
/*	
	for(i = 0; i < size4; i+=4) {
		sum_g += (double) srcRGBA[i + 1] ;
		sum_r += (double) srcRGBA[i] ;
		sum_r2 += (double) (srcRGBA[i] * srcRGBA[i]) ;
		sum_b += (double) srcRGBA[i + 2] ;
		sum_b2 += (double) (srcRGBA[i + 2] * srcRGBA[i + 2]) ;
		if(srcRGBA[i + 1] > max_g) {
			max_g = srcRGBA[i + 1] ;
		}
		if(srcRGBA[i] > max_r) {
			max_r = srcRGBA[i] ;
		}
		if(srcRGBA[i + 2] > max_b) {
			max_b = srcRGBA[i + 2] ;
		}
	}

	d = max_r * sum_r2 - max_r * max_r * sum_r ;
	u_r = (max_r * sum_g - max_g * sum_r) / d ;
	v_r = (max_g * sum_r2 - sum_g * max_r * max_r) / d ; 

	d = max_b * sum_b2 - max_b * max_b * sum_b ;
	u_b = (max_b * sum_g - max_g * sum_b) / d ;
	v_b = (max_g * sum_b2 - sum_g * max_b * max_b) / d ; 

	for(i = 0; i < size4; i += 4) {
		r = (int) (u_r * srcRGBA[i] * srcRGBA[i] + v_r * srcRGBA[i]) ;
		b = (int) (u_b * srcRGBA[i + 2] * srcRGBA[i + 2] + v_b * srcRGBA[i + 2]) ;
		g = srcRGBA[i + 1] ;

		if(r < 0) r = 0 ;
		if(r > 255) r = 255 ;
		if(g < 0) g = 0 ;
		if(g > 255) g = 255 ;
		if(b < 0) b = 0 ;
		if(b > 255) b = 255 ;

		destRGBA[i] = r ;
		destRGBA[i + 1] = g ;
		destRGBA[i + 2] = b ;
		destRGBA[i + 3] = srcRGBA[i + 3] ;
	}
*/	       
//	memcpy(destRGBA, srcRGBA, width * height * 4) ;
}
