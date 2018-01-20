#include <stdafx.h>
#include <windows.h>
#include "cpuid.h"


#pragma unmanaged
void __cpuid_unmanaged(int cpuInfo[4], int infoType) {
	__cpuid(cpuInfo, infoType) ;
}

void __cpuidex_unmanaged(int cpuInfo[4], int infoType, int ecxValue) {
	__cpuidex(cpuInfo, infoType, ecxValue) ;
}

#pragma managed

//#define _TPRINTF(...) _tprintf(__VA_ARGS__)
#define _TPRINTF(...) {};

static const TCHAR* __cpuSzFeatures[] =
{
	_T("x87 FPU On Chip"),
	_T("Virtual-8086 Mode Enhancement"),
	_T("Debugging Extensions"),
	_T("Page Size Extensions"),
	_T("Time Stamp Counter"),
	_T("RDMSR and WRMSR Support"),
	_T("Physical Address Extensions"),
	_T("Machine Check Exception"),
	_T("CMPXCHG8B Instruction"),
	_T("APIC On Chip"),
	_T("Unknown1"),
	_T("SYSENTER and SYSEXIT"),
	_T("Memory Type Range Registers"),
	_T("PTE Global Bit"),
	_T("Machine Check Architecture"),
	_T("Conditional Move/Compare Instruction"),
	_T("Page Attribute Table"),
	_T("36-bit Page Size Extension"),
	_T("Processor Serial Number"),
	_T("CFLUSH Extension"),
	_T("Unknown2"),
	_T("Debug Store"),
	_T("Thermal Monitor and Clock Ctrl"),
	_T("MMX Technology"),
	_T("FXSAVE/FXRSTOR"),
	_T("SSE Extensions"),
	_T("SSE2 Extensions"),
	_T("Self Snoop"),
	_T("Multithreading Technology"),
	_T("Thermal Monitor"),
	_T("Unknown4"),
	_T("Pending Break Enable")
};

class CPUInfo {
	char CPUStringData[0x20];
	TCHAR CPUString[0x20];
	char CPUBrandStringData[0x40];
	TCHAR CPUBrandString[0x40];
	int cpuInfoData[4];
	int nSteppingID;
	int nModel;
	int nFamily;
	int nProcessorType;
	int nExtendedmodel;
	int nExtendedfamily;
	int nBrandIndex;
	int nCLFLUSHcachelinesize;
	int nLogicalProcessors;
	int nAPICPhysicalID;
	int nFeatureInfo;
	int nCacheLineSize;
	int nL2Associativity;
	int nCacheSizeK;
	int nPhysicalAddress;
	int nVirtualAddress;
	int nRet;

	int nCores;
	int nCacheType;
	int nCacheLevel;
	int nMaxThread;
	int nSysLineSize;
	int nPhysicalLinePartitions;
	int nWaysAssociativity;
	int nNumberSets;

	unsigned    nIds, nExIds, i;

	bool    bSSE3Instructions;
	bool    bMONITOR_MWAIT;
	bool    bCPLQualifiedDebugStore;
	bool    bVirtualMachineExtensions;
	bool    bEnhancedIntelSpeedStepTechnology;
	bool    bThermalMonitor2;
	bool    bSupplementalSSE3;
	bool    bL1ContextID;
	bool    bCMPXCHG16B;
	bool    bxTPRUpdateControl;
	bool    bPerfDebugCapabilityMSR;
	bool    bSSE41Extensions;
	bool    bSSE42Extensions;
	bool    bPOPCNT;

	bool    bMultithreading;

	bool    bLAHF_SAHFAvailable;
	bool    bCmpLegacy;
	bool    bSVM;
	bool    bExtApicSpace;
	bool    bAltMovCr8;
	bool    bLZCNT;
	bool    bSSE4A;
	bool    bMisalignedSSE;
	bool    bPREFETCH;
	bool    bSKINITandDEV;
	bool    bSYSCALL_SYSRETAvailable;
	bool    bExecuteDisableBitAvailable;
	bool    bMMXExtensions;
	bool    bFFXSR;
	bool    b1GBSupport;
	bool    bRDTSCP;
	bool    b64Available;
	bool    b3DNowExt;
	bool    b3DNow;
	bool    bNestedPaging;
	bool    bLBRVisualization;
	bool    bFP128;
	bool    bMOVOptimization;

