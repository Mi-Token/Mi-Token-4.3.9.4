

#include "DebugMessages.h"
#include "NamedPipeClient.h"
#include "Passthroughs.h"
#include "Wrappers.h"
#include <assert.h>

#include <NTSecPkg.h>

#include <time.h>


NamedPipeClient NPC;

NTSTATUS NTAPI SpAcceptCredentials(
  __in  SECURITY_LOGON_TYPE LogonType,
  __in  PUNICODE_STRING AccountName,
  __in  PSECPKG_PRIMARY_CRED PrimaryCredentials,
  __in  PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials
);

NTSTATUS NTAPI SpInitialize(
  __in  ULONG_PTR PackageId,
  __in  PSECPKG_PARAMETERS Parameters,
  __in  PLSA_SECPKG_FUNCTION_TABLE FunctionTable
);


#define PassthroughFunction(functionType, functionName, typedParameters, functionTable, functionTableName,  Parameters)	\
	functionType functionName typedParameters { return functionTable -> functionTableName Parameters ; }

//Smaller PTF Definition.
//The table the user for all the PTFs
//#define PTFTable privTable;

void blankFunc()
{}
HANDLE mutex = CreateMutex(NULL, false, NULL);

void lockMutex(int uid)
{
	LARGE_INTEGER HF;
	QueryPerformanceCounter(&HF);
	DEBUG_VERBOSE("Locking Mutex [%08lX] @ [%08lX:%08lX]\n", uid, HF.HighPart, HF.LowPart);
	//WaitForSingleObject(mutex, INFINITE);
	DEBUG_VERBOSE("Mutex Locked [%08lX]\n", uid);
	QueryPerformanceFrequency(&HF);
	DEBUG_VERBOSE("Counter Freq [%08lX:%08lX]\n", HF.HighPart, HF.LowPart);
}

void unlockMutex()
{
	DEBUG_VERBOSE("Unlocking Mutex\n");
	//ReleaseMutex(mutex);
	DEBUG_VERBOSE("Mutex Unlocked\n");
}

void (*mutexLock) (int) = lockMutex;
void (*mutexUnlock) (void) = unlockMutex;

//void* createHotPatch(void* functionToPatch, void* destination, void** callToAdd);
/*
void* rtlFreeHeapRetAdd;
BOOLEAN __stdcall RtlFreeHeapHook(PVOID HeapHandle, ULONG Flags, PVOID HeapBase);

void* rtlAllocHeapRetAdd;
PVOID __stdcall RtlAllocHeapHook(PVOID HeapHandle, ULONG Flags, SIZE_T size);
*/

#ifndef _DllName
#define DllName "MiTokenAP.dll"
#else
#define DllName "Debug CAP Loader.dll"
#endif

#define DEBUG_MESSAGE_INFUNC(function) DEBUG_MESSAGE(DllName " : Debug In Function : " function);

ULONG LSAVersion;

int passHitCount = 0;

#define PTF(functionType, calltype, table, subTable, tablePrefix, functionName, typedParameters, parameters) \
	functionType calltype ptf_ ## tablePrefix ## _ ## functionName typedParameters { \
		mutexLock((int)(table . subTable . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HitC = %04d]}\n : Debug In Function : [" #tablePrefix "] [%c:%c]" #functionName, tbuffer, passHitCount, (PreOverrideTables . subTable . functionName) ? 'E' : '-', (PostOverrideTables . subTable . functionName) ? 'O' : '-'); \
		if((PreOverrideTables . subTable . functionName) != NULL) \
		{ \
			PreOverrideTables . subTable . functionName parameters ; \
		} \
		functionType o = table . subTable . functionName parameters ; \
		if((PostOverrideTables . subTable . functionName) != NULL) \
		{ \
			PostOverrideTables . subTable . functionName parameters; \
		} \
		DEBUG_LOG(" (NTS = %08lX)\n", o); \
		mutexUnlock(); \
		return o; \
	} \
	functionType calltype ptf_ ## tablePrefix ## _ ## functionName ## _blank typedParameters { \
		mutexLock((int)(table . subTable . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HitC = %04d]}\n : Debug In Function : [" #tablePrefix "] " #functionName " (BLANK)\n", tbuffer, passHitCount); \
		mutexUnlock(); \
		return 0; \
	} 

//Same as PTF but without storing the result of the passed call. (Done to Allow MutexLocking)
#define PTFVoid(calltype, table, subTable, tablePrefix, functionName, typedParameters, parameters) \
	void calltype ptf_ ## tablePrefix ## _ ## functionName typedParameters { \
		mutexLock((int)(table . subTable . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HC = %04d]}\n : Debug In Function : [" #tablePrefix "] " #functionName " (void)\n", tbuffer, passHitCount); \
		if((PreOverrideTables . subTable . functionName) != NULL) \
		{ \
			PreOverrideTables . subTable . functionName parameters ; \
		} \
		table . subTable . functionName parameters; \
		if((PostOverrideTables . subTable . functionName) != NULL) \
		{ \
			PostOverrideTables . subTable . functionName parameters; \
		} \
		mutexUnlock(); \
	} \
	void calltype ptf_ ## tablePrefix ## _ ## functionName ## _blank typedParameters { \
		mutexLock((int)(table . subTable . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HC = %04d]}\n : Debug In Function : [" #tablePrefix "] " #functionName " (BLANK)\n", tbuffer, passHitCount); \
		mutexUnlock(); \
	}

#define RemoveIfNoPassthrough(msvPointer, localTable, functionName) \
	if( msvPointer -> functionName == NULL) localTable -> functionName = NULL;

#define SetPTF(localTable, tablePrefix, functionName) \
	localTable . functionName = & ( ptf_ ## tablePrefix ## _ ## functionName ) ;

#define SetPTFB(localTable, tablePrefix, functionName) \
	localTable . functionName = & ( ptf_ ## tablePrefix ## _ ## functionName ## _blank );


#include <stdlib.h>
#include <stdio.h>


//Defining DLLBindName as a value will override the default DLL of msv1_0.dll
#ifdef DLLBindDebug
#define DLLBind L"C:\\Debug\\DebugCAP_bound.dll"
#else
#define DLLBind L"C:\\Windows\\System32\\msv1_0.dll"
#endif

ULONG PACKAGE_VERSION = 0x00000005;
PSECPKG_FUNCTION_TABLE removeTables;
ULONG removeTableLength;

HMODULE msvDll = NULL;

SECPKG_FUNCTION_TABLE* privTable;

/*struct functionTables
{
	SECPKG_FUNCTION_TABLE LsaModeTable;
	SECPKG_USER_FUNCTION_TABLE UserModeTable;
	LSA_SECPKG_FUNCTION_TABLE SSPModeTable;
}*/
functionTables FuncTables, RemoteFuncTables;
functionTables PreOverrideTables = {0};
functionTables PostOverrideTables = {0};
//Passthrough Functions
//Function can be referenced as ptf_(NormalFunctionName)
//Variable names are kept short because we dont even use them
//Really, don't touch anything you dont need to - these functions will just call whatever is in privTable and return its result

//LSA Mode Function Table
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, InitializePackage, (ULONG APID, PLSA_DISPATCH_TABLE LSADT, PLSA_STRING DB, PLSA_STRING Con, PLSA_STRING * APN), (APID, LSADT, DB, Con, APN));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, LogonUser, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID AI, PVOID CAB, ULONG AIL, PVOID* PB, PULONG PBL, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PLSA_UNICODE_STRING* AN, PLSA_UNICODE_STRING* AA), (CR, LT, AI, CAB, AIL, PB, PBL, LID, SS, TIT, TI, AN, AA));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, CallPackage, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (CR, PSB, CBB, SBL, PRB, RBL, PS));
PTFVoid(      __stdcall, RemoteFuncTables, LsaModeTable, LSA, LogonTerminated, (PLUID LID), (LID));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, CallPackageUntrusted, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (CR, PSB, CBB, SBL, PRB, RBL, PS));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, CallPackagePassthrough, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (CR, PSB, CBB, SBL, PRB, RBL, PS));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, LogonUserEx, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID AI, PVOID CAB, ULONG AIL, PVOID* PB, PULONG PBL, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PUNICODE_STRING* AN, PUNICODE_STRING* AA, PUNICODE_STRING* MN), (CR, LT, AI, CAB, AIL, PB, PBL, LID, SS, TIT, TI, AN, AA, MN));

