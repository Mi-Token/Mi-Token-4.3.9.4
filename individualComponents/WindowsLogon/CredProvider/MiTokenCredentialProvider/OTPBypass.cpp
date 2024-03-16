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


void MiTokenAPI_debugMessage (char* format, ...)
{
	va_list argp;
	va_start(argp, format);
	DEBUG_LOGLIST(format, argp);
	va_end(argp);
}

//checks if the hash of the bypassCodes given by the server match the hash of the current bypass codes
//True - they match, no update is required. False otherwise
bool updateBypassCheckHash(HKEY regKey, OTPVerifyExParser::OTPBypassData* bypassCodes)
{
	DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() of bypass codes\n");
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
		DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() Failed to read Reg Value - Error [%08lX]. - Update Required\n", result);
		return false;
	}

	if(length != 40)
	{
		DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() Invalid Hash Length [%d] - Update Required\n", length);
		return false;
	}

	//Calculate the hash from bypassCodes
	int bypassHashLength;
	BYTE* checkHash = bypassCodes->GenerateHash(bypassHashLength);
	if(bypassHashLength != 40)
	{
		DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() Bypass : Generate Hash -> Unexpected hash length [%d] - Update Required\n");
		free(checkHash);
		return false;
	}

	if(memcmp(checkHash, serverHash, 40) == 0)
	{
		DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() Hashes match - No update is required\n");
		free(checkHash);
		return true;
	}
	DEBUG_VERBOSE("OTPByPass::updateBypassCheckHash() Hashes do not match - Update required\n");
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
	DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Updating Bypass Codes from Server\n");
	OTPVerifyExParser::OTPBypassData newBypassCodes;
	
	int result = OTPVerifyExParser::GetBypass(APIData, length, newBypassCodes);
	if(result != APIEX_ERROR_SUCCESS)
	{
		DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Error Parsing API Stream : [%08lX]", result);
		return;
	}

	DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Updating Bypass Registry\n");

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
				DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Invalid PCKey Len - Aborting update\n");
				RegCloseKey(subKey);
				return; 
			}

			//Get the server hash first - don't delete if this fails somehow
			int hashLength;
			BYTE* serverHash = newBypassCodes.GenerateHash(hashLength);
			if((hashLength != 40) || (serverHash == NULL))
			{
				DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Could not get Server Hash - Aborting Update\n");
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
				DEBUG_VERBOSEW(L"OTPByPass::updateBypassCodesFromServer() Setting bypass codes for SID %s\n", wSID);
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
		DEBUG_LOG("OTPByPass::updateBypassCodesFromServer() Could not open Registry Subkey [%08lX], Update Failed\n", result);
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
	DEBUG_LOG("OTPByPass::allowedAtTime() Checking if bypass is enabled at Day/Time [%d/%d] : \n", ptm->tm_wday, ptm->tm_hour);

	//the byte is determined by the weekday * 3 + the hour / 8 (each byte is 1  * 8 hour block, of which there are 3 every day)
	int byte = (ptm->tm_wday * 3 + (ptm->tm_hour / 8));
	//the bit depends on which hour in the 8 hour block we are in
	int bit = ptm->tm_hour % 8;
	//if the bit is set, bypasses are allowed
	if(pBypass->timeMask[byte] & (1 << bit))
	{
		DEBUG_LOG("OTPByPass::allowedAtTime() Allowed\n");	
		return true;
	}
	DEBUG_LOG("OTPByPass::allowedAtTime() Disabled\n");
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
			DEBUG_VERBOSE("OTPByPass::hashContained() Checking Hash : \n");
			/*
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)hashes[i], 20);
			DEBUG_SENSITIVE(true, " vs ");
			DEBUG_SENSITIVEBYTES(true, (unsigned char*)hash, 20);
			*/
			if(memcmp(&(hashes[i]), hash, 20) == 0)
			{
				DEBUG_VERBOSE("OTPByPass::hashContained() PASSED\n");
				return true;
			}
			DEBUG_VERBOSE("OTPByPass::hashContained() FAILED\n");
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

	DEBUG_MESSAGE("OTPByPass::FetchSID() Running Fetch SID");
	DEBUG_MESSAGE("OTPByPass::FetchSID() Username : ");
	DEBUG_WMESSAGE(username);
	DEBUG_MESSAGE("OTPByPass::FetchSID() System Name : ");
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
private:
	static void printSSID(PSID sid)
	{
		DEBUG_LOG("SID @ ");
		DEBUG_LOG("%08lX ", sid);
		LPWSTR ssid = nullptr;
		ConvertSidToStringSid(sid, &ssid);
		DEBUG_LOGW(L"%s", ssid);
		LocalFree(ssid);
		ssid = nullptr;
	}

public:
	GROUP_USERS_INFO_0* buffer;
	SID** SIDs;
	DWORD count;

	_USER_GROUP_BUFFER()
	{
		buffer = nullptr;
		SIDs = nullptr;
		count = 0;
	}

	void setGroupData(LPWSTR domain, LPWSTR user)
	{
		DEBUG_LOGW(L"OTPByPass::setGroupData() Setting group data for Domain %s and User %s\r\n", (domain == nullptr ? L"nullptr" : domain), user);
		SIDs = new SID*[count];
		for(int i = 0 ; i < count ; ++i)
		{
			memset(&SIDs[i], 0, sizeof(SID*));
			DWORD cbSid = 0;
			WCHAR buffer[100];
			memset(buffer, 0, 100);
			DWORD refDomainName;
			SID_NAME_USE sidUse;
			LookupAccountName(domain, this->buffer[i].grui0_name, SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
			SIDs[i] = (SID*)calloc(cbSid, 1);
			LookupAccountName(domain, this->buffer[i].grui0_name, SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
			DEBUG_LOG("OTPByPass::setGroupData() SID %d = ", i);
			printSSID(SIDs[i]);
			DEBUG_LOG("\r\n");
		}
	}

	int containsSID(PSID sid)
	{
		LPWSTR ssid = nullptr;
		ConvertSidToStringSid(sid, &ssid);
		DEBUG_LOGW(L"OTPByPass::containsSID() SID We are looking for is : %s\r\n", ssid);
		LocalFree(ssid);
		ssid = nullptr;

		if(SIDs == nullptr)
		{
			DEBUG_LOG("OTPByPass::containsSID() No SIDs contained. Cannot compare values\r\n");
			return -1;
		}

		DEBUG_LOG("OTPByPass::containsSID() Root SID Location : " );
		DEBUG_LOG("%08lX\r\n", SIDs);
		

		for(int gid = 0 ; gid < count ; ++gid)
		{
			DEBUG_LOGW(L"OTPByPass::containsSID() (Group Buffer) :%s\r\n", buffer[gid].grui0_name);
			if(SIDs[gid] != nullptr)
			{
				/*
				if(ConvertSidToStringSid(SIDs[gid], &ssid) == 0)
				{
					DEBUG_LOG("Failed to convert SID to StringSID. GLE = %d\r\n", GetLastError());
					continue;
				}
				*/
				DEBUG_LOGW(L"OTPByPass::containsSID() SID in buffer : ");
				printSSID(SIDs[gid]);
				//LocalFree(ssid);
				ssid = nullptr;

				if(EqualSid(SIDs[gid], sid))
				{
					DEBUG_LOG("OTPByPass::containsSID() Same SID\r\n");
					return gid;
				}
			}
		}

		DEBUG_LOG("OTPByPass::containsSID() Could not find SID\r\n");
		return -1;
	}
	
} USER_GROUP_BUFFER;

NET_API_STATUS getUserGroups(USER_GROUP_BUFFER* localBuffer, USER_GROUP_BUFFER* domainBuffer, CString username, CString domainAndUser, bool& useAPIForDomain, volatile bool* earlyKill)
{
	
	bool useForcedSlowDown = false;

	if(useForcedSlowDown)
	{
		DEBUG_LOG("OTPByPass::getUserGroups() Caution : Using forced slowdown of GetUserGroups to emulate a slow system, this should be removed for customer releases!");
	}
	

	DEBUG_LOG("OTPByPass::getUserGroups() Getting User Groups:\n");
	if((localBuffer == NULL) || (domainBuffer == NULL))
		return (DWORD)-1;

	wchar_t* wDomainAndUser = convertToWchar(domainAndUser.GetBuffer());
	DEBUG_LOGW(L"\nOTPByPass::getUserGroups() Domain and User = %s\n", wDomainAndUser);
	
	DWORD totalEntries;
	time_t stime = time(NULL);
	if(*earlyKill) 
	{
		//if earlyKill is set, we don't care about this result
		return 0; 
	}
	NET_API_STATUS status = NetUserGetLocalGroups(NULL, wDomainAndUser, 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&(localBuffer->buffer), MAX_PREFERRED_LENGTH, &localBuffer->count, &totalEntries);
	localBuffer->setGroupData(nullptr, wDomainAndUser);
	/*
	localBuffer->SIDs = new SID*[localBuffer->count];

	for(int i = 0 ; i < localBuffer->count ; ++i)
	{
		memset(&localBuffer->SIDs[i], 0, sizeof(SID*));
		DWORD cbSid = 0;
		WCHAR buffer[100];
		memset(buffer, 0, 100);
		DWORD refDomainName;
		SID_NAME_USE sidUse;
		LookupAccountName(nullptr, wDomainAndUser, localBuffer->SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
		localBuffer->SIDs[i] = (SID*)calloc(cbSid, 1);
		LookupAccountName(nullptr, wDomainAndUser, localBuffer->SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
	}
	*/

	if(useForcedSlowDown)
	{
		//Sleep(10000);
	}
	DEBUG_LOG("OTPByPass::getUserGroups() Query Took %I64d Seconds\n", time(NULL) - stime);
	domainAndUser.ReleaseBuffer();
	free(wDomainAndUser);

	DEBUG_LOG("OTPByPass::getUserGroups() Results : Count = %d, Total Entries = %d [Status = %d]\n", localBuffer->count, totalEntries, status);
	for(int i  = 0 ; i < localBuffer->count ; ++i)
	{
		DEBUG_LOGW(L"OTPByPass::getUserGroups() Item [%d] : [%s]\r\n", i, localBuffer->buffer[i].grui0_name);
	}
	wchar_t* wDomain = getDomainString(domainAndUser);
	wchar_t* wUsername = convertToWchar(username.GetBuffer());
	username.ReleaseBuffer();
	DEBUG_LOGW(L"OTPByPass::getUserGroups() Domain / Username = %s / %s\n", wDomain, wUsername);
	stime = time(NULL);
	if(*earlyKill)
	{
		//if earlyKill is set, we don't care about this result
	}
	NET_API_STATUS status2 = NetUserGetGroups(wDomain, wUsername, 0, (LPBYTE*)&(domainBuffer->buffer), MAX_PREFERRED_LENGTH, &domainBuffer->count, &totalEntries);
	domainBuffer->setGroupData(wDomain, wUsername);
	/*
	domainBuffer->SIDs = new SID*[domainBuffer->count];

	for(int i = 0 ; i < domainBuffer->count ; ++i)
	{
		memset(&domainBuffer->SIDs[i], 0, sizeof(SID));
		DWORD cbSid = 0;
		WCHAR buffer[100];
		memset(buffer, 0, 100);
		DWORD refDomainName;
		SID_NAME_USE sidUse;
		LookupAccountName(wDomain, wUsername, domainBuffer->SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
		domainBuffer->SIDs[i] = (SID*)calloc(cbSid, 1);
		LookupAccountName(wDomain, wUsername, domainBuffer->SIDs[i], &cbSid, buffer, &refDomainName, &sidUse);
	}
	*/
	if(useForcedSlowDown)
	{
		Sleep(12000);
	}
	DEBUG_LOG("OTPByPass::getUserGroups() Query Took %I64d Seconds\n", time(NULL) - stime);
	DEBUG_LOG("OTPByPass::getUserGroups() Results : Count = %d, Total Entries = %d [Status = %d]\n", domainBuffer->count, totalEntries, status2);
	for(int i  = 0 ; i < domainBuffer->count ; ++i)
	{
		DEBUG_LOGW(L"OTPByPass::getUserGroups() Item [%d] : [%s]\r\n", i, domainBuffer->buffer[i].grui0_name);
	}

	free(wUsername);
	free(wDomain);

	if(status2 == 5)
	{
		//Access Denied - XP has this due to permissions.
		//We will use the API to get the domain SIDs this user belongs to
		DEBUG_LOG("OTPByPass::getUserGroups() Could not get Domain Based Groups - Will use the API for this\r\n");
		useAPIForDomain = true;
	}

	if(status != 0)
		return status;
	return status2;
}
void debugGroupBuffer(USER_GROUP_BUFFER * data)
{
	DEBUG_MESSAGE("OTPByPass::debugGroupBuffer() Start Debug : Group Buffer");
	if(data == NULL)
	{
		DEBUG_MESSAGE("OTPByPass::debugGroupBuffer() Data is NULL");
	}
	else
	{
		DEBUG_MESSAGEd(data->count);
		for(unsigned int i = 0 ; i < data->count ; ++i)
		{
			DEBUG_WMESSAGE(data->buffer[i].grui0_name);
		}
		DEBUG_LOG("OTPByPass::debugGroupBuffer() SID Location : %08lX\r\n", data->SIDs);
	}
	DEBUG_MESSAGE("OTPByPass::debugGroupBuffer() End Debug : Group Buffer");
}

bool checkBufferHashes(USER_GROUP_BUFFER* pBuffer, TCHAR groupName[256], BYTE* valueData, DWORD valueDataLen, unsigned char actualHash[20], PSID sid)
{
	DEBUG_LOG("OTPByPass::checkBufferHashes() Group Count : %d\n", pBuffer->count);

	int index = pBuffer->containsSID(sid);
	if(index != -1) //it is contained at index
	{
		if(hashContained(valueData, valueDataLen, actualHash))
		{
			DEBUG_MESSAGE("OTPByPass::checkBufferHashes() Hash Found");
			return true;
		}
	}

	return false;
}

bool APIGroupCheck(CString username, CString domainAndUser, unsigned char actualHash[20])
{
	DEBUG_VERBOSE("OTPByPass::APIGroupCheck() In APIGroupCheck\r\n");
	//API Function String.
	//AnonGetGroupSID?domain=(domain)&username=(username)
	MiTokenAPI API;
	MiTokenServers servers;

	API.debugMessage = MiTokenAPI_debugMessage;
	int maxKey, maxData;
	servers.getMaxSizeRequired(maxKey, maxData);

	maxKey++;
	maxData++;

	TCHAR *key;
	BYTE *data;
	key = (TCHAR*)malloc(maxKey * sizeof(TCHAR));
	data = (BYTE*)malloc(maxData * sizeof(BYTE));
	DOMAIN_CONTROLLER_INFO* pDCI = nullptr;
	DEBUG_VERBOSE("OTPByPass::APIGroupCheck() Getting DcName\r\n");
	DWORD v = DsGetDcName(NULL, NULL, NULL, NULL, 0, &pDCI);
	DEBUG_VERBOSE("OTPByPass::APIGroupCheck() Got DCName\r\n");
	
	wchar_t* wDomain;
	if((pDCI == NULL) || (pDCI->DomainName == NULL))
	{
		DEBUG_LOG("OTPByPass::APIGroupCheck() Setting Domain Name to Passed in value : ");
		wDomain = getDomainString(domainAndUser);
		DEBUG_LOGW(L"OTPByPass::APIGroupCheck() Domain is [%s]\r\n", wDomain);
	}
	else
	{
		DEBUG_LOG("OTPByPass::APIGroupCheck() Setting Domain Name to DsGetDCName : \r\n");
		wDomain = pDCI->DomainName;
		DEBUG_LOGW(L"\t[%d] : [%s]\r\n", lstrlenW(wDomain), wDomain);
	}

	wchar_t* wUsername = convertToWchar(username.GetBuffer());

	bool userValidated = false;
	
	while(servers.hasMoreServers() && (wDomain != NULL))
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
		DEBUG_LOGW(L"OTPByPass::APIGroupCheck() GetNextAPIServer returned with key [%s] and value \r\n", key);
		DEBUG_LOG("\r\n");
		DEBUG_LOGBYTES(data, dataLen);
		DEBUG_LOG("\r\n");
		DEBUG_LOG("\r\n\tWith Retcode %d\r\n", retCode);
		API.setServer((wchar_t*)data);
		int dataLength;
		void* data = API.AnonGetGroupSID(wDomain, wUsername, dataLength);
		if(dataLength > 0)
		{
			DEBUG_LOG("OTPByPass::APIGroupCheck() API Call returned [%d] bytes\r\n", dataLength);
			DEBUG_VERBOSEBYTES((unsigned char*)data, dataLength);
			DEBUG_LOG("\r\n");
			if(((wchar_t*)data)[0] == 'S')
			{
				DEBUG_LOG("OTPByPass::APIGroupCheck() Data starts with an 'S', Most likely contains SIDs\r\n");
				//we have GUIDs
				wchar_t* pSIDs = (wchar_t*)data;
				HKEY RegKey = NULL;
				RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Bypass", 0, KEY_READ, &RegKey);
				if(RegKey)
				{
					DEBUG_LOG("OTPByPass::APIGroupCheck() Bypass codes found... Iterating over SIDs to check for Bypass Codes that match\r\n");
					while(*pSIDs != 0)
					{
						DEBUG_SENSITIVEW(true, L"OTPByPass::APIGroupCheck() Reading Registry Value [%s]\r\n", pSIDs);
						if(RegQueryValueEx(RegKey, pSIDs, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
						{
							DEBUG_VERBOSE("OTPByPass::APIGroupCheck() Ret Code was not ERROR_FILE_NOT_FOUND\r\n");
							//userValidated = checkBufferHashes(&domainBuffer, groupName, valueData, valueDataLen, actualHash);
							BYTE* pHashData = NULL;
							DWORD hashDataLength = 0;
							RegQueryValueEx(RegKey, pSIDs, NULL, NULL, pHashData, &hashDataLength);
							if(hashDataLength)
							{
								DEBUG_VERBOSE("OTPByPass::APIGroupCheck() Allocating [%d] bytes for hashes\r\n", hashDataLength);
								pHashData = (BYTE*)malloc(hashDataLength);
								if(RegQueryValueEx(RegKey, pSIDs, NULL, NULL, pHashData, &hashDataLength) == ERROR_SUCCESS)
								{
									userValidated |= hashContained(pHashData, hashDataLength, actualHash);
									DEBUG_VERBOSE("OTPByPass::APIGroupCheck() User Validated now = [%s]\r\n", userValidated ? "TRUE" : "FALSE");
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
					DEBUG_LOG("OTPByPass::APIGroupCheck() Failed to open RegKey\r\n");
				}
			}
			free(data);
		}
		else
		{
			DEBUG_LOG("OTPByPass::APIGroupCheck() API Call returned with 0 bytes\r\n");
		}
	}


	DEBUG_VERBOSE("OTPByPass::APIGroupCheck() APIGroupCheck finished\n");


	return userValidated;
}



struct AsyncGroupLookupData
{
	int myStartTime;
	CString username;
	CString domainAndUser;
	bool* myEarlyKill;

};

struct AsyncGroupLookupPersistentData
{
	volatile int mostRecentTime;
	HANDLE mutex;
	HANDLE hasDataEvent;
	bool* lastEarlyKill;

	USER_GROUP_BUFFER domainBuffer;
	USER_GROUP_BUFFER localBuffer;
	bool useAPIForGroups;
	volatile bool hasData;

	CString username;
	CString domainAndUser;

	AsyncGroupLookupPersistentData()
	{
		mostRecentTime = 0;
		mutex = INVALID_HANDLE_VALUE;
		hasDataEvent = INVALID_HANDLE_VALUE;
		domainBuffer.buffer = nullptr;
		domainBuffer.count = 0;
		localBuffer.buffer = nullptr;
		localBuffer.count = 0;
		useAPIForGroups = false;
		lastEarlyKill = nullptr;
	}
};

AsyncGroupLookupPersistentData asyncGroupLookupPersistentData;


DWORD WINAPI ayncGroupLookup(LPVOID param)
{
	AsyncGroupLookupData* data = reinterpret_cast<AsyncGroupLookupData*>(param);

	USER_GROUP_BUFFER loc, dom;
	bool useAPI;

	getUserGroups(&loc, &dom, data->username, data->domainAndUser, useAPI, data->myEarlyKill);

	WaitForSingleObject(asyncGroupLookupPersistentData.mutex, INFINITE);
	if(asyncGroupLookupPersistentData.mostRecentTime == data->myStartTime)
	{
		if(asyncGroupLookupPersistentData.domainBuffer.buffer)
		{
			NetApiBufferFree(asyncGroupLookupPersistentData.domainBuffer.buffer);
		}
		if(asyncGroupLookupPersistentData.localBuffer.buffer)
		{
			NetApiBufferFree(asyncGroupLookupPersistentData.localBuffer.buffer);
		}

		DEBUG_LOG("OTPByPass::ayncGroupLookup() ASYNC has data now\r\n");
		asyncGroupLookupPersistentData.domainBuffer.buffer = dom.buffer;
		asyncGroupLookupPersistentData.domainBuffer.count = dom.count;
		asyncGroupLookupPersistentData.domainBuffer.SIDs = dom.SIDs;
		asyncGroupLookupPersistentData.localBuffer.buffer = loc.buffer;
		asyncGroupLookupPersistentData.localBuffer.count = loc.count;
		asyncGroupLookupPersistentData.localBuffer.SIDs = loc.SIDs;
		asyncGroupLookupPersistentData.useAPIForGroups = useAPI;
		asyncGroupLookupPersistentData.lastEarlyKill = nullptr;
		asyncGroupLookupPersistentData.hasData = true;
		SetEvent(asyncGroupLookupPersistentData.hasDataEvent);
	}
	else
	{
		NetApiBufferFree(loc.buffer);
		NetApiBufferFree(dom.buffer);
	}
	ReleaseMutex(asyncGroupLookupPersistentData.mutex);

	//Clear up the allocated memory
	delete data->myEarlyKill;
	delete data;

	return 0;
}

void startNewAsyncGroupLookup(CString username, CString domainAndUser);

void startNewAsyncGroupLookupW(WCHAR* username, WCHAR* domainAndUser)
{
	DEBUG_LOGW(L"OTPByPass::startNewAsyncGroupLookupW() Starting new ASYNC Group lookup [User : %s] and [Domain and User : %s]\r\n", username, domainAndUser);
	startNewAsyncGroupLookup(username, domainAndUser);
}

void startNewAsyncGroupLookup(CString username, CString domainAndUser)
{
	if(asyncGroupLookupPersistentData.mutex == INVALID_HANDLE_VALUE)
	{
		asyncGroupLookupPersistentData.mutex = CreateMutex(nullptr, FALSE, nullptr);
	}
	if(asyncGroupLookupPersistentData.hasDataEvent == INVALID_HANDLE_VALUE)
	{
		asyncGroupLookupPersistentData.hasDataEvent = CreateEvent(nullptr, true, false, nullptr);
	}

	AsyncGroupLookupData* lookupData = new AsyncGroupLookupData();

	WaitForSingleObject(asyncGroupLookupPersistentData.mutex, INFINITE);

	//check if it is the same data that we are already looking up
	if(username == asyncGroupLookupPersistentData.username)
	{
		if(domainAndUser == asyncGroupLookupPersistentData.domainAndUser)
		{
			DEBUG_VERBOSE("OTPByPass::startNewAsyncGroupLookupW() Username and DomainAndUser are the same - Skipping starting a new ASYNC Group Lookup\r\n");
			ReleaseMutex(asyncGroupLookupPersistentData.mutex);
			return;
		}
	}

	DEBUG_VERBOSE("OTPByPass::startNewAsyncGroupLookupW() New Username and DomainAndUser - Starting new ASYNC Group Lookup\r\n");

	asyncGroupLookupPersistentData.username = username;
	asyncGroupLookupPersistentData.domainAndUser = domainAndUser;

	int ctime = time(nullptr);
	lookupData->myStartTime = ctime;
	asyncGroupLookupPersistentData.mostRecentTime = ctime;
	asyncGroupLookupPersistentData.hasData = false;
	if(asyncGroupLookupPersistentData.lastEarlyKill != nullptr)
	{
		*asyncGroupLookupPersistentData.lastEarlyKill = true;
	}
	asyncGroupLookupPersistentData.lastEarlyKill = new bool();
	*asyncGroupLookupPersistentData.lastEarlyKill = false;
	lookupData->myEarlyKill = asyncGroupLookupPersistentData.lastEarlyKill;
	CreateThread(nullptr, 0, ayncGroupLookup, lookupData, 0, nullptr);
	lookupData->domainAndUser = domainAndUser;
	lookupData->username = username;
	ReleaseMutex(asyncGroupLookupPersistentData.mutex);
}

int maxGroupLookupDelay()
{
	static int _groupLookupDelay = 0;

	if(_groupLookupDelay != 0)
	{
		return _groupLookupDelay;
	}

	_groupLookupDelay = 1000;

	DEBUG_VERBOSE("OTPByPass::maxGroupLookupDelay() Getting Max Group Lookup Delay\r\n");
	HKEY regKey;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ, &regKey) == ERROR_SUCCESS)
	{
		DWORD val = 0;
		DWORD valSize = sizeof(val);
		if(RegQueryValueEx(regKey, L"MaxGroupLookupDelay", nullptr, nullptr, (LPBYTE)&val, &valSize) == ERROR_SUCCESS)
		{
			_groupLookupDelay = val;
		}
	}

	return _groupLookupDelay;

}

bool checkOTPBypassDomain(CString username, CString domainAndUser, HKEY& subKey, unsigned char actualHash[20])
{
	LONG result;
	//Now check all groups incase the user is a member of them.
	bool userValidated = false;

	DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() User Group Ret Check");
	
	USER_GROUP_BUFFER domainBuffer;
	USER_GROUP_BUFFER localBuffer;

	bool useAPIForGroups = false;
	
	bool needToWaitForGroups = true;

	
	while(needToWaitForGroups)
	{
		DEBUG_LOG("OTPByPass::checkOTPBypassDomain() Checking Group ASYNC\r\n");
		WaitForSingleObject(asyncGroupLookupPersistentData.mutex, INFINITE);
		if(!asyncGroupLookupPersistentData.hasData)
		{
			DEBUG_LOG("OTPByPass::checkOTPBypassDomain() Data still required\r\n");
			ResetEvent(asyncGroupLookupPersistentData.hasDataEvent);
			ReleaseMutex(asyncGroupLookupPersistentData.mutex);
			if(WaitForSingleObject(asyncGroupLookupPersistentData.hasDataEvent, maxGroupLookupDelay()) == WAIT_TIMEOUT)
			{
				DEBUG_LOG("OTPByPass::checkOTPBypassDomain() CAUTION : Took over MaxGroupLookupDelay seconds for the ASYNC Group Lookup Data Event to be signalled. Group Bypass checks will not be done!\r\n");
				return false;
			}
		}
		else
		{
			DEBUG_LOG("OTPByPass::checkOTPBypassDomain() Data exists\r\n");
			domainBuffer.buffer = asyncGroupLookupPersistentData.domainBuffer.buffer;
			domainBuffer.count = asyncGroupLookupPersistentData.domainBuffer.count;
			domainBuffer.SIDs = asyncGroupLookupPersistentData.domainBuffer.SIDs;
			localBuffer.buffer = asyncGroupLookupPersistentData.localBuffer.buffer;
			localBuffer.count = asyncGroupLookupPersistentData.localBuffer.count;
			localBuffer.SIDs = asyncGroupLookupPersistentData.localBuffer.SIDs;
			useAPIForGroups = asyncGroupLookupPersistentData.useAPIForGroups;

			asyncGroupLookupPersistentData.domainBuffer.buffer = nullptr;
			asyncGroupLookupPersistentData.localBuffer.buffer = nullptr;
			asyncGroupLookupPersistentData.domainBuffer.count = asyncGroupLookupPersistentData.localBuffer.count = 0;
			asyncGroupLookupPersistentData.hasData = false;
			needToWaitForGroups = false;
			ReleaseMutex(asyncGroupLookupPersistentData.mutex);
		}
	}
		
	//getUserGroups(&localBuffer, &domainBuffer, username, domainAndUser, useAPIForGroups);

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
	DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Starting Group Bypass Check");
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

	DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Getting Registry Values V1.0.2");
	if(result == ERROR_SUCCESS)
	{
		TCHAR * valueName = (TCHAR*)calloc(maxValueLen + 1, sizeof(TCHAR));
		BYTE * valueData = (BYTE*)calloc(maxValueDataLen, sizeof(BYTE));
		DWORD valueLen, valueDataLen;
		valueLen = maxValueLen; valueDataLen = maxValueDataLen;
		DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() count");
		DEBUG_MESSAGEd(valueCount);
		DEBUG_MESSAGEd(maxValueLen);
		DEBUG_MESSAGEd(maxValueDataLen);
		for(unsigned int i = 0 ; ((i < valueCount) && (!userValidated)) ; ++i)
		{
			DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Getting Next Value");
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
				DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Value Name : ");
				DEBUG_WMESSAGE(valueName);
				DEBUG_MESSAGE(valueName);
				//so we now have an SID (valueName) and hashes (valueData)
				PSID mSID = NULL;
				SID_NAME_USE sidUse;
				DWORD nameLength, domainNameLength;
				DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Converting StringSID to SID");
				if(ConvertStringSidToSid(valueName, &mSID))
				{
					DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Looking up account sid");
					sidUse = SidTypeInvalid;
					TCHAR groupName[MAX_NAME];
					TCHAR domainName[MAX_NAME];

					nameLength = MAX_NAME;
					domainNameLength = MAX_NAME;
					if(LookupAccountSid(NULL, mSID, groupName, &nameLength, domainName, &domainNameLength, &sidUse) != 0)
					{
						DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Worked\nSID USE:");
						DEBUG_MESSAGEd(sidUse);
						if((sidUse == SidTypeGroup) || (sidUse == SidTypeWellKnownGroup) || (sidUse == SidTypeAlias))
						{
							DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() domainNameLength");
							DEBUG_MESSAGEd(domainNameLength);
							DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Name Length");
							DEBUG_MESSAGEd(nameLength);
							DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Group Name");
							DEBUG_TMESSAGE(groupName);
							DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Domain Name");
							DEBUG_TMESSAGE(domainName);
							DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() SID Use");
							DEBUG_MESSAGEd(sidUse);
							CT2CW wDomainName(domainName);
							if(wDomain != NULL)
							{
								DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() wDomain =");
								DEBUG_WMESSAGE(wDomain);
								DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() vs");
								DEBUG_WMESSAGE(wDomainName);
								if(wcscmp(wDomain, wDomainName) == 0)
								{
									DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Same : Checking Domain Groups");
									//it is a domain based SID
									userValidated = checkBufferHashes(&domainBuffer, groupName, valueData, valueDataLen, actualHash, mSID);

									userValidated |= checkBufferHashes(&localBuffer, groupName, valueData, valueDataLen, actualHash, mSID);
								}
								else
								{
									DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Different : Checking Local Groups");
									//it is a local based SID
									userValidated = checkBufferHashes(&localBuffer, groupName, valueData, valueDataLen, actualHash, mSID);
								}
							}
							else
							{
								DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() wDomain == NULL");
								//we only have local based SIDs
								userValidated = checkBufferHashes(&localBuffer, groupName, valueData, valueDataLen, actualHash, mSID);
							}
						}
						//we are done. Nothing to dealloc
					}
					else //Lookup Account SID failed
					{
						DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() lookup account SID Failed");
						int error = GetLastError();
						DEBUG_MESSAGEd(error);
					}

					LocalFree(mSID);
					mSID = NULL;
				}
				else //convert string sid to sid failed
				{
					DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Convert String SID to SID failed");
					int error = GetLastError();
					DEBUG_MESSAGEd(error);
				}

				
			}
		}
		DEBUG_MESSAGE("OTPByPass::checkOTPBypassDomain() Freeing valueName / valueData");
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
	LPTSTR sidStr = nullptr;

	DEBUG_MESSAGE("OTPByPass::checkOTPBypass() BYPASS V2.1");

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
			DEBUG_LOG("OTPByPass::checkOTPBypass() Invalid Hash size of [%d] bytes for All Users", hashSize);
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
	if(ConvertSidToStringSid(sid, &sidStr) == 0) //failed
	{
		DEBUG_LOG("OTPByPass::checkOTPBypass() Convert SID to StringSID Failed with ErrorCode : %d\r\n", GetLastError());
	}
	else
	{
		//We have the UserSID as a string, so let's use it
		DEBUG_MESSAGE("OTPByPass::checkOTPBypass() USER SID:");
		DEBUG_TMESSAGE(sidStr);

		//Check for a registry key with this name
		result = RegQueryValueEx(subKey, sidStr, 0, NULL, NULL, &hashSize);
		if((result == ERROR_SUCCESS) && (((hashSize - 21) % 20) == 0))
		{
			DEBUG_MESSAGE("OTPByPass::checkOTPBypass() HASHES EXIST");
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

		//safely free sidStr
		if(sidStr != nullptr)
		{
			LocalFree(sidStr);
			sidStr = nullptr;
		}
	

			
		//While CheckOTPBypassDomain has its own checking internally, it also has code that can take >15 seconds when it fails
		//Assume that when SidToStringSid fails, then the FindGroupsOfUser will also fail.
		if(checkOTPBypassDomain(username, domainAndUser, subKey, actualHash))
		{
			RegCloseKey(subKey);
			return true;
		}
	}



	


	

	RegCloseKey(subKey);
	return false;
}



