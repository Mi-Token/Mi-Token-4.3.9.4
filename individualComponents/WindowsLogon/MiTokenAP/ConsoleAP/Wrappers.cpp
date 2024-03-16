
#include "Wrappers.h"
#include "DebugMessages.h"
#include "Passthroughs.h"
#include "..\MiTokenAP\SmartcardCachedCreds.h"
#include "OTPCheck.h"
#include <Sddl.h>

#define WRAPPER

#ifdef WRAPPER

#define MESSAGE_WITH2FA 0x2000
#define MESSAGE_CREDS_WITH_ID 0x1000
#define MESSAGE_CARD_ID 0x1001

void removeLUID(__int64 LUID);
__int64 LUIDtoInt64(PLUID pLUID);

typedef struct _CREDS_WITH_2FA : public KERB_INTERACTIVE_UNLOCK_LOGON
{
	UNICODE_STRING OTP;
	KERB_LOGON_SUBMIT_TYPE passType;
}CREDS_WITH_2FA;

typedef struct _CREDS_WITH_CARD_ID : public KERB_INTERACTIVE_UNLOCK_LOGON
{
	unsigned __int64 CardID;
	KERB_LOGON_SUBMIT_TYPE passType;
}CREDS_WITH_CARD_ID;

typedef struct _CREDS_AUTOLOGIN
{
	KERB_LOGON_SUBMIT_TYPE logonType;
	__int64 cardID;
}CREDS_AUTOLOGIN;

#include <map>

//NamedPipeClient NPC;

#define HEADERV2_BIND_LUID 0x23
#define HEADERV2_UNBIND_LUID 0x24

std::map<__int64, wchar_t*> maps;

__int64 LUIDtoInt64(LUID& v)
{
	return ((__int64(v.HighPart) << 32) + (__int64(v.LowPart)));
}

char* cname = "MiTokenAP";
wchar_t* name = L"MiTokenAP";


LSA_DISPATCH_TABLE _lsaDispatchTable;



int getInfoHitCount = 0;

NTSTATUS NTAPI LsaPost_GetInfo ( PSecPkgInfo PackageInfo )
{
	DEBUG_MESSAGE("LsaPost : GetInfo");
	wchar_t buffer[400];
	PackageInfo->Name = (SEC_WCHAR*)CoTaskMemAlloc((1 + wcslen(name)) * sizeof(wchar_t));
	memcpy(PackageInfo->Name, name, (wcslen(name) + 1) * sizeof(wchar_t));
	getInfoHitCount++;
	swprintf_s(buffer, L"HC=[%d]\nPI = [%08lX]\nCapabilities = %08lX\nVersion = %d\nRPCID = %d\nMaxToken = %d\nName [%08lX] = %s\nComment [%08lX] = %s\n", getInfoHitCount, PackageInfo, PackageInfo->fCapabilities, PackageInfo->wVersion, PackageInfo->wRPCID, PackageInfo->cbMaxToken, PackageInfo->Name, PackageInfo->Name, PackageInfo->Name, PackageInfo->Comment);
	DEBUG_WMESSAGE(buffer);
	//change the package name
	


	return STATUS_SUCCESS;
	
}

