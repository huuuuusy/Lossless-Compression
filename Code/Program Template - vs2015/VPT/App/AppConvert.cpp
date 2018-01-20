#include "StdAfx.h"
#include "AppConvert.h"

CAppConvert::CAppConvert(void)
{
	// Class Constructor
	mode = 0 ;
}

CAppConvert::~CAppConvert(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppConvert::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
	if(mode == 0) {
		CreateTunnel(width, height, &pOutput420) ;
	}
}

void CAppConvert::CustomFinal(void) {
	// Add custom finalization code here

	RemoveTunnel(pOutput420) ;
}

// This function converts input RGB image to a YUV image.
void CAppConvert::RGBtoYUV(unsigned char *pRGB, unsigned char *pYUV) {

	// You should implement RGB to YUV conversion here.


	// Sample code start - You may delete these sample code
	// The following code illustrates generation of grayscale images YUV images

	int i, j ;
	int r, g, b ;
	int Y, U, V ;
	
	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			b = pRGB[(i + j * width) * 3] ;
			g = pRGB[(i + j * width) * 3 + 1] ;
			r = pRGB[(i + j * width) * 3 + 2] ;

			//Solution 1: In range [0,255]

			Y = 0.299 * r + 0.587 * g + 0.114 * b;
			U = -0.169 * r - 0.331 * g + 0.500 * b + 128;
			V = 0.500 * r - 0.419 * g - 0.081 * b + 128;
			if (Y < 0) {Y = 0;}
			else if (Y > 255) { Y = 255; }
			if (U < 0) { U = 0; }
			else if (U > 255) { U = 255; }
			if (V < 0) { V = 0; }
			else if (V > 255) { V = 255; }
			
			
			
			/*Y = 0.299*r + 0.587*g + 0.114*b;
			U = -0.14713*r - 0.28886*g + 0.436*b;
			V = 0.615*r - 0.51499*g - 0.10001*b;
			if (Y < 0) { Y = 0; }
			else if (Y > 255) { Y = 255; }
			if (U < -111) { U = -111; }
			else if (U > 111) { U = 111; }
			if (V < -157) { V = -157; }
			else if (V > 157) { V = 157; }*/

			/*Y = 0.257*r + 0.504*g + 0.098*b + 16;
			U = 0.148*r - 0.291*g + 0.439*b + 128;
			V = 0.439*r - 0.368*g - 0.071*b + 128;
			if (Y < 0) { Y = 0; }
			else if (Y > 255) { Y = 255; }
			if (U < 0) { U = 0; }
			else if (U > 255) { U = 255; }
			if (V < 0) { V = 0; }
			else if (V > 255) { V = 255; }*/



		

			pYUV[(i + j * width) * 3] = Y  ;
			pYUV[(i + j * width) * 3 + 1] = U  ;
			pYUV[(i + j * width) * 3 + 2] = V  ;
		}
	}

	// Sample Code end
}

// This function converts input YUV image to a RGB image.
void CAppConvert::YUVtoRGB(unsigned char *pYUV, unsigned char *pRGB) {

	// You should implement YUV to RGB conversion here.

	// Sample code start - You may delete these sample code
	int i, j ;
	int y, u, v;
	int R, G, B;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			y = pYUV[(i + j * width) * 3] ;
			u = pYUV[(i + j * width) * 3 + 1];
			v = pYUV[(i + j * width) * 3 + 2];
			
		
			//Solution 1: In range [0,255]
			R = y + 1.4 * (v - 128);
			G = y - 0.343 * (u - 128) - 0.711 * (v - 128);
			B = y + 1.765 * (u - 128);

			if (R < 0) { R = 0; }
			else if (R > 255) { R = 255; }
			if (G < 0) { G = 0; }
			else if (G > 255) { G = 255; }
			if (B < 0) { B = 0; }
			else if (B > 255) { B = 255; }

			

			/*R = y + 1.13983*v;
			G = y - 0.39465*u - 0.58060*v;
			B = y + 2.03211*u;
			if (R < 0) { R = 0; }
			else if (R > 255) { R = 255; }
			if (G < 0) { G = 0; }
			else if (G > 255) { G = 255; }
			if (B < 0) { B = 0; }
			else if (B > 255) { B = 255; }*/

			/*B = 1.164*(y - 16) + 2.018*(u - 128);
			G = 1.164*(y - 16) - 0.813*(v - 128) - 0.391*(u - 128);
			R = 1.164*(y - 16) + 1.596*(v - 128);
			if (R < 0) { R = 0; }
			else if (R > 255) { R = 255; }
			if (G < 0) { G = 0; }
			else if (G > 255) { G = 255; }
			if (B < 0) { B = 0; }
			else if (B > 255) { B = 255; }*/

						
			pRGB[(i + j * width) * 3] = B ;
			pRGB[(i + j * width) * 3 + 1] = G;
			pRGB[(i + j * width) * 3 + 2] = R;
		}
	}

	// Sample Code end
}



