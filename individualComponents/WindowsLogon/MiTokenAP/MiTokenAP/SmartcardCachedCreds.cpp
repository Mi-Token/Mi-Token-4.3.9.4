#include "SmartcardCachedCreds.h"
#include <assert.h>
#include <stdlib.h>
//#include "card32.h"
#include <assert.h>
#include "..\ConsoleAP\Wrappers.h"



#define HEADER_CARD_ID 0x10
#define HEADER_USERNAME 0x11
#define HEADER_PASSWORD 0x12
#define HEADER_DEBUG_STRING 0x01
#define HEADER_ASK_STORE 0x02
#define HEADER_STORED_INFO 0x03
#define HEADER_WRITE_CACHE 0x04



#define HEADERV2_CARD_ID 0x20
#define HEADERV2_STORED_BLOB 0x21
#define HEADERV2_STORE_BLOB 0x22
#define HEADERV2_BIND_LUID 0x23
#define HEADERV2_UNBIND_LUID 0x24

#define NO_V1
#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { free(p); (p) = NULL; }
#endif

//NamedPipeClient NPC;

struct SC_CredCache
{
	unsigned long CardID;
	unsigned int usernameLength;
	unsigned int passwordLength;
	unsigned char * username;
	unsigned char * password;
	int scanned;
};


struct SC_CredCacheV2
{
	unsigned __int64 CardID;
	unsigned __int32 usernameLength;
	unsigned __int32 passwordLength;
	unsigned char* username;
	unsigned char* password;
	unsigned __int32 scanned;
};

struct SC_PackagedCacheHeader
{
	unsigned int headerSize;
	unsigned int dataLength;
	unsigned long CardID;
	unsigned int usernameStart;
	unsigned int usernameLength;
	unsigned int passwordStart;
	unsigned int passwordLength;
};

struct SC_PackagedCacheHeaderV2
{
	unsigned __int32 headerSize;
	unsigned __int32 dataLength;
	unsigned __int64 CardID;
	unsigned __int32 usernameStart;
	unsigned __int32 usernameLength;
	unsigned __int32 passwordStart;
	unsigned __int32 passwordLength;
};

struct SC_PackagedCache
{
	SC_PackagedCacheHeader* header;
	unsigned char* data;
};

struct SC_PackagedCacheV2
{
	SC_PackagedCacheHeaderV2* header;
	unsigned char* data;
};

SC_CredCache Cred_Cache = {0};
SC_CredCacheV2 Cred_CacheV2 = {0};

void NPC_CheckRetData(unsigned char* data, DWORD bytesRead)
{
	if(bytesRead < 2)
	{
		assert(0);
		return;
	}
	DWORD totalLength = (DWORD)data[1];
	if(totalLength == 2)
	{
		return;
	}
	if(totalLength != bytesRead)
	{
		assert(0);
		return;
	}
	SC_PackagedCache cache;
	cache.header = (SC_PackagedCacheHeader*)&(data[2]);
	cache.data = (unsigned char*)&(data[2 + cache.header->headerSize]);

	Cred_Cache.CardID = cache.header->CardID;
	
	Cred_Cache.passwordLength = cache.header->passwordLength;
	Cred_Cache.password = (unsigned char*)calloc(cache.header->passwordLength, sizeof(unsigned char));
	memcpy(Cred_Cache.password, &(cache.data[cache.header->passwordStart]), cache.header->passwordLength);
	
	Cred_Cache.usernameLength = cache.header->usernameLength;
	Cred_Cache.username = (unsigned char*)calloc(cache.header->usernameLength, sizeof(unsigned char));
	memcpy(Cred_Cache.username, &(cache.data[cache.header->usernameStart]), cache.header->usernameLength);

	Cred_Cache.scanned = 1;
}

BOOL SCCreds_Init()
{
	if(NPC.Connect(_T("\\\\.\\pipe\\MiTokenContactlessCred"), NULL) != 0)
		return FALSE;
	/*
	unsigned char ask = HEADER_ASK_STORE;
	NPC.SendBytes(&ask, 1);
	unsigned char retData[1024];
	DWORD bytesRead;
	NPC.readPipeBytes(retData, 1024, &bytesRead);

	NPC_CheckRetData(retData, bytesRead);
	*/
	return TRUE;
}

