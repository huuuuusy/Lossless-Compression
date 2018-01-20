#pragma once
#include "vfw.h"

class CVFWInterface
{
public:
	CVFWInterface(void);
	~CVFWInterface(void);
	HIC Init(DWORD fcc, BOOL modeComp, int video_width = 352, int video_height = 288, int bit = 24, BOOL config = FALSE) ;
	void Config(HWND hwnd);
	int Compress(unsigned char *src, unsigned char **cdata);
	int Decompress(unsigned char *cdata, int size, unsigned char *out) ;
	BOOL Ready(void);
	void Close(void);
	DWORD GetFCC(void) ;
private:
	DWORD fcc ;
	DWORD ifcc ;
	HIC hic ;
	BITMAPINFOHEADER bih ;
	LPBITMAPINFOHEADER pbih ;
	DWORD dwFlags ;
	DWORD dwCkID ;
	BOOL compMode ;
	BOOL ready ;
	unsigned char *data ;
	unsigned char *prevFrame ;
	int fnum ;
};
