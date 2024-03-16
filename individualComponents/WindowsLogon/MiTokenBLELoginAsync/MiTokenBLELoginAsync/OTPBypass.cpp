#include "OTPBypass.h"

#include "oath.h"

#include <Sddl.h>

#pragma comment(lib, "netapi32.lib")
#include <lm.h>
#include "DebugLogging.h"
#include <time.h>

#include "../../Gina/OTPCheck/OTPVerifyExParser.h"
#include "MiTokenAPI.h"
#include "MiTokenServers.h"
#include <DsGetDC.h>
#include "MultiString.h"


#define MAX_NAME 256

//Some #defines to make dealing with unicode and non-unicode a little easier

#ifdef UNICODE
//Definitions for when we are in unicode
#else
//Definitions for when we are not in unicode

#endif

wchar_t* convertToWchar(TCHAR* buffer)
{
	wchar_t* ret;
	int length;
#ifdef UNICODE
	length = wcslen(buffer);
	ret = (wchar_t*)calloc((length + 1), sizeof(wchar_t));
	memcpy(ret, buffer, length * sizeof(wchar_t));
#else
	length = strlen(buffer);
	ret = (wchar_t*)calloc((length + 1), sizeof(wchar_t));
	mbstowcs(ret, buffer, length);
#endif
	return ret;
}

wchar_t* convertC2WChar(char* buffer, int length)
{
	wchar_t* ret;
	ret = (wchar_t*)calloc((length + 1), sizeof(wchar_t));
	mbstowcs(ret, buffer, length);
	return ret;
}

wchar_t* convertC2WChar(char* buffer)
{
	int length = strlen(buffer);
	return convertC2WChar(buffer, length);
}


