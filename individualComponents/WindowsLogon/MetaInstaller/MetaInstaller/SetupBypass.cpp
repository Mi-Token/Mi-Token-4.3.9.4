#include "SetupBypass.h"
#include "oath.h"
#include <Sddl.h>

#define DEBUGLOGGING

#ifndef DEBUGLOGGING
#define DEBUG_STRING(p)
#else
#define DEBUG_STRING(p) { printf("%s", p); }
#endif


#ifndef MAX_NAME
#define MAX_NAME 256
#endif

HKEY RegKey;

typedef struct _SIDBypass
{
	char * userSID;
	BYTE ** bypassCode;
	int bypassCount;
} SIDBypass;

typedef struct _BypassFileConfig
{
	int debugLogging;
	int filterXPLocal;
	int filterXPRDP;
	int filterCPLocal;
	int filterCPRDP;
	int useDynamicPassword;
	int useHTTP;
	int useCredInSafeMode;
	char ** servers;
	int serverCount;
	SIDBypass ** userBypasses;
	int userBypassCount;
} BypassFileConfig;

BypassFileConfig bypassConfig;

const char* errorString = NULL;

#ifdef DEBUGLOGGING
void debugLogConfig()
{
	printf("DEBUG LOG CONFIG\n-----------------------------------\n");
	printf("Servers : \n\tCount : %d\n", bypassConfig.serverCount);
	for(int i = 0 ; i < bypassConfig.serverCount ; ++i)
	{
		printf("\t\t%s\n", bypassConfig.servers[i]);
	}
	printf("User SIDs:\n\tCount : %d\n", bypassConfig.userBypassCount);
	for(int i = 0 ; i < bypassConfig.userBypassCount ; ++i)
	{
		SIDBypass* userBypass = bypassConfig.userBypasses[i];
		printf("\t\tUser SID: %s\n\t\tUser Bypass Count: %d\n", userBypass->userSID, userBypass->bypassCount);
		for(int j = 0 ; j < userBypass->bypassCount ; j++)
		{
			printf("\t\t\t%s\n", userBypass->bypassCode[j]);
		}
	}
	printf("----------------------------------------\n");
}
#else
#define debugLogConfig()
#endif

void appendItem(void* item, void*** location, int* count)
{
	void** curPack = *location;
	void** newPack = (void**)calloc((*count) + 1, sizeof(void*));
	memcpy(newPack, curPack, sizeof(void*) * (*count));
	newPack[*count] = item;
	(*count)++;
	
	SAFE_SET(*location, newPack);	
}