/*
Note:
	If the following line fails to compile due to LogonUserEx2 being defined as __cdecl not __stdcall you will need to
	Open up your windows SDK Include Folder (C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include on my system)
	Open up NTSecPkg.h in any text editor (may need Elevated Privileges to write to this file)
	Goto the declaration of LSA_AP_LOGON_USER_EX2 (Line 1496 in SDK v7.0A)
		typedef NTSTATUS
		(LSA_AP_LOGON_USER_EX2) (
	And edit it to be an __stdcall (NTAPI is the typedef normally used)
		typedef NTSTATUS
		(NTAPI LSA_AP_LOGON_USER_EX2) (

	You can blame Microsoft for giving out a dodgy SDK with invalid header files :P

	(Needed in SDKs upto at least V7.0A)
*/
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, LogonUserEx2, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID PSB, PVOID CBB, ULONG SBS, PVOID* PB, PULONG PBS, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PUNICODE_STRING*AN, PUNICODE_STRING* AA, PUNICODE_STRING* MA, PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED_ARRAY* SCA), (CR, LT, PSB, CBB, SBS, PB, PBS, LID, SS, TIT, TI, AN, AA, MA, PC, SCA));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, Initialize, (ULONG_PTR PID, PSECPKG_PARAMETERS P, PLSA_SECPKG_FUNCTION_TABLE FT), (PID, P, FT));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, Shutdown, (VOID), ());
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, GetInfo, (PSecPkgInfo PI), (PI));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, AcceptCredentials, (SECURITY_LOGON_TYPE LT, PUNICODE_STRING AN, PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED SC), (LT, AN, PC, SC));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, AcquireCredentialsHandle, (PUNICODE_STRING PN, ULONG CUF, PLUID LID, PVOID AD, PVOID GKF, PVOID GKA, PLSA_SEC_HANDLE CH, PTimeStamp ET), (PN, CUF, LID, AD, GKF, GKA, CH, ET));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, QueryCredentialsAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B), (CH, CA, B));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, FreeCredentialsHandle, (LSA_SEC_HANDLE CH), (CH));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, SaveCredentials, (LSA_SEC_HANDLE CH, PSecBuffer C), (CH, C));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, GetCredentials, (LSA_SEC_HANDLE CH, PSecBuffer C), (CH, C));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, DeleteCredentials, (LSA_SEC_HANDLE CH, PSecBuffer K), (CH, K));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, InitLsaModeContext, (LSA_SEC_HANDLE CH1, LSA_SEC_HANDLE CH2, PUNICODE_STRING TN, ULONG CR, ULONG TDR, PSecBufferDesc IB, PLSA_SEC_HANDLE NCH, PSecBufferDesc OB, PULONG CA, PTimeStamp ET, PBOOLEAN MC, PSecBuffer CD), (CH1, CH2, TN, CR, TDR, IB, NCH, OB, CA, ET, MC, CD));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, AcceptLsaModeContext, (LSA_SEC_HANDLE CH1, LSA_SEC_HANDLE CH2, PSecBufferDesc IB, ULONG CR, ULONG TDR, PLSA_SEC_HANDLE NCH, PSecBufferDesc OB, PULONG CA, PTimeStamp ET, PBOOLEAN MC, PSecBuffer CD), (CH1, CH2, IB, CR, TDR, NCH, OB, CA, ET, MC, CD));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, DeleteContext, (LSA_SEC_HANDLE CH), (CH));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, ApplyControlToken, (LSA_SEC_HANDLE CH, PSecBufferDesc CT), (CH, CT));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, GetUserInfo, (PLUID LID, ULONG F, PSecurityUserData* UD), (LID, F, UD));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, GetExtendedInformation, (SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION* I), (C, I));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, QueryContextAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B), (CH, CA, B));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, AddCredentials, (LSA_SEC_HANDLE CH, PUNICODE_STRING PN, PUNICODE_STRING P, ULONG CUF, PVOID AD, PVOID GKF, PVOID GKA, PTimeStamp ET), (CH, PN, P, CUF, AD, GKF, GKA, ET));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, SetExtendedInformation, (SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION I), (C, I));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, SetContextAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B, ULONG BS), (CH, CA, B, BS));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, SetCredentialsAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B, ULONG BS), (CH, CA, B, BS));

/*
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, QueryMetaData, (LSA_SEC_HANDLE CH, PUNICODE_STRING TN, ULONG CR, PULONG MDL, PUCHAR* MD, PLSA_SEC_HANDLE CH1), (CH, TN, CR, MDL, MD, CH1));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, ExchangeMetaData, (LSA_SEC_HANDLE CH, PUNICODE_STRING TN, ULONG CR, ULONG MDL, PUCHAR MD, PLSA_SEC_HANDLE CH1), (CH, TN, CR, MDL, MD, CH1));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, GetCredUIContext, (LSA_SEC_HANDLE CH, GUID* CT, PULONG FCUICL, PUCHAR* FCUIC), (CH, CT, FCUICL, FCUIC));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, UpdateCredentials, (LSA_SEC_HANDLE CH, GUID* CT, ULONG FCUICL, PUCHAR FCUIC), (CH, CT, FCUICL, FCUIC));
PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, ValidateTargetInfo, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PSECPKG_TARGETINFO TI), (CR, PSB, CBB, SBL, TI));
*/

PTF(NTSTATUS, __stdcall, RemoteFuncTables, LsaModeTable, LSA, ChangeAccountPasswordW, (PUNICODE_STRING DN, PUNICODE_STRING AN, PUNICODE_STRING OP, PUNICODE_STRING NP, BOOLEAN I, PSecBufferDesc O), (DN, AN, OP, NP, I, O));