	bool    bSelfInit;
	bool    bFullyAssociative;

public:
	CPUInfo(void) {
		cpuInfoData[0]=0xFFFFFFFF;
		cpuInfoData[1]=0xFFFFFFFF;
		cpuInfoData[2]=0xFFFFFFFF;
		cpuInfoData[3]=0xFFFFFFFF;
		nSteppingID = 0;
		nModel = 0;
		nFamily = 0;
		nProcessorType = 0;
		nExtendedmodel = 0;
		nExtendedfamily = 0;
		nBrandIndex = 0;
		nCLFLUSHcachelinesize = 0;
		nLogicalProcessors = 0;
		nAPICPhysicalID = 0;
		nFeatureInfo = 0;
		nCacheLineSize = 0;
		nL2Associativity = 0;
		nCacheSizeK = 0;
		nPhysicalAddress = 0;
		nVirtualAddress = 0;
		nRet = 0;

		nCores = 0;
		nCacheType = 0;
		nCacheLevel = 0;
		nMaxThread = 0;
		nSysLineSize = 0;
		nPhysicalLinePartitions = 0;
		nWaysAssociativity = 0;
		nNumberSets = 0;

		bSSE3Instructions = false;
		bMONITOR_MWAIT = false;
		bCPLQualifiedDebugStore = false;
		bVirtualMachineExtensions = false;
		bEnhancedIntelSpeedStepTechnology = false;
		bThermalMonitor2 = false;
		bSupplementalSSE3 = false;
		bL1ContextID = false;
		bCMPXCHG16B = false;
		bxTPRUpdateControl = false;
		bPerfDebugCapabilityMSR = false;
		bSSE41Extensions = false;
		bSSE42Extensions = false;
		bPOPCNT = false;

		bMultithreading = false;

		bLAHF_SAHFAvailable = false;
		bCmpLegacy = false;
		bSVM = false;
		bExtApicSpace = false;
		bAltMovCr8 = false;
		bLZCNT = false;
		bSSE4A = false;
		bMisalignedSSE = false;
		bPREFETCH = false;
		bSKINITandDEV = false;
		bSYSCALL_SYSRETAvailable = false;
		bExecuteDisableBitAvailable = false;
		bMMXExtensions = false;
		bFFXSR = false;
		b1GBSupport = false;
		bRDTSCP = false;
		b64Available = false;
		b3DNowExt = false;
		b3DNow = false;
		bNestedPaging = false;
		bLBRVisualization = false;
		bFP128 = false;
		bMOVOptimization = false;

		bSelfInit = false;
		bFullyAssociative = false;

		// __cpuid with an InfoType argument of 0 returns the number of
		// valid Ids in cpuInfoData[0] and the CPU identification string in
		// the other three array elements. The CPU identification string is
		// not in linear order. The code below arranges the information 
		// in a human readable form.
		__cpuid_unmanaged(cpuInfoData, 0);
		nIds = cpuInfoData[0];
		memset(CPUStringData, 0, sizeof(CPUStringData));
		*((int*)CPUStringData) = cpuInfoData[1];
		*((int*)(CPUStringData+4)) = cpuInfoData[3];
		*((int*)(CPUStringData+8)) = cpuInfoData[2];

		// Get the information associated with each valid Id
		for (i=0; i<=nIds; ++i)
		{
			__cpuid_unmanaged(cpuInfoData, i);
			_TPRINTF(_T("\nFor InfoType %d\n"), i); 
			_TPRINTF(_T("cpuInfoData[0] = 0x%x\n"), cpuInfoData[0]);
			_TPRINTF(_T("cpuInfoData[1] = 0x%x\n"), cpuInfoData[1]);
			_TPRINTF(_T("cpuInfoData[2] = 0x%x\n"), cpuInfoData[2]);
			_TPRINTF(_T("cpuInfoData[3] = 0x%x\n"), cpuInfoData[3]);

			// Interpret CPU feature information.
			if  (i == 1)
			{
				nSteppingID = cpuInfoData[0] & 0xf;
				nModel = (cpuInfoData[0] >> 4) & 0xf;
				nFamily = (cpuInfoData[0] >> 8) & 0xf;
				nProcessorType = (cpuInfoData[0] >> 12) & 0x3;
				nExtendedmodel = (cpuInfoData[0] >> 16) & 0xf;
				nExtendedfamily = (cpuInfoData[0] >> 20) & 0xff;
				nBrandIndex = cpuInfoData[1] & 0xff;
				nCLFLUSHcachelinesize = ((cpuInfoData[1] >> 8) & 0xff) * 8;
				nLogicalProcessors = ((cpuInfoData[1] >> 16) & 0xff);
				nAPICPhysicalID = (cpuInfoData[1] >> 24) & 0xff;
				bSSE3Instructions = (cpuInfoData[2] & 0x1) || false;
				bMONITOR_MWAIT = (cpuInfoData[2] & 0x8) || false;
				bCPLQualifiedDebugStore = (cpuInfoData[2] & 0x10) || false;
				bVirtualMachineExtensions = (cpuInfoData[2] & 0x20) || false;
				bEnhancedIntelSpeedStepTechnology = (cpuInfoData[2] & 0x80) || false;
				bThermalMonitor2 = (cpuInfoData[2] & 0x100) || false;
				bSupplementalSSE3 = (cpuInfoData[2] & 0x200) || false;
				bL1ContextID = (cpuInfoData[2] & 0x300) || false;
				bCMPXCHG16B= (cpuInfoData[2] & 0x2000) || false;
				bxTPRUpdateControl = (cpuInfoData[2] & 0x4000) || false;
				bPerfDebugCapabilityMSR = (cpuInfoData[2] & 0x8000) || false;
				bSSE41Extensions = (cpuInfoData[2] & 0x80000) || false;
				bSSE42Extensions = (cpuInfoData[2] & 0x100000) || false;
				bPOPCNT= (cpuInfoData[2] & 0x800000) || false;
				nFeatureInfo = cpuInfoData[3];
				bMultithreading = (nFeatureInfo & (1 << 28)) || false;
			}
		}

		// Calling __cpuid with 0x80000000 as the InfoType argument
		// gets the number of valid extended IDs.
		__cpuid_unmanaged(cpuInfoData, 0x80000000);
		nExIds = cpuInfoData[0];
		memset(CPUBrandStringData, 0, sizeof(CPUBrandStringData));

		// Get the information associated with each extended ID.
		for (i=0x80000000; i<=nExIds; ++i)
		{
			__cpuid_unmanaged(cpuInfoData, i);
			_TPRINTF(_T("\nFor InfoType %x\n"), i); 
			_TPRINTF(_T("cpuInfoData[0] = 0x%x\n"), cpuInfoData[0]);
			_TPRINTF(_T("cpuInfoData[1] = 0x%x\n"), cpuInfoData[1]);
			_TPRINTF(_T("cpuInfoData[2] = 0x%x\n"), cpuInfoData[2]);
			_TPRINTF(_T("cpuInfoData[3] = 0x%x\n"), cpuInfoData[3]);

			if  (i == 0x80000001)
			{
				bLAHF_SAHFAvailable = (cpuInfoData[2] & 0x1) || false;
				bCmpLegacy = (cpuInfoData[2] & 0x2) || false;
				bSVM = (cpuInfoData[2] & 0x4) || false;
				bExtApicSpace = (cpuInfoData[2] & 0x8) || false;
				bAltMovCr8 = (cpuInfoData[2] & 0x10) || false;
				bLZCNT = (cpuInfoData[2] & 0x20) || false;
				bSSE4A = (cpuInfoData[2] & 0x40) || false;
				bMisalignedSSE = (cpuInfoData[2] & 0x80) || false;
				bPREFETCH = (cpuInfoData[2] & 0x100) || false;
				bSKINITandDEV = (cpuInfoData[2] & 0x1000) || false;
				bSYSCALL_SYSRETAvailable = (cpuInfoData[3] & 0x800) || false;
				bExecuteDisableBitAvailable = (cpuInfoData[3] & 0x10000) || false;
				bMMXExtensions = (cpuInfoData[3] & 0x40000) || false;
				bFFXSR = (cpuInfoData[3] & 0x200000) || false;
				b1GBSupport = (cpuInfoData[3] & 0x400000) || false;
				bRDTSCP = (cpuInfoData[3] & 0x8000000) || false;
				b64Available = (cpuInfoData[3] & 0x20000000) || false;
				b3DNowExt = (cpuInfoData[3] & 0x40000000) || false;
				b3DNow = (cpuInfoData[3] & 0x80000000) || false;
			}

			// Interpret CPU brand string and cache information.
			if  (i == 0x80000002)
				memcpy(CPUBrandStringData, cpuInfoData, sizeof(cpuInfoData));
			else if  (i == 0x80000003)
				memcpy(CPUBrandStringData + 16, cpuInfoData, sizeof(cpuInfoData));
			else if  (i == 0x80000004)
				memcpy(CPUBrandStringData + 32, cpuInfoData, sizeof(cpuInfoData));
			else if  (i == 0x80000006)
			{
				nCacheLineSize = cpuInfoData[2] & 0xff;
				nL2Associativity = (cpuInfoData[2] >> 12) & 0xf;
				nCacheSizeK = (cpuInfoData[2] >> 16) & 0xffff;
			}
			else if  (i == 0x80000008)
			{
			   nPhysicalAddress = cpuInfoData[0] & 0xff;
			   nVirtualAddress = (cpuInfoData[0] >> 8) & 0xff;
			}
			else if  (i == 0x8000000A)
			{
				bNestedPaging = (cpuInfoData[3] & 0x1) || false;
				bLBRVisualization = (cpuInfoData[3] & 0x2) || false;
			}
			else if  (i == 0x8000001A)
			{
				bFP128 = (cpuInfoData[0] & 0x1) || false;
				bMOVOptimization = (cpuInfoData[0] & 0x2) || false;
			}
		}

		// Display all the information in user-friendly format.

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, CPUStringData, 32, CPUString, 32) ;
		_TPRINTF(_T("\n\nCPU String: %s\n"), CPUString);

