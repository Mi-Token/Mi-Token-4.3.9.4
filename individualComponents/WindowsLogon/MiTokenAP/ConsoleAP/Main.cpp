
#include "Passthroughs.h"



#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

typedef PVOID *PLSA_CLIENT_REQUEST;

typedef NTSTATUS 
	(NTAPI SPLMI)(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);


typedef NTSTATUS 
	(NTAPI SPINT) (
		__in ULONG_PTR PackageId,
		__in PSECPKG_PARAMETERS Params,
		__in PLSA_SECPKG_FUNCTION_TABLE FunctionTable
		);
int main()
{
//	if(msvDll == INVALID_HANDLE_VALUE)
	//{
		//printf("msvDLL Failed to load");
	//}
	//else
	{
		
		/*
		ULONG_PTR packageID;
		SECPKG_PARAMETERS Params;
		LSA_SECPKG_FUNCTION_TABLE FunctionTable;

		SPINT* lpSPINT = (SPINT*)GetProcAddress(msvDll,"SpInitialize");
		
		packageID = 0;
		memset(&Params, 0, sizeof(Params));
		memset(&FunctionTable, 0, sizeof(FunctionTable));

		NTSTATUS nts = lpSPINT(packageID, &Params, &FunctionTable);
		*/

		ULONG LsaVersion = 0x10000;
		ULONG PackageVersion;
		PSECPKG_FUNCTION_TABLE pTables;
		ULONG cTables;
		/*
		SPLMI* lpSPLMI = (SPLMI*)GetProcAddress(msvDll, "SpLsaModeInitialize");
		LsaVersion = 0x1000; //brute force resulted in this working
		PackageVersion = 0;
		pTables = NULL;
		cTables = 0;
		NTSTATUS nts = 0xC000000D;
		while(nts == 0xC000000D)
		{
			nts = lpSPLMI(LsaVersion, &PackageVersion, &pTables, &cTables);
			LsaVersion++;
			if(LsaVersion > 0x100000)
				break;
		}

		printf("%08lX : %d", nts, LsaVersion);
		*/
		//init();
		/*
		ULONG packageVersion;
		PSECPKG_FUNCTION_TABLE pTables;
		ULONG cTables;
		*/

		SpLsaModeInitialize( LsaVersion, &PackageVersion, &pTables, &cTables);
		//FreeLibrary(msvDll);

		
		getchar();
	}

}