//LSA SSP FUNCTION TABLE
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CreateLogonSession, (PLUID LID), (LID));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, DeleteLogonSession, (PLUID LID), (LID));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, AddCredential, (PLUID LID, ULONG AP, PLSA_STRING PKV, PLSA_STRING C), (LID, AP, PKV, C));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetCredentials, (PLUID LID, ULONG AP, PULONG QC, BOOLEAN RAC, PLSA_STRING PKV, PULONG PKL, PLSA_STRING C), (LID, AP, QC, RAC, PKV, PKL, C));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, DeleteCredential, (PLUID LID, ULONG AP, PLSA_STRING PKV), (LID, AP, PKV));
PTF(PVOID   , NTAPI, RemoteFuncTables, SSPModeTable, SSP, AllocateLsaHeap, (ULONG L), (L));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, FreeLsaHeap, (PVOID B), (B));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, AllocateClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG LR, PVOID* CBA), (CR, LR, CBA));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, FreeClientBuffer, (PLSA_CLIENT_REQUEST CR, PVOID CBA), (CR, CBA));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CopyToClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG L, PVOID CBA, PVOID BTC), (CR, L, CBA, BTC));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CopyFromClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG L, PVOID BTC, PVOID CBA), (CR, L, BTC, CBA));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, ImpersonateClient, (), ());
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, UnloadPackage, (), ());
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, DuplicateHandle, (HANDLE SH, PHANDLE DH), (SH, DH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, SaveSupplementalCredentials, (PLUID LID, ULONG SCS, PVOID SC, BOOLEAN S), (LID, SCS, SC, S));
PTF(HANDLE  , NTAPI, RemoteFuncTables, SSPModeTable, SSP, CreateThread, (SEC_ATTRS SA, ULONG SS, SEC_THREAD_START SF, PVOID TP, ULONG CF, PULONG TID), (SA, SS, SF, TP, CF, TID));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetClientInfo, (PSECPKG_CLIENT_INFO CI), (CI));
PTF(HANDLE  , NTAPI, RemoteFuncTables, SSPModeTable, SSP, RegisterNotification, (SEC_THREAD_START SF, PVOID P, ULONG NT, ULONG NC, ULONG NF, ULONG IM, HANDLE WE), (SF, P, NT, NC, NF, IM, WE));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CancelNotification, (HANDLE NH), (NH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, MapBuffer, (PSecBuffer IB, PSecBuffer OB), (IB, OB));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CreateToken, (PLUID LID, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, SECURITY_IMPERSONATION_LEVEL IL, LSA_TOKEN_INFORMATION_TYPE TIT, PVOID TI, PTOKEN_GROUPS TG, PUNICODE_STRING AN, PUNICODE_STRING AN1, PUNICODE_STRING W, PUNICODE_STRING PP, PHANDLE T, PNTSTATUS SS), (LID, TS, LT, IL, TIT, TI, TG, AN, AN1, W, PP, T, SS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, AuditLogon, (NTSTATUS S, NTSTATUS SS, PUNICODE_STRING AN, PUNICODE_STRING AA, PUNICODE_STRING WN, PSID USID, SECURITY_LOGON_TYPE LT, PTOKEN_SOURCE TS, PLUID LID), (S, SS, AN, AA, WN, USID, LT, TS, LID));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CallPackage, (PUNICODE_STRING AP, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, PSB, SBL, PRB, RBL, PS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, FreeReturnBuffer, (PVOID B), (B));
PTF(BOOLEAN , NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetCallInfo, (PSECPKG_CALL_INFO I),(I));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CallPackageEx, (PUNICODE_STRING AP, PVOID CBB, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, CBB, PSB, SBL, PRB, RBL, PS));
PTF(PVOID   , NTAPI, RemoteFuncTables, SSPModeTable, SSP, CreateSharedMemory, (ULONG MS, ULONG IS), (MS, IS));
PTF(PVOID   , NTAPI, RemoteFuncTables, SSPModeTable, SSP, AllocateSharedMemory, (PVOID SM, ULONG S), (SM, S));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, FreeSharedMemory, (PVOID SM, PVOID M), (SM, M));
PTF(BOOLEAN , NTAPI, RemoteFuncTables, SSPModeTable, SSP, DeleteSharedMemory, (PVOID SM), (SM));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, OpenSamUser, (PSECURITY_STRING N, SECPKG_NAME_TYPE NT, PSECURITY_STRING P, BOOLEAN AG, ULONG R, PVOID* UH), (N, NT, P, AG, R, UH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetUserCredentials, (PVOID UH, PVOID* PC, PULONG PCS, PVOID* SC, PULONG SCS), (UH, PC, PCS, SC, SCS));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetUserAuthData, (PVOID UH, PUCHAR* UAD, PULONG UADS), (UH, UAD, UADS));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CloseSamUser, (PVOID UH), (UH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, ConvertAuthDataToToken, (PVOID UAD, ULONG UADS, SECURITY_IMPERSONATION_LEVEL IL, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, PUNICODE_STRING AN, PHANDLE T, PLUID LID, PUNICODE_STRING AN1, PNTSTATUS SS), (UAD, UADS, IL, TS, LT, AN, T, LID, AN1, SS));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, ClientCallback, (PCHAR C, ULONG_PTR A1, ULONG_PTR A2, PSecBuffer I, PSecBuffer O), (C, A1, A2, I, O));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, UpdateCredentials, (PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED_ARRAY C), (PC, C));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, GetAuthDataForUser, (PSECURITY_STRING N, SECPKG_NAME_TYPE NT, PSECURITY_STRING P, PUCHAR* UAD, PULONG UADS, PUNICODE_STRING UFN), (N, NT, P, UAD, UADS, UFN));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrackSingleName, (ULONG FO, BOOLEAN PAGC, PUNICODE_STRING NI, PUNICODE_STRING P, ULONG RF, PUNICODE_STRING CN, PUNICODE_STRING DDN, PULONG SS), (FO, PAGC, NI, P, RF, CN, DDN, SS));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, AuditAccountLogon, (ULONG AID, BOOLEAN S, PUNICODE_STRING S1, PUNICODE_STRING CN, PUNICODE_STRING MN, NTSTATUS S2), (AID, S, S1, CN, MN, S2));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CallPackagePassthrough, (PUNICODE_STRING AP, PVOID CBB, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, CBB, PSB, SBL, PRB, RBL, PS));
#ifdef _WINCRED_H_
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrediRead, (PLUID LID, ULONG CF, LPWSTR TN, ULONG T, ULONG F, PENCRYPTED_CREDENTIALW *C), (LID, CF, TN, T, F, C));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrediReadDomainCredentials, (PLUID LID, ULONG CF, PCREDENTIAL_TARGET_INFORMATIONW TI, ULONG F, PULONG C, PENCRYPTED_CREDENTIALW **C1), (LID, CF, TI, F, C, C1));
PTFVoid(	  NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrediFreeCredentials, (ULONG C, PENCRYPTED_CREDENTIALW * C1), (C, C1));
#else
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, DummyFunction1, (PVOID B, ULONG BS), (B, BS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, DummyFunction2, (PVOID B, ULONG BS), (B, BS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, DummyFunction3, (PVOID B, ULONG BS), (B, BS));
#endif
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, LsaProtectMemory, (PVOID B, ULONG BS), (B, BS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, LsaUnprotectMemory, (PVOID B, ULONG BS), (B, BS));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, OpenTokenByLogonId, (PLUID LID, HANDLE* RTH), (LID, RTH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, ExpandAuthDataForDomain, (PUCHAR UAD, ULONG UADS, PVOID R, PUCHAR* EAD, PULONG EADS), (UAD, UADS, R, EAD, EADS));
PTF(PVOID   , NTAPI, RemoteFuncTables, SSPModeTable, SSP, AllocatePrivateHeap, (SIZE_T L), (L));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, FreePrivateHeap, (PVOID B), (B));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CreateTokenEx, (PLUID LID, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, SECURITY_IMPERSONATION_LEVEL IL, LSA_TOKEN_INFORMATION_TYPE TIT, PVOID TI, PTOKEN_GROUPS TG, PUNICODE_STRING W, PUNICODE_STRING PP, PVOID SI, SECPKG_SESSIONINFO_TYPE SIT, PHANDLE T, PNTSTATUS SS), (LID, TS, LT, IL, TIT, TI, TG, W, PP, SI, SIT, T, SS));
#ifdef _WINCRED_H_
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrediWrite, (PLUID LID, ULONG CF, PENCRYPTED_CREDENTIALW C, ULONG F), (LID, CF, C, F));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, SSPModeTable, SSP, CrediUnmarshalandDecodeString, (LPWSTR MS, LPBYTE * B, ULONG* BS, BOOLEAN * IFF), (MS, B, BS, IFF));
#else
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, DummyFunction4, (PVOID B, ULONG BS), (B, BS));
PTFVoid(      NTAPI, RemoteFuncTables, SSPModeTable, SSP, DummyFunction5, (PVOID B, ULONG BS), (B, BS));
#endif

//USER FUNCTION TABLE
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, InstanceInit, (ULONG V, PSECPKG_DLL_FUNCTIONS FT, PVOID * UF), (V, FT, UF));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, InitUserModeContext, (LSA_SEC_HANDLE CH, PSecBuffer PC), (CH, PC));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, MakeSignature, (LSA_SEC_HANDLE CH, ULONG QOP, PSecBufferDesc MB, ULONG MSN), (CH, QOP, MB, MSN));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, VerifySignature, (LSA_SEC_HANDLE CH, PSecBufferDesc MB, ULONG MSN, PULONG QOP), (CH, MB, MSN, QOP));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, SealMessage, (LSA_SEC_HANDLE CH, ULONG QOP, PSecBufferDesc MB, ULONG MSN), (CH, QOP, MB, MSN));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, UnsealMessage, (LSA_SEC_HANDLE CH, PSecBufferDesc MB, ULONG MSN, PULONG QOP), (CH, MB, MSN, QOP));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, GetContextToken, (LSA_SEC_HANDLE CH, PHANDLE IT), (CH, IT));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, QueryContextAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B), (CH, CA, B));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, CompleteAuthToken, (LSA_SEC_HANDLE CH, PSecBufferDesc IB), (CH, IB));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, DeleteUserModeContext, (LSA_SEC_HANDLE CH), (CH));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, FormatCredentials, (PSecBuffer C, PSecBuffer FC), (C, FC));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, MarshallSupplementalCreds, (ULONG CS, PUCHAR C, PULONG MCS, PVOID* MC), (CS, C, MCS, MC));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, ExportContext, (LSA_SEC_HANDLE C, ULONG F, PSecBuffer PC, PHANDLE T), (C, F, PC, T));
PTF(NTSTATUS, NTAPI, RemoteFuncTables, UserModeTable, USR, ImportContext, (PSecBuffer PC, HANDLE T, PLSA_SEC_HANDLE C), (PC, T, C));