SCCred_CARD_RESULT SCCreds_CardScanned(unsigned long CardID, bool overwrite)
{
	if((!overwrite) && (Cred_Cache.scanned != 0))
	{
		if(Cred_Cache.CardID != CardID)
			return SCCred_NotDefaultCard;
		else
			return SCCred_AutoLoginUser;
	}
	else
	{
		Cred_Cache.CardID = CardID;
		return SCCred_NewDefaultCard;
	}
}

void SCCreds_SendData()
{
#ifdef NO_V1
	assert(0);
#endif
	//the cache we will make
	SC_PackagedCache cache;
	//the data to send
	unsigned char * sendData;

	//Header calloc
	cache.header = (SC_PackagedCacheHeader*)calloc(1, sizeof(SC_PackagedCacheHeader));
	
	//Header initialization
	cache.header->headerSize = sizeof(SC_PackagedCacheHeader);
	cache.header->CardID = Cred_Cache.CardID;
	
	cache.header->usernameStart = 0;
	cache.header->usernameLength = Cred_Cache.usernameLength;
	
	cache.header->passwordStart = cache.header->usernameLength;
	cache.header->passwordLength = Cred_Cache.passwordLength;
	
	cache.header->dataLength = Cred_Cache.passwordLength + Cred_Cache.usernameLength;
	
	//we now know the length of sendData (OP_CODE + LEN_BYTE + header + data)
	int sendLength = 1 + 1 + cache.header->headerSize + cache.header->dataLength;
	sendData = (unsigned char*)calloc(sendLength, sizeof(unsigned char));

	//the data will start at 2 + header_len bytes
	cache.data = &(sendData[2 + cache.header->headerSize]);

	sendData[0] = HEADER_WRITE_CACHE;
	sendData[1] = (unsigned char)(sendLength);
	memcpy(sendData + 2, cache.header, cache.header->headerSize);

	//copy buffers into data
	memcpy(cache.data + cache.header->usernameStart, Cred_Cache.username, cache.header->usernameLength);
	memcpy(cache.data + cache.header->passwordStart, Cred_Cache.password, cache.header->passwordLength);

	NPC.SendBytes(sendData, sendLength);

	//free calloc'd data
	free(sendData);
	free(cache.header);
}

BOOL SCCreds_AddNameAndPass(unsigned char* username, int usernameLength, unsigned char* password, int passwordLength)
{
#ifdef NO_V1
	assert(0);
#endif
	SAFE_FREE(Cred_Cache.username);
	SAFE_FREE(Cred_Cache.password);

	Cred_Cache.username = (unsigned char*)calloc(usernameLength, sizeof(unsigned char));
	Cred_Cache.password = (unsigned char*)calloc(passwordLength, sizeof(unsigned char));

	if((Cred_Cache.username == NULL) || (Cred_Cache.password == NULL))
	{
		//We ran out of memory - free it and return
		SAFE_FREE(Cred_Cache.username);
		SAFE_FREE(Cred_Cache.password);
		return FALSE;
	}

	Cred_Cache.usernameLength = usernameLength;
	Cred_Cache.passwordLength = passwordLength;

	memcpy(Cred_Cache.username, username, usernameLength);
	memcpy(Cred_Cache.password, password, passwordLength);

	return TRUE;
}


HRESULT SCCreds_GetUsernameAndPass(unsigned char* username, int* usernameLength, unsigned char* password, int* passwordLength)
{
#ifdef NO_V1
	assert(0);
#endif
	if((usernameLength == 0) || (passwordLength == 0))
		return ERROR_INVALID_ADDRESS;

	HRESULT currentError = ERROR_SUCCESS;

	//Process Username - Do not return in this block - we will still need to process password
	if((username == NULL) || (*usernameLength == 0))
	{
		*usernameLength = Cred_Cache.usernameLength;

		//only set to ERROR_MORE_DATA if there is actually data
		if(Cred_Cache.usernameLength != 0)
			currentError = ERROR_MORE_DATA;
	}
	else
	{
		if(*(unsigned int*)usernameLength >= Cred_Cache.usernameLength)
		{
			memcpy(username, Cred_Cache.username, Cred_Cache.usernameLength);
		}
		else
		{
			*(unsigned int*)usernameLength = Cred_Cache.usernameLength;
			currentError = ERROR_MORE_DATA;
		}
	}

	//Process Password
	if((password == NULL) || (*passwordLength == 0))
	{
		*passwordLength = Cred_Cache.passwordLength;

		//only set to ERROR_MORE_DATA if there is actually data
		if(Cred_Cache.passwordLength != 0)
			currentError = ERROR_MORE_DATA;
	}
	else
	{
		if(*(unsigned int*)passwordLength >= Cred_Cache.passwordLength)
		{
			memcpy(password, Cred_Cache.password, Cred_Cache.passwordLength);
		}
		else
		{
			*(unsigned int*)passwordLength = Cred_Cache.passwordLength;
			currentError = ERROR_MORE_DATA;
		}
	}

	return currentError;
}