BYTE* UnpackInitalConfig(BYTE* data, BYTE* endPoint)
{
	DEBUG_STRING("Unpack Inital Config\n");

	bypassConfig.filterCPLocal = bypassConfig.filterCPRDP = bypassConfig.filterXPLocal = bypassConfig.filterXPRDP = 1;

	if(data == NULL)
		return NULL;

	if(data == endPoint)
	{
		errorString = "UnpackInitalConfig: Early EOF";
		return NULL;
	}
	printf("Data = [%02lX]\n", *data);
	if(*data == '0')
		bypassConfig.debugLogging = 0;
	else if(*data == '1')
		bypassConfig.debugLogging = 1;
	else
	{
		if(*data == 'V')
			{
			//we are dealing with version 2+. check which version it is
			data++;
			if(data == endPoint)
			{
				errorString = "UnpackInitalConfig: Early termination of config stream - Expected Version ID";
				return NULL;
			}
			if(*data == 0)
			{
				printf("ACS Is Version 2.0\n");
				//version 2. - 5 bytes [debugLogging / filterXPLocal / filterXPRDP / filterCPLocal / filterCPRDP] - in binary notation
				data++;
				if(data + 4 >= endPoint)
				{
					errorString = "UnpackInitalConfig: Early termination of config stream - Expected Filter Modes";
					return NULL;
				}
				bypassConfig.debugLogging = data[0];
				bypassConfig.filterXPLocal = data[1];
				bypassConfig.filterXPRDP = data[2];
				bypassConfig.filterCPLocal = data[3];
				bypassConfig.filterCPRDP = data[4];
				bypassConfig.useDynamicPassword = 0;
				bypassConfig.useHTTP = 0;
				bypassConfig.useCredInSafeMode = 0;
				data += 4;
			}
			else if(*data == 1)
			{
				printf("ACS Is Version 2.1\n");
				//version 2.1, - 6 bytes [debuglogging / filterXPLocal / filterXPRDP / filterCPLocal / filterCPRDP / useDynamicPasswords] - in binary notation
				data++;
				if(data + 5 >= endPoint)
				{
					errorString = "UnpackInitalConfig: Early termination of config stream - Expected Filter Modes";
					return NULL;
				}
				bypassConfig.debugLogging = data[0];
				bypassConfig.filterXPLocal = data[1];
				bypassConfig.filterXPRDP = data[2];
				bypassConfig.filterCPLocal = data[3];
				bypassConfig.filterCPRDP = data[4];
				bypassConfig.useDynamicPassword = data[5];
				bypassConfig.useHTTP = 0;
				bypassConfig.useCredInSafeMode = 0;
				data += 5;
			}
			else if(*data == 2)
			{
				printf("ACS Is Version 2.2\n");
				//version 2.1, - 6 bytes [debuglogging / filterXPLocal / filterXPRDP / filterCPLocal / filterCPRDP / useDynamicPasswords] - in binary notation
				data++;
				if(data + 7 >= endPoint)
				{
					errorString = "UnpackInitalConfig: Early termination of config stream - Expected Filter Modes";
					return NULL;
				}
				bypassConfig.debugLogging = data[0];
				bypassConfig.filterXPLocal = data[1];
				bypassConfig.filterXPRDP = data[2];
				bypassConfig.filterCPLocal = data[3];
				bypassConfig.filterCPRDP = data[4];
				bypassConfig.useDynamicPassword = data[5];
				bypassConfig.useHTTP = data[6];
				bypassConfig.useCredInSafeMode = data[7];
				data += 7;
			}
			else
			{
				errorString = "UnpackInitalConfig: Unknown Version ID";
				return NULL;
			}
		}
		else
		{
			char* buffer = (char*)calloc(100, sizeof(char));
			sprintf_s(buffer, 100, "UnpackInitalConfig: Bypass Config File has a bad format [%02lX]", *data);
			errorString = buffer;
			return NULL; //bad format that we cannot recover from - return null which will pass it through 
		}
	}
	//we have read 1 byte of data, so move the data pointer forward one, and decrease the length by 1
	data++;

	return data;
}

bool UnpackAPIServer(BYTE** data, BYTE* endPoint)
{
	DEBUG_STRING("Unpack API Server\n");
	BYTE* str = *data;
	if(str == NULL)
		return false;

	if(*str == '|')
	{
		(*data) += 1; 
		return false; //no more servers
	}
	
	int strLen = 0;
	while(endPoint != (&(str[strLen])) && (str[strLen] != '|'))
		strLen++;
	if(endPoint == (&(str[strLen])))
	{
		*data = NULL;
		errorString = "UnpackAPIServer: Early EOF";
		return false;
	}
	strLen++;
	char* server = (char*)calloc(strLen, sizeof(char));
	memcpy(server, str, strLen - 1);
	DEBUG_STRING("Found server: ");
	DEBUG_STRING(server);
	DEBUG_STRING("\n");
	appendItem((void*)server, (void***)(&bypassConfig.servers), &bypassConfig.serverCount);

	*data += strLen;
	return true;

}
BYTE* UnpackAPIServers(BYTE* data, BYTE* endPoint)
{
	DEBUG_STRING("Unpacking API Servers\n");
	if(data == NULL)
		return NULL;

	//Unpack API Servers
	while(UnpackAPIServer(&data, endPoint));
	
	DEBUG_STRING("All servers are unpacked\n");
	if(data == NULL)
		return NULL;


	if(memcmp(data, "\r\n", 2 * sizeof(char)) == 0)
	{
		data += 2;
		return data;
	}


	char buffer[200] = {0};
	sprintf_s(buffer, 200 * sizeof(char), "Error : Expected (0x%02lX, 0x%02lX), Received (0x%02lX, 0x%02lX)\n", '\r', '\n', data[0], data[1]);
	memcpy(buffer, data, 2);
	DEBUG_STRING(buffer);
	DEBUG_STRING("\n");
	errorString = "UnpackAPIServers: Invalid Config File Format";
	return NULL;
}