/*
PassthroughFunction(NTSTATUS, ptf_LsaApInitializePackage, (ULONG APID, PLSA_DISPATCH_TABLE LSADT, PLSA_STRING DB, PLSA_STRING Con, PLSA_STRING * APN), privTable, InitializePackage, (APID, LSADT, DB, Con, APN));
PassthroughFunction(NTSTATUS, ptf_LogonUser, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID AI, PVOID CAB, ULONG AIL, PVOID* PB, PULONG PBL, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PLSA_UNICODE_STRING* AN, PLSA_UNICODE_STRING* AA), privTable, LogonUser,(CR, LT, AI, CAB, AIL, PB, PBL, LID, SS, TIT, TI, AN, AA));
PassthroughFunction(NTSTATUS, ptf_CallPackage, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), privTable, CallPackage, (CR, PSB, CBB, SBL, PRB, RBL, PS));
PassthroughFunction(VOID, ptf_LogonTerminated, (PLUID LID), privTable, LogonTerminated, (LID));
PassthroughFunction(NTSTATUS, ptf_CallPackageUntrusted, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), privTable, CallPackageUntrusted, (CR, PSB, CBB, SBL, PRB, RBL, PS));
PassthroughFunction(NTSTATUS, ptf_CallPacakgePassthrough, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), privTable, CallPackagePassthrough, (CR, PSB, CBB, SBL, PRB, RBL, PS));
PassthroughFunction(NTSTATUS, ptf_LogonUserEx, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID AI, PVOID CAB, ULONG AIL, PVOID* PB, PULONG PBL, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PUNICODE_STRING* AN, PUNICODE_STRING* AA, PUNICODE_STRING* MN), privTable, LogonUserEx, (CR, LT, AI, CAB, AIL, PB, PBL, LID, SS, TIT, TI, AN, AA, MN));
PassthroughFunction(NTSTATUS, ptf_LogonUserEx2, (PLSA_CLIENT_REQUEST CR, SECURITY_LOGON_TYPE LT, PVOID PSB, PVOID CBB, ULONG SBS, PVOID* PB, PULONG PBS, PLUID LID, PNTSTATUS SS, PLSA_TOKEN_INFORMATION_TYPE TIT, PVOID* TI, PUNICODE_STRING*AN, PUNICODE_STRING* AA, PUNICODE_STRING* MA, PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED_ARRAY* SCA), privTable, LogonUserEx2, (CR, LT, PSB, CBB, SBS, PB, PBS, LID, SS, TIT, TI, AN, AA, MA, PC, SCA));
PassthroughFunction(NTSTATUS, ptf_Initialize, (ULONG_PTR PID, PSECPKG_PARAMETERS P, PLSA_SECPKG_FUNCTION_TABLE FT), privTable, Initialize, (PID, P, FT));
PassthroughFunction(NTSTATUS, ptf_Shutdown, (VOID), privTable, Shutdown, ());
PassthroughFunction(NTSTATUS, ptf_GetInfo, (PSecPkgInfo PI), privTable, GetInfo, (PI));
PassthroughFunction(NTSTATUS, ptf_AcceptCredentials, (SECURITY_LOGON_TYPE LT, PUNICODE_STRING AN, PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED SC), privTable, AcceptCredentials, (LT, AN, PC, SC));
PassthroughFunction(NTSTATUS, ptf_AcquireCredentialsHandle, (PUNICODE_STRING PN, ULONG CUF, PLUID LID, PVOID AD, PVOID GKF, PVOID GKA, PLSA_SEC_HANDLE CH, PTimeStamp ET), privTable, AcquireCredentialsHandle, (PN, CUF, LID, AD, GKF, GKA, CH, ET));
PassthroughFunction(NTSTATUS, ptf_QueryCredentialsAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B), privTable, QueryCredentialsAttributes, (CH, CA, B));
PassthroughFunction(NTSTATUS, ptf_FreeCredentialsHandle, (LSA_SEC_HANDLE CH), privTable, FreeCredentialsHandle, (CH));
PassthroughFunction(NTSTATUS, ptf_SaveCredentials, (LSA_SEC_HANDLE CH, PSecBuffer C), privTable, SaveCredentials, (CH, C));
PassthroughFunction(NTSTATUS, ptf_GetCredentials, (LSA_SEC_HANDLE CH, PSecBuffer C), privTable, GetCredentials, (CH, C));
PassthroughFunction(NTSTATUS, ptf_DeleteCredentials, (LSA_SEC_HANDLE CH, PSecBuffer K), privTable, DeleteCredentials, (CH, K));
PassthroughFunction(NTSTATUS, ptf_InitLsaModeContex, (LSA_SEC_HANDLE CH1, LSA_SEC_HANDLE CH2, PUNICODE_STRING TN, ULONG CR, ULONG TDR, PSecBufferDesc IB, PLSA_SEC_HANDLE NCH, PSecBufferDesc OB, PULONG CA, PTimeStamp ET, PBOOLEAN MC, PSecBuffer CD), privTable, InitLsaModeContext, (CH1, CH2, TN, CR, TDR, IB, NCH, OB, CA, ET, MC, CD));
PassthroughFunction(NTSTATUS, ptf_AcceptLsaModeContext, (LSA_SEC_HANDLE CH1, LSA_SEC_HANDLE CH2, PSecBufferDesc IB, ULONG CR, ULONG TDR, PLSA_SEC_HANDLE NCH, PSecBufferDesc OB, PULONG CA, PTimeStamp ET, PBOOLEAN MC, PSecBuffer CD), privTable, AcceptLsaModeContext, (CH1, CH2, IB, CR, TDR, NCH, OB, CA, ET, MC, CD));
PassthroughFunction(NTSTATUS, ptf_DeleteContext, (LSA_SEC_HANDLE CH), privTable, DeleteContext, (CH));
PassthroughFunction(NTSTATUS, ptf_ApplyControlToken, (LSA_SEC_HANDLE CH, PSecBufferDesc CT), privTable, ApplyControlToken, (CH, CT));
PassthroughFunction(NTSTATUS, ptf_GetUserInfo, (PLUID LID, ULONG F, PSecurityUserData* UD), privTable, GetUserInfo, (LID, F, UD));
PassthroughFunction(NTSTATUS, ptf_GetExtendedInformation, (SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION* I), privTable, GetExtendedInformation, (C, I));
PassthroughFunction(NTSTATUS, ptf_QueryContextAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B), privTable, QueryContextAttributes, (CH, CA, B));
PassthroughFunction(NTSTATUS, ptf_AddCredentials, (LSA_SEC_HANDLE CH, PUNICODE_STRING PN, PUNICODE_STRING P, ULONG CUF, PVOID AD, PVOID GKF, PVOID GKA, PTimeStamp ET), privTable, AddCredentials, (CH, PN, P, CUF, AD, GKF, GKA, ET));
PassthroughFunction(NTSTATUS, ptf_SetExtendedInformation, (SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION I), privTable, SetExtendedInformation, (C, I));
PassthroughFunction(NTSTATUS, ptf_SetContextAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B, ULONG BS), privTable, SetContextAttributes, (CH, CA, B, BS));
PassthroughFunction(NTSTATUS, ptf_SetCredentialsAttributes, (LSA_SEC_HANDLE CH, ULONG CA, PVOID B, ULONG BS), privTable, SetCredentialsAttributes, (CH, CA, B, BS));
//PassthroughFunction(NTSTATUS, ptf_SetExtendedInformation, (SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION I), privTable, SetExtendedInformation, (C, I));
PassthroughFunction(NTSTATUS, ptf_QueryMetaData, (LSA_SEC_HANDLE CH, PUNICODE_STRING TN, ULONG CR, PULONG MDL, PUCHAR* MD, PLSA_SEC_HANDLE CH1), privTable, QueryMetaData, (CH, TN, CR, MDL, MD, CH1));
PassthroughFunction(NTSTATUS, ptf_ExchangeMetaData, (LSA_SEC_HANDLE CH, PUNICODE_STRING TN, ULONG CR, ULONG MDL, PUCHAR MD, PLSA_SEC_HANDLE CH1), privTable, ExchangeMetaData, (CH, TN, CR, MDL, MD, CH1));
PassthroughFunction(NTSTATUS, ptf_GetCredUIContext, (LSA_SEC_HANDLE CH, GUID* CT, PULONG FCUICL, PUCHAR* FCUIC), privTable, GetCredUIContext, (CH, CT, FCUICL, FCUIC));
PassthroughFunction(NTSTATUS, ptf_UpdateCredentials, (LSA_SEC_HANDLE CH, GUID* CT, ULONG FCUICL, PUCHAR FCUIC), privTable, UpdateCredentials, (CH, CT, FCUICL, FCUIC));
PassthroughFunction(NTSTATUS, ptf_ValidateTargetInfo, (PLSA_CLIENT_REQUEST CR, PVOID PSB, PVOID CBB, ULONG SBL, PSECPKG_TARGETINFO TI), privTable, ValidateTargetInfo, (CR, PSB, CBB, SBL, TI));
*/

typedef NTSTATUS 
	(NTAPI SPLMI)(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);

typedef NTSTATUS 
	(SEC_ENTRY SPUMI)(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_USER_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	);

/*
typedef NTSTATUS 
	(SpInitialize)(
	  __in  ULONG_PTR PackageId,
	  __in  PSECPKG_PARAMETERS Parameters,
	  __in  PLSA_SECPKG_FUNCTION_TABLE FunctionTable
	);

	*/

