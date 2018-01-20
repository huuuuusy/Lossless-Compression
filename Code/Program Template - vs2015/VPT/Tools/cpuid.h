#ifndef _INC_CPUID
#define _INC_CPUID

#include "TCHAR.H"
#include "Intrin.h"
#include "Windows.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
#endif

int cpuMMX(void) ;
int cpuSSE(void) ;
int cpuSSE2(void) ;
int cpu3DNOW(void) ;
int cpuFeature(void) ;

#endif