BYTE* UnpackUserSID(BYTE* data, BYTE* endPoint, SIDBypass** ppUserSID)
{
	if(ppUserSID == NULL)
	{
		errorString = "UnpackUserSID: userSID** is NULL\n";
		return NULL;
	}

	if(data == NULL)
		return NULL;

	if(*data != '{')
	{
		printf("Invalid Token [%c {%02lX}] @ %08lX / %08lX [- %08lX], expected '{'", *data, *data, data, endPoint, (int)endPoint - (int)data);
		errorString = "UnpackUserSID: Invalid Config File Format\n";
		return NULL;
	}

	int strLen = 0;
	data++;
	while(endPoint != (&(data[strLen])) && (data[strLen] != ','))
		strLen++;

	if(endPoint == (&(data[strLen])))
	{
		errorString = "UnpackUserSID: Unexpected EOF\n";
		return NULL;
	}

	char * userSID = (char*)calloc(strLen + 1, sizeof(char));
	memcpy(userSID, data, strLen);
	
	SIDBypass* sid = (SIDBypass*)calloc(1, sizeof(SIDBypass));
	appendItem((void*)sid, (void***)(&bypassConfig.userBypasses), &bypassConfig.userBypassCount);

	sid->userSID = userSID;

	(data) += (strLen + 1);
	*ppUserSID = sid;
	return data;
}

bool UnpackUserBypass(BYTE** data, BYTE* endPoint, SIDBypass* userSID)
{
	if(data == NULL)
	{
		errorString = "UnpackUserBypass: data = NULL\n";
		return false;
	}
	if(*data == NULL)
		return false;

	if(**data == '|')
	{
		(*data)++;
		return false;
	}

	if(**data != '[')
	{
		printf("Expected %02lX, Received %02lX\n", '[', **data);
		(*data) = NULL;
		errorString = "UnpackUserBypass: Invalid Config File Format\n";
		return false;
	}

	(*data)++;

	int strLen = 0;
	BYTE* str = *data;
	while(endPoint != (&(str[strLen])) && ((str[strLen]) != ']'))
		strLen++;

	if(endPoint == (&(str[strLen])))
	{
		(*data) = NULL;
		errorString = "UnpackUserBypass: Unexpected EOF\n";
		return false;
	}

	char* bypassCode = (char*)calloc(strLen + 1, sizeof(char));
	memcpy(bypassCode, *data, strLen);
	(*data) += (strLen + 1);

	appendItem((void*)bypassCode, (void***)(&userSID->bypassCode), &(userSID->bypassCount));
	
	if((**data) != '|')
	{
		(*data) = NULL;
		errorString = "UnpackUserBypass: Invalid Config File Format(1)\n";
		return false;
	}

	(*data)++;

	return true;
}

BYTE* UnpackSIDBypasses(BYTE* data, BYTE* endPoint)
{
	DEBUG_STRING("Unpacking SID Bypasses\n");
	if(data == NULL)
		return NULL;

	while((data != NULL) && (data != endPoint))
	{
		SIDBypass* userSID = NULL;
		data = UnpackUserSID(data, endPoint, &userSID);
		//Unpack all the user bypasses
		while(UnpackUserBypass(&data, endPoint, userSID));
		DEBUG_STRING("Unpacked User Bypasses\n");
		if(data == NULL)
			break;

		if(data == endPoint)
		{
			data = NULL;
			errorString = "UnpackSIDBypass: Unexpected EOF\n";
			break;
		}

		if(*data != '}')
		{
			data = NULL;
			errorString = "UnpackSIDBypass: Invalid Config File Format\n";
			break;
		}
		data++;
	}
	
	return data;
}