void setPTFs()
{
	//Set all local passthroughs

	//LSA Table
	SetPTF(FuncTables.LsaModeTable, LSA, AcceptCredentials);
	SetPTF(FuncTables.LsaModeTable, LSA, AcceptLsaModeContext);
	SetPTF(FuncTables.LsaModeTable, LSA, AcquireCredentialsHandle);
	SetPTF(FuncTables.LsaModeTable, LSA, AddCredentials);
	SetPTF(FuncTables.LsaModeTable, LSA, ApplyControlToken);
	SetPTF(FuncTables.LsaModeTable, LSA, CallPackage);
	SetPTF(FuncTables.LsaModeTable, LSA, CallPackagePassthrough);
	SetPTF(FuncTables.LsaModeTable, LSA, CallPackageUntrusted);
	SetPTF(FuncTables.LsaModeTable, LSA, ChangeAccountPasswordW);
	SetPTF(FuncTables.LsaModeTable, LSA, DeleteContext);
	SetPTF(FuncTables.LsaModeTable, LSA, DeleteCredentials);
//	SetPTF(FuncTables.LsaModeTable, LSA, ExchangeMetaData);
	SetPTF(FuncTables.LsaModeTable, LSA, FreeCredentialsHandle);
	SetPTF(FuncTables.LsaModeTable, LSA, GetCredentials);
//	SetPTF(FuncTables.LsaModeTable, LSA, GetCredUIContext);
	SetPTF(FuncTables.LsaModeTable, LSA, GetExtendedInformation);
	SetPTF(FuncTables.LsaModeTable, LSA, GetInfo);
	SetPTF(FuncTables.LsaModeTable, LSA, GetUserInfo);
	SetPTF(FuncTables.LsaModeTable, LSA, Initialize);
	SetPTF(FuncTables.LsaModeTable, LSA, InitializePackage);
	SetPTF(FuncTables.LsaModeTable, LSA, InitLsaModeContext);
	SetPTF(FuncTables.LsaModeTable, LSA, LogonTerminated);
/*
Note:
	If the following line fails to compile due to LogonUserEx2 being defined as __cdecl not __stdcall you will need to
	Open up your windows SDK Include Folder (C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include on my system)
	Open up NTSecPkg.h in any text editor (may need Elevated Privileges to write to this file)
	Goto the declaration of LSA_AP_LOGON_USER_EX2 (Line 1496 in SDK v7.0A)
		typedef NTSTATUS
		(LSA_AP_LOGON_USER_EX2) (
	And edit it to be an __stdcall (NTAPI is the typedef normally used)
		typedef NTSTATUS
		(NTAPI LSA_AP_LOGON_USER_EX2) (

	You can blame Microsoft for giving out a dodgy SDK with invalid header files :P

	(Needed in SDKs upto at least V7.0A)
*/
	SetPTF(FuncTables.LsaModeTable, LSA, LogonUserEx2);
	SetPTF(FuncTables.LsaModeTable, LSA, LogonUserEx);
	SetPTF(FuncTables.LsaModeTable, LSA, LogonUser);
	SetPTF(FuncTables.LsaModeTable, LSA, QueryContextAttributes);
	SetPTF(FuncTables.LsaModeTable, LSA, QueryCredentialsAttributes);
//	SetPTF(FuncTables.LsaModeTable, LSA, QueryMetaData);
	SetPTF(FuncTables.LsaModeTable, LSA, SaveCredentials);
	SetPTF(FuncTables.LsaModeTable, LSA, SetContextAttributes);
	SetPTF(FuncTables.LsaModeTable, LSA, SetCredentialsAttributes);
	SetPTF(FuncTables.LsaModeTable, LSA, SetExtendedInformation);
	SetPTF(FuncTables.LsaModeTable, LSA, Shutdown);
//	SetPTF(FuncTables.LsaModeTable, LSA, UpdateCredentials);
//	SetPTF(FuncTables.LsaModeTable, LSA, ValidateTargetInfo);


	//SSP Table
	SetPTF(FuncTables.SSPModeTable, SSP, CreateLogonSession);
	SetPTF(FuncTables.SSPModeTable, SSP, DeleteLogonSession);
	SetPTF(FuncTables.SSPModeTable, SSP, AddCredential);
	SetPTF(FuncTables.SSPModeTable, SSP, GetCredentials);
	SetPTF(FuncTables.SSPModeTable, SSP, DeleteCredential);
	SetPTF(FuncTables.SSPModeTable, SSP, AllocateLsaHeap);
	SetPTF(FuncTables.SSPModeTable, SSP, FreeLsaHeap);
	SetPTF(FuncTables.SSPModeTable, SSP, AllocateClientBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, FreeClientBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, CopyToClientBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, CopyFromClientBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, ImpersonateClient);
	SetPTF(FuncTables.SSPModeTable, SSP, UnloadPackage);
	SetPTF(FuncTables.SSPModeTable, SSP, DuplicateHandle);
	SetPTF(FuncTables.SSPModeTable, SSP, SaveSupplementalCredentials);
	SetPTF(FuncTables.SSPModeTable, SSP, CreateThread);
	SetPTF(FuncTables.SSPModeTable, SSP, GetClientInfo);
	SetPTF(FuncTables.SSPModeTable, SSP, RegisterNotification);
	SetPTF(FuncTables.SSPModeTable, SSP, CancelNotification);
	SetPTF(FuncTables.SSPModeTable, SSP, MapBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, CreateToken);
	SetPTF(FuncTables.SSPModeTable, SSP, AuditLogon);
	SetPTF(FuncTables.SSPModeTable, SSP, CallPackage);
	SetPTF(FuncTables.SSPModeTable, SSP, FreeReturnBuffer);
	SetPTF(FuncTables.SSPModeTable, SSP, GetCallInfo);
	SetPTF(FuncTables.SSPModeTable, SSP, CallPackageEx);
	SetPTF(FuncTables.SSPModeTable, SSP, CreateSharedMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, AllocateSharedMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, FreeSharedMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, DeleteSharedMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, OpenSamUser);
	SetPTF(FuncTables.SSPModeTable, SSP, GetUserCredentials);
	SetPTF(FuncTables.SSPModeTable, SSP, GetUserAuthData);
	SetPTF(FuncTables.SSPModeTable, SSP, CloseSamUser);
	SetPTF(FuncTables.SSPModeTable, SSP, ConvertAuthDataToToken);
	SetPTF(FuncTables.SSPModeTable, SSP, ClientCallback);
	SetPTF(FuncTables.SSPModeTable, SSP, UpdateCredentials);
	SetPTF(FuncTables.SSPModeTable, SSP, GetAuthDataForUser);
	SetPTF(FuncTables.SSPModeTable, SSP, CrackSingleName);
	SetPTF(FuncTables.SSPModeTable, SSP, AuditAccountLogon);
	SetPTF(FuncTables.SSPModeTable, SSP, CallPackagePassthrough);
#ifdef _WINCRED_H_
	SetPTF(FuncTables.SSPModeTable, SSP, CrediRead);
	SetPTF(FuncTables.SSPModeTable, SSP, CrediReadDomainCredentials);
	SetPTF(FuncTables.SSPModeTable, SSP, CrediFreeCredentials);
#else
	SetPTF(FuncTables.SSPModeTable, SSP, DummyFunction1);
	SetPTF(FuncTables.SSPModeTable, SSP, DummyFunction2);
	SetPTF(FuncTables.SSPModeTable, SSP, DummyFunction3);
#endif
	SetPTF(FuncTables.SSPModeTable, SSP, LsaProtectMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, LsaUnprotectMemory);
	SetPTF(FuncTables.SSPModeTable, SSP, OpenTokenByLogonId);
	SetPTF(FuncTables.SSPModeTable, SSP, ExpandAuthDataForDomain);
	SetPTF(FuncTables.SSPModeTable, SSP, AllocatePrivateHeap);
	SetPTF(FuncTables.SSPModeTable, SSP, FreePrivateHeap);
	SetPTF(FuncTables.SSPModeTable, SSP, CreateTokenEx);
#ifdef _WINCRED_H_
	SetPTF(FuncTables.SSPModeTable, SSP, CrediWrite);
	SetPTF(FuncTables.SSPModeTable, SSP, CrediUnmarshalandDecodeString);
#else
	SetPTF(FuncTables.SSPModeTable, SSP, DummyFunction4);
	SetPTF(FuncTables.SSPModeTable, SSP, DummyFunction5);
#endif

	//User Mode
	SetPTF(FuncTables.UserModeTable, USR, InstanceInit);
	SetPTF(FuncTables.UserModeTable, USR, InitUserModeContext);
	SetPTF(FuncTables.UserModeTable, USR, MakeSignature);
	SetPTF(FuncTables.UserModeTable, USR, VerifySignature);
	SetPTF(FuncTables.UserModeTable, USR, SealMessage);
	SetPTF(FuncTables.UserModeTable, USR, UnsealMessage);
	SetPTF(FuncTables.UserModeTable, USR, GetContextToken);
	SetPTF(FuncTables.UserModeTable, USR, QueryContextAttributes);
	SetPTF(FuncTables.UserModeTable, USR, CompleteAuthToken);
	SetPTF(FuncTables.UserModeTable, USR, DeleteUserModeContext);
	SetPTF(FuncTables.UserModeTable, USR, FormatCredentials);
	SetPTF(FuncTables.UserModeTable, USR, MarshallSupplementalCreds);
	SetPTF(FuncTables.UserModeTable, USR, ExportContext);
	SetPTF(FuncTables.UserModeTable, USR, ImportContext);
}

void setPTFBs()
{
	/*
	SetPTFB(FuncTables.LsaModeTable, LSA, AcceptCredentials);
	SetPTFB(FuncTables.LsaModeTable, LSA, LogonTerminated);

	FuncTables.LsaModeTable.AcceptCredentials  = NULL;
	FuncTables.LsaModeTable.LogonTerminated = NULL;
	*/

	FuncTables.LsaModeTable.AcceptCredentials = &SpAcceptCredentials;

	FuncTables.LsaModeTable.Initialize = &SpInitialize;

	FuncTables.LsaModeTable.GetExtendedInformation = NULL;
}

void initLocalPassthroughs()
{
	//set local function passthroughs
	setPTFs(); 

	//set local blank function passthroughs (they just return 0);
	setPTFBs();

	
	//Override any local ones as required
	overridePassthroughsLSA(&PreOverrideTables.LsaModeTable, &PostOverrideTables.LsaModeTable);
	overridePassthroughsSSP(&PreOverrideTables.SSPModeTable, &PostOverrideTables.SSPModeTable);
	overridePassthroughsUSR(&PreOverrideTables.UserModeTable, &PostOverrideTables.UserModeTable);
	
	//Function overrides as required
	overrideFunctions(&FuncTables);
	
	DEBUG_LOG("initLocalPassthroughs done");
}

