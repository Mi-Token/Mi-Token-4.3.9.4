#include "DllHotSwap.h"
#include "DebugMessages.h"

#ifndef NO_HOT_SWAP

#include "Passthroughs.h"

#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { free(p); p = NULL; }
#endif

struct DLLFunctionTable
{
	int (APIENTRY *DLL_PACK)(BYTE** data, int* cbytes);
	int (APIENTRY *DLL_UNPACK)(BYTE* data, int cbytes);
	int (APIENTRY *DLL_FREE)(BYTE* data);
	int (APIENTRY *DLL_LOAD)(void);
	int (APIENTRY *DLL_UNLOAD)(void);
	int (APIENTRY *DLL_REINIT)(void* lsaBlob);
};


struct dllPackDataLS
{
	unsigned int Start;
	unsigned int Length;
};

struct dllPackHeader
{
	//The only thing we really care about (atm) is the remote table redirection
	//So that is all the header will have
	unsigned char headerVersion;
	ULONG LsaVersion;
	dllPackDataLS functionTable;
};

int APIENTRY DLL_PACK(unsigned char** data, int* cbytes)
{
	DEBUG_MESSAGE("In DLL_PACK");
	dllPackHeader dPH;
	dPH.headerVersion = 2;
	dPH.LsaVersion = LSAVersion;
	dPH.functionTable.Start = 0;
	int ebytes = 0;
	if(privTable == NULL)
	{
		dPH.functionTable.Length = 0;
	}
	else
	{
		dPH.functionTable.Length = sizeof(functionTables);
		ebytes += dPH.functionTable.Length;
	}

	*data = (unsigned char*)calloc(sizeof(dPH) + ebytes, 1);
	unsigned char* estart = (unsigned char*)((int)data + sizeof(dPH));
	memcpy(*data, &dPH, sizeof(dPH));
	memcpy(estart + dPH.functionTable.Start, &RemoteFuncTables, dPH.functionTable.Length);
	*cbytes = sizeof(dPH) + ebytes;

	return 0;
}

int APIENTRY DLL_UNPACK(unsigned char* data, int cbytes)
{
	DEBUG_MESSAGE("In DLL_UNPACK");
	dllPackHeader dPH;
	if(cbytes < sizeof(dPH))
		return -1;
	memcpy(&dPH, data, sizeof(dPH));
	unsigned char* edata = (unsigned char*)((int)data + sizeof(dPH));
	if(dPH.headerVersion != 2)
	{
		return -2; //only works with version 2 atm
	}
	LSAVersion = dPH.LsaVersion;
	if(dPH.functionTable.Length == 0)
	{
		privTable = NULL;
	}
	else
	{
		memcpy(&RemoteFuncTables, edata + dPH.functionTable.Start, dPH.functionTable.Length);
	}
	return 0;
}

int APIENTRY DLL_FREE(unsigned char* data)
{
	DEBUG_MESSAGE("In DLL_FREE");
	SAFE_FREE(data);
	return 0;
}

//create everything that is required by this DLL at load time - currently nothing
int APIENTRY DLL_LOAD(void)
{
	DEBUG_MESSAGE("In DLL_LOAD");
	return 0;
}

//dispose of everything that is no longer required by this dll
int APIENTRY DLL_UNLOAD(void)
{
	DEBUG_MESSAGE("In DLL_UNLOAD");
	SAFE_FREE(privTable);
	return 0;
}

int APIENTRY DLL_REINIT(void* lsaBlob)
{
	DEBUG_MESSAGE("In DLL_REINIT");
	/*
	if(lsaBlob != NULL)
	{
		struct lsaB
		{
			ULONG LsaVersion;
			PULONG PackVer;
			PSECPKG_FUNCTION_TABLE* ppTables;
			PULONG cTables;
		};
		lsaB* plsaB = (lsaB*)lsaBlob;
		SpLsaModeInitialize(LsaVersion, plsaB->PackVer, plsaB->ppTables, plsaB->cTables);
	}
	*/
	initLocalPassthroughs();
	initRemovePassthroughsV2(&FuncTables.LsaModeTable, &RemoteFuncTables.LsaModeTable, sizeof(FuncTables.LsaModeTable) / sizeof(int*));
	initRemovePassthroughsV2(&FuncTables.UserModeTable, &RemoteFuncTables.UserModeTable, sizeof(FuncTables.UserModeTable) / sizeof(int*));
	memcpy(lsaBlob, &FuncTables, sizeof(FuncTables));
	return 0;
}
int APIENTRY DLL_FunctionTable(DLLFunctionTable* funcTable)
{
	funcTable->DLL_LOAD = &DLL_LOAD;
	funcTable->DLL_PACK = &DLL_PACK;
	funcTable->DLL_UNLOAD = &DLL_UNLOAD;
	funcTable->DLL_UNPACK = &DLL_UNPACK;
	funcTable->DLL_FREE = &DLL_FREE;
	funcTable->DLL_REINIT = &DLL_REINIT;
	return 0;
}

#endif