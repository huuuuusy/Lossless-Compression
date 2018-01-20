#ifndef VIDEO_ENHANCE_H
#define VIDEO_ENHANCE_H

void buildLogTable(void) ;
int histogramEqualize(int *histOrg, int *weight, int n, int *map) ;
void histogramSpecification(int *histOrg, int *histSpec, int n, int *map) ;
void averagingFilter(unsigned char *in, unsigned char *out, int halfFilterStep, int width, int height) ;
void exponentialFilter(unsigned char *in, unsigned char *out, int filterStep, int width, int height) ;
void videoEnhance_yuv(unsigned char *srcYUV, unsigned char *destYUV, int width, int height) ;
void videoEnhance_rgba(unsigned char *srcRGBA, unsigned char *destRGBA, int width, int height) ;

#endif