		if  (nIds >= 1)
		{
			if  (nSteppingID)
				_TPRINTF(_T("Stepping ID = %d\n"), nSteppingID);
			if  (nModel)
				_TPRINTF(_T("Model = %d\n"), nModel);
			if  (nFamily)
				_TPRINTF(_T("Family = %d\n"), nFamily);
			if  (nProcessorType)
				_TPRINTF(_T("Processor Type = %d\n"), nProcessorType);
			if  (nExtendedmodel)
				_TPRINTF(_T("Extended model = %d\n"), nExtendedmodel);
			if  (nExtendedfamily)
				_TPRINTF(_T("Extended family = %d\n"), nExtendedfamily);
			if  (nBrandIndex)
				_TPRINTF(_T("Brand Index = %d\n"), nBrandIndex);
			if  (nCLFLUSHcachelinesize)
				_TPRINTF(_T("CLFLUSH cache line size = %d\n"),
						 nCLFLUSHcachelinesize);
			if (bMultithreading && (nLogicalProcessors > 0))
			   _TPRINTF(_T("Logical Processor Count = %d\n"), nLogicalProcessors);
			if  (nAPICPhysicalID)
				_TPRINTF(_T("APIC Physical ID = %d\n"), nAPICPhysicalID);

			if  (nFeatureInfo || bSSE3Instructions ||
				 bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
				 bVirtualMachineExtensions || bEnhancedIntelSpeedStepTechnology ||
				 bThermalMonitor2 || bSupplementalSSE3 || bL1ContextID || 
				 bCMPXCHG16B || bxTPRUpdateControl || bPerfDebugCapabilityMSR || 
				 bSSE41Extensions || bSSE42Extensions || bPOPCNT || 
				 bLAHF_SAHFAvailable || bCmpLegacy || bSVM ||
				 bExtApicSpace || bAltMovCr8 ||
				 bLZCNT || bSSE4A || bMisalignedSSE ||
				 bPREFETCH || bSKINITandDEV || bSYSCALL_SYSRETAvailable || 
				 bExecuteDisableBitAvailable || bMMXExtensions || bFFXSR || b1GBSupport ||
				 bRDTSCP || b64Available || b3DNowExt || b3DNow || bNestedPaging || 
				 bLBRVisualization || bFP128 || bMOVOptimization )
			{
				_TPRINTF(_T("\nThe following features are supported:\n"));

				if  (bSSE3Instructions)
					_TPRINTF(_T("\tSSE3\n"));
				if  (bMONITOR_MWAIT)
					_TPRINTF(_T("\tMONITOR/MWAIT\n"));
				if  (bCPLQualifiedDebugStore)
					_TPRINTF(_T("\tCPL Qualified Debug Store\n"));
				if  (bVirtualMachineExtensions)
					_TPRINTF(_T("\tVirtual Machine Extensions\n"));
				if  (bEnhancedIntelSpeedStepTechnology)
					_TPRINTF(_T("\tEnhanced Intel SpeedStep Technology\n"));
				if  (bThermalMonitor2)
					_TPRINTF(_T("\tThermal Monitor 2\n"));
				if  (bSupplementalSSE3)
					_TPRINTF(_T("\tSupplemental Streaming SIMD Extensions 3\n"));
				if  (bL1ContextID)
					_TPRINTF(_T("\tL1 Context ID\n"));
				if  (bCMPXCHG16B)
					_TPRINTF(_T("\tCMPXCHG16B Instruction\n"));
				if  (bxTPRUpdateControl)
					_TPRINTF(_T("\txTPR Update Control\n"));
				if  (bPerfDebugCapabilityMSR)
					_TPRINTF(_T("\tPerf\\Debug Capability MSR\n"));
				if  (bSSE41Extensions)
					_TPRINTF(_T("\tSSE4.1 Extensions\n"));
				if  (bSSE42Extensions)
					_TPRINTF(_T("\tSSE4.2 Extensions\n"));
				if  (bPOPCNT)
					_TPRINTF(_T("\tPPOPCNT Instruction\n"));

				i = 0;
				nIds = 1;
				while (i < (sizeof(__cpuSzFeatures)/sizeof(const char*)))
				{
					if  (nFeatureInfo & nIds)
					{
						_TPRINTF(_T("\t"));
						_TPRINTF(__cpuSzFeatures[i]);
						_TPRINTF(_T("\n"));
					}

					nIds <<= 1;
					++i;
				}
				if (bLAHF_SAHFAvailable)
					_TPRINTF(_T("\tLAHF/SAHF in 64-bit mode\n"));
				if (bCmpLegacy)
					_TPRINTF(_T("\tCore multi-processing legacy mode\n"));
				if (bSVM)
					_TPRINTF(_T("\tSecure Virtual Machine\n"));
				if (bExtApicSpace)
					_TPRINTF(_T("\tExtended APIC Register Space\n"));
				if (bAltMovCr8)
					_TPRINTF(_T("\tAltMovCr8\n"));
				if (bLZCNT)
					_TPRINTF(_T("\tLZCNT instruction\n"));
				if (bSSE4A)
					_TPRINTF(_T("\tSSE4A (EXTRQ, INSERTQ, MOVNTSD, MOVNTSS)\n"));
				if (bMisalignedSSE)
					_TPRINTF(_T("\tMisaligned SSE mode\n"));
				if (bPREFETCH)
					_TPRINTF(_T("\tPREFETCH and PREFETCHW Instructions\n"));
				if (bSKINITandDEV)
					_TPRINTF(_T("\tSKINIT and DEV support\n"));
				if (bSYSCALL_SYSRETAvailable)
					_TPRINTF(_T("\tSYSCALL/SYSRET in 64-bit mode\n"));
				if (bExecuteDisableBitAvailable)
					_TPRINTF(_T("\tExecute Disable Bit\n"));
				if (bMMXExtensions)
					_TPRINTF(_T("\tExtensions to MMX Instructions\n"));
				if (bFFXSR)
					_TPRINTF(_T("\tFFXSR\n"));
				if (b1GBSupport)
					_TPRINTF(_T("\t1GB page support\n"));
				if (bRDTSCP)
					_TPRINTF(_T("\tRDTSCP instruction\n"));
				if (b64Available)
					_TPRINTF(_T("\t64 bit Technology\n"));
				if (b3DNowExt)
					_TPRINTF(_T("\t3Dnow Ext\n"));
				if (b3DNow)
					_TPRINTF(_T("\t3Dnow! instructions\n"));
				if (bNestedPaging)
					_TPRINTF(_T("\tNested Paging\n"));
				if (bLBRVisualization)
					_TPRINTF(_T("\tLBR Visualization\n"));
				if (bFP128)
					_TPRINTF(_T("\tFP128 optimization\n"));
				if (bMOVOptimization)
					_TPRINTF(_T("\tMOVU Optimization\n"));
			}
		}