void writeRegServers()
{
	//first read all servers from the list
	DWORD valueCount, maxValueLen, maxValueDataLen;

	DWORD result = RegQueryInfoKey(RegKey, NULL, NULL, NULL, NULL, NULL, NULL,
									&valueCount, &maxValueLen, &maxValueDataLen,
									NULL, NULL);

	DWORD span = maxValueDataLen + 1;
	char* cData = (char*)calloc(span * valueCount, sizeof(char));
	TCHAR* valueName = (TCHAR*)calloc(maxValueLen + 2, sizeof(TCHAR));
	for(DWORD i = 0 ; i < valueCount ; ++i)
	{
		valueName[0] = '\0';
		DWORD valueNameLen = maxValueLen + 1;
		DWORD valueDataLen = maxValueDataLen;
		result = RegEnumValue(RegKey, i, valueName, &valueNameLen, NULL, NULL, (LPBYTE)&(cData[span * i]), & valueDataLen);
	}
	free(valueName);
	int regServerID = 1;
	


	//2ndly go through each server to add it if it isn't already there
	for(int server = 0 ; server < bypassConfig.serverCount ; ++server)
	{
		//first check if the server exists
		bool exists = false;
		for(int i = 0 ; i < valueCount ; ++i)
		{
			if(strcmp(&(cData[span * i]), bypassConfig.servers[server]) == 0)
			{
				printf("\t\tBypass server [%s] already exists in the registry.\n", bypassConfig.servers[server]);
				exists = true;
				break;
			}
		}
		if(exists)
			continue;
		//2ndly add it to a value name that doesn't already exist
		bool keyExists = true;
		while(keyExists)
		{
			char keyname[50];
			sprintf_s(keyname, 50 * sizeof(char), "Server%d", regServerID);
			DWORD length = 0;
			if(RegQueryValueEx(RegKey, keyname, NULL, NULL, NULL, &length) == ERROR_FILE_NOT_FOUND)
			{
				//Perfect, write the value in
				printf("\t\tWriting Bypass [%s] to value [%s]\n", bypassConfig.servers[server], keyname);
				RegSetValueEx(RegKey, keyname, NULL, REG_SZ, (BYTE*)bypassConfig.servers[server], strlen(bypassConfig.servers[server]));
				keyExists = false;
			}
			regServerID++;
		}

	}
	free(cData);
}


