// DebugCAPLoader.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windows.h>
#include "common_dll.h"
#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include "..\ConsoleAP\Passthroughs.h"
#include "..\ConsoleAP\DebugMessages.h"

wchar_t* libraryAddress = L"C:\\Debug\\MiTokenAP.dll";
wchar_t* libraryBoundAddress = L"C:\\Debug\\DebugCAP_bound.dll";
bool running;

HANDLE hMutex;
bool supportNewLoad;
HANDLE thread;
HMODULE hLibrary;
DLLFunctionTable functionTable;

#define SAFE_CALL_FUNCTION(func, run, result) if((func) != NULL) { result = run; } else { result = -1; }

struct lsaB
{
	ULONG LsaVersion;
	PULONG PackVer;
	PSECPKG_FUNCTION_TABLE* ppTables;
	PULONG cTables;
};

/*
	The following files allow us to call the remote DLL functions that
	are required for loading / unloading dlls
	They are NOT needed for the application interfacing with this dll
	-Hence they are not APIENTRY or exported in functionList or mutexLocked
*/
int DLL_LOAD();
int DLL_UNLOAD();
int DLL_FREE(BYTE* pByte);
int DLL_PACK(BYTE** data, int* cbytes);
int DLL_UNPACK(BYTE* data, int cbytes);
int DLL_REINIT(void* lsaBlob);

void waitForMutex()
{
	//only wait on supportNewLoad
	if(supportNewLoad)
	{
		WaitForSingleObject(hMutex, INFINITE);
	}
}

void releaseMutex()
{
	//We only need to release on supportNewLoad
	if(supportNewLoad)
	{
		ReleaseMutex(hMutex);
	}
}


int unloadLibrary()
{
	if(hLibrary != NULL)
	{
		if(FreeLibrary(hLibrary) == TRUE)
			return 0;
		return 1;
	}
}
bool copyLibrary()
{
	FILE* fp = NULL;
	fp  = _wfopen(libraryAddress, L"rb+" );
	if(fp == NULL)
	{
		return false;
	}
	
	fseek(fp, 0, SEEK_END);
	fpos_t loc;
	fgetpos(fp, &loc);
	BYTE* buffer = (BYTE*)calloc(loc, sizeof(BYTE));
	fseek(fp, 0, SEEK_SET);
	fread(buffer, sizeof(BYTE), loc, fp);
	fclose(fp);

	fp = NULL;
	fp = _wfopen(libraryBoundAddress, L"wb+");
	if(fp == NULL)
	{
		free(buffer);
		return false;
	}
	fwrite(buffer, sizeof(BYTE), loc, fp);
	fclose(fp);

	free(buffer);

	return true;
}

int loadLibrary()
{
	//firstly we need to move the library to a new location
	if(!copyLibrary())
		return -2;

	hLibrary = LoadLibrary(libraryBoundAddress);
	if(hLibrary == NULL)
	{
		//we couldn't load the library = do something
		return -1;
	}
	else
	{
		DLL_GetFunctionTable procFuncTable;
		procFuncTable = (DLL_GetFunctionTable)GetProcAddress(hLibrary, "DLL_FunctionTable");
		if(procFuncTable == NULL)
		{
			//we couldn't find the table - do something
			return -2;
		}
		else
		{
			procFuncTable(&functionTable);
			return 0;
		}
	}
}