void SCCreds_Finished()
{
#ifdef NO_V1
	assert(0);
#endif
	NPC.close();
	SAFE_FREE(Cred_Cache.password);
	SAFE_FREE(Cred_Cache.username);
}

/*
void SCCreds_Beep()
{
		HANDLE icdev = NULL;
	int ret = 0;
	

	icdev = dc_init(100,115200);
	if (icdev <= 0)
	{
		return;
	}

	dc_beep(icdev, 50);
	ret = dc_exit(icdev);
	if (ret != 0)
	{
		return;
	}

}

BOOL TryReadSerialNumber(unsigned long* serialNumber)
{
	HANDLE icdev = NULL;
	int ret = 0;
	unsigned long snr = 0;


	icdev = dc_init(100,115200);
	if (icdev <= 0)
	{
		return FALSE;
	}

	ret = dc_card(icdev, 0, &snr);
	if (ret != 0)
	{
		return FALSE;
	}


	//Shut it up for testing.
	//dc_beep(icdev, 50);
	ret = dc_exit(icdev);
	if (ret != 0)
	{
		return FALSE;
	}

	*serialNumber = snr;
	return TRUE;
}
*/

BOOL SCCreds_HasCachedCreds()
{
#ifdef NO_V1
	assert(0);
#endif
	return (Cred_Cache.scanned) ? TRUE : FALSE;
}


