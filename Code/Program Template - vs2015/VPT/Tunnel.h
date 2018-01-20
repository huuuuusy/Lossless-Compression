#pragma once
#include "VPT.h"
#include "VPTDoc.h"
#include "TCHAR.H"
#include "Tools/LinkList.h"

#define MaxTunnelCount 100

class CTunnel
{
public:
	int count ;
	ArrayList<CVPTDoc *> docList ;
	ArrayList<unsigned char **> framePtr ;
//	CVPTDoc *docList[MaxTunnelCount] ;
//	unsigned char **framePtr[MaxTunnelCount] ;
	CView *pView ;

public:
	CTunnel(void);
	~CTunnel(void);
	CVPTDoc *Create(int width, int height, unsigned char **) ;
	void PutDC(CVPTDoc *pDoc) ;
	void PutDC(unsigned char *bits) ;
	HDC GetDC(CVPTDoc *pDoc) ;
	HDC GetDC(unsigned char *bits) ;
	void GLStart(CVPTDoc *pDoc) ;
	void GLStart(unsigned char *bits) ;
	void GLFinish(CVPTDoc *pDoc) ;
	void GLFinish(unsigned char *bits) ;
	int Remove(CVPTDoc *pDoc) ;
	int Remove(unsigned char *bits) ;
	int RemoveAll(void) ;
	void SetTitle(unsigned char *bits, TCHAR *fmt, ...) ;
	void SetTitle(CVPTDoc *pDoc, TCHAR *fmt, ...) ;
	void Update(void) ;
};