BYTE* CreateKey()
{
	HCRYPTPROV cryptoProvider;
	if(CryptAcquireContext(&cryptoProvider, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		BYTE* key = (BYTE*)calloc(20, sizeof(BYTE));
		if(CryptGenRandom(cryptoProvider, 20, key))
		{
			CryptReleaseContext(cryptoProvider, 0);
			return key;
		}
		else
		{
			SAFE_FREE(key);
			DWORD err = GetLastError();
			CryptReleaseContext(cryptoProvider, 0);
			SetLastError(err);
			return NULL;
		}

	}
	else
	{
		return NULL;
	}
}

void AddHashes(SIDBypass* userBypass, BYTE* hashes)
{
	BYTE* curHashes = NULL;
	DWORD curHashesLen = 0;
	BOOL hasTimes = FALSE;
	if(RegQueryValueEx(RegKey, userBypass->userSID, NULL, NULL, curHashes, &curHashesLen) != ERROR_FILE_NOT_FOUND)
	{
		//check for bypasstimes
		if(curHashesLen % 20 != 0)
		{
			if(((curHashesLen - 21) % 20) == 0)
			{
				//we have bypasstimes (length is 21 + 20n, where n is number of hashes)
				hasTimes = TRUE;
			}
		}
		curHashes = (BYTE*)calloc(curHashesLen, sizeof(BYTE));
		RegQueryValueEx(RegKey, userBypass->userSID, NULL, NULL, curHashes, &curHashesLen);
		if(hasTimes)
		{
			curHashes += 21; //move past the time config so most of the lower down code still works fine (it doesn't expect timeconfig prefix)
			curHashesLen -= 21; //take the length down by 21 as we don't want to count the bypass times
		}
	}

	BYTE* combinedHashes;
	DWORD combinedHashCount;
	/*
	if(curHashes == NULL)
	{
		combinedHashes = hashes;
		combinedHashCount = userBypass->bypassCount;
	}
	else
	{
	*/


	//length is 21 for BypassTimes prefix + 20n for all the hashes we have + current hashes length
	combinedHashes = (BYTE*)calloc(21 + (20 * userBypass->bypassCount) + curHashesLen, sizeof(BYTE));
	combinedHashes += 21; //move past the time prefix section
	memcpy(combinedHashes, curHashes, curHashesLen);
	combinedHashCount = curHashesLen / 20;
	for(int i = 0 ; i < userBypass->bypassCount ; ++i)
	{
		bool found = false;
		for(int j = 0 ; j < (curHashesLen) ; j += 20)
		{
			if(memcmp(&(curHashes[j]), &(hashes[i * 20]), 20 * sizeof(BYTE)) == 0)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			memcpy(&(combinedHashes[combinedHashCount * 20]), &(hashes[i * 20]), 20 * sizeof(BYTE));
			combinedHashCount++;
		}
	}

	//}

	combinedHashes -= 21; //move back to the bypass time config section
	if(hasTimes)
	{
		curHashes -= 21; //move curHashes back to the time bypass section
		memcpy(combinedHashes, curHashes, 21); //copy the section across
	}
	else
	{
		//we need to generate our own times - just default to enable all (21 0xFFs)
		memset(combinedHashes, 0xFF, 21);
	}
	
	printf("\t\t\tWriting [%d] bytes\n", 21 + (combinedHashCount * 20));
	RegSetValueEx(RegKey, userBypass->userSID, 0, REG_BINARY, combinedHashes, 21 + (combinedHashCount * 20));


	//free allocated memory
	free(combinedHashes);
	free(curHashes);
}

bool writeUserBypasses(BYTE* key)
{
	bool wroteABypass = false;
	for(int userID = 0; userID < bypassConfig.userBypassCount ; ++userID)
	{
		printf("\r\t\t%d/%d\n", userID + 1, bypassConfig.userBypassCount);
		SIDBypass* userBypass = bypassConfig.userBypasses[userID];
		char* sidStr = userBypass->userSID;
		if(strcmp("*", sidStr) != 0)
		{
			PSID userSID;
			if(!ConvertStringSidToSid(sidStr, &userSID))
			{
				DWORD errorID = GetLastError();
				if(errorID == ERROR_INVALID_PARAMETER)
				{
					printf("Error : ConvertStringToSid [%s] contains an invalid parameter\n", sidStr);
				}
				else if(errorID == ERROR_INVALID_SID)
				{
					printf("Error : ConvertStringToSid [%s] contains an invalid SID\n", sidStr);
				}
				else
				{
					printf("Error : ConvertStringToSid [%s] returned an unexpected error code [%d]\n", sidStr, errorID);
				}
				continue;
			}

			{
				//if(LookupAccountSid(NULL, mSID, groupName, &nameLength, domainName, &domainNameLength, &sidUse) != 0)
				TCHAR username[MAX_NAME];
				TCHAR domainName[MAX_NAME];
				DWORD usernameLen = MAX_NAME;
				DWORD domainNameLen = MAX_NAME;
				SID_NAME_USE sidUse;

				if(!LookupAccountSid(NULL, userSID, username, &usernameLen, domainName, &domainNameLen, &sidUse))
				{
					DWORD errID = GetLastError();
					if(errID = 1789) //Trust relationship between PC and SIDs domain is invalid
					{
						printf("Importing of SID [%s] Failed because : The trust relationship between this workstation and the primary domain failed.\n", sidStr);
					}
					else
					{
						printf("Importing of SID [%s] Failed because : Error ID [%d]\n", sidStr, errID);
					}
					LocalFree(userSID);
					continue;
				}
			}
			LocalFree(userSID);
		}
		BYTE * hashes = (BYTE*)calloc(userBypass->bypassCount * 20, sizeof(char));
		for(int bypass = 0 ; bypass < userBypass->bypassCount ; ++bypass)
		{
			//generate the hash here :)
			BYTE* hashStart = &(hashes[bypass * 20]);

			//convert base64 string to BYTE*
			BYTE* tempHash = (BYTE*)calloc(21, sizeof(BYTE));
			unsigned short len = 21;
			int errCode = Base64ToChar(userBypass->bypassCode[bypass], strlen((const char*)userBypass->bypassCode[bypass]), tempHash, &len);
			if(errCode != 1)
			{
				printf("\nFailed to Convert bypass from Base64 to Byte* [%d]\n[[%s]]\n", errCode, userBypass->bypassCode[bypass]);
				continue;
			}
			if(len != 21)
			{
				printf("\nUnexpected length %d\n", len);
			}
			//hmac_sha1((unsigned char *)actualHash, (const unsigned char *)salt, (unsigned int)sizeof(salt), (unsigned char*)tempHash, (unsigned int)sizeof(tempHash));
			hmac_sha1(hashStart, key, 20, tempHash, 20);
		}
		AddHashes(userBypass, hashes);
		wroteABypass = true;
	}
	return wroteABypass;
}

BYTE* UnpackAllData(BYTE* slidingPointer, BYTE* endPoint)
{
	slidingPointer = UnpackInitalConfig(slidingPointer, endPoint);
	while(UnpackAPIServer(&slidingPointer, endPoint));
	return UnpackSIDBypasses(slidingPointer, endPoint);
}

void WriteFilterModes()
{
	//get windows version
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionEx(&versionInfo);
	//printf("Version Info : %d.%d build %d", versionInfo.dwMajorVersion , versionInfo.dwMinorVersion, versionInfo.dwBuildNumber);
	if(versionInfo.dwMajorVersion == 5)
	{
		//XP
		RegSetValueEx(RegKey, "filterLocal", NULL, REG_DWORD, (BYTE*)&bypassConfig.filterXPLocal, sizeof(bypassConfig.filterXPLocal));
		RegSetValueEx(RegKey, "filterRDP", NULL, REG_DWORD, (BYTE*) &bypassConfig.filterXPRDP, sizeof(bypassConfig.filterXPRDP));
	}
	else if(versionInfo.dwMajorVersion == 6)
	{
		//CP
		RegSetValueEx(RegKey, "filterLocal", NULL, REG_DWORD, (BYTE*)&bypassConfig.filterCPLocal, sizeof(bypassConfig.filterCPLocal));
		RegSetValueEx(RegKey, "filterRDP", NULL, REG_DWORD, (BYTE*) &bypassConfig.filterCPRDP, sizeof(bypassConfig.filterCPRDP));
	}
	else
	{
		printf("Unexpected Version ID : %d\n", versionInfo.dwMajorVersion);
	}
}

void WriteConfigSettings()
{
	DWORD ret;
	RegSetValueEx(RegKey, "CPDebugMode", NULL, REG_DWORD, (BYTE*)&bypassConfig.debugLogging, sizeof(bypassConfig.debugLogging));
	HKEY ConfigKey;
	ret = RegCreateKeyEx(RegKey, "Config", NULL, NULL, REG_OPTION_NON_VOLATILE,  KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &ConfigKey, NULL);
	if(ret != ERROR_SUCCESS)
	{
		printf("\n\nError [%d] : Could not open up Config Subkey\n\tDynamicPassword mode will need to be set manually to enable it\n\tUse HTTP mode will need to be set manually to enable it\n", ret);
	}
	else
	{
		RegSetValueEx(ConfigKey, "DynamicPassword", NULL, REG_DWORD, (BYTE*)&bypassConfig.useDynamicPassword, sizeof(bypassConfig.useDynamicPassword));
		RegSetValueEx(ConfigKey, "APIUseHTTP", NULL, REG_DWORD, (BYTE*)&bypassConfig.useHTTP, sizeof(bypassConfig.useHTTP));
		RegCloseKey(ConfigKey);
	}

	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\W indows\\CurrentVersion\\Authentication\\Credential Providers", 
		NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &ConfigKey, NULL);
	if(ret != ERROR_SUCCESS)
	{
		printf("\nError [%d] : Could not open up CP Subkey : Could not set the requested setting for CP on Safe Mode\n", ret);
	}
	else
	{
		RegSetValueEx(ConfigKey, "ProhibitFallBacks", NULL, REG_DWORD, (BYTE*)&bypassConfig.useCredInSafeMode, sizeof(bypassConfig.useCredInSafeMode));
		RegCloseKey(ConfigKey);
	}
}