SCCred_CARD_RESULTV2 SCCreds_CardScannedV2(__int64 CardID)
{
	//ask the service if it has data for that card
	unsigned char data[1 + sizeof(__int64)];
	//byte 0 is header (HEADERV2_CARD_ID)
	data[0] = HEADERV2_CARD_ID;
	//byte 1 -> 8 is the CardID
	memcpy(data + 1, &CardID, sizeof(__int64));
	NPC.SendBytes(data, 1 + sizeof(__int64));

	//wait for the response
	//presume 1024 bytes is enough
	unsigned char* resp = (unsigned char*)calloc(1024, sizeof(unsigned char));
	DWORD bytesRead;
	NPC.readPipeBytes(resp, 1024, &bytesRead);
	//first byte should be the correct header
	assert(resp[0] == HEADERV2_STORED_BLOB);
	//we should read at least 5 bytes
	assert(bytesRead >= 5);
	__int32 length;
	memcpy(&length, resp + 1, sizeof(__int32));
	if(length == 0)
	{
		//no data was sent
		Cred_CacheV2.CardID = CardID;
		return SCCred_CardDoesntExist;
	}
	//we actually have data
	SC_PackagedCacheHeaderV2* PCV2 = (SC_PackagedCacheHeaderV2*)&resp[5];
	//d points to the location after PCV2
	unsigned char* d = (unsigned char*)PCV2 + PCV2->headerSize;
	Cred_CacheV2.CardID = PCV2->CardID;
	Cred_CacheV2.scanned = true;
	Cred_CacheV2.passwordLength = PCV2->passwordLength;
	Cred_CacheV2.usernameLength = PCV2->usernameLength;
	Cred_CacheV2.password = (unsigned char*)calloc(Cred_CacheV2.passwordLength, sizeof(char));
	Cred_CacheV2.username = (unsigned char*)calloc(Cred_CacheV2.usernameLength, sizeof(char));
	memcpy(Cred_CacheV2.password, d + PCV2->passwordStart, PCV2->passwordLength);
	memcpy(Cred_CacheV2.username, d + PCV2->usernameStart, PCV2->usernameLength);
	//we are done dealloc resp
	free(resp);
	
	return SCCred_CardExists;
}
BOOL SCCreds_AddNameAndPassV2(unsigned char* username, int usernameLength, unsigned char* password, int passwordLength)
{
	if(Cred_CacheV2.username)
		free(Cred_CacheV2.username);
	if(Cred_CacheV2.password)
		free(Cred_CacheV2.password);
	Cred_CacheV2.usernameLength = usernameLength;
	Cred_CacheV2.passwordLength = passwordLength;
	Cred_CacheV2.username = (unsigned char*)calloc(usernameLength, sizeof(char));
	Cred_CacheV2.password = (unsigned char*)calloc(passwordLength, sizeof(char));
	memcpy(Cred_CacheV2.username, username, usernameLength);
	memcpy(Cred_CacheV2.password, password, passwordLength);
	return TRUE;
}
HRESULT SCCreds_GetUsernameAndPassV2(unsigned char* username, int* usernameLength, unsigned char* password, int* passwordLength)
{
	if((usernameLength == 0) || (passwordLength == 0))
		return ERROR_INVALID_ADDRESS;

	HRESULT currentError = ERROR_SUCCESS;

	//Process Username - Do not return in this block - we will still need to process password
	if((username == NULL) || (*usernameLength == 0))
	{
		*usernameLength = Cred_CacheV2.usernameLength;

		//only set to ERROR_MORE_DATA if there is actually data
		if(Cred_CacheV2.usernameLength != 0)
			currentError = ERROR_MORE_DATA;
	}
	else
	{
		if(*(unsigned int*)usernameLength >= Cred_CacheV2.usernameLength)
		{
			memcpy(username, Cred_CacheV2.username, Cred_CacheV2.usernameLength);
		}
		else
		{
			*(unsigned int*)usernameLength = Cred_CacheV2.usernameLength;
			currentError = ERROR_MORE_DATA;
		}
	}

	//Process Password
	if((password == NULL) || (*passwordLength == 0))
	{
		*passwordLength = Cred_CacheV2.passwordLength;

		//only set to ERROR_MORE_DATA if there is actually data
		if(Cred_CacheV2.passwordLength != 0)
			currentError = ERROR_MORE_DATA;
	}
	else
	{
		if(*(unsigned int*)passwordLength >= Cred_CacheV2.passwordLength)
		{
			memcpy(password, Cred_CacheV2.password, Cred_CacheV2.passwordLength);
		}
		else
		{
			*(unsigned int*)passwordLength = Cred_CacheV2.passwordLength;
			currentError = ERROR_MORE_DATA;
		}
	}

	return currentError;
}

void SCCreds_SendDataV2()
{
	//first get the length of everything
	__int32 length = 1 + 4 + sizeof(SC_PackagedCacheHeaderV2) + Cred_CacheV2.passwordLength + Cred_CacheV2.usernameLength;
	unsigned char * data = (unsigned char*)calloc(length, sizeof(char));
	data[0] = HEADERV2_STORE_BLOB;
	memcpy(data + 1, &length, sizeof(__int32));
	SC_PackagedCacheHeaderV2* header = (SC_PackagedCacheHeaderV2*)&data[5];
	header->CardID = Cred_CacheV2.CardID;
	header->dataLength = Cred_CacheV2.passwordLength + Cred_CacheV2.usernameLength;
	header->passwordStart = 0;
	header->passwordLength = Cred_CacheV2.passwordLength;
	header->usernameStart = Cred_CacheV2.passwordLength;
	header->usernameLength = Cred_CacheV2.usernameLength;
	header->headerSize = sizeof(SC_PackagedCacheHeaderV2);
	unsigned char* extdata = (unsigned char*)header + sizeof(SC_PackagedCacheHeaderV2);
	memcpy(extdata + header->passwordStart, Cred_CacheV2.password, header->passwordLength);
	memcpy(extdata  + header->usernameStart, Cred_CacheV2.username, header->usernameLength);
	//data is done
	if(NPC.SendBytes(data, length) != 0)
	{
		//reconnect and resend
		if(NPC.Connect(_T("\\\\.\\pipe\\MiTokenContactlessCred"), NULL) == 0)
		{
			NPC.SendBytes(data, length);
		}
	}

	free(data);
}
void SCCreds_FinishedV2()
{
	NPC.close();
	SAFE_FREE(Cred_CacheV2.password);
	SAFE_FREE(Cred_CacheV2.username);
}

void SCCreds_SetCardID(__int64 CardID)
{
	Cred_CacheV2.CardID = CardID;
}
