#pragma once

#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <NTSecAPI.h>

#define SECURITY_WIN32
#include <Sspi.h>

#include <wincred.h>

#include <NTSecPkg.h>

#include <stdio.h>
#include <stdlib.h>
#include "NamedPipeClient.h"

extern void (*mutexLock) (int);
extern void (*mutexUnlock) (void);

int init();

NTSTATUS __declspec(dllexport) NTAPI SpLsaModeInitialize(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);

NTSTATUS __declspec(dllexport) SEC_ENTRY SpUserModeInitialize(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_USER_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);


struct functionTables
{
	SECPKG_FUNCTION_TABLE LsaModeTable;
	SECPKG_USER_FUNCTION_TABLE UserModeTable;
	LSA_SECPKG_FUNCTION_TABLE SSPModeTable;
};

extern functionTables FuncTables, RemoteFuncTables, PreOverrideTables, PostOverrideTables;
extern SECPKG_FUNCTION_TABLE* privTable;
extern ULONG LSAVersion;

void initRemovePassthroughsV2(void* localTable, void* remoteTable, int entries);
void initLocalPassthroughs();

extern NamedPipeClient NPC;

extern LSA_SECPKG_FUNCTION_TABLE secpkg_fcn;
extern ULONG _packageID;