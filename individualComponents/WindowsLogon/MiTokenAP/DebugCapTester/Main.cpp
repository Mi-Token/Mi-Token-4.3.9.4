
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <NTSecAPI.h>

#define SECURITY_WIN32
#include <Sspi.h>

#include <NTSecPkg.h>

#include <stdio.h>
#include <stdlib.h>

//#include "..\DebugCAPLoader\common_dll.h"
//typedef int (APIENTRY *MYPROC)(void);

typedef NTSTATUS (NTAPI *SpLsaModeInitialize)(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);

typedef NTSTATUS (NTAPI *SpInitialize)(
  __in  ULONG_PTR PackageId,
  __in  PSECPKG_PARAMETERS Parameters,
  __in  PLSA_SECPKG_FUNCTION_TABLE FunctionTable
);

void clearInput(char old)
{
	while(old != 10)
		old = getchar();
}


void mainFunction(HMODULE libHandle)
{
	//this is where the main work of this program will go in regards to calling the Debug DLL Loader
	//DLLFunctionTable funcTable;
	//DLL_GetFunctionTable ProcFuncTable = (DLL_GetFunctionTable)GetProcAddress(libHandle, "SpLsaModeInitialize");
	SpLsaModeInitialize SLMI = (SpLsaModeInitialize)GetProcAddress(libHandle, "SpLsaModeInitialize");
	SpInitialize SPI = (SpInitialize)GetProcAddress(libHandle, "SpInitialize");

	if(SLMI == NULL)
		return;
	ULONG packV;
	PSECPKG_FUNCTION_TABLE pTab;
	ULONG cTab;
	SLMI(0x10000, &packV, &pTab, &cTab);

	if(SPI == NULL)
		return;

	
	ULONG_PTR id = 0x12345678;
	SECPKG_PARAMETERS p = {0};
	LSA_SECPKG_FUNCTION_TABLE ft = {0};

	SPI(id, &p, &ft);
	printf("All Ready for user input\n");
	char c;
	while((c = getchar()) != 'q')
	{
		clearInput(c);
	}

}
int main()
{
	HMODULE libHandle;
	//getchar();
	
	libHandle = LoadLibrary("C:\\Mi-Token\\mitoken-central\\WindowsLogon\\MiTokenAP\\x64\\Release\\MiTokenAP.dll");

	if(libHandle == NULL)
	{
		printf("Failed, GetLastError = %d\n", GetLastError());
	}
	else
	{
		printf("Success, Library Loaded Successfully\n");

		mainFunction(libHandle);
		FreeLibrary(libHandle);
	}

	getchar();
}