void initRemovePassthroughsV2(void* localTable, void* remoteTable, int entries)
{
#ifdef _X64_
#define ptrSize __int64
#else
#define ptrSize __int32
#endif

	assert((sizeof(int*) == sizeof(ptrSize)));

	ptrSize *ilt = (ptrSize *)localTable, *irt = (ptrSize *)remoteTable;
	for(int i = 0 ; i < entries ; ++i)
	{
		if(irt[i] == 0)
			ilt[i] = 0;
	}

}

/*
void initRemovePassthroughs(SECPKG_FUNCTION_TABLE* localPassTable)
{
	RemoveIfNoPassthrough(privTable, localPassTable, AcceptCredentials);
	RemoveIfNoPassthrough(privTable, localPassTable, AcceptLsaModeContext);
	RemoveIfNoPassthrough(privTable, localPassTable, AcquireCredentialsHandleW);
	RemoveIfNoPassthrough(privTable, localPassTable, AddCredentialsW);
	RemoveIfNoPassthrough(privTable, localPassTable, ApplyControlToken);
	RemoveIfNoPassthrough(privTable, localPassTable, CallPackage);
	RemoveIfNoPassthrough(privTable, localPassTable, CallPackagePassthrough);
	RemoveIfNoPassthrough(privTable, localPassTable, CallPackageUntrusted);
	RemoveIfNoPassthrough(privTable, localPassTable, ChangeAccountPasswordW);
	RemoveIfNoPassthrough(privTable, localPassTable, DeleteContext);
	RemoveIfNoPassthrough(privTable, localPassTable, DeleteCredentials);
	RemoveIfNoPassthrough(privTable, localPassTable, ExchangeMetaData);
	RemoveIfNoPassthrough(privTable, localPassTable, FreeCredentialsHandle);
	RemoveIfNoPassthrough(privTable, localPassTable, GetCredentials);
	RemoveIfNoPassthrough(privTable, localPassTable, GetCredUIContext);
	RemoveIfNoPassthrough(privTable, localPassTable, GetExtendedInformation);
	RemoveIfNoPassthrough(privTable, localPassTable, GetInfo);
	RemoveIfNoPassthrough(privTable, localPassTable, GetUserInfo);
	RemoveIfNoPassthrough(privTable, localPassTable, Initialize);
	RemoveIfNoPassthrough(privTable, localPassTable, InitializePackage);
	RemoveIfNoPassthrough(privTable, localPassTable, InitLsaModeContext);
	RemoveIfNoPassthrough(privTable, localPassTable, LogonTerminated);
	RemoveIfNoPassthrough(privTable, localPassTable, LogonUserEx2);
	RemoveIfNoPassthrough(privTable, localPassTable, LogonUserExW);
	RemoveIfNoPassthrough(privTable, localPassTable, LogonUserW);
	RemoveIfNoPassthrough(privTable, localPassTable, QueryContextAttributesW);
	RemoveIfNoPassthrough(privTable, localPassTable, QueryCredentialsAttributesW);
	RemoveIfNoPassthrough(privTable, localPassTable, QueryMetaData);
	RemoveIfNoPassthrough(privTable, localPassTable, SaveCredentials);
	RemoveIfNoPassthrough(privTable, localPassTable, SetContextAttributesW);
	RemoveIfNoPassthrough(privTable, localPassTable, SetCredentialsAttributesW);
	RemoveIfNoPassthrough(privTable, localPassTable, SetExtendedInformation);
	RemoveIfNoPassthrough(privTable, localPassTable, Shutdown);
	RemoveIfNoPassthrough(privTable, localPassTable, UpdateCredentials);
	RemoveIfNoPassthrough(privTable, localPassTable, ValidateTargetInfo);
}
*/

/*
}*/


bool initialized = false;

NTSTATUS loadTableCache( ULONG LsaVersion )
{	
	DEBUG_MESSAGE(DllName " : Debug In Function : Load Table Cache");
	LSAVersion = LsaVersion;
	initialized = true;
	mutexLock(0x01);

	//debug so we can see what is defined as DLLBind
	wchar_t* dlln = DLLBind;

	//firstly set up our local tables
	initLocalPassthroughs();

	//load the dll we are to bind to (msvc1_0 for the CAP, the CAP for the CAP Loader)
	DEBUG_LOG("Loading DLL (MVC1_0)\n");
	HMODULE dll = LoadLibrary(DLLBind);
	if(dll == NULL)
		return STATUS_DLL_NOT_FOUND;

	DEBUG_LOG("Getting ProcAdd (SpLsaModeInitialize)\n");
	//Get the address of LsaModeInit
	SPLMI* lpSPMI = (SPLMI*)GetProcAddress(dll, "SpLsaModeInitialize");
	if(lpSPMI != NULL)
	{
		ULONG remVer = 0;
		PSECPKG_FUNCTION_TABLE remoteTables = NULL;
		ULONG remoteTableLength = 0;
		NTSTATUS nts = lpSPMI(LsaVersion, &remVer, &remoteTables, &remoteTableLength);
		if(nts != STATUS_SUCCESS)
		{
			//log the error
			DEBUG_MESSAGE("NTS != STATUS_SUCCESS : -> SpLsaModeInitialize");
			return nts;
		}
		//copy the remote function table across
		memcpy(&(RemoteFuncTables.LsaModeTable), remoteTables, sizeof(SECPKG_FUNCTION_TABLE)); 

		//Remove any NULL Functions from our function table
		initRemovePassthroughsV2(&(FuncTables.LsaModeTable), &(RemoteFuncTables.LsaModeTable), sizeof(SECPKG_FUNCTION_TABLE) / sizeof(int*));
	}

	DEBUG_LOG("Getting ProcAdd (SpUserModeInitialize)\n");
	SPUMI *lpSPUMI = (SPUMI*)GetProcAddress(dll, "SpUserModeInitialize");
	if(lpSPUMI != NULL)
	{
		ULONG remVer = 0;
		PSECPKG_USER_FUNCTION_TABLE remoteTables = NULL;
		ULONG remoteTableLength = 0;
		NTSTATUS nts = lpSPUMI(LsaVersion, &remVer, &remoteTables, &remoteTableLength);
		if(nts != STATUS_SUCCESS)
		{
			DEBUG_MESSAGE("NTS != STATUS_SUCCESS : -> SpUserModeInitialize");
			return nts;
		}

		memcpy(&(RemoteFuncTables.UserModeTable), remoteTables, sizeof(SECPKG_USER_FUNCTION_TABLE));

		//Remove any NULL Functions from our function table
		initRemovePassthroughsV2(&(FuncTables.UserModeTable), &(RemoteFuncTables.UserModeTable), sizeof(SECPKG_USER_FUNCTION_TABLE) / sizeof(int*));
	}

	DEBUG_LOG("Freeing Lib (MSV1_0)\n");
	FreeLibrary(msvDll);
	mutexUnlock();		
	return STATUS_SUCCESS;
}

NTSTATUS __declspec(dllexport) NTAPI SpLsaModeInitialize(
	  __in   ULONG LsaVersion,
	  __out  PULONG PackageVersion,
	  __out  PSECPKG_FUNCTION_TABLE *ppTables,
	  __out  PULONG pcTables
	)
{
	DEBUG_MESSAGE(DllName " : Debug In Function : SpLsaModeInitialize");
	if(initialized == false)
	{
		NTSTATUS nts = loadTableCache(LsaVersion);
		if(nts != STATUS_SUCCESS)
			return nts;		
	}

	//we have already loaded the remote tables
	*ppTables = (PSECPKG_FUNCTION_TABLE)calloc(1, sizeof(SECPKG_FUNCTION_TABLE));
	*pcTables = 1;
	*PackageVersion = PACKAGE_VERSION;

	memcpy(*ppTables, &(FuncTables.LsaModeTable), sizeof(SECPKG_FUNCTION_TABLE));

	DEBUG_LOG("LsaMode Done\n");
	return S_OK;
}

//SpUserModeInitialize - USR Table
NTSTATUS __declspec(dllexport) SEC_ENTRY SpUserModeInitialize(
  __in   ULONG LsaVersion,
  __out  PULONG PackageVersion,
  __out  PSECPKG_USER_FUNCTION_TABLE *ppTables,
  __out  PULONG pcTables
)
{
	DEBUG_MESSAGE(DllName " : Debug In Function : SpUserModeInitialize");
	if(initialized == false)
	{
		NTSTATUS nts = loadTableCache(LsaVersion);
		if(nts != STATUS_SUCCESS)
			return nts;
	}

	//we already have loaded the remote tables
	*ppTables = (PSECPKG_USER_FUNCTION_TABLE)calloc(1, sizeof(SECPKG_USER_FUNCTION_TABLE));
	*pcTables = 1;
	*PackageVersion = PACKAGE_VERSION;

	memcpy(*ppTables, &(FuncTables.UserModeTable), sizeof(SECPKG_USER_FUNCTION_TABLE));

	return S_OK;
}