//checks if the hash of the bypassCodes given by the server match the hash of the current bypass codes
//True - they match, no update is required. False otherwise
bool updateBypassCheckHash(HKEY regKey, OTPVerifyExParser::OTPBypassData* bypassCodes)
{
	DEBUG_VERBOSE("Checking Hash of bypass codes\n");
	if(RegQueryValueEx(regKey, L"serverhash", NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND)
	{
		DEBUG_VERBOSE("No hash found - update required\n");
		return false; //failed the hash check - no hash to compare with
	}
	DWORD length = 40;
	BYTE serverHash[40];
	DWORD result =  RegQueryValueEx(regKey, L"serverhash", NULL, NULL, &(serverHash[0]), &length);
	if(result != ERROR_SUCCESS)
	{
		DEBUG_VERBOSE("Failed to read Reg Value - Error [%08lX]. - Update Required\n", result);
		return false;
	}

	if(length != 40)
	{
		DEBUG_VERBOSE("Invalid Hash Length [%d] - Update Required\n", length);
		return false;
	}

	//Calculate the hash from bypassCodes
	int bypassHashLength;
	BYTE* checkHash = bypassCodes->GenerateHash(bypassHashLength);
	if(bypassHashLength != 40)
	{
		DEBUG_VERBOSE("Bypass : Generate Hash -> Unexpected hash length [%d] - Update Required\n");
		free(checkHash);
		return false;
	}

	if(memcmp(checkHash, serverHash, 40) == 0)
	{
		DEBUG_VERBOSE("Hashes match - No update is required\n");
		free(checkHash);
		return true;
	}
	DEBUG_VERBOSE("Hashes do not match - Update required\n");
	free(checkHash);
	return false;
}

BYTE* secondHashBypassCodes(BYTE* BypassData, int Length, BYTE* pcKey)
{
	BYTE* ret = (BYTE*)malloc(Length);
	//copy first 21 bytes (enabled times)
	memcpy(ret, BypassData, 21);

	//apply the 2nd hmac_sha1 to each hash.
	for(int start = 21 ; start < Length ; start += 20)
	{
		BYTE hash[20];
		BYTE outHash[20];
		memcpy(hash, BypassData + start, 20);
		hmac_sha1(outHash, pcKey, 20, hash, 20);
		memcpy(ret + start, outHash, 20);
	}
	return ret;
}

void updateBypassCodesFromServer(void* APIData, int length)
{
	DEBUG_LOG("Updating Bypass Codes from Server\n");
	OTPVerifyExParser::OTPBypassData newBypassCodes;
	
	int result = OTPVerifyExParser::GetBypass(APIData, length, newBypassCodes);
	if(result != APIEX_ERROR_SUCCESS)
	{
		DEBUG_LOG("Error Parsing API Stream : [%08lX]", result);
		return;
	}

	DEBUG_LOG("Updating Bypass Registry\n");

	int bypassCount = newBypassCodes.GetSIDBlockCount();

	//delete the old bypass codes

	HKEY subKey;
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Bypass", 0, KEY_ALL_ACCESS, &subKey);
	if(result == ERROR_SUCCESS)
	{
		//check if we need to update
		if(!updateBypassCheckHash(subKey, &newBypassCodes))
		{
			//Grab the pc's key from the registry
			DWORD dataLen = 20;
			BYTE pckey[20];
			RegQueryValueEx(subKey, NULL, NULL, NULL, &pckey[0], &dataLen);
			if(dataLen != 20)
			{
				DEBUG_LOG("Invalid PCKey Len - Aborting update\n");
				RegCloseKey(subKey);
				return; 
			}

			//Get the server hash first - don't delete if this fails somehow
			int hashLength;
			BYTE* serverHash = newBypassCodes.GenerateHash(hashLength);
			if((hashLength != 40) || (serverHash == NULL))
			{
				DEBUG_LOG("Could not get Server Hash - Aborting Update\n");
				RegCloseKey(subKey);
				return;
			}

			//Delete the bypass keys
			SHDeleteKey(subKey, NULL);

			//Re-write the pc key
			RegSetValueEx(subKey, NULL, 0, REG_BINARY, &pckey[0], dataLen);

			//Write the serverHash
			RegSetValueEx(subKey, L"serverhash", 0, REG_BINARY, serverHash, 40);

			//Write the new bypass codes
			int bypassCount = newBypassCodes.GetSIDBlockCount();
			for(int i = 0 ; i < bypassCount ; ++i)
			{
				OTPVerifyExParser::BypassSidData* pBSD = newBypassCodes.GetSIDBlock(i);
				wchar_t* wSID = convertC2WChar((char*)pBSD->SID, pBSD->SIDLength);
				DEBUG_VERBOSEW(L"Setting bypass codes for SID %s\n", wSID);
				BYTE* bypassHash = secondHashBypassCodes((BYTE*)pBSD->BypassData, pBSD->BypassLength(), pckey);
				RegSetValueEx(subKey, wSID, 0, REG_BINARY, bypassHash, pBSD->BypassLength());
				//free allocated memory
				free(bypassHash);
				free(wSID);
				free(pBSD);
			}
		}
		RegCloseKey(subKey);

	}
	else
	{
		DEBUG_LOG("Could not open Registry Subkey [%08lX], Update Failed\n", result);
		return;
	}
}

struct timeBasedBypass
{
	unsigned char timeMask[21];
};

bool allowedAtTime(timeBasedBypass* pBypass)
{
	time_t t = time(NULL);
	tm* ptm = localtime(&t);
	DEBUG_LOG("Checking if bypass is enabled at Day/Time [%d/%d] : ", ptm->tm_wday, ptm->tm_hour);

	//the byte is determined by the weekday * 3 + the hour / 8 (each byte is 1  * 8 hour block, of which there are 3 every day)
	int byte = (ptm->tm_wday * 3 + (ptm->tm_hour / 8));
	//the bit depends on which hour in the 8 hour block we are in
	int bit = ptm->tm_hour % 8;
	//if the bit is set, bypasses are allowed
	if(pBypass->timeMask[byte] & (1 << bit))
	{
		DEBUG_LOG("Allowed\n");	
		return true;
	}
	DEBUG_LOG("Disabled\n");
	return false;
}

bool hashContained(const unsigned char* hashes, const int hashesLength, const unsigned char* hash)
{
	timeBasedBypass TBB;
	memcpy(&TBB.timeMask, hashes, 21);

	if(allowedAtTime(&TBB)) //if we are not allowed to use these bypasses atm - don't even check them
	{
		for(int i = 21 ; i < hashesLength ; i += 20)
		{
			DEBUG_VERBOSE("Checking Hash : \n");
			/*
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)hashes[i], 20);
			DEBUG_SENSITIVE(true, " vs ");
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)hash, 20);
			*/
			if(memcmp(&(hashes[i]), hash, 20) == 0)
			{
				DEBUG_VERBOSE("PASSED\n");
				return true;
			}
			DEBUG_VERBOSE("FAILED\n");
		}
	}
	return false;
}

DWORD FetchSID(LPTSTR username, LPTSTR systemName, PSID *ppSID, LPTSTR *domainName)
{
	DWORD returnValue;
	PSID mySID = NULL;
	DWORD sidSize = 0;
	DWORD domainSize = 0;
	SID_NAME_USE siduse;
	LPTSTR domainStr = NULL;

	DEBUG_MESSAGE("Running Fetch SID");
	DEBUG_MESSAGE("Username : ");
	DEBUG_WMESSAGE(username);
	DEBUG_MESSAGE("System Name : ");
	DEBUG_WMESSAGE(systemName);

	LookupAccountName(systemName,
						username,
						NULL,
						&sidSize,
						NULL,
						&domainSize,
						&siduse);
	if(sidSize)
	{
		mySID = (PSID)malloc(sidSize);
		memset(mySID, 0, sidSize);
	}
	else
	{
		returnValue = ERROR_INVALID_PARAMETER;
		goto exitFunc;
	}

	if(domainSize)
	{
		domainStr = (LPTSTR)calloc(domainSize, sizeof(TCHAR));
	}

	//Now we get the SID and domain name
	if(!LookupAccountName(systemName,
							username,
							mySID,
							&sidSize,
							domainStr,
							&domainSize,
							&siduse))
	{
		returnValue = ERROR_INVALID_ACCESS;
		goto exitFunc;
	}
#ifdef ENABLE_LOGGING
	//SPAM the user SID
	{
		if(debugLog)
		{
			FILE* fspam;
			fopen_s(&fspam, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\UserSID.log", "a+");
			if(fspam != NULL)
			{
				fprintf(fspam, "USER SID : ");
				LPTSTR Sidstr = NULL;
				ConvertSidToStringSid(mySID, &Sidstr);
				fprintf(fspam, "%s\n", Sidstr);
				DEBUG_MESSAGE(Sidstr);
				LocalFree(Sidstr);

				fclose(fspam);
			}
		}
	}
#endif

	*ppSID = mySID;
	*domainName = domainStr;
	return 0;

exitFunc:
	*ppSID = NULL;
	*domainName = NULL;
	if(domainStr) free(domainStr);
	if(mySID) free(mySID);

	return returnValue;
}



wchar_t * getDomainName(LPCWSTR domainString)
{
	int count = 0;
	size_t strlen = wcslen(domainString);
	while(count < strlen)
	{
		if(domainString[count] == '.')
		{
			break;
		}
		count++;
	}
	wchar_t* retBuffer = (wchar_t*)calloc(count + 1, sizeof(wchar_t));
	for(int i = 0 ; i < count ; ++i)
	{
		//Make uppercase
		wchar_t c = domainString[i];
		if((c >= 'a' && c <= 'z'))
			c += (wchar_t)('A' - 'a');
		retBuffer[i] = c;
	}

	return retBuffer;
}
wchar_t * getDomainString(CString domainAndUser)
{
	CT2CW wstr(domainAndUser.GetBuffer());
	domainAndUser.ReleaseBuffer();

	int count = 0;
	size_t strlen = wcslen(wstr);
	bool found = false;
	while(count < strlen)
	{
		if(wstr[count] == '\\')
		{
			found = true;
			break;
		}
		count++;
	}
	if(found)
	{
		wchar_t * retbuffer = (wchar_t*)calloc(count + 1, sizeof(wchar_t));
		memcpy(retbuffer, wstr, count * sizeof(wchar_t));
		return retbuffer;
	}
	return NULL;
}

typedef struct _USER_GROUP_BUFFER
{
	GROUP_USERS_INFO_0* buffer;
	DWORD count;
} USER_GROUP_BUFFER;

NET_API_STATUS getUserGroups(USER_GROUP_BUFFER* localBuffer, USER_GROUP_BUFFER* domainBuffer, CString username, CString domainAndUser, bool& useAPIForDomain)
{
	
	DEBUG_LOG("Getting User Groups:\n");
	if((localBuffer == NULL) || (domainBuffer == NULL))
		return (DWORD)-1;

	wchar_t* wDomainAndUser = convertToWchar(domainAndUser.GetBuffer());
	DEBUG_LOGW(L"\nDomain and User = %s\n", wDomainAndUser);
	
	DWORD totalEntries;
	time_t stime = time(NULL);
	NET_API_STATUS status = NetUserGetLocalGroups(NULL, wDomainAndUser, 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&(localBuffer->buffer), MAX_PREFERRED_LENGTH, &localBuffer->count, &totalEntries);
	DEBUG_LOG("Query Took %I64d Seconds\n", time(NULL) - stime);
	domainAndUser.ReleaseBuffer();
	free(wDomainAndUser);

	DEBUG_LOG("Results : Count = %d, Total Entries = %d [Status = %d]\n", localBuffer->count, totalEntries, status);

	wchar_t* wDomain = getDomainString(domainAndUser);
	wchar_t* wUsername = convertToWchar(username.GetBuffer());
	username.ReleaseBuffer();
	DEBUG_LOGW(L"Domain / Username = %s / %s", wDomain, wUsername);
	stime = time(NULL);
	NET_API_STATUS status2 = NetUserGetGroups(wDomain, wUsername, 0, (LPBYTE*)&(domainBuffer->buffer), MAX_PREFERRED_LENGTH, &domainBuffer->count, &totalEntries);
	DEBUG_LOG("Query Took %I64d Seconds\n", time(NULL) - stime);
	DEBUG_LOG("Results : Count = %d, Total Entries = %d [Status = %d]\n", domainBuffer->count, totalEntries, status2);
	free(wUsername);
	free(wDomain);

	if(status2 == 5)
	{
		//Access Denied - XP has this due to permissions.
		//We will use the API to get the domain SIDs this user belongs to
		DEBUG_LOG("Could not get Domain Based Groups - Will use the API for this\r\n");
		useAPIForDomain = true;
	}

	if(status != 0)
		return status;
	return status2;
}
void debugGroupBuffer(USER_GROUP_BUFFER * data)
{
	DEBUG_MESSAGE("Start Debug : Group Buffer");
	if(data == NULL)
	{
		DEBUG_MESSAGE("Data is NULL");
	}
	else
	{
		DEBUG_MESSAGEd(data->count);
		for(unsigned int i = 0 ; i < data->count ; ++i)
		{
			DEBUG_WMESSAGE(data->buffer[i].grui0_name);
		}
	}
	DEBUG_MESSAGE("End Debug : Group Buffer");
}

bool checkBufferHashes(USER_GROUP_BUFFER* pBuffer, TCHAR groupName[256], BYTE* valueData, DWORD valueDataLen, unsigned char actualHash[20])
{
	DEBUG_LOG("Group Count : %d\n", pBuffer->count);
	for(unsigned int groupID = 0 ; groupID < pBuffer->count; ++groupID)
	{
		CT2CW groupBuffer(groupName);
		DEBUG_MESSAGE("Comparing Groups");
		DEBUG_MESSAGE("(Group Buffer) :");
		DEBUG_WMESSAGE(pBuffer->buffer[groupID].grui0_name);
		DEBUG_MESSAGE("(Group Name) :");
		DEBUG_WMESSAGE(groupBuffer);
		if(wcscmp(pBuffer->buffer[groupID].grui0_name, groupBuffer) == 0)
		{
			DEBUG_MESSAGE("Same Group");
			//Same Group :)
			if(hashContained(valueData, valueDataLen, actualHash))
			{
				DEBUG_MESSAGE("Hash found");
				return true;
			}
		}
		else
		{
			DEBUG_MESSAGE("Different Group");
		}
	}
	return false;
}

bool APIGroupCheck(CString username, CString domainAndUser, unsigned char actualHash[20])
{
	DEBUG_VERBOSE("In APIGroupCheck\r\n");
	//API Function String.
	//AnonGetGroupSID?domain=(domain)&username=(username)
	MiTokenAPI API;
	MiTokenServers servers;

	int maxKey, maxData;
	servers.getMaxSizeRequired(maxKey, maxData);

	maxKey++;
	maxData++;

	TCHAR *key;
	BYTE *data;
	key = (TCHAR*)malloc(maxKey * sizeof(TCHAR));
	data = (BYTE*)malloc(maxData * sizeof(BYTE));
	DOMAIN_CONTROLLER_INFO* pDCI;
	DWORD v = DsGetDcName(NULL, NULL, NULL, NULL, 0, &pDCI);
	wchar_t* wDomain;
	if((pDCI == NULL) || (pDCI->DomainName == NULL))
	{
		DEBUG_LOG("Setting Domain Name to Passed in value : ");
		wDomain = getDomainString(domainAndUser);
		DEBUG_LOGW(L"[%s]\r\n", wDomain);
	}
	else
	{
		DEBUG_LOG("Setting Domain Name to DsGetDCName : ");
		wDomain = pDCI->DomainName;
		DEBUG_LOGW(L"[%s]\r\n", wDomain);
	}

	wchar_t* wUsername = convertToWchar(username.GetBuffer());

	bool userValidated = false;
	
	while(servers.hasMoreServers())
	{
		memset(key, 0, maxKey * sizeof(TCHAR));
		memset(data, 0, maxData * sizeof(BYTE));
		int keyLen, dataLen;
		keyLen = maxKey;
		dataLen = maxData;
		bool moreServers = true;
		int retCode = servers.getNextAPIServer(data, dataLen, key, keyLen, moreServers);
		if(!moreServers)
		{
			break;
		}
		DEBUG_LOGW(L"GetNextAPIServer returned with key [%s] and value ", key);
		DEBUG_LOGBYTES(data, dataLen);
		DEBUG_LOG("\r\n\tWith Retcode %d\r\n", retCode);
		API.setServer((wchar_t*)data);
		int dataLength;
		void* data = API.AnonGetGroupSID(wDomain, wUsername, dataLength);
		if(dataLength > 0)
		{
			DEBUG_LOG("API Call returned [%d] bytes\r\n", dataLength);
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)data, dataLength);
			DEBUG_LOG("\r\n");
			if(((wchar_t*)data)[0] == 'S')
			{
				DEBUG_LOG("Data starts with an 'S', Most likely contains SIDs\r\n");
				//we have GUIDs
				wchar_t* pSIDs = (wchar_t*)data;
				HKEY RegKey = NULL;
				RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Bypass", 0, KEY_READ, &RegKey);
				if(RegKey)
				{
					DEBUG_LOG("Bypass codes found... Iterating over SIDs to check for Bypass Codes that match\r\n");
					while(*pSIDs != 0)
					{
						DEBUG_SENSITIVEW(true, L"Reading Registry Value [%s]\r\n", pSIDs);
						if(RegQueryValueEx(RegKey, pSIDs, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
						{
							DEBUG_VERBOSE("Ret Code was not ERROR_FILE_NOT_FOUND\r\n");
							//userValidated = checkBufferHashes(&domainBuffer, groupName, valueData, valueDataLen, actualHash);
							BYTE* pHashData = NULL;
							DWORD hashDataLength = 0;
							RegQueryValueEx(RegKey, pSIDs, NULL, NULL, pHashData, &hashDataLength);
							if(hashDataLength)
							{
								DEBUG_VERBOSE("Allocating [%d] bytes for hashes\r\n", hashDataLength);
								pHashData = (BYTE*)malloc(hashDataLength);
								if(RegQueryValueEx(RegKey, pSIDs, NULL, NULL, pHashData, &hashDataLength) == ERROR_SUCCESS)
								{
									userValidated |= hashContained(pHashData, hashDataLength, actualHash);
									DEBUG_VERBOSE("User Validated now = [%s]\r\n", userValidated ? "TRUE" : "FALSE");
								}
								free(pHashData);
							}
						}
						//Move forward 1 item in the list
						pSIDs = &(pSIDs[wcslen(pSIDs) + 1]);
					}
				}
				else
				{
					DEBUG_LOG("Failed to open RegKey\r\n");
				}
			}
			free(data);
		}
		else
		{
			DEBUG_LOG("API Call returned with 0 bytes\r\n");
		}
	}


	DEBUG_VERBOSE("APIGroupCheck finished");


	return userValidated;
}

bool checkOTPBypassDomain(CString username, CString domainAndUser, HKEY& subKey, unsigned char actualHash[20])
{
	LONG result;
	//Now check all groups incase the user is a member of them.
	bool userValidated = false;

	DEBUG_MESSAGE("User Group Ret Check");
	
	USER_GROUP_BUFFER domainBuffer;
	USER_GROUP_BUFFER localBuffer;

	bool useAPIForGroups = false;
	
	getUserGroups(&localBuffer, &domainBuffer, username, domainAndUser, useAPIForGroups);

	useAPIForGroups = true;
	debugGroupBuffer(&localBuffer); 
	debugGroupBuffer(&domainBuffer);

	LPTSTR userStr;
	userStr = domainAndUser.GetBuffer();
	DEBUG_TMESSAGE(userStr);
	
	CT2CW wstr(userStr);

	DEBUG_WMESSAGE(wstr);
	domainAndUser.ReleaseBuffer();

	
	//First get all registry subkeys
	DEBUG_MESSAGE("Starting Group Bypass Check");
	DWORD valueCount;
	DWORD maxValueLen;
	DWORD maxValueDataLen;
	result = RegQueryInfoKey(subKey,
								NULL,
								NULL,
								NULL,
								NULL,
								NULL,
								NULL,
								&valueCount,
								&maxValueLen,
								&maxValueDataLen,
								NULL,
								NULL);

	DEBUG_MESSAGE("Getting Registry Values V1.0.2");
	if(result == ERROR_SUCCESS)
	{
		TCHAR * valueName = (TCHAR*)calloc(maxValueLen + 1, sizeof(TCHAR));
		BYTE * valueData = (BYTE*)calloc(maxValueDataLen, sizeof(BYTE));
		DWORD valueLen, valueDataLen;
		valueLen = maxValueLen; valueDataLen = maxValueDataLen;
		DEBUG_MESSAGE("count");
		DEBUG_MESSAGEd(valueCount);
		DEBUG_MESSAGEd(maxValueLen);
		DEBUG_MESSAGEd(maxValueDataLen);
		for(unsigned int i = 0 ; ((i < valueCount) && (!userValidated)) ; ++i)
		{
			DEBUG_MESSAGE("Getting Next Value");
			valueName[0] = '\0'; //required to set valueName[0] to 0 for how RegEnumValue works
			valueLen = maxValueLen + 1;
			valueDataLen = maxValueDataLen;
			result = RegEnumValue(subKey, i,
									valueName,
									&valueLen,
									NULL,
									NULL,
									valueData,
									&valueDataLen);

			DEBUG_MESSAGEd(result);

			wchar_t* wDomainString = getDomainString(domainAndUser);
			wchar_t* wDomain = NULL;
			if(wDomainString != NULL)
			{
				wDomain = getDomainName(wDomainString);
				free(wDomainString);
			}
			if(result == ERROR_SUCCESS)
			{
				DEBUG_MESSAGE("Value Name : ");
				DEBUG_WMESSAGE(valueName);
				DEBUG_MESSAGE(valueName);
				//so we now have an SID (valueName) and hashes (valueData)
				PSID mSID = NULL;
				SID_NAME_USE sidUse;
				DWORD nameLength, domainNameLength;
				DEBUG_MESSAGE("Converting StringSID to SID");
				if(ConvertStringSidToSid(valueName, &mSID))
				{
					DEBUG_MESSAGE("Looking up account sid");
					sidUse = SidTypeInvalid;
					TCHAR groupName[MAX_NAME];
					TCHAR domainName[MAX_NAME];

					nameLength = MAX_NAME;
					domainNameLength = MAX_NAME;
					if(LookupAccountSid(NULL, mSID, groupName, &nameLength, domainName, &domainNameLength, &sidUse) != 0)
					{
						DEBUG_MESSAGE("Worked\nSID USE:");
						DEBUG_MESSAGEd(sidUse);
						if((sidUse == SidTypeGroup) || (sidUse == SidTypeWellKnownGroup) || (sidUse == SidTypeAlias))
						{
							DEBUG_MESSAGE("domainNameLength");
							DEBUG_MESSAGEd(domainNameLength);
							DEBUG_MESSAGE("Name Length");
							DEBUG_MESSAGEd(nameLength);
							DEBUG_MESSAGE("Group Name");
							DEBUG_TMESSAGE(groupName);
							DEBUG_MESSAGE("Domain Name");
							DEBUG_TMESSAGE(domainName);
							DEBUG_MESSAGE("SID Use");
							DEBUG_MESSAGEd(sidUse);
							CT2CW wDomainName(domainName);
							if(wDomain != NULL)
							{
								DEBUG_MESSAGE("wDomain =");
								DEBUG_WMESSAGE(wDomain);
								DEBUG_MESSAGE("vs");
								DEBUG_WMESSAGE(wDomainName);
								if(wcscmp(wDomain, wDomainName) == 0)
								{
									DEBUG_MESSAGE("Same : Checking Domain Groups");
									//it is a domain based SID
									userValidated = checkBufferHashes(&domainBuffer, groupName, valueData, valueDataLen, actualHash);
								}
								else
								{
									DEBUG_MESSAGE("Different : Checking Local Groups");
									//it is a local based SID
									userValidated = checkBufferHashes(&localBuffer, groupName, valueData, valueDataLen, actualHash);
								}
							}
							else
							{
								DEBUG_MESSAGE("wDomain == NULL");
								//we only have local based SIDs
								userValidated = checkBufferHashes(&localBuffer, groupName, valueData, valueDataLen, actualHash);
							}
						}
						//we are done. Nothing to dealloc
					}
					else //Lookup Account SID failed
					{
						DEBUG_MESSAGE("Failed");
						int error = GetLastError();
						DEBUG_MESSAGEd(error);
					}

					LocalFree(mSID);
					mSID = NULL;
				}
				else //convert string sid to sid failed
				{
					DEBUG_MESSAGE("Convert String SID to SID failed");
					int error = GetLastError();
					DEBUG_MESSAGEd(error);
				}

				
			}
		}
		DEBUG_MESSAGE("Freeing valueName / valueData");
		free(valueName);
		free(valueData);
	
		NetApiBufferFree(localBuffer.buffer);
		NetApiBufferFree(domainBuffer.buffer);
			
	}

	if(userValidated)
	{
		return true;
	}

	if(useAPIForGroups)
	{
		return APIGroupCheck(username, domainAndUser, actualHash);
	}
	return false;
}

bool checkOTPBypass(CString username, CString OTP, TCHAR * keyPath, CString domainAndUser)
{
	HKEY subKey = NULL;
	unsigned char salt[20];
	unsigned char staticSalt[20] = {0xF7, 0x05, 0x8C, 0xE3, 0x30, 0x86, 0x2E, 0x07, 0x22, 0x3C, 0x61, 0x7A, 0xDC, 0xBE, 0xDC, 0x7D, 0x65, 0x1A, 0x25, 0x9F };

	unsigned char* hashes = NULL;
	DWORD hashSize;
	DWORD saltSize;
	unsigned char actualHash[20];
	unsigned char tempHash[20];
	char *otp;
	PSID sid;
	LPTSTR sidDomain;
	LPTSTR sidStr;

	DEBUG_MESSAGE("BYPASS V2.1");

	//open the bypass key
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &subKey);
	if(result != ERROR_SUCCESS)
		return false;

	//read the salt value

	result = RegQueryValueEx(subKey, _T(""), 0, NULL, NULL, &saltSize);
	/*
		saltSize should be 20, if it isn't return false
	*/
	if(saltSize != 20)
	{
		RegCloseKey(subKey);
		return false;
	}

	result = RegQueryValueEx(subKey, _T(""), 0, NULL, &salt[0], &saltSize);
	if(result != ERROR_SUCCESS)
	{
		RegCloseKey(subKey);
		return false;
	}

	MultiString msOTP;
	msOTP = OTP.GetBuffer();
	OTP.ReleaseBuffer();
	otp = msOTP.getCharCopy();
	// Calculate SHA1-HMAC




	hmac_sha1((unsigned char *)tempHash,(const unsigned char *)staticSalt,(unsigned int)sizeof(staticSalt),(unsigned char *)otp,(unsigned int)strlen(otp));
	hmac_sha1((unsigned char *)actualHash, (const unsigned char *)salt, (unsigned int)sizeof(salt), (unsigned char*)tempHash, (unsigned int)sizeof(tempHash));

	//free otp as it was generated by MultiString::getCharCopy()
	free(otp);

	//now read all allowed bypasses (username is '*')
	result = RegQueryValueEx(subKey, _T("*"), 0, NULL, NULL, &hashSize);
	if(result == ERROR_SUCCESS)
	{
		//hashes are 20 bytes, so hashSize should be n*20 + 21 (21 for the time based enabled for OTPBypasses)
		if((((hashSize - 21) % 20) != 0) && (hashSize != 0))
		{
			DEBUG_LOG("Invalid Hash size of [%d] bytes for All Users", hashSize);
			RegCloseKey(subKey);
			return false;
		}

		hashes = (unsigned char*)calloc(hashSize, sizeof(unsigned char));

		result = RegQueryValueEx(subKey, _T("*"), 0, NULL, hashes, &hashSize);
		if(result != ERROR_SUCCESS)
		{
			free(hashes);
			RegCloseKey(subKey);
			return false;
		}

		//check if the hash is contained - also checks if the bypass is allowed given the current time
		if(hashContained(hashes, hashSize, actualHash))
		{
			free(hashes);
			RegCloseKey(subKey);
			return true;
		}

		//free hashes, so we can reuse it for the user specific hashes
		free(hashes);

	}
	

	//Username SID Check

	//Get the SID for this user
	FetchSID(username.GetBuffer(), NULL, &sid, &sidDomain);
	username.ReleaseBuffer();
	
	//Convert SID to string form
	ConvertSidToStringSid(sid, &sidStr);

	DEBUG_MESSAGE("USER SID:");
	DEBUG_TMESSAGE(sidStr);

	//Check for a registry key with this name
	result = RegQueryValueEx(subKey, sidStr, 0, NULL, NULL, &hashSize);
	if((result == ERROR_SUCCESS) && (((hashSize - 21) % 20) == 0))
	{
		DEBUG_MESSAGE("HASHES EXIST");
		//the key exists
		hashes = (unsigned char*)calloc(hashSize, sizeof(unsigned char));
		result = RegQueryValueEx(subKey, sidStr, 0, NULL, hashes, &hashSize);
		if(result == ERROR_SUCCESS)
		{
			if(hashContained(hashes, hashSize, actualHash))
			{
				free(hashes);
				LocalFree(sidStr);
				RegCloseKey(subKey);
				return true;
			}
		}
		free(hashes);
	}
	LocalFree(sidStr);
	
	if(checkOTPBypassDomain(username, domainAndUser, subKey, actualHash))
	{
		RegCloseKey(subKey);
		return true;
	}

	


	

	RegCloseKey(subKey);
	return false;
}