void CAppConvert::YUVtoYUV420(unsigned char *pYUV, unsigned char *pYUV420) {

	int i, j ;
	int sum ;
	int si0, si1, sj0, sj1 ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			pYUV420[(i + j * width) * 3] = pYUV[(i + j * width) * 3] ;
		}
	}

	for(j = 0; j < height; j+=2) {
		sj0 = j ;
		sj1 = (j + 1 < height) ? j + 1 : j ;
	
		for(i = 0; i < width; i+=2) {
			si0 = i ;
			si1 = (i + 1 < width) ? i + 1 : i ;

			sum =  pYUV[(si0 + sj0 * width) * 3 + 1] ; 
			sum += pYUV[(si1 + sj0 * width) * 3 + 1] ; 
			sum += pYUV[(si0 + sj1 * width) * 3 + 1] ; 
			sum += pYUV[(si1 + sj1 * width) * 3 + 1] ; 

			sum = sum / 4 ;

			pYUV420[(si0 + sj0 * width) * 3 + 1] = sum ;
			pYUV420[(si1 + sj0 * width) * 3 + 1] = sum ;
			pYUV420[(si0 + sj1 * width) * 3 + 1] = sum ;
			pYUV420[(si1 + sj1 * width) * 3 + 1] = sum ;

			sum =  pYUV[(si0 + sj0 * width) * 3 + 2] ; 
			sum += pYUV[(si1 + sj0 * width) * 3 + 2] ; 
			sum += pYUV[(si0 + sj1 * width) * 3 + 2] ; 
			sum += pYUV[(si1 + sj1 * width) * 3 + 2] ; 

			sum = sum / 4 ;

			pYUV420[(si0 + sj0 * width) * 3 + 2] = sum ;
			pYUV420[(si1 + sj0 * width) * 3 + 2] = sum ;
			pYUV420[(si0 + sj1 * width) * 3 + 2] = sum ;
			pYUV420[(si1 + sj1 * width) * 3 + 2] = sum ;
		}
	}
}

void CAppConvert::Process(void) {

	// Don't change anything within this function.

	unsigned char *yuvBuf = new unsigned char[width * height * 3] ;
	unsigned char *yuv420Buf = new unsigned char[width * height * 3] ;

	RGBtoYUV(pInput, yuvBuf) ;

	if(mode == 0) {
		SetTitle(pOutput, _T("YUV444")) ;
		SetTitle(pOutput420, _T("YUV420")) ;

		YUVtoYUV420(yuvBuf, yuv420Buf) ;

		YUVtoRGB(yuvBuf, pOutput) ;
		YUVtoRGB(yuv420Buf, pOutput420) ;
	} else if(mode == 1) {
		SetTitle(pOutput, _T("Y")) ;
	
		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 1] = 128 ;
			yuvBuf[i * 3 + 2] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	} else if(mode == 2) {
		SetTitle(pOutput, _T("U")) ;

		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 0] = 128 ;
			yuvBuf[i * 3 + 2] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	} else if(mode == 3) {
		SetTitle(pOutput, _T("V")) ;

		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 0] = 128 ;
			yuvBuf[i * 3 + 1] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	}

	delete [] yuvBuf ;
	delete [] yuv420Buf ;

	PutDC(pOutput) ;
	if(mode == 0) {
		PutDC(pOutput420) ;
	}
}