NTSTATUS NTAPI LsaPre_GetExtendedInfo(SECPKG_EXTENDED_INFORMATION_CLASS C, PSECPKG_EXTENDED_INFORMATION *I)
{
	DEBUG_MESSAGE("LsaPre : GetExtendedInfo ");
	DEBUG_LOG("Class ID [%d]\n", C);

	return STATUS_SUCCESS;
}
NTSTATUS NTAPI LsaPre_GetInfo( PSecPkgInfo PackageInfo )
{
	DEBUG_MESSAGE("LsaPre : GetInfo");
	char buffer[200];
	sprintf_s(buffer, "PI = [%08lX]\nName = [%08lX]\nComment = [%08lX]\n", PackageInfo, PackageInfo->Name, PackageInfo->Comment);
	DEBUG_MESSAGE(buffer);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI LsaPre_SpInitLsaModeContext(
  __in   LSA_SEC_HANDLE CredentialHandle,
  __in   LSA_SEC_HANDLE ContextHandle,
  __in   PUNICODE_STRING TargetName,
  __in   ULONG ContextRequirements,
  __in   ULONG TargetDataRep,
  __in   PSecBufferDesc InputBuffers,
  __out  PLSA_SEC_HANDLE NewContextHandle,
  __out  PSecBufferDesc OutputBuffers,
  __out  PULONG ContextAttributes,
  __out  PTimeStamp ExpirationTime,
  __out  PBOOLEAN MappedContext,
  __out  PSecBuffer ContextData
)
{
	DEBUG_LOG(" LsaPre : InitLsaModeContext : TargetName = ");
	wchar_t buffer[250] = {0};
	memcpy(buffer, TargetName->Buffer, TargetName->Length);
	DEBUG_LOGW(L"%s [%d : %d]\n", buffer, ContextRequirements, TargetDataRep);

	return STATUS_SUCCESS;
}

void NTAPI LsaPre_LogonTerminated(PLUID LogonID)
{
	DEBUG_LOG(" LUID = [%08lX : %08lX] ", LogonID->HighPart, LogonID->LowPart);

}

NTSTATUS NTAPI LsaPre_AcceptCreds(SECURITY_LOGON_TYPE LogonType, PUNICODE_STRING AccountName, PSECPKG_PRIMARY_CRED PrimCreds, PSECPKG_SUPPLEMENTAL_CRED SuppCreds)
{
	wchar_t buffer[250] = {0};
	memcpy(buffer, AccountName->Buffer, AccountName->Length);
	DEBUG_LOGW(L" Type = %d, Name = %s, LUID = [ %08lX : %08lX ] ", LogonType, buffer, PrimCreds->LogonId.HighPart, PrimCreds->LogonId.LowPart);

	return STATUS_SUCCESS;
}

void overridePassthroughsLSA(SECPKG_FUNCTION_TABLE* pre_passthroughs, SECPKG_FUNCTION_TABLE* post_passthroughs)
{
	DEBUG_LOG("Override Passthroughs LSA\n");
	//No Overrides in the current version
#if 0
	pre_passthroughs->GetInfo = &LsaPre_GetInfo;
	pre_passthroughs->InitLsaModeContext = &LsaPre_SpInitLsaModeContext;

	pre_passthroughs->LogonTerminated = &LsaPre_LogonTerminated;

	pre_passthroughs->AcceptCredentials = &LsaPre_AcceptCreds;

	pre_passthroughs->GetExtendedInformation = &LsaPre_GetExtendedInfo;

#endif
	post_passthroughs->GetInfo = &LsaPost_GetInfo;
	
//	pre_passthroughs->LogonUserEx2 = &LsaPre_LogonUserEx2;
//	post_passthroughs->LogonUserEx2 = &LsaPost_LogonUserEx2;
//	post_passthroughs->LogonTerminated = &LsaPost_LogonTerminated;

}

void overridePassthroughsSSP(LSA_SECPKG_FUNCTION_TABLE* pre_passthroughTable, LSA_SECPKG_FUNCTION_TABLE* post_passthroughTable)
{
}

void overridePassthroughsUSR(SECPKG_USER_FUNCTION_TABLE* pre_passthroughTable, SECPKG_USER_FUNCTION_TABLE* post_passthroughTable)
{}

class storedCred
{
public:
	char* username;
	unsigned int usernameLength;
	unsigned int linkCount;
	__int64* linkedCardIDs;
	int linkedCardCount;
	char* password;
	unsigned int passwordLength;
	char* domain;
	unsigned int domainLength;
};

std::map<__int64, storedCred*> dataMap;
std::map<__int64, __int64> cardLUT;


void printStoredCred(storedCred* pDB)
{
	DEBUG_LOG("STORED CRED [BINARY]: \n");
	DEBUG_LOG("[");
	DEBUG_LOGBYTES((unsigned char*)pDB, sizeof(*pDB));
	DEBUG_LOGHEXBYTES((unsigned char*)pDB, sizeof(*pDB));
	DEBUG_LOG("{");
	DEBUG_LOGHEXBYTES(pDB->domain, pDB->domainLength);
	DEBUG_LOG("}");
	DEBUG_LOG("{");
	DEBUG_LOGHEXBYTES(pDB->username, pDB->usernameLength);
	DEBUG_LOG("}");
	DEBUG_LOG("{");
	DEBUG_LOGHEXBYTES(pDB->password, pDB->passwordLength);
	DEBUG_LOG("}]\n");
	DEBUG_LOG("STORED CRED [HUMAN READABLE]:\nUsername ptr : %08lX [%d]\nPassword ptr : %08lX [%d]\nDomain ptr : %08lX [%d]\nLinked CIDs ptr : %08lX [%d]\nLUID link count %d",
			pDB->username, pDB->usernameLength, pDB->password, pDB->passwordLength, pDB->domain, pDB->domainLength, pDB->linkedCardIDs, pDB->linkedCardCount, pDB->linkCount);
}

void printAllCreds()
{
	for(std::map<__int64, storedCred*>::iterator it = dataMap.begin(); it != dataMap.end(); it++)
	{
		printStoredCred((*it).second);
	}
}

__int64 LUIDtoInt64(PLUID pLUID)
{
	DEBUG_LOG("Converting LUID to Int64\n");
	return ((unsigned __int64)pLUID->HighPart << 32) | ((unsigned __int64)pLUID->LowPart);
}
void bindLUIDtoCID(__int64 LUID, __int64 CID)
{
	
	DEBUG_LOG("bind LUID [%I64d] to CID [%I64d]\n", LUID, CID);
	if(dataMap.count(CID) == 0)
	{
		DEBUG_LOG("ERROR! CID does not exist in the card map\n");
		//error. CID doesn't exist in the card map
	}
	else
	{
		//We will use Verbose logging for finding missing links
		DEBUG_VERBOSE("%08lX : [%d] += %I64d", dataMap[CID], dataMap[CID]->linkCount, LUID);

		DEBUG_LOG("Creating Bind\n");
		cardLUT[LUID] = CID;
		DEBUG_LOG("Incrementing Link Count\n");
		dataMap[CID]->linkCount++; //one more LUID is linked to this CID
		DEBUG_LOG("The Cresd assossiated with CID %I64d now has %d LUIDs bound to them\n", CID, dataMap[CID]->linkCount);
	}
}

void removeLUID(__int64 LUID)
{
	DEBUG_LOG("Remove LUID [%I64d]\n", LUID);
	if(cardLUT.count(LUID) == 0)
	{
		DEBUG_LOG("LUID %I64d does not exist in the map\n", LUID);
		return; //it doesn't exist to begin with
	}
	__int64 CID = cardLUT[LUID];
	if(dataMap.count(CID) == 0)
	{
		DEBUG_LOG("ERROR! LUID existed in the Map, However the CID it points to (%I64d) does not exist in the card map\n", CID);
		//error. CID doesn't exist in card Map
	}
	else
	{
		//We will use Verbose logging for finding missing links
		DEBUG_VERBOSE("%08lX : [%d] -= %I64d", dataMap[CID], dataMap[CID]->linkCount, LUID);

		DEBUG_LOG("Decrementing dataMap[CID]'s link Count\n");
		dataMap[CID]->linkCount--;
		DEBUG_LOG("dataMap[CID]'s currently has %d links\n", dataMap[CID]->linkCount);
		
		DEBUG_LOG("Erasing the link between LUID and CID\n");
		cardLUT.erase(LUID);
		if(dataMap[CID]->linkCount == 0)
		{
			DEBUG_LOG("All links are gone, removing cached cred\n");
			//all the links are gone - remove the data blob
			storedCred* pDB = dataMap[CID];
			//remove all CID -> pDB maps
			DEBUG_LOG("Cred has %d CIDs linked to it, Removing them\n", pDB->linkedCardCount);
			for(int i = 0 ; i < pDB->linkedCardCount ; ++i)
			{
				if(dataMap.count(pDB->linkedCardIDs[i]) == 1)
				{
					DEBUG_LOG("CID [%I64d] Removed from the dataMap\n", pDB->linkedCardIDs[i]);
					dataMap.erase(pDB->linkedCardIDs[i]);
				}
				else
				{
					DEBUG_LOG("ERROR! CID [%I64d] was linked to Cred, but doesn't exist in the dataMap\n", pDB->linkedCardIDs[i]);
					//error. CID doesn't exist in card Map
				}
			}
			DEBUG_LOG("Freeing linked Card IDs\n");
			free(pDB->linkedCardIDs);
			DEBUG_LOG("Freeing Domain + Pass + Username\n");
			free(pDB->domain);
			free(pDB->password);
			free(pDB->username);
			DEBUG_LOG("Freeing Cached Cred\n");
			free(pDB);
			DEBUG_LOG("Everything has been freed\n");
		}
	}
}

void addNewCID(__int64 CID, char* dataBlob, int usernameStart, int usernameLength, int domainStart, int domainLength, int passwordStart, int passwordLength)
{
	DEBUG_LOG("Add New CID [%I64d] [%d:%d] [%d:%d] [%d:%d]\n", CID, usernameStart, usernameLength, domainStart, domainLength, passwordStart, passwordLength);
	//first we need to check if any of the current card maps have the same username
	__int64 otherCID = 0;
	bool found = false;
	for(std::map<__int64, storedCred*>::iterator it = dataMap.begin() ; it != dataMap.end(); it++)
	{
		char* storedUsername = it->second->username;
		int storedUsernameLen = it->second->usernameLength;
		if(storedUsernameLen == usernameLength)
		{
			if(memcmp(storedUsername, dataBlob + usernameStart, usernameLength) == 0)
			{
				DEBUG_LOG("User with supplied username already exists bound to card %I64d\n", it->first);
				otherCID = it->first;
				found = true;
				break;
			}
		}
	}
	if(found)
	{
		DEBUG_LOG("Adding CID [%I64d] to the linked card list\n", CID);
		storedCred* pDB = dataMap[otherCID];
		//add the card
		//Create a new mem location
		DEBUG_LOG("Allocating new memory buffer\n");
		__int64* linkedCardBuffer = (__int64*)calloc(pDB->linkedCardCount + 1, sizeof(__int64));
		//copy over the old data
		DEBUG_LOG("Copying old memory buffer\n");
		memcpy(linkedCardBuffer, pDB->linkedCardIDs, pDB->linkedCardCount * sizeof(__int64));
		//add new the data
		DEBUG_LOG("Appending new CID\n");
		memcpy(linkedCardBuffer + pDB->linkedCardCount, &CID, sizeof(__int64));
		//free the old data
		DEBUG_LOG("Freeing old data\n");
		free(pDB->linkedCardIDs);
		//set the new mem location
		
		pDB->linkedCardIDs = linkedCardBuffer;
		pDB->linkedCardCount++;
		DEBUG_LOG("There are now %d cards linked\n", pDB->linkedCardCount);
		linkedCardBuffer = NULL;

		//link the new map value to point to the same pointer
		DEBUG_LOG("Adding a new link between CID [%I64d] and the shared cred\n",CID);
		dataMap[CID] = pDB; 
	}
	else
	{
		//create a new pDB
		DEBUG_LOG("Creating a new Stored Cred\n");
		storedCred* pDB = (storedCred*)calloc(1, sizeof(storedCred));
		pDB->linkCount = 0;
		DEBUG_LOG("Setting lengths\n");
		//set the lengths
		pDB->domainLength = domainLength;
		pDB->passwordLength = passwordLength;
		pDB->usernameLength = usernameLength;
		DEBUG_LOG("Allocating Space\n");
		//allocate space
		pDB->domain = (char*)calloc(domainLength, sizeof(char));
		pDB->password = (char*)calloc(passwordLength, sizeof(char));
		pDB->username = (char*)calloc(usernameStart, sizeof(char));
		//memcpy
		DEBUG_LOG("Copying memory buffers\n");
		memcpy(pDB->domain, dataBlob + domainStart, domainLength);
		memcpy(pDB->password, dataBlob + passwordStart, passwordLength);
		memcpy(pDB->username, dataBlob + usernameStart, usernameLength);
		//set linked cards
		DEBUG_LOG("Setting 1 linked card\n");
		pDB->linkedCardIDs = (__int64*)calloc(1, sizeof(__int64));
		*pDB->linkedCardIDs = CID;
		pDB->linkedCardCount = 1;

		//and store it
		DEBUG_LOG("Storing a link in the dataMap between CID and the stored Cred\n");
		dataMap[CID] = pDB;

		printAllCreds();

	}

}

void debugUnicodeString(wchar_t* heading, wchar_t* output, UNICODE_STRING u, void* base)
{
	DEBUG_SENSITIVEW(true, L"%s [%d : %d] = %s  [@%08lX]\n\t", heading, u.Buffer, u.Length, output, output);
	DEBUG_SENSITIVEBYTES(true, (unsigned char*)base + (int)u.Buffer, u.Length);
	DEBUG_SENSITIVE(true, "\n");

}

void copyUnicodeString(wchar_t* output, UNICODE_STRING ustr, void* base)
{
	memcpy(output, (unsigned char*)base + (int)(ustr.Buffer), ustr.Length);
	output[ustr.Length / 2] = 0;
}

void debugLogSidAndAtts(PSID_AND_ATTRIBUTES p)
{
	LPTSTR sidStr = NULL;
	if(ConvertSidToStringSid(p->Sid, &sidStr))
	{
		DEBUG_LOGW(L"%s => %08lX", sidStr, p->Attributes);
		LocalFree(sidStr);
	}
	else
	{
		DEBUG_LOG("Sid -> String Sid Failed [%08lX]", GetLastError());
	}
}

void debugLogSid(PSID p)
{
	if(p)
	{
		LPTSTR sidStr = NULL;
		if(ConvertSidToStringSid(p, &sidStr))
		{
			DEBUG_LOGW(L"%s", sidStr);
			LocalFree(sidStr);
		}
		else
		{
			DEBUG_LOG("Sid -> String Sid Failed [%08lX]", GetLastError());
		}
	}
	else
	{
		DEBUG_LOG("[NULL]");
	}
}

NTSTATUS NTAPI LsaApInitializePackage(
  _In_      ULONG AuthenticationPackageId,
  _In_      PLSA_DISPATCH_TABLE LsaDispatchTable,
  _In_opt_  PLSA_STRING Database,
  _In_opt_  PLSA_STRING Confidentiality,
  _Out_     PLSA_STRING *AuthenticationPackageName
)
{
	_packageID = AuthenticationPackageId;
	memcpy(&_lsaDispatchTable, LsaDispatchTable, sizeof(_lsaDispatchTable));
	*AuthenticationPackageName = (PLSA_STRING)_lsaDispatchTable.AllocateLsaHeap(sizeof(LSA_STRING));
	(*AuthenticationPackageName)->Length = strlen(cname) + 1;
	(*AuthenticationPackageName)->MaximumLength = strlen(cname) + 1;
	(*AuthenticationPackageName)->Buffer = (PCHAR)_lsaDispatchTable.AllocateLsaHeap((*AuthenticationPackageName)->MaximumLength);
	memcpy((*AuthenticationPackageName)->Buffer, cname, strlen(cname));

	return STATUS_SUCCESS;
}

NTSTATUS NTAPI LsaOverride_LogonUserEx2(
  __in   PLSA_CLIENT_REQUEST ClientRequest,
  __in   SECURITY_LOGON_TYPE LogonType,
  __in   PVOID ProtocolSubmitBuffer,
  __in   PVOID ClientBufferBase,
  __in   ULONG SubmitBufferSize,
  __out  PVOID *ProfileBuffer,
  __out  PULONG ProfileBufferSize,
  __out  PLUID LogonId,
  __out  PNTSTATUS SubStatus,
  __out  PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
  __out  PVOID *TokenInformation,
  __out  PUNICODE_STRING *AccountName,
  __out  PUNICODE_STRING *AuthenticatingAuthority,
  __out  PUNICODE_STRING *MachineName,
  __out  PSECPKG_PRIMARY_CRED PrimaryCredentials,
  __out  PSECPKG_SUPPLEMENTAL_CRED_ARRAY *SupplementalCredentials
)
{
	DEBUG_LOG("%s", "In Function LsaOverride_LogonUserEx2\n");
	NTSTATUS stat;
	//First find out what type of message we are dealing with
	
	mutexLock(0x02);
	int MID = *(int*)ProtocolSubmitBuffer;
	DEBUG_LOG("Received request of %d Bytes with MID = %d\n", SubmitBufferSize, MID);
	if(MID < 16)
	{
		DEBUG_LOG("MID < 16. Therefore passing message unaltered to MSV1_0 to process\n");
		 //MID is in the range that is dealt with directly by MSV1_0. We do nothing with this range
		NTSTATUS retValue =  RemoteFuncTables.LsaModeTable.LogonUserEx2(ClientRequest, LogonType, ProtocolSubmitBuffer, ClientBufferBase, SubmitBufferSize, ProfileBuffer, ProfileBufferSize, LogonId, SubStatus,
												TokenInformationType, TokenInformation, AccountName, AuthenticatingAuthority, MachineName, PrimaryCredentials, SupplementalCredentials);

		mutexUnlock();
		return retValue;
	}

	if(MID == MESSAGE_WITH2FA)
	{
		NTSTATUS ntstat = STATUS_UNSUCCESSFUL;
		DEBUG_LOG("MID == MESSAGE_WITH2FA\n");
		//PSB is a CREDS_WITH2FA structure
		CREDS_WITH_2FA* pC2FA = (CREDS_WITH_2FA*)ProtocolSubmitBuffer;
		//grab the domain / username / otp into new buffers
		wchar_t *domain, *username, *otp, *pass;
		int domLen, useLen, otpLen, pasLen;
		domLen = pC2FA->Logon.LogonDomainName.Length;
		useLen = pC2FA->Logon.UserName.Length;
		otpLen = pC2FA->OTP.Length;
		pasLen = pC2FA->Logon.Password.Length;
		domain = (wchar_t*)CoTaskMemAlloc(2 + pC2FA->Logon.LogonDomainName.Length);
		username = (wchar_t*)CoTaskMemAlloc(2 + pC2FA->Logon.UserName.Length);
		otp = (wchar_t*)CoTaskMemAlloc(2 + pC2FA->OTP.Length);
		pass = (wchar_t*)CoTaskMemAlloc(2 + pasLen);
		DEBUG_SENSITIVE(true, "pC2FA Bytes:\n");
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)pC2FA, SubmitBufferSize);
		DEBUG_SENSITIVE(true, "\n");
		copyUnicodeString(domain, pC2FA->Logon.LogonDomainName, pC2FA);
		debugUnicodeString(L"Domain", domain, pC2FA->Logon.LogonDomainName, pC2FA);

		copyUnicodeString(username, pC2FA->Logon.UserName, pC2FA);
		debugUnicodeString(L"Username", username, pC2FA->Logon.UserName, pC2FA);

		copyUnicodeString(otp, pC2FA->OTP, pC2FA);
		debugUnicodeString(L"OTP", otp, pC2FA->OTP, pC2FA);

		copyUnicodeString(pass, pC2FA->Logon.Password, pC2FA);
		debugUnicodeString(L"Pass", pass, pC2FA->Logon.Password, pC2FA);

		DEBUG_SENSITIVEW(true, L"Domain / Username / Pass / OTP = %s / %s / %s / %s\n", domain, username, pass, otp);
		
		wchar_t *domainAndUser = (wchar_t*)CoTaskMemAlloc((domLen + useLen + 4));
		memset(domainAndUser, 0, (domLen + useLen + 4));
		memcpy(domainAndUser, domain, domLen);
		domainAndUser[domLen / 2] = L'\\'; /* /2 as domLen is how many bytes whilst domainAndUser is in wchar_t*/
		memcpy(domainAndUser + (domLen / 2) + 1, username, useLen);

		wchar_t messageOut[250];
		if(checkOTP(username, otp, otp, messageOut, 250, domainAndUser))
		{
			DEBUG_LOG("OTP Validated, Preparing for MSV1_0 call\n");
			//package a request to send of to MSV1_0 for user/pass auth
			int cb = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) + (domLen + pasLen + useLen);
			
			KERB_INTERACTIVE_UNLOCK_LOGON *pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*)CoTaskMemAlloc(cb);
			pkil->Logon.MessageType = pC2FA->passType;
			pkil->Logon.Password.Length = pkil->Logon.Password.MaximumLength = pasLen;
			pkil->Logon.LogonDomainName.Length = pkil->Logon.LogonDomainName.MaximumLength = domLen;
			pkil->Logon.UserName.Length = pkil->Logon.UserName.MaximumLength = useLen;
			DEBUG_LOG("Appending Logon Information onto the buffer\n");
			void* vp = (void*)((int)pkil + sizeof(KERB_INTERACTIVE_UNLOCK_LOGON));
			//Domain Name
			pkil->Logon.LogonDomainName.Buffer = (PWSTR)((int)vp - (int)pkil);
			memcpy(vp, domain, domLen);
			vp = (void*)((int)vp + domLen);
			//Username
			pkil->Logon.UserName.Buffer = (PWSTR)((int)vp - (int)pkil);
			memcpy(vp, username, useLen);
			vp = (void*)((int)vp + useLen);
			//Password
			pkil->Logon.Password.Buffer = (PWSTR)((int)vp - (int)pkil);
			memcpy(vp, pass, pasLen);

			
			pC2FA->Logon.MessageType = pC2FA->passType;
			LSA_TOKEN_INFORMATION_V2 *pTT2;
				
			DEBUG_VERBOSE("MSV1_0 Buffer [v1.0.1] : [%d]\n", cb);
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)pkil, cb);
			DEBUG_VERBOSE("\n");
			DEBUG_LOG("Sending the buffer to MSV1_0 [%08lX v %08lX]\n", pkil, ClientBufferBase);
			try
			{
				int tokenMode = 0;

				if(tokenMode == 0)
				{
					/* MSV1_0 passthrough */
					ntstat = RemoteFuncTables.LsaModeTable.LogonUserEx2(ClientRequest, LogonType, pkil, ClientBufferBase, cb, ProfileBuffer, ProfileBufferSize, LogonId, SubStatus,
														TokenInformationType, TokenInformation, AccountName, AuthenticatingAuthority, MachineName, PrimaryCredentials, SupplementalCredentials);

					DEBUG_VERBOSE("\n\nNTSTAT = %d\n\n", ntstat);

					if(ntstat == 0)
					{
						/*
						//everything worked, lets chsange the LastLoggedSAMUser value - RDP doesn't automatically do this
						HKEY regKey;
						LONG keyResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI", 0, KEY_READ | KEY_WRITE, &regKey);
						if(keyResult == ERROR_SUCCESS)
						{
							DWORD RegSetValueEx(regKey, 
						}
						*/

					}
					pTT2 = (LSA_TOKEN_INFORMATION_V2*)(*TokenInformation);
				}
				else
				{
					/* create our own stuff */
					{
						DEBUG_LOG("Generating AP Buffer locally\n");
						/* generate token information */
						int tokensize;
						PSID psid = NULL;
						SID_NAME_USE use;
						DWORD cbsid = 0;
						DWORD domSize = 0;
						wchar_t* domName = 0;
						void* scratch;
						/* get the account sid */
						LookupAccountName(domain, username, NULL, &cbsid, domName, &domSize, &use);
						psid = malloc(cbsid);
						if(psid == NULL)
							return ERROR_NOT_ENOUGH_MEMORY;

						//domSize = 0; /* reset domSize to 0, we don't actually want the dom */
						domName = (wchar_t*)malloc(domSize * 2 + 2);
						memset(domName, 0, domSize * 2 + 2);
						LookupAccountName(domain, username, psid, &cbsid, domName, &domSize, &use);

					
					
						
						PSID groupSid;
						DWORD cbGroupSid = 0, cbGroupDomSize = 0;
						wchar_t* groupDomName;
						SID_NAME_USE groupUse;

						LookupAccountName(domain, L"None", NULL, &cbGroupSid, NULL, &cbGroupDomSize, &groupUse);
						groupSid = malloc(cbGroupSid);
						if(groupSid == NULL)
							return ERROR_NOT_ENOUGH_MEMORY;

						groupDomName = (wchar_t*)malloc(cbGroupDomSize * 2 + 2);
						memset(groupDomName, 0, cbGroupDomSize * 2 + 2);
						LookupAccountName(domain, L"None", groupSid, &cbGroupSid, groupDomName, &cbGroupDomSize, &groupUse);


						DEBUG_LOG("SID Size : %d and %d\n", cbsid, cbGroupSid);

						/*
						for this test lets use a static group ID, we will get this working properly once the AP is working fine
						The size is 0x1c per group (there are 2)
						*/
					
						DEBUG_LOG("Calculating size of token buffer\n");

						*TokenInformationType = LsaTokenInformationV2;
						tokensize = sizeof(LSA_TOKEN_INFORMATION_V2);
						tokensize += sizeof(TOKEN_GROUPS);
						tokensize += cbsid;
						tokensize += (cbGroupSid * 2);

						DEBUG_LOG("Token buffer is expected to be %d bytes\n", tokensize);

						*TokenInformation = secpkg_fcn.AllocateLsaHeap(tokensize);

						if(*TokenInformation == NULL)
							return ERROR_NOT_ENOUGH_MEMORY;
					
						DEBUG_LOG("Writing data to PTIV2\n");
						LSA_TOKEN_INFORMATION_V2 * pTIV2;
						pTIV2 = (LSA_TOKEN_INFORMATION_V2*)*TokenInformation;
						scratch = (void*)(pTIV2 + 1);

						pTIV2->ExpirationTime.HighPart = 0x7FFFFFFF;
						pTIV2->ExpirationTime.LowPart = 0xFFFFFFFF;
						pTIV2->User.User.Sid = (PSID)scratch;
						CopySid( cbsid, pTIV2->User.User.Sid, psid);
						scratch = (void*)((int)scratch + cbsid);
						pTIV2->User.User.Attributes = 0x0; //TODO - Figure out where this magic number comes from
						pTIV2->Groups = (PTOKEN_GROUPS)scratch;
						pTIV2->Groups->GroupCount = 1;
						scratch = (void*)((int)scratch + sizeof(TOKEN_GROUPS));
						pTIV2->Groups->Groups[0].Attributes = 7;
						pTIV2->Groups->Groups[0].Sid = scratch;
						CopySid( cbGroupSid, pTIV2->Groups->Groups[0].Sid, groupSid);
						scratch = (void*)((int)scratch + cbGroupSid);

						pTIV2->PrimaryGroup.PrimaryGroup = scratch;
						CopySid( cbGroupSid, pTIV2->PrimaryGroup.PrimaryGroup, groupSid);
						scratch = (void*)((int)scratch + cbGroupSid);

						pTIV2->Privileges = NULL;
						pTIV2->Owner.Owner = NULL;
						pTIV2->DefaultDacl.DefaultDacl = NULL;
						/* NO TokenGroups SIDS [NULL] */
					
						DEBUG_LOG("Getting LUID :\n");
						/* Create Logon Session */
						if(!AllocateLocallyUniqueId(LogonId))
						{
							DEBUG_LOG("Failed\n");
							secpkg_fcn.FreeLsaHeap(pTIV2);
							*TokenInformation = NULL;
							return STATUS_INSUFFICIENT_RESOURCES;
						}
						DEBUG_LOG("\tDone [%d`%d]\n", LogonId->HighPart, LogonId->LowPart);

						DEBUG_LOG("Creating Logon Session\n");
						if((ntstat = secpkg_fcn.CreateLogonSession(LogonId)) != STATUS_SUCCESS)
						{
							DEBUG_LOG("Failed [%d]\n", ntstat);
							secpkg_fcn.FreeLsaHeap(pTIV2);
							*TokenInformation = NULL;
							return stat;
						}
						DEBUG_LOG("Created\n");

						DEBUG_LOG("Creating AccountName/AuthenAuth/MachineName\n");
						/* Create AccountName */
						(*AccountName) = (UNICODE_STRING*)secpkg_fcn.AllocateLsaHeap(sizeof(UNICODE_STRING));
						(*AccountName)->Length =  useLen;
						(*AccountName)->MaximumLength = useLen + 2;
						(*AccountName)->Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(useLen + 2));
						memset((*AccountName)->Buffer, 0, useLen + 2);
						memcpy((*AccountName)->Buffer, username, useLen);

						/* Create AuthenticatingAuth */
						(*AuthenticatingAuthority) = (UNICODE_STRING*)secpkg_fcn.AllocateLsaHeap(sizeof(UNICODE_STRING));
						(*AuthenticatingAuthority)->Length = domLen;
						(*AuthenticatingAuthority)->MaximumLength = domLen + 2;
						(*AuthenticatingAuthority)->Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(domLen + 2));
						memset((*AuthenticatingAuthority)->Buffer, 0, domLen + 2);
						memcpy((*AuthenticatingAuthority)->Buffer, domain, domLen);

						/* Create MachineName */
						(*MachineName) = (UNICODE_STRING*)secpkg_fcn.AllocateLsaHeap(sizeof(UNICODE_STRING));
						(*MachineName)->Length = domLen;
						(*MachineName)->MaximumLength = domLen + 2;
						(*MachineName)->Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(domLen + 2));
						memset((*MachineName)->Buffer, 0, domLen + 2);
						memcpy((*MachineName)->Buffer, domain, domLen);

						DEBUG_LOG("\tDone\n");

						DEBUG_LOG("Creating Prim Creds\n");
						/* Create Prim Creds */
						PrimaryCredentials->LogonId = *LogonId;
						PrimaryCredentials->DownlevelName.Length = useLen;
						PrimaryCredentials->DownlevelName.MaximumLength = useLen;
						PrimaryCredentials->DownlevelName.Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(useLen));
						memcpy(PrimaryCredentials->DownlevelName.Buffer, username, useLen);

						PrimaryCredentials->DomainName.Length = PrimaryCredentials->DomainName.MaximumLength = domLen;
						PrimaryCredentials->DomainName.Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(domLen));
						memcpy(PrimaryCredentials->DomainName.Buffer, domain, domLen);

						PrimaryCredentials->Password.Length = PrimaryCredentials->Password.MaximumLength = pasLen;
						PrimaryCredentials->Password.Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(pasLen));
						memcpy(PrimaryCredentials->Password.Buffer, pass, pasLen);

						PrimaryCredentials->OldPassword.Length = PrimaryCredentials->OldPassword.MaximumLength = 0;
						PrimaryCredentials->OldPassword.Buffer = NULL;

						PrimaryCredentials->UserSid = (PSID)secpkg_fcn.AllocateLsaHeap(cbsid);
						CopySid(cbsid, PrimaryCredentials->UserSid, psid);

						PrimaryCredentials->Flags = 0xa0000001; //TODO - What is this for?

						PrimaryCredentials->DnsDomainName.Length = PrimaryCredentials->DnsDomainName.MaximumLength = 0;
						PrimaryCredentials->DnsDomainName.Buffer = NULL;

						PrimaryCredentials->Upn.Length = PrimaryCredentials->Upn.MaximumLength = 0;
						PrimaryCredentials->Upn.Buffer = NULL;

						/* Logon Server - for the moment just use the same as the domain - not valid but will fix later */
						PrimaryCredentials->LogonServer.Length = PrimaryCredentials->LogonServer.MaximumLength = domLen;
						PrimaryCredentials->LogonServer.Buffer = (wchar_t*)(secpkg_fcn.AllocateLsaHeap(domLen));
						memcpy(PrimaryCredentials->LogonServer.Buffer, domain, domLen);

						PrimaryCredentials->Spare1.Length = PrimaryCredentials->Spare1.MaximumLength = 0;
						PrimaryCredentials->Spare2.Length = PrimaryCredentials->Spare2.MaximumLength = 0;
						PrimaryCredentials->Spare3.Length = PrimaryCredentials->Spare3.MaximumLength = 0;
						PrimaryCredentials->Spare4.Length = PrimaryCredentials->Spare4.MaximumLength = 0;
						PrimaryCredentials->Spare1.Buffer = PrimaryCredentials->Spare2.Buffer = NULL;
						PrimaryCredentials->Spare3.Buffer = PrimaryCredentials->Spare4.Buffer = NULL;

						DEBUG_LOG("\tDone\n");



						/* Create SecCreds */
						SupplementalCredentials = NULL;

						/*
							AddCredential
							-We need to call LSA's AddCredential function to add our credential to the store
							-For this POC, we will just use the username as the primary key, and the pass as the credentials
								-If this works, we should look into using a better key/value pair
									(Note, it is actually more of a key/multi-value map)
						*/

						DEBUG_LOG("Creating Key/Val for AddCred\n");
						PLSA_STRING key;
						PLSA_STRING value;
						key = (PLSA_STRING)secpkg_fcn.AllocateLsaHeap(sizeof(LSA_STRING));
						value = (PLSA_STRING)secpkg_fcn.AllocateLsaHeap(sizeof(LSA_STRING));
						//at the moment just use a static key and value pair
						key->Length = key->MaximumLength = 4;
						key->Buffer = (PCHAR)secpkg_fcn.AllocateLsaHeap(4);
						memcpy(key->Buffer, "POC", 4);
						value->Length = value->MaximumLength = 4;
						value->Buffer = (PCHAR)secpkg_fcn.AllocateLsaHeap(4);
						memcpy(key->Buffer, "POD",  4);
						DEBUG_LOG("Calling AddCred\n");
						ntstat = secpkg_fcn.AddCredential(LogonId, _packageID, key, value);
						DEBUG_LOG("\tDone [%d]\n", ntstat);
					}
				}
			}
			catch(char* s)
			{
				DEBUG_LOG("Exception %s\n", s);
				ntstat = STATUS_LOGON_FAILURE;
			}
			catch(wchar_t* ws)
			{
				DEBUG_LOGW(L"Exception[w] %s\n", ws);
				ntstat = STATUS_LOGON_FAILURE;
			}
			catch(...)
			{
				DEBUG_LOG("Exception Caught\n");
				ntstat = STATUS_LOGON_FAILURE;
			}

			if(ntstat == STATUS_SUCCESS)
			{
				DEBUG_LOG("Login Succeeded\n");
				/*
				DEBUG_LOG("Writing output values...\n");
				DEBUG_LOG("Profile Buffer [%d bytes] : ", *ProfileBufferSize);
				DEBUG_BYTES((unsigned char*)ProfileBuffer, *ProfileBufferSize);
				DEBUG_LOG("\nLUID [%08lX : %08lX]\n", LogonId->HighPart, LogonId->LowPart);
				DEBUG_LOG("Substatus : %d\n", *SubStatus);
				DEBUG_LOG("Token Type : %d\n", *TokenInformationType);
				switch(*TokenInformationType)
				{
				case LsaTokenInformationNull:
					{
						LSA_TOKEN_INFORMATION_NULL * pLsaTIN = (LSA_TOKEN_INFORMATION_NULL *)TokenInformation;
						DEBUG_LOG("\tExpiration Time [%08lX : %08lX]\n", pLsaTIN->ExpirationTime.HighPart, pLsaTIN->ExpirationTime.LowPart);
						DEBUG_LOG("\tSIDS : [%d]\n", pLsaTIN->Groups->GroupCount);
						for(int i = 0 ; i < pLsaTIN->Groups->GroupCount ; ++i)
						{
							SID_AND_ATTRIBUTES* pSAA = (SID_AND_ATTRIBUTES*)&(pLsaTIN->Groups->Groups[i]);
							DEBUG_LOG("\t\t[%d] : ", i);
							debugLogSidAndAtts(pSAA);
							DEBUG_LOG("\n");
						}
					}
					break;
				case LsaTokenInformationV1:
				case LsaTokenInformationV2:

					{
						LSA_TOKEN_INFORMATION_V2 * pLsaTIV = (LSA_TOKEN_INFORMATION_V2 *)TokenInformation;
						DEBUG_LOG("LSA TIV Size [%d] @[%08lX]\n", sizeof(LSA_TOKEN_INFORMATION_V2), TokenInformation);

						DEBUG_LOGBYTES((unsigned char*)TokenInformation, sizeof(LSA_TOKEN_INFORMATION_V2));

						/*
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->ExpirationTime), sizeof(pLsaTIV->ExpirationTime));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->User), sizeof(pLsaTIV->User));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->Groups), sizeof(pLsaTIV->Groups));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->PrimaryGroup), sizeof(pLsaTIV->PrimaryGroup));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->Privileges), sizeof(pLsaTIV->Privileges));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->Owner), sizeof(pLsaTIV->Owner));
						DEBUG_LOG("\n[%02ld : %02ld]", (int)pLsaTIV - (int)&(pLsaTIV->DefaultDacl), sizeof(pLsaTIV->DefaultDacl));


						DEBUG_LOG("\n\tExpiration Time [%08lX : %08lX]\n", pLsaTIV->ExpirationTime.HighPart, pLsaTIV->ExpirationTime.LowPart);
						if(pLsaTIV->Groups != NULL)
						{
							DEBUG_LOG("\tSIDS ");
							DEBUG_LOG("[%08lX] : ", pLsaTIV->Groups);
							TOKEN_GROUPS tg = {0};
							DEBUG_LOG("About to copy data to client buffer [%08lX]\n", secpkg_fcn.CopyToClientBuffer);
							NTSTATUS nts;
							nts = secpkg_fcn.CopyToClientBuffer(ClientRequest, sizeof(TOKEN_GROUPS), &tg, pLsaTIV->Groups);
							
							
							DEBUG_LOG("Copy completed [%08lX] / [%08lX]\n", nts, LsaNtStatusToWinError(nts));
							DEBUG_LOG("[%d]\n", tg.GroupCount);
							for(int i = 0 ; (i < tg.GroupCount) && (i < 10) ; ++i)
							{

								SID_AND_ATTRIBUTES SAA;// = (SID_AND_ATTRIBUTES*)&(pLsaTIV->Groups->Groups[i]);
								DEBUG_LOG("\t\t[%d]\n\t\t\tCopying data...", i);
								DEBUG_LOG(" [%08lX] ", &(tg.Groups[i]));
								secpkg_fcn.CopyToClientBuffer(ClientRequest, sizeof(SID_AND_ATTRIBUTES), &SAA, &tg.Groups[i]);
								DEBUG_LOG("done\n\t\t\t");
								debugLogSidAndAtts(&SAA);
								DEBUG_LOG("\n");
							}
						}
						else
						{
							DEBUG_LOG("\t SIDS : [NULL]\n");
						}
						DEBUG_LOG("\tUser : ");
						debugLogSidAndAtts(&pLsaTIV->User.User);
						DEBUG_LOG("\n\tPrimary Group : ");
						debugLogSid(pLsaTIV->PrimaryGroup.PrimaryGroup);
						if(pLsaTIV->Privileges)
						{
							DEBUG_LOG("\n\tPrivilegeCount : %d\n", pLsaTIV->Privileges->PrivilegeCount);
							for(int i = 0 ; (i < pLsaTIV->Privileges->PrivilegeCount) && (i < 15) ; ++i)
							{
								PLUID_AND_ATTRIBUTES pLAA = (PLUID_AND_ATTRIBUTES)&(pLsaTIV->Privileges->Privileges[i]);
								DEBUG_LOG("\t\t[%d] : LUID [%08lX : %08lX] => %08lX\n", i, pLAA->Luid.HighPart, pLAA->Luid.LowPart, pLAA->Attributes);
							}
						}
						else
						{
							DEBUG_LOG("\n\tPrivileges : [None]\n");
						}

						DEBUG_LOG("\tToken Owner : ");
						debugLogSid(pLsaTIV->Owner.Owner);
						*//*
						DEBUG_LOG("Not printing LsaTokenInformation structure atm\n");
					}
					break;

				default:
					DEBUG_LOG("\tUnknown Token Type\n");
					break;
					*/
				
				/*
				DEBUG_LOG("Account Name [%d] : ", (*AccountName)->Length);
				DEBUG_LOGBYTES((unsigned char*)(*AccountName)->Buffer, (*AccountName)->Length);

				DEBUG_LOG("\nAuthenticating Auth [%d] : ", (*AuthenticatingAuthority)->Length);
				DEBUG_LOGBYTES((unsigned char*)(*AuthenticatingAuthority)->Buffer, (*AuthenticatingAuthority)->Length);

				DEBUG_LOG("\Machine Name  [%d] : ", (*MachineName)->Length);
				DEBUG_LOGBYTES((unsigned char*)(*MachineName)->Buffer, (*MachineName)->Length);
				}
				*/
		
			}
			else
			{
				DEBUG_LOG("Failed [%d]\n", ntstat);
			}
			DEBUG_LOG("1\n");
			
			//CoTaskMemFree(pkil);
			
		}

		
		DEBUG_LOG("2\n");
		CoTaskMemFree(domain);
		DEBUG_LOG("3\n");
		CoTaskMemFree(username);
		DEBUG_LOG("4\n");
		CoTaskMemFree(otp);
		DEBUG_LOG("5\n");
		CoTaskMemFree(pass);
		DEBUG_LOG("6\n");
		CoTaskMemFree(domainAndUser);
		
		DEBUG_LOG("Returning [%d]", ntstat);

		mutexUnlock();
		return ntstat;
	}

	DEBUG_LOG("Received an MID outside the passthrough range (<16) but not equal to an custom MID. Returning INVALID_PARAMETER_1\n");
	mutexUnlock();
	return STATUS_INVALID_PARAMETER_1;

}

void overrideFunctions (functionTables* funcTable)
{
#if 1
	DEBUG_LOG("Override Functions\n");
	//We need a lot more functionality on LogonUserEx2, so override the function call
	funcTable->LsaModeTable.LogonUserEx2 = (PLSA_AP_LOGON_USER_EX2)LsaOverride_LogonUserEx2;
	funcTable->LsaModeTable.InitializePackage = (PLSA_AP_INITIALIZE_PACKAGE)LsaApInitializePackage;
	DEBUG_LOG("Override Functions Done\n");
#endif
}

#else

void overridePassthroughsLSA(SECPKG_FUNCTION_TABLE* pre_passthroughs, SECPKG_FUNCTION_TABLE* post_passthroughs)
{}

void overridePassthroughsSSP(LSA_SECPKG_FUNCTION_TABLE* pre_passthroughTable, LSA_SECPKG_FUNCTION_TABLE* post_passthroughTable)
{}

void overridePassthroughsUSR(SECPKG_USER_FUNCTION_TABLE* pre_passthroughTable, SECPKG_USER_FUNCTION_TABLE* post_passthroughTable)
{}

void overrideFunctions (functionTables* funcTable)
{}
#endif