		if  (nExIds >= 0x80000004) {
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, CPUBrandStringData, 64, CPUBrandString, 64) ;
			_TPRINTF(_T("\nCPU Brand String: %s\n"), CPUBrandString);
		}

		if  (nExIds >= 0x80000006)
		{
			_TPRINTF(_T("Cache Line Size = %d\n"), nCacheLineSize);
			_TPRINTF(_T("L2 Associativity = %d\n"), nL2Associativity);
			_TPRINTF(_T("Cache Size = %dK\n"), nCacheSizeK);
		}


		for (i=0;;i++)
		{
			__cpuidex_unmanaged(cpuInfoData, 0x4, i);
			if(!(cpuInfoData[0] & 0xf0)) break;

			if(i == 0)
			{
				nCores = cpuInfoData[0] >> 26;
				_TPRINTF(_T("\n\nNumber of Cores = %d\n"), nCores + 1);
			}

			nCacheType = (cpuInfoData[0] & 0x1f);
			nCacheLevel = (cpuInfoData[0] & 0xe0) >> 5;
			bSelfInit = ((cpuInfoData[0] & 0x100) >> 8) ? true: false ;
			bFullyAssociative = ((cpuInfoData[0] & 0x200) >> 9) ? true : false ;
			nMaxThread = (cpuInfoData[0] & 0x03ffc000) >> 14;
			nSysLineSize = (cpuInfoData[1] & 0x0fff);
			nPhysicalLinePartitions = (cpuInfoData[1] & 0x03ff000) >> 12;
			nWaysAssociativity = (cpuInfoData[1]) >> 22;
			nNumberSets = cpuInfoData[2];

			_TPRINTF(_T("\n"));

			_TPRINTF(_T("ECX Index %d\n"), i);
			switch (nCacheType)
			{
				case 0:
					_TPRINTF(_T("   Type: Null\n"));
					break;
				case 1:
					_TPRINTF(_T("   Type: Data Cache\n"));
					break;
				case 2:
					_TPRINTF(_T("   Type: Instruction Cache\n"));
					break;
				case 3:
					_TPRINTF(_T("   Type: Unified Cache\n"));
					break;
				default:
					 _TPRINTF(_T("   Type: Unknown\n"));
			}

			_TPRINTF(_T("   Level = %d\n"), nCacheLevel + 1); 
			if (bSelfInit)
			{
				_TPRINTF(_T("   Self Initializing\n"));
			}
			else
			{
				_TPRINTF(_T("   Not Self Initializing\n"));
			}
			if (bFullyAssociative)
			{
				_TPRINTF(_T("   Is Fully Associatve\n"));
			}
			else
			{
				_TPRINTF(_T("   Is Not Fully Associatve\n"));
			}
			_TPRINTF(_T("   Max Threads = %d\n"), 
				nMaxThread+1);
			_TPRINTF(_T("   System Line Size = %d\n"), 
				nSysLineSize+1);
			_TPRINTF(_T("   Physical Line Partions = %d\n"), 
				nPhysicalLinePartitions+1);
			_TPRINTF(_T("   Ways of Associativity = %d\n"), 
				nWaysAssociativity+1);
			_TPRINTF(_T("   Number of Sets = %d\n"), 
				nNumberSets+1);
		}
	}

	bool withMMX() {
		return (nFeatureInfo & (1 << 21)) ? true : false ;
	}

	bool withSSE() {
		return (nFeatureInfo & (1 << 24)) ? true : false ;
	}

	bool withSSE2() {
		return (nFeatureInfo & (1 << 25)) ? true : false ;
	}

	bool withSSE3() {
		return bSSE3Instructions ;
	}

	bool with3DNow() {
		return b3DNow ;
	}

	int withFeature() {
		return nFeatureInfo ;
	}
};

static CPUInfo cpuInfo ;

int cpuMMX(void) {

	return cpuInfo.withMMX() ;
}

int cpuSSE(void) {

	return cpuInfo.withSSE() ;
}

int cpuSSE2(void) {

//	return cpuInfo.withSSE2() ; ;
	return 0 ;
}

int cpu3DNOW(void) {

	return cpuInfo.with3DNow() ;
}

int cpuFeature(void) {

	return cpuInfo.withFeature() ;
}
