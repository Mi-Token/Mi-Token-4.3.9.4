// DebugCAP.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\DebugCAPLoader\common_dll.h"
#include <stdlib.h>

int* stack = NULL;
int stacksize;

#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { free(p); (p) = NULL; }
#endif

#define PACKED_VERSION_1 0x00000001

struct packedHeader
{
	int version;
	int dataLength; //length of packData structure + extra data
};

struct packedData_V1
{
	packedHeader header;
	int stackLength;
	int dataStart;
};


int APIENTRY DLL_PACK(BYTE** data, int* cbytes)
{
	packedData_V1 dV1;
	dV1.header.version = PACKED_VERSION_1;
	dV1.stackLength = stacksize;
	dV1.dataStart = sizeof(dV1);
	dV1.header.dataLength = sizeof(dV1) + (stacksize * sizeof(int));
	
	BYTE* pData = (BYTE*)calloc(dV1.header.dataLength, sizeof(BYTE));
	memcpy(pData, &dV1, sizeof(dV1));
	BYTE* pExData = (BYTE*)((int)pData) + dV1.dataStart;
	memcpy(pExData, stack, stacksize * sizeof(int));

	*data = pData;
	*cbytes = dV1.header.dataLength;
	return 0;
}

int UnpackVersion1(BYTE* data, int cbytes)
{
	packedData_V1 dV1;
	memcpy(&dV1, data, sizeof(dV1));
	BYTE* pbStack = (BYTE*)(((int)data) + dV1.dataStart);

	SAFE_FREE(stack);
	stacksize = dV1.stackLength;
	stack = (int*)calloc(dV1.stackLength, sizeof(int));
	memcpy(stack, pbStack, stacksize * sizeof(int));

	return 0;
}

int APIENTRY DLL_UNPACK(BYTE* data, int cbytes)
{
	int version = ((packedHeader*)data)->version;
	switch(version)
	{
	case PACKED_VERSION_1:
		return UnpackVersion1(data, cbytes);		
	default:
		//unknown version
		return -1;
		break;
	}
	return -2;
}

int APIENTRY DLL_FREE(BYTE* data)
{
	SAFE_FREE(data);
	return  0;
}
int APIENTRY DLL_LOAD(void)
{
	return -1;
}

int APIENTRY DLL_UNLOAD(void)
{
	return -1;
}



int APIENTRY InitPrivateStack(int cbytes)
{
	if(stack != NULL)
		free(stack);
	stack = (int*)calloc(cbytes, sizeof(int));
	stacksize = cbytes;
	return 0;
}

int APIENTRY WriteToStack(int location, int value)
{
	if(location >= stacksize)
		return -1;
	stack[location] = value;
	return 0;
}

int APIENTRY GetResult(int* result)
{
	*result = 0;
	for(int i = 0 ; i < stacksize ; ++i)
		*result = *result + ( 2 * stack[i] ) ;
	return 0;
}

int APIENTRY DLL_FunctionTable(DLLFunctionTable* funcTable)
{
	assert(funcTable != 0);
	funcTable->DLL_LOAD = &DLL_LOAD;
	funcTable->DLL_PACK = &DLL_PACK;
	funcTable->DLL_UNLOAD = &DLL_UNLOAD;
	funcTable->DLL_UNPACK = &DLL_UNPACK;
	funcTable->DLL_FREE = &DLL_FREE;
	return 0;
}

