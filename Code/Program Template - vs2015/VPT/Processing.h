#pragma once
#include "VPTDoc.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "TCHAR.H"
#include "Tools/linklist.h"

//#define MaxTunnelNum 32

class CProcessing
{
protected:
	CView *pView ;						// CView (CVPTView) class pointer for current document
	CDocument *pDoc ;					// CDocument (CVPTDoc) class pointer for current document
	unsigned char *pOutput ;			// Output buffer pointer in the tunnel document
	unsigned char *pInput ;				// Current input image buffer pointer
	ArrayList<unsigned char *> tunnelBits ;		// Buffer pointers of tunnels
	int width, height ;					// Width and Height of the image
	int tunnelCount ;
public:
	CProcessing(void);
	BOOL TunnelValid(void) ;			// Check whether the tunnel document is still there ...
	unsigned char *GetBits(void) ;
	unsigned char *GetSrc(void) ;		// Get the current input image buffer pointer and assign it to pInput
	void printf(const TCHAR *fmt, ...) ;	// Debug printf, it output messages to consoleview window
	void GLStart(unsigned char *bits = NULL) ;		// Setup GL context for GL operations on input/tunnel image (default: on 1st tunnel image)
	void GLFinish(unsigned char *bits = NULL) ;		// Release GL context for GL operations on input/tunnel image (default: on 1st tunnel image)
	void CreateTunnel(int width, int height, unsigned char **bits) ; // Create Tunnel
	void RemoveTunnel(unsigned char *bits) ;						// Remove Tunnel)
	void UpdateTunnel(void);			// Update the tunnel images
	void SetTitle(unsigned char *bits, TCHAR *fmt, ...) ; // Set the title for tunnel document
	void PutDC(unsigned char *bits = NULL) ;		// PutDC operations (default: PutDC for 1st tunnel image)
	int GetFrameNum(void) ;
	HDC GetDC(unsigned char *bits = NULL) ;
	void Init(CView *pView) ;			// Initializes the pView, pDoc, pInput and pOutput, it call CustomInit in the derived class
	virtual void CustomInit(CView *pView) ;	// Customizeable Init() in the derived class
	virtual void Perform(void) ;		// Perform the processing task required. (This will be overwritten in the derived class by legacy application)
	virtual void Process(void) ;		// This should be overwritten instead of Perform(void)
	void Final(void) ;					// Cleanup function to release resources associated with the current processing task, it call CustomFinal in the derived class
	virtual void CustomFinal(void) ;	// Customizable Final() in the derived class
public:
	~CProcessing(void);
};