bool newWrite(const FILETIME* pNewTime, const FILETIME* pOldTime)
{
	return (
				(pNewTime->dwHighDateTime > pOldTime->dwHighDateTime)
				||
				(
					(pNewTime->dwHighDateTime == pOldTime->dwHighDateTime)
					&&
					(pNewTime->dwLowDateTime > pOldTime->dwLowDateTime)
				)
			);

}
DWORD WINAPI _threadFunction(void* data)
{
	FILETIME lastWrite;
	memset(&lastWrite, 0, sizeof(lastWrite));

	while(running && supportNewLoad)
	{
		HANDLE fhandle = CreateFile(libraryAddress, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if(fhandle != NULL)
		{
			//we have a handle to the file. Check its last write time
			FILETIME curWrite;
			int ret = GetFileTime(fhandle, NULL, NULL, &curWrite);
			CloseHandle(fhandle); // close the handle
			if(ret == 0)
			{
				//we got an error
				//TODO : Find a nice way to log this
			}
			else
			{
				if(newWrite(&curWrite, &lastWrite))
				{
					//New file write, copy over the old last time
					memcpy(&lastWrite, &curWrite, sizeof(lastWrite));

					//lock the semaphore
					waitForMutex();

					//get the file's data
					BYTE* data = NULL;
					int bytes = 0;
					if(DLL_PACK(&data, &bytes) != 0)
					{
						//it didn't worked
						bytes = 0;
						data = NULL;
					}

					//create a local buffer to store the files data in
					BYTE* localBuffer = NULL;
					if(data != NULL)
					{
						localBuffer = (BYTE*)calloc(bytes, sizeof(BYTE));
						memcpy(localBuffer, data, bytes);
						DLL_FREE(data); //send the pointer back to the DLL for it to free it
					}


					//unload the library
					unloadLibrary();
					//load the new library
					loadLibrary();

					//give the new library the old libraries file data
					if(localBuffer != NULL)
					{
						DLL_UNPACK(localBuffer, bytes);
						free(localBuffer);
					}

					//unlock the semaphore
					releaseMutex();
					DLL_REINIT(NULL);
				}
			}		
		}

		Sleep(10000); //we'll sleep for 60 seconds
	}
	//CloseHandle(thread);
	return TRUE;
}

int Bind(void)
{
	DEBUG_LOG("Debug CAP Loader : In Bind\n");
	//Create a mutex
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if(hMutex == NULL)
	{
		//We cannot do runtime unbinding and rebinding... or anything else that requires the mutex
		//Not the best situation but we can live with it
		//Note : The bound DLL should have it's own mutex to stop multithreaded access where it cannot have it
		supportNewLoad = false;
	}
	else
	{
		supportNewLoad = true;
	
		mutexLock = waitForMutex;
		mutexUnlock = releaseMutex;

		running = true;
		thread = CreateThread(NULL, 0, &_threadFunction, NULL, 0, NULL);
		if(thread == NULL)
		{
			//we failed to create the new thread - turn off supportNewLoad
			supportNewLoad = false;
		}
		Sleep(1000);
	}
	if(!supportNewLoad)
	{
		//Normally we will just let the threading function load the new library
		//However without supportNewLoad on we know that the threaded function will not load it
		//So we will do it here
		loadLibrary();
	}
	return 0;

}

int Unbind(void)
{
	DEBUG_LOG("Debug CAP Loader : In UnBind\n");
	running = false;
	if(thread != NULL)
	{
		//waits for the thread to stop
		supportNewLoad = false;
		WaitForSingleObject(thread, 10000);
		CloseHandle(thread);
	}
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
	}
	return 0;
}







/*
	The following files allow us to call the remote DLL functions that
	are required for loading / unloading dlls
	They are NOT needed for the application interfacing with this dll
	-Hence they are not APIENTRY or exported in functionList or mutexLocked
*/

int DLL_LOAD()
{
	int ret;
	SAFE_CALL_FUNCTION(functionTable.DLL_LOAD, functionTable.DLL_LOAD(), ret);
	return ret;
}
int DLL_UNLOAD()
{
	int ret;
	SAFE_CALL_FUNCTION(functionTable.DLL_UNLOAD, functionTable.DLL_UNLOAD(), ret);
	return ret;
}
int DLL_FREE(BYTE* pByte)
{
	int ret;
	SAFE_CALL_FUNCTION(functionTable.DLL_FREE, functionTable.DLL_FREE(pByte), ret);
	return ret;
}

int DLL_PACK(BYTE** data, int* cbytes)
{
	int ret;
	SAFE_CALL_FUNCTION(functionTable.DLL_PACK, functionTable.DLL_PACK(data, cbytes), ret);
	return ret;
}
int DLL_UNPACK(BYTE* data, int cbytes)
{
	int ret;
	SAFE_CALL_FUNCTION(functionTable.DLL_UNLOAD, functionTable.DLL_UNPACK(data, cbytes), ret);
	return ret;
}

int DLL_REINIT(void* lsaData)
{
	int ret;
	void* d = calloc(1, sizeof(FuncTables));
	
	SAFE_CALL_FUNCTION(functionTable.DLL_REINIT, functionTable.DLL_REINIT(d), ret);
	
	memcpy(&RemoteFuncTables, d, sizeof(RemoteFuncTables));

	initRemovePassthroughsV2(&FuncTables.LsaModeTable, &RemoteFuncTables.LsaModeTable, sizeof(FuncTables.LsaModeTable) / sizeof(int*));
	initRemovePassthroughsV2(&FuncTables.UserModeTable, &RemoteFuncTables.UserModeTable, sizeof(FuncTables.UserModeTable) / sizeof(int*));
	return ret;
}

/*
int APIENTRY GetResult(int* result)
{
	int ret;
	waitForMutex();
	SAFE_CALL_FUNCTION(functionTable.GetResult, functionTable.GetResult(result), ret);
	releaseMutex();
	return ret;
}
int APIENTRY InitPrivateStack(int size)
{
	int ret;
	waitForMutex();
	SAFE_CALL_FUNCTION(functionTable.InitPrivateStack, functionTable.InitPrivateStack(size), ret);
	releaseMutex();
	return ret;
}
int APIENTRY WriteToStack(int position, int value)
{
	int ret;
	waitForMutex();
	SAFE_CALL_FUNCTION(functionTable.WriteToStack, functionTable.WriteToStack(position, value), ret);
	releaseMutex();
	return ret;
}
*/

/*
int APIENTRY DLL_FunctionTable(DLLFunctionTable* funcTable)
{
	assert(funcTable != 0);
	funcTable->DLL_LOAD = NULL;
	funcTable->DLL_PACK = NULL;
	funcTable->DLL_UNLOAD = NULL;
	funcTable->DLL_UNPACK = NULL;
	return 0;
}
*/