void WriteRegistryData()
{
	printf("\tWriting Servers\n");
	writeRegServers();
	printf("\tServers Written\n");
	printf("\tWriting Filter Modes\n");
	WriteFilterModes();
	printf("\tFilter Modes Written\n");
	printf("\tWriting Debug/Dynamic Password options\n");
	WriteConfigSettings();
	printf("\tDebug Mode/Dynamic Pass set\n");

}

bool GeneratePCBypassKey(BYTE*& key, const char** errorMessage)
{
	printf("\t\tGenerating new PC Specific Key\n");
	key = CreateKey();

	if(key == NULL)
	{
		char* buffer = (char*)calloc(50, sizeof(char));
		sprintf_s(buffer, 50 * sizeof(char), "Error : Failed to create CryptoKey : %d", GetLastError());
		*errorMessage = (const char*)buffer;
		return false;
	}
	printf("Key Generated\n"
	"%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,"
	"%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX,%02lX\n",
	key[ 0],key[ 1],key[ 2],key[ 3],key[ 4],
	key[ 5],key[ 6],key[ 7],key[ 8],key[ 9],
	key[10],key[11],key[12],key[13],key[14],
	key[15],key[16],key[17],key[18],key[19]);

	RegSetValueEx(RegKey, NULL, 0, REG_BINARY, key, 20);
	printf("\t\tWrote PC Specific Key\n");
	return true;
}