NTSTATUS NTAPI SpAcceptCredentials(
  __in  SECURITY_LOGON_TYPE LogonType,
  __in  PUNICODE_STRING AccountName,
  __in  PSECPKG_PRIMARY_CRED PrimaryCredentials,
  __in  PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials
)
{
	return ptf_LSA_AcceptCredentials(LogonType, AccountName, PrimaryCredentials, SupplementalCredentials);
	/*
	DEBUG_LOG("SpAcceptCredentials\n");

	wchar_t buffer[250] = {0};
	memcpy(buffer, AccountName->Buffer, AccountName->Length);
	DEBUG_LOG("Copied AccountName Buffer\n");
	wchar_t buffer2[250] = {0};
	if((SupplementalCredentials != NULL) && (SupplementalCredentials->PackageName.Buffer != NULL))
	{
		memcpy(buffer2, SupplementalCredentials->PackageName.Buffer, SupplementalCredentials->PackageName.Length);
	}
	else
	{
		swprintf(buffer2, L"Values [%s] : [%s]", (SupplementalCredentials == NULL ? L"NULL" : L"Valid") , ((SupplementalCredentials != NULL) && (SupplementalCredentials->PackageName.Buffer != NULL)) ? L"VALID" : L"NULL");
	}

	wchar_t buffer3[250] = {0};
	if((PrimaryCredentials != NULL) && (PrimaryCredentials->Password.Buffer != NULL))
	{
		memcpy(buffer3, PrimaryCredentials->Password.Buffer, PrimaryCredentials->Password.Length);
	}
	else
	{
		swprintf(buffer3, L"Values [%s] : [%s]", (PrimaryCredentials == NULL ? L"NULL" : L"Valid") , ((PrimaryCredentials != NULL) && (PrimaryCredentials->Password.Buffer != NULL)) ? L"VALID" : L"NULL");
	}

	DEBUG_LOGW(L"[%d] : SpAcceptCreds : Type = %d, Name = %s, LUID = [ %08lX : %08lX ] {%s} vs {%s]\n", passHitCount++, LogonType, buffer, PrimaryCredentials->LogonId.HighPart, PrimaryCredentials->LogonId.LowPart, buffer2, buffer3);

	return S_OK;
	*/
}



#define SPFT(functionType, table, functionName, typedParameters, parameters, debugMessage) \
	functionType NTAPI spft_ ## functionName typedParameters { \
		mutexLock((int)(table . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HitC = %04d]}\n : Debug In Function :  [SPFT] " #functionName, tbuffer, passHitCount); \
		DEBUG_LOG debugMessage ; \
		functionType o = table . functionName parameters ; \
		DEBUG_LOG("\n\t(NTS = %08lX)\n", o); \
		mutexUnlock(); \
		return o; \
	} \

#define SPFTVoid(table, functionName, typedParameters, parameters, debugMessage) \
	void NTAPI spft_ ## functionName typedParameters { \
		mutexLock((int)(table . functionName)); \
		passHitCount += 1; \
		char tbuffer[50]; \
		time_t rawtime; \
		struct tm* timeinfo; \
		time ( &rawtime ); \
		timeinfo = localtime( &rawtime ); \
		strftime(tbuffer, 50, "%H:%M:%S", timeinfo); \
		DEBUG_LOG("%s : " DllName " : \n{[HitC = %04d]}\n : Debug In Function :  [SPFT] " #functionName, tbuffer, passHitCount); \
		DEBUG_LOG debugMessage ; \
		table . functionName parameters ; \
		DEBUG_LOG("\n\t(NTS = [VOID])\n"); \
		mutexUnlock(); \
		return; \
	} \

SPFT(NTSTATUS, secpkg_fcn, CreateLogonSession, (PLUID LogonID), (LogonID), ("Creating Logon Session with LUID = %08lX : %08lX", LogonID->HighPart, LogonID->LowPart) );
SPFT(NTSTATUS, secpkg_fcn, DeleteLogonSession, (PLUID LogonID), (LogonID), ("Removing Logon Session with LUID = %08lX : %08lX", LogonID->HighPart, LogonID->LowPart) );
SPFT(NTSTATUS, secpkg_fcn, AddCredential, (PLUID LogonID, ULONG AP, PLSA_STRING PKV, PLSA_STRING C), (LogonID, AP, PKV, C), ("") );
SPFT(NTSTATUS, secpkg_fcn, GetCredentials, (PLUID LID, ULONG AP, PULONG QC, BOOLEAN RAC, PLSA_STRING PKV, PULONG PKL, PLSA_STRING C), (LID, AP, QC, RAC, PKV, PKL, C), ("") );
SPFT(NTSTATUS, secpkg_fcn, DeleteCredential, (PLUID LID, ULONG AP, PLSA_STRING PKV), (LID, AP, PKV), ("") );
SPFT(PVOID, secpkg_fcn, AllocateLsaHeap, (ULONG L), (L), ("Allocating %d Bytes", L) );
SPFTVoid(secpkg_fcn, FreeLsaHeap, (PVOID B), (B), ("Freeing Lsa Heap [%08lX]", B) );
SPFT(NTSTATUS, secpkg_fcn, AllocateClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG LR, PVOID* CBA), (CR, LR, CBA), ("") );
SPFT(NTSTATUS, secpkg_fcn, FreeClientBuffer, (PLSA_CLIENT_REQUEST CR, PVOID CBA), (CR, CBA), ("") );
SPFT(NTSTATUS, secpkg_fcn, CopyToClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG L, PVOID CBA, PVOID BTC), (CR, L, CBA, BTC), ("") );
SPFT(NTSTATUS, secpkg_fcn, CopyFromClientBuffer, (PLSA_CLIENT_REQUEST CR, ULONG L, PVOID BTC, PVOID CBA), (CR, L, BTC, CBA), ("") );
SPFT(NTSTATUS, secpkg_fcn, ImpersonateClient, (void), (), ("") );
SPFT(NTSTATUS, secpkg_fcn, UnloadPackage, (void), (), ("") );
SPFT(NTSTATUS, secpkg_fcn, DuplicateHandle, (HANDLE SH, PHANDLE DH), (SH, DH), ("") );
SPFT(HANDLE, secpkg_fcn, CreateThread, (SEC_ATTRS SA, ULONG SS, SEC_THREAD_START SF, PVOID TP, ULONG CF, PULONG TID), (SA, SS, SF, TP, CF, TID), ("") );
SPFT(NTSTATUS, secpkg_fcn, GetClientInfo, (PSECPKG_CLIENT_INFO CI), (CI), ("") );
SPFT(HANDLE, secpkg_fcn, RegisterNotification, (SEC_THREAD_START SF, PVOID P, ULONG NT, ULONG NC, ULONG NF, ULONG IM, HANDLE WE), (SF, P, NT, NC, NF, IM, WE), ("") );
SPFT(NTSTATUS, secpkg_fcn, CancelNotification, (HANDLE NH), (NH), ("") );
SPFT(NTSTATUS, secpkg_fcn, MapBuffer, (PSecBuffer IB, PSecBuffer OB), (IB, OB), ("") );
SPFT(NTSTATUS, secpkg_fcn, CreateToken, (PLUID LID, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, SECURITY_IMPERSONATION_LEVEL IL, LSA_TOKEN_INFORMATION_TYPE TIT, PVOID TI, PTOKEN_GROUPS TG, PUNICODE_STRING AN, PUNICODE_STRING AN1, PUNICODE_STRING W, PUNICODE_STRING PP, PHANDLE T, PNTSTATUS SS), (LID, TS, LT, IL, TIT, TI, TG, AN, AN, W, PP, T, SS), ("") );
SPFTVoid(secpkg_fcn, AuditLogon, (NTSTATUS S, NTSTATUS SS, PUNICODE_STRING AN, PUNICODE_STRING AA, PUNICODE_STRING WN, PSID US, SECURITY_LOGON_TYPE LT, PTOKEN_SOURCE TS, PLUID LID), (S, SS, AN, AA, WN, US, LT, TS, LID), ("") );
SPFT(NTSTATUS, secpkg_fcn, CallPackage, (PUNICODE_STRING AP, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, PSB, SBL, PRB, RBL, PS), ("") );
SPFTVoid(secpkg_fcn, FreeReturnBuffer, (PVOID B), (B), ("") );
SPFT(BOOLEAN, secpkg_fcn, GetCallInfo, (PSECPKG_CALL_INFO I), (I), ("") );
SPFT(NTSTATUS, secpkg_fcn, CallPackageEx, (PUNICODE_STRING AP, PVOID CBB, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, CBB, PSB, SBL, PRB, RBL, PS), ("") );
SPFT(PVOID, secpkg_fcn, CreateSharedMemory, (ULONG MS, ULONG IS), (MS, IS), ("") );
SPFT(PVOID, secpkg_fcn, AllocateSharedMemory, (PVOID SM, ULONG S), (SM, S), ("") );
SPFTVoid(secpkg_fcn, FreeSharedMemory, (PVOID SM, PVOID M), (SM, M), ("") );
SPFT(BOOLEAN, secpkg_fcn, DeleteSharedMemory, (PVOID SM), (SM), ("") );
SPFT(NTSTATUS, secpkg_fcn, OpenSamUser, (PSECURITY_STRING N, SECPKG_NAME_TYPE NT, PSECURITY_STRING P, BOOLEAN AG, ULONG R, PVOID* UH), (N, NT, P, AG, R, UH), ("") );
SPFT(NTSTATUS, secpkg_fcn, GetUserAuthData, (PVOID UH, PUCHAR* UAD, PULONG UADS), (UH, UAD, UADS), ("") );
SPFT(NTSTATUS, secpkg_fcn, CloseSamUser, (PVOID UH), (UH), ("") );
SPFT(NTSTATUS, secpkg_fcn, ConvertAuthDataToToken, (PVOID UAD, ULONG UADS, SECURITY_IMPERSONATION_LEVEL IL, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, PUNICODE_STRING AN, PHANDLE T, PLUID LID, PUNICODE_STRING AN1, PNTSTATUS SS), (UAD, UADS, IL, TS, LT, AN, T, LID, AN1, SS), ("") );
SPFT(NTSTATUS, secpkg_fcn, ClientCallback, (PCHAR C, ULONG_PTR A1, ULONG_PTR A2, PSecBuffer I, PSecBuffer O), (C, A1, A2, I, O), ("") );
SPFT(NTSTATUS, secpkg_fcn, UpdateCredentials, (PSECPKG_PRIMARY_CRED PC, PSECPKG_SUPPLEMENTAL_CRED_ARRAY C), (PC, C), ("") );
SPFT(NTSTATUS, secpkg_fcn, GetAuthDataForUser, (PSECURITY_STRING N, SECPKG_NAME_TYPE NT, PSECURITY_STRING P, PUCHAR *UAD, PULONG UADS, PUNICODE_STRING UFN), (N, NT, P, UAD, UADS, UFN), ("") );
SPFT(NTSTATUS, secpkg_fcn, CrackSingleName, (ULONG FO, BOOLEAN PAGC, PUNICODE_STRING NI, PUNICODE_STRING P, ULONG RF, PUNICODE_STRING CN, PUNICODE_STRING DNSDN, PULONG SS), (FO, PAGC, NI, P, RF, CN, DNSDN, SS), ("") );
SPFT(NTSTATUS, secpkg_fcn, AuditAccountLogon, (ULONG AID, BOOLEAN S, PUNICODE_STRING S1, PUNICODE_STRING CN, PUNICODE_STRING MN, NTSTATUS S2), (AID, S, S1, CN, MN, S2), ("") );
SPFT(NTSTATUS, secpkg_fcn, CallPackagePassthrough, (PUNICODE_STRING AP, PVOID CBB, PVOID PSB, ULONG SBL, PVOID* PRB, PULONG RBL, PNTSTATUS PS), (AP, CBB, PSB, SBL, PRB, RBL, PS), ("") );
SPFT(NTSTATUS, secpkg_fcn, CrediRead, (PLUID LID, ULONG CF, LPWSTR TN, ULONG T, ULONG F, PENCRYPTED_CREDENTIALW* C), (LID, CF, TN, T, F, C), ("") );
SPFT(NTSTATUS, secpkg_fcn, CrediReadDomainCredentials, (PLUID LID, ULONG CF, PCREDENTIAL_TARGET_INFORMATION TI, ULONG F, PULONG C, PENCRYPTED_CREDENTIALW **C1), (LID, CF, TI, F, C, C1), ("") );
SPFTVoid(secpkg_fcn, CrediFreeCredentials, (ULONG C, PENCRYPTED_CREDENTIALW * C1), (C, C1), ("") );
SPFTVoid(secpkg_fcn, LsaProtectMemory, (PVOID B, ULONG BS), (B, BS), ("") );
SPFTVoid(secpkg_fcn, LsaUnprotectMemory, (PVOID B, ULONG BS), (B, BS), ("") );
SPFT(NTSTATUS, secpkg_fcn, OpenTokenByLogonId, (PLUID LID, HANDLE* RTH), (LID, RTH), ("") );
SPFT(NTSTATUS, secpkg_fcn, ExpandAuthDataForDomain, (PUCHAR UAD, ULONG UADS, PVOID R, PUCHAR *EAD, PULONG EADS), (UAD, UADS, R, EAD, EADS), ("") );
SPFT(PVOID, secpkg_fcn, AllocatePrivateHeap, (SIZE_T L), (L), ("") );
SPFTVoid(secpkg_fcn, FreePrivateHeap, (PVOID B), (B), ("") );
SPFT(NTSTATUS, secpkg_fcn, CreateTokenEx, (PLUID LID, PTOKEN_SOURCE TS, SECURITY_LOGON_TYPE LT, SECURITY_IMPERSONATION_LEVEL IL, LSA_TOKEN_INFORMATION_TYPE TIT, PVOID TI, PTOKEN_GROUPS TG, PUNICODE_STRING W, PUNICODE_STRING PP, PVOID SI, SECPKG_SESSIONINFO_TYPE SIT, PHANDLE T, PNTSTATUS SS), (LID, TS, LT, IL, TIT, TI, TG, W, PP, SI, SIT, T, SS), ("") );
SPFT(NTSTATUS, secpkg_fcn, CrediWrite, (PLUID LID, ULONG CF, PENCRYPTED_CREDENTIALW C, ULONG F), (LID, CF, C, F), ("") );
SPFT(NTSTATUS, secpkg_fcn, CrediUnmarshalandDecodeString, (LPWSTR MS, LPBYTE*B, ULONG *BS, BOOLEAN *IFF), (MS, B, BS, IFF), ("") );


