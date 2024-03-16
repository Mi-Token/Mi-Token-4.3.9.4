#pragma once

#include <windows.h>


//typedefs of the function types we support
struct DLLFunctionTable;

typedef int (APIENTRY *DLL_GetFunctionTable) (DLLFunctionTable* funcTable);
/*
typedef int (APIENTRY *DLL_PPBYTE_PINT) (BYTE**, int*);
typedef int (APIENTRY *DLL_PBYTE_INT) (BYTE*, int);
typedef int (APIENTRY *DLL_PBYTE) (BYTE*);
typedef int (APIENTRY *DLL_VOID) (void);
typedef int (APIENTRY *DLL_INT) (int);
typedef int (APIENTRY *DLL_PINT) (int*);
typedef int (APIENTRY *DLL_INT_INT) (int, int);
*/

//Functions that should be exported by the DLL
struct DLLFunctionTable
{
	int (APIENTRY *DLL_PACK)(BYTE** data, int* cbytes);
	int (APIENTRY *DLL_UNPACK)(BYTE* data, int cbytes);
	int (APIENTRY *DLL_FREE)(BYTE* data);
	int (APIENTRY *DLL_LOAD)(void);
	int (APIENTRY *DLL_UNLOAD)(void);
	int (APIENTRY *DLL_REINIT)(void* lsaBlob);
	//int (APIENTRY *InitPrivateStack)(int cbytes);
	//int (APIENTRY *WriteToStack)(int value, int location);
	//int (APIENTRY *GetResult)(int* result);
};