bool LoadPCBypassKey(BYTE*& key, const char** errorMessage)
{
	DWORD dataLength;
	if((RegQueryValueEx(RegKey, NULL, NULL, NULL, NULL, &dataLength) == ERROR_FILE_NOT_FOUND) || (dataLength != 20))
	{
		return GeneratePCBypassKey(key, errorMessage);
	}
	else
	{
		printf("\t\tReading PC Specific Key from the Registry\n");
		key = (BYTE*)calloc(20, sizeof(BYTE));
		RegQueryValueEx(RegKey, NULL, NULL, NULL, key, &dataLength);
		return true;
	}
}

void ProcessManualConfiguration(bool* rollback)
{
	while(true)
	{
		printf("Continue? ('C'ontinue/'M'anually Configure/'U'ninstall)");
		char c = getchar();
		if((c == 'c') || (c == 'C'))
		{
			printf("Disabling Mi-Token OTP : ");
			HKEY enabled;
			DWORD ret;
			if((ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon", 0, NULL, 0, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &enabled, nullptr)) != ERROR_SUCCESS)
			{
				printf("Failed to open Registry (4). Err ID : %d\n", ret);
			}
			else
			{
				DWORD val = 0;
				RegSetValueEx(enabled, "Enabled", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
				RegCloseKey( enabled );
			}

			DWORD val = 0;
			LSTATUS stat = RegSetValue(RegKey, "Enabled", REG_DWORD, (LPCSTR)&val, sizeof(val));
			printf("%d\r\n", val);
			printf("\r\n--------------------------------------------------------------------------------\r\nImportant Info!\r\n");
			printf("Mi-Token OTP Checking is currently disabled.\r\nThis means anything can be typed into the OTP field of the Mi-Token Credential Provider and it's value will not be checked\r\nTo enable Mi-Token OTP Checking Please:\r\n\t1: Open up RegEdit\r\n\t2: Go to the HKLM\\Software\\Mi-Token\\Logon folder\r\n\t3: Change the value for the \"Enabled\" key from 0 to 1.\r\n");
			printf("Press enter to continue...");
			//flush input
			while(c != '\n')
			{
				c = getchar();
			}
			break;
		}
		else if((c == 'M') || (c == 'm'))
		{
			while((c = getchar()) != '\n');
			STARTUPINFO startInfo;
			ZeroMemory(&startInfo, sizeof(STARTUPINFO));
			startInfo.cb = sizeof(STARTUPINFO);
			startInfo.dwFlags = STARTF_FORCEONFEEDBACK;
			PROCESS_INFORMATION procInfo;
			if(CreateProcess(NULL, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\API Setup.exe", NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo) == 0)
			{
				printf("Failed to start Manual Configuration utility\n");
				continue;
			}
			else
			{
				printf("Waiting until the Manual Configuration utility is finished\n");
				WaitForSingleObject(procInfo.hProcess, INFINITE);
				DWORD exitCode;
				if(GetExitCodeProcess(procInfo.hProcess, &exitCode))
					printf("Process terminated with exit code %d\n", exitCode);
				continue;
			}
		}
		else if((c == 'u') || (c == 'U'))
		{
			//rollback
			*rollback = true;
			break;
		}
		printf("Invalid input\n");
		while(c != '\n')
			c = getchar();
	}
	while(getchar() != '\n');
}

bool Enable2FA(DWORD state, const char** errorMessage)
{
	HKEY enabled;
	DWORD ret;
	if((ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon", 0, NULL, 0, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &enabled, &state)) != ERROR_SUCCESS)
	{
		char* buffer = (char*)calloc(50, sizeof(char));
		sprintf_s(buffer, 50 * sizeof(char), "Failed to open Registry (3). Err ID : %d\n", ret);
		*errorMessage = (const char*)buffer;
		return false;
	}
	DWORD val = 1;
	RegSetValueEx(enabled, "Enabled", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
	RegCloseKey( enabled );
	printf("\t2FA is now enabled on this system\n");
	return true;
}

bool WriteBypassToRegistry(bool* rollback, const char** errorMessage)
{
	DWORD state = 0;
	
	DWORD ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon\\Bypass", 0, NULL, 0, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &RegKey, &state);
	if(ret != ERROR_SUCCESS)
	{
		char* buffer = (char*)calloc(50, sizeof(char));
		sprintf_s(buffer, 50 * sizeof(char), "Failed to open Registry (2). Err ID : %d\n", ret);
		*errorMessage = (const char*)buffer;
		return false;
	}
	

	DWORD dataLength;
	BYTE* key = NULL;
	
	if(!LoadPCBypassKey(key, errorMessage))
		return false;

	printf("\t\tWriting SID Specific bypasses to the registry\n");
	if(key != NULL)
	{
		if(!writeUserBypasses(key))
		{
			printf("\nWARNING!\nNo user bypasses were written.\nCaution : If you continue and are logged in as a non-admin account you will NOT be able to get UAC elevation without a valid OTP.\n"); 
			ProcessManualConfiguration(rollback);
		}
		else
		{
			//lastly write the Enabled reg key
			if(!Enable2FA(state, errorMessage))
			{
				return false;
			}
		}
		printf("\tFinished Writing Bypass Settings\n");
	}

	RegCloseKey( RegKey );
	return true;
}

bool WriteBypassSettings(BYTE* data, int length, const char** errorMessage, bool * rollback)
{
	long ret;
	*rollback = false;
	if(data == NULL)
		errorString = "Error : No config file existed";

	memset(&bypassConfig, 0, sizeof(bypassConfig));

	BYTE* slidingPointer = data;
	BYTE* endPoint = data + length;
	slidingPointer = UnpackAllData(slidingPointer, endPoint);
	DEBUG_STRING("BypassSettings unpacked\n");

	DEBUG_STRING("Error: ");
	DEBUG_STRING(errorString == NULL ? "(NULL)" : errorString);
	DEBUG_STRING("\n");

	debugLogConfig();
	*errorMessage = errorString;
	if(slidingPointer == NULL)
		return false;
	//Time to write the registry keys
	printf("Opening registry key\n");
	DWORD state;
	HKEY tempkey;
	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token", 0, NULL, 0, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &tempkey, &state);
	if(ret != ERROR_SUCCESS)
	{
		char* buffer = (char*)calloc(50, sizeof(char));
		sprintf_s(buffer, 50 * sizeof(char), "Failed to create Mi-Token Key. Err ID : %d\n", ret);
		*errorMessage = (const char*)buffer;
		return false;
	}
	RegCloseKey(tempkey); //we only created tempkey to make sure it existed.

	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon", 0, NULL, 0, KEY_ALL_ACCESS | (wow64 ? KEY_WOW64_64KEY : 0), NULL, &RegKey, &state);
	if(ret != ERROR_SUCCESS)
	{
		char* buffer = (char*)calloc(50, sizeof(char));
		sprintf_s(buffer, 50 * sizeof(char), "Failed to open Registry. Err ID : %d\n", ret);
		*errorMessage = (const char*)buffer;
		return false;
	}

	printf("Writing Registry Keys\n");
	WriteRegistryData();
	printf("Finished Writing Registry Keys\n");
	RegCloseKey(RegKey);

	printf("\tWriting Bypass Settings\n");
	if(!WriteBypassToRegistry(rollback, errorMessage))
	{
		return false;
	}

	printf("Done\n");



	return true;

}