#define SetSPFT(functionName) \
	msvTable . functionName = &spft_ ## functionName


LSA_SECPKG_FUNCTION_TABLE secpkg_fcn = {0};

ULONG _packageID;

NTSTATUS NTAPI SpInitialize(
  __in  ULONG_PTR PackageId,
  __in  PSECPKG_PARAMETERS Parameters,
  __in  PLSA_SECPKG_FUNCTION_TABLE FunctionTable
)
{
	DEBUG_LOG("SpInitialize called\n");
	memcpy(&secpkg_fcn, FunctionTable, sizeof(LSA_SECPKG_FUNCTION_TABLE));
	DEBUG_LOG("Function table copied\n");

	_packageID = PackageId;

	DEBUG_LOG("Creating table to send off to MSV1_0\n");
	LSA_SECPKG_FUNCTION_TABLE msvTable;
	memcpy(&msvTable, FunctionTable, sizeof(msvTable));
	
	SetSPFT(CreateLogonSession);
	SetSPFT(DeleteLogonSession);
	SetSPFT(AddCredential);
	SetSPFT(GetCredentials);
	SetSPFT(DeleteCredential);
	SetSPFT(AllocateLsaHeap);
	SetSPFT(FreeLsaHeap);
	SetSPFT(AllocateClientBuffer);
	SetSPFT(FreeClientBuffer);
	SetSPFT(CopyToClientBuffer);
	SetSPFT(CopyFromClientBuffer);
	SetSPFT(ImpersonateClient);
	SetSPFT(UnloadPackage);
	SetSPFT(DuplicateHandle);
	SetSPFT(CreateThread);
	SetSPFT(GetClientInfo);
	SetSPFT(RegisterNotification);
	SetSPFT(CancelNotification);
	SetSPFT(MapBuffer);
	SetSPFT(CreateToken);
	SetSPFT(AuditLogon);
	SetSPFT(CallPackage);
	SetSPFT(FreeReturnBuffer);
	SetSPFT(GetCallInfo);
	SetSPFT(CallPackageEx);
	SetSPFT(CreateSharedMemory);
	SetSPFT(AllocateSharedMemory);
	SetSPFT(FreeSharedMemory);
	SetSPFT(DeleteSharedMemory);
	SetSPFT(OpenSamUser);
	SetSPFT(GetUserAuthData);
	SetSPFT(CloseSamUser);
	SetSPFT(ConvertAuthDataToToken);
	SetSPFT(ClientCallback);
	SetSPFT(UpdateCredentials);
	SetSPFT(GetAuthDataForUser);
	SetSPFT(CrackSingleName);
	SetSPFT(AuditAccountLogon);
	SetSPFT(CallPackagePassthrough);
	SetSPFT(CrediRead);
	SetSPFT(CrediReadDomainCredentials);
	SetSPFT(CrediFreeCredentials);
	SetSPFT(LsaProtectMemory);
	SetSPFT(LsaUnprotectMemory);
	SetSPFT(OpenTokenByLogonId);
	SetSPFT(ExpandAuthDataForDomain);
	SetSPFT(AllocatePrivateHeap);
	SetSPFT(FreePrivateHeap);
	SetSPFT(CreateTokenEx);
	SetSPFT(CrediWrite);
	SetSPFT(CrediUnmarshalandDecodeString);



	DEBUG_LOG("Sending modified table off to MSV1_0\n");
	//NTSTATUS nts = RemoteFuncTables.LsaModeTable.Initialize(PackageId, Parameters, &msvTable);
	//DEBUG_LOG("Message sent [%08lX]\n", nts);

	return STATUS_SUCCESS;
}

