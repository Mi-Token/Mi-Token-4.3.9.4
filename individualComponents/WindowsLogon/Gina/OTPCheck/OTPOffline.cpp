/*
OTPOffline.cpp

Allows offline OTP verification using cached OTPs
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "base64.h"


#include "OTPOffline.h"
#include "DataBuilder.h"
#include "DebugLogging.h"
#include "OTPBypass.h"
#include "OTPCheck.h"
#include "SDDLs.h"
#include <Sddl.h>



/* Include to get access to extra stubs */
char cacheHeader[4] = { 'M', 'T', 'C', 0x06 };
wchar_t * OTPOffline_ServerName = NULL;
bool OTPOffline_UseHTTP = false;
#define FILE_LOCATION "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Cache.mtc"

#define FLIP_ENDIAN(v) (((v & 0xFF000000) >> 24) | ((v & 0x00FF0000) >> 8) | ((v & 0x0000FF00) << 8) | ((v & 0x000000FF) << 24))

//Used to parse the returned data from AnonVerifyOTPEx (save some code rewriting)
#include "OTPVerifyExParser.h"

#if defined(_M_X64) || defined(__amd64__)

/* Gets the offset value between dest and start */
#define GET_OFFSET(dest, start) (void*)((__int64)(dest) - (__int64)(start))

/* Gets the real location from start and offset*/

#define REV_OFFSET(offset, start) (void*) ((__int64)(start) + (__int64)(offset))

// Same as GET_OFFSET - however sets dest to the result (also takes the type so it is typesafe)
#define GET_OFFSET2(dest, start, type) (dest) = (type*)((__int64)(dest) - (__int64)(start))

// Same as REV_OFFSET - however also sets dest to the result (also takes the type so it is typesafe)
#define REV_OFFSET2(dest, start, type) (dest) = (type*)((__int64)(start) + (__int64)(dest))

#else
/* Gets the offset value between dest and start */
#define GET_OFFSET(dest, start) (void*)((int)(dest) - (int)(start))

/* Gets the real location from start and offset*/

#define REV_OFFSET(offset, start) (void*) ((int)(start) + (int)(offset))

// Same as GET_OFFSET - however sets dest to the result (also takes the type so it is typesafe)
#define GET_OFFSET2(dest, start, type) (dest) = (type*)((int)(dest) - (int)(start))

// Same as REV_OFFSET - however also sets dest to the result (also takes the type so it is typesafe)
#define REV_OFFSET2(dest, start, type) (dest) = (type*)((int)(start) + (int)(dest))

#endif

/* 5 minute token window */
//#define TOKEN_TIME_WINDOW 300

/* allow a drift of upto 2.5 minutes */
//#define TOKEN_BACK_WINDOW 150

#define TOKEN_FORWARD_WINDOW_COUNT 20
#define TOKEN_BACKWARD_WINDOW_COUNT 20

/* 10 event based tokens at once */
#define TOKEN_EVENT_WINDOW 10

#define memcpy(dest,src,size) memcpy_s(dest,size,src,size)


//Flags for AnonVerifyOTPEx
#define OTPEX_FLAG_CENTRAL_BYPASS (0x01)
#define OTPEX_FLAG_OTP_CACHING (0x02)

const int exponent = 3;


#pragma pack(push)
#pragma pack(1)
typedef struct _GENERIC_OTP_INFO
{
	int tokenSize;
	int count;
	int lastUsed;
	void* values;
} GENERIC_OTP_INFO, *PGENERIC_OTP_INFO;

typedef struct _TIME_OTP_INFO
{
	GENERIC_OTP_INFO genInfo;
	int timeStep;
}TIME_OTP_INFO, *PTIME_OTP_INFO;

typedef struct _EVENT_OTP_INFO
{
	GENERIC_OTP_INFO genInfo;
	int IDLength;
	unsigned char ID[12];
} EVENT_OTP_INFO, *PEVENT_OTP_INFO;

typedef struct _EVENT_OTP_INFO_V4
{
	GENERIC_OTP_INFO genInfo;
} EVENT_OTP_INFO_V4, *PEVENT_OTP_INFO_V4;

typedef struct _TOKEN_KEY_INFO
{
	int privKeyLen;
	int pubKeyLen;
	void* privKey;
	void* pubKey;	
} TOKEN_KEY_INFO, *PTOKEN_KEY_INFO;

#pragma pack(pop)
/*
TokenCache storage structure
Note: Pointers in this structure are offsets from the structures start, not pointers to valid points in memory
*/

unsigned char encVersion = 1;

unsigned char masterKey[32] = {0};

void ReadMasterKey()
{
	DEBUG_VERBOSE("Reading Master Key\r\n");
	HKEY RegKey = NULL;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Config", 0, KEY_READ | KEY_WRITE, &RegKey);
	if(RegKey)
	{
		DEBUG_VERBOSE("RegKey Opened\r\n");
		if(RegQueryValueEx(RegKey, L"MKey", NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
		{
			DEBUG_VERBOSE("Key Found\r\n");
			DWORD mkeylength = 0;
			RegQueryValueEx(RegKey, L"MKey", NULL, NULL, &(masterKey[0]), &mkeylength);
			if(mkeylength == 0x20)
			{
				RegQueryValueEx(RegKey, L"MKey", NULL, NULL, &(masterKey[0]), &mkeylength);
			}
			DEBUG_VERBOSE("Key Read\r\n");
		}
		else
		{
			DEBUG_VERBOSE("Generating Key\r\n");
			//At least seed the random number
			srand(time(NULL));
			for(int i = 0 ; i < 0x20 ; ++i)
			{
				masterKey[i] = rand() & 0xFF;	
			}
			DEBUG_VERBOSE("Key Generated\r\n");
			RegSetValueEx(RegKey, L"MKey", NULL, NULL, &(masterKey[0]), 0x20);
			DEBUG_VERBOSE("Key Written\r\n");
		}
		DEBUG_VERBOSE("RegKey Closed\r\n");
		RegCloseKey(RegKey);
	}
	else
	{
		DEBUG_VERBOSE("Failed to open RegKey\r\n");
	}

}

void EncryptRoutine(unsigned char* tokenStart, int tokenSize, int tokenID, int& keyIndex, int& countIndex)
{
	if(encVersion > 0)
	{
		unsigned char lastLetterVal = 0;
		for(int tokenSpot = 0 ; tokenSpot < tokenSize ; ++tokenSpot)
		{
			unsigned char mask = lastLetterVal;
			lastLetterVal = tokenStart[tokenSpot];
			mask ^= masterKey[keyIndex];
			keyIndex++;
			keyIndex &= 0x1F; //% 0x20 | 32
			mask ^= (tokenID * tokenSize);
			mask ^= tokenSpot;
			tokenStart[tokenSpot] ^= mask;
		}
	}
}

void DecryptRoutine(unsigned char* tokenStart, int tokenSize, int tokenID, int& keyIndex, int& countIndex)
{
	if(encVersion > 0)
	{
		unsigned char lastLetterVal = 0;
		for(int tokenSpot = 0 ; tokenSpot < tokenSize ; ++tokenSpot)
		{
			unsigned char mask = lastLetterVal;
			mask ^= masterKey[keyIndex];
			keyIndex++;
			keyIndex &= 0x1F; //% 0x20 | 32
			mask ^= (tokenID * tokenSize);
			mask ^= tokenSpot;
			tokenStart[tokenSpot] ^= mask;
			lastLetterVal = tokenStart[tokenSpot];
		}
	}
}

void EncryptTokenList(GENERIC_OTP_INFO* pTokenInfo)
{
	int keyIndex = 0;
	int countIndex = 0;
	int tokenSize = pTokenInfo->tokenSize;
	for(int tokenCode = 0 ; tokenCode < pTokenInfo->count; ++tokenCode)
	{
		unsigned char* tokenStart = &(((unsigned char*)pTokenInfo->values)[tokenCode * tokenSize]);
		EncryptRoutine(tokenStart, tokenSize, tokenCode, keyIndex, countIndex);
	}
}

void DecryptTokenList(GENERIC_OTP_INFO* pTokenInfo)
{
	int keyIndex = 0;
	int countIndex = 0;
	int tokenSize = pTokenInfo->tokenSize;
	for(int tokenCode = 0 ; tokenCode < pTokenInfo->count; ++tokenCode)
	{
		unsigned char* tokenStart = &(((unsigned char*)pTokenInfo->values)[tokenCode * tokenSize]);
		DecryptRoutine(tokenStart, tokenSize, tokenCode, keyIndex, countIndex);
	}
}


int* allocFlipEndian(unsigned int* src, int length)
{
	int i;

	int* dest = (int*)malloc(length * 4);
	for(i = 0 ; i < length ; ++i)
	{
		dest[i] = FLIP_ENDIAN(src[i]);
	}

	return dest;
}


void RSA_BigEndian_Inner::setFromRSABigNum(BIGNUM* source)
{
	length = BN_num_bytes(source);
	data = malloc(length);
	BN_bn2bin(source, (unsigned char*)data);
}
void RSA_BigEndian_Inner::convertToRamMode(void* start)
{
	DEBUG_VERBOSE("\tConverting : %08lX vs %08lX -> ", this->data, start);
	REV_OFFSET2(this->data, start, void);
	DEBUG_VERBOSE("%08lX\n", this->data);
}
void RSA_BigEndian_Inner::convertToStorageMode(void* start)
{
	GET_OFFSET2(this->data, start, void);
}

void RSA_BigEndian_Inner::DebugKeyInner(char* name, bool inRam)
{
	int i;
	if(inRam)
	{
		DEBUG_VERBOSE("\t\t\t%s Bytes @[%08lX] [First %d] :\n\t\t\t\t", name, this->data, this->length < 16 ? this->length : 16);
		for(i = 0 ; (i < 16) && (i < this->length) ; ++i)
		{
			DEBUG_VERBOSE("%02lX ", ((unsigned char*)this->data)[i]);
		}
		DEBUG_VERBOSE("\n");
	}
	else
	{
		DEBUG_VERBOSE("\t\t\t\%s : +[%08lX]\n", name, this->data);
	}
}
void RSA_BigEndian_Inner::copyFromBuffer(RSA_BigEndian_Inner* source, void** pScratch)
{
	void* scratch = *pScratch;
	this->length = source->length;
	this->data = scratch;
	memcpy(scratch, source->data, this->length);
	*pScratch = REV_OFFSET(this->length, scratch);
}

void RSA_BigEndian_Inner::createBackup(RSA_BigEndian_Inner * source)
{
	this->length = source->length;
	this->data = malloc(this->length);
	memcpy(this->data, source->data, this->length);
}


void RSA_BigEndian::converToRamMode(void* start)
{
	DEBUG_LOG("Converting Key from Storage Mode\n");
	exponent.convertToRamMode(start);
	modulus.convertToRamMode(start);
	D.convertToRamMode(start);
	DP.convertToRamMode(start);
	DQ.convertToRamMode(start);
	inverseQ.convertToRamMode(start);
	P.convertToRamMode(start);
	Q.convertToRamMode(start);

	serverExponent.convertToRamMode(start);
	serverModulus.convertToRamMode(start);
}
void RSA_BigEndian::convertToStorageMode(void* start)
{
	DEBUG_LOG("Converting Key to Storage Mode\n");
	exponent.convertToStorageMode(start);
	modulus.convertToStorageMode(start);
	D.convertToStorageMode(start);
	DP.convertToStorageMode(start);
	DQ.convertToStorageMode(start);
	inverseQ.convertToStorageMode(start);
	P.convertToStorageMode(start);
	Q.convertToStorageMode(start);

	serverExponent.convertToStorageMode(start);
	serverModulus.convertToStorageMode(start);
}
void RSA_BigEndian::debugPrint(bool inRAM)
{
	DEBUG_VERBOSE("\t\tRSA_BigEndian Info:\n");
	DEBUG_VERBOSE("\t\t\tExponent Length : %d\n", this->exponent.length);
	DEBUG_VERBOSE("\t\t\tModulus Length : %d\n", this->modulus.length);
	DEBUG_VERBOSE("\t\t\tD Length : %d\n", this->D.length);
	DEBUG_VERBOSE("\t\t\tDP Length : %d\n", this->DP.length);
	DEBUG_VERBOSE("\t\t\tDQ Length : %d\n", this->DQ.length);
	DEBUG_VERBOSE("\t\t\tInverse Q Length : %d\n", this->inverseQ.length);
	DEBUG_VERBOSE("\t\t\tP Length : %d\n", this->P.length);
	DEBUG_VERBOSE("\t\t\tQ Length : %d\n", this->Q.length);

	this->exponent.DebugKeyInner("Exponent Key", inRAM);
	this->modulus.DebugKeyInner("Modulus Key", inRAM);
	this->D.DebugKeyInner("D Key", inRAM);
	this->DP.DebugKeyInner("DP Key", inRAM);
	this->DQ.DebugKeyInner("DQ Key", inRAM);
	this->inverseQ.DebugKeyInner("Inverse Q Key", inRAM);
	this->P.DebugKeyInner("P Key", inRAM);
	this->Q.DebugKeyInner("Q Key", inRAM);

	DEBUG_LOG("\t\tServer RSA Info:\n");
	DEBUG_LOG("\t\t\tExponent Length : %d\n", this->serverExponent.length);
	DEBUG_LOG("\t\t\tModulus Length : %d\n", this->serverModulus.length);

	this->serverExponent.DebugKeyInner("Exponent Key", inRAM);
	this->serverModulus.DebugKeyInner("Modulus Key", inRAM);
}

int RSA_BigEndian::totalKeySize()
{
	int ret = 0;
	ret += this->exponent.length;
	ret += this->modulus.length;
	ret += this->D.length;
	ret += this->DP.length;
	ret += this->DQ.length;
	ret += this->inverseQ.length;
	ret += this->P.length;
	ret += this->Q.length;

	ret += this->serverExponent.length;
	ret += this->serverModulus.length;
	return ret;
}

void RSA_BigEndian::copyFromBuffer(_RSA_BigEndian* source, void** pScratch)
{
	exponent.copyFromBuffer(&source->exponent, pScratch);
	modulus.copyFromBuffer(&source->modulus, pScratch);
	D.copyFromBuffer(&source->D, pScratch);
	DP.copyFromBuffer(&source->DP, pScratch);
	DQ.copyFromBuffer(&source->DQ, pScratch);
	inverseQ.copyFromBuffer(&source->inverseQ, pScratch);
	P.copyFromBuffer(&source->P, pScratch);
	Q.copyFromBuffer(&source->Q, pScratch);
	serverExponent.copyFromBuffer(&source->serverExponent, pScratch);
	serverModulus.copyFromBuffer(&source->serverModulus, pScratch);
}

void RSA_BigEndian::createBackup(_RSA_BigEndian *source)
{
	this->exponent.createBackup(&source->exponent);
	this->modulus.createBackup(&source->modulus);

	this->D.createBackup(&source->D);
	this->DP.createBackup(&source->DP);
	this->DQ.createBackup(&source->DQ);
	this->inverseQ.createBackup(&source->inverseQ);
	this->P.createBackup(&source->P);
	this->Q.createBackup(&source->Q);
	this->serverExponent.createBackup(&source->serverExponent);
	this->serverModulus.createBackup(&source->serverModulus);	
}


RSA_BigEndian * RSA_FlipEndian(RSA * rsa)
{
	DEBUG_VERBOSE("RSA_BigEndian Flipping RSA Endian\n");
	RSA_BigEndian * ret = (RSA_BigEndian*) malloc(sizeof(RSA_BigEndian));
	
	ret->modulus.setFromRSABigNum(rsa->n);
	DEBUG_VERBOSE("RSA_BigEndian Mod Flipped\n");
	
	DWORD exp = 3;
	//ret->exponent.data = malloc(4);
	//memcpy(ret->exponent.data, &exp, 4);
	ret->exponent.data = allocFlipEndian((unsigned int*)&exp, 4);
	ret->exponent.length = 4;
	DEBUG_VERBOSE("RSA_BigEndian EXP Flipped\n");

	ret->D.setFromRSABigNum(rsa->d);
	DEBUG_VERBOSE("RSA_BigEndian D Flipped\n");
	ret->P.setFromRSABigNum(rsa->p);
	DEBUG_VERBOSE("RSA_BigEndian P Flipped\n");
	ret->Q.setFromRSABigNum(rsa->q);
	DEBUG_VERBOSE("RSA_BigEndian Q Flipped\n");
	ret->DP.setFromRSABigNum(rsa->dmp1);
	DEBUG_VERBOSE("RSA_BigEndian DMP1 Flipped\n");
	ret->DQ.setFromRSABigNum(rsa->dmq1);
	DEBUG_VERBOSE("RSA_BigEndian DMQ1 Flipped\n");
	ret->inverseQ.setFromRSABigNum(rsa->iqmp);
	DEBUG_VERBOSE("RSA_BigEndian IQMP Flipped\n");

	ret->serverExponent.data = NULL;
	ret->serverExponent.length = 0;
	ret->serverModulus.data = NULL;
	ret->serverModulus.length = 0;

	return ret;
}

/*
#pragma pack(push)
#pragma pack(4)
typedef struct _TOKEN_CACHE
{
	char cacheHeader[4];
	int pointerSize;
	tokenCacheMode curMode;
	int totalSize;
	struct
	{
		int userlen;
		const void* username;
		RSA_BigEndian keyInfo;
	} header;
	struct
	{
		int timeTokens;
		int _padding;
		__int64 timeStart;
		PTIME_OTP_INFO timeInfos;
	}timeOTPHeader;
	struct 
	{
		int eventTokens;
		PEVENT_OTP_INFO eventInfos;
	}eventOTPHeader;
}TOKEN_CACHE, *PTOKEN_CACHE;
#pragma pack(pop)
*/

void debugTokenCache(PTOKEN_CACHE cache)
{
	
	int inRam = 0;
	int i;
	DEBUG_LOG("Debug Token Cache [%08lX]\n", cache);
	if(cache == NULL)
		return;
	DEBUG_LOG("\tHeader : \n");
	DEBUG_LOG("\t\tVersion : %c %c %c %02X\n", cache->cacheHeader[0], cache->cacheHeader[1], cache->cacheHeader[2], cache->cacheHeader[3]);
	DEBUG_LOG("\t\tPointer Size : %d\n", cache->pointerSize);
	DEBUG_LOG("\t\tCurrent Mode : %d ", cache->curMode);
	switch(cache->curMode)
	{
	case ramMode:
		DEBUG_LOG("[RAM]\n");
		inRam = 1;
		break;
	case storageMode:
		DEBUG_LOG("[STORAGE]\n");
		break;
	default:
		DEBUG_LOG("[UNKNOWN]\n");
		break;
	}
	DEBUG_LOG("\t\tTotal Size : %d\n", cache->totalSize);
	DEBUG_LOG("\t\tUsername Length : %d\n", cache->header.userlen);
	if(inRam)
	{
		DEBUG_LOG("\t\tUsername :");
		for(i = 0 ; i < cache->header.userlen ; ++i)
		{
			DEBUG_LOG(" %02lX", ((char*)cache->header.username)[i]);
		}
		DEBUG_LOG("\n");
	}
	else
		DEBUG_LOG("\t\tUsername : +[%08lX]\n", cache->header.username);
	
	cache->header.keyInfo.debugPrint(inRam);

	DEBUG_LOG("\tTOTP Info\n");
	DEBUG_LOG("\t\tCount : %d\n", cache->timeOTPHeader.timeTokens);
	DEBUG_LOG("\t\tTimeStart : %I64d\n", cache->timeOTPHeader.timeStart);
	if(inRam)
	{
		DEBUG_LOG("\t\tInfo\n");
		for(i = 0 ; i < cache->timeOTPHeader.timeTokens ; ++i)
		{
			DEBUG_LOG("\t\t\tTimestep : %d\n", cache->timeOTPHeader.timeInfos[i].timeStep);
			DEBUG_LOG("\t\t\tToken Length : %d\n", cache->timeOTPHeader.timeInfos[i].genInfo.tokenSize);
			DEBUG_LOG("\t\t\tToken Count : %d\n", cache->timeOTPHeader.timeInfos[i].genInfo.count);
			DEBUG_LOG("\t\t\tLast Used : %d\n", cache->timeOTPHeader.timeInfos[i].genInfo.lastUsed);
		}
	}
	else
		DEBUG_LOG("\t\tInfo Block : +[%08lX]\n", cache->timeOTPHeader.timeInfos);

	DEBUG_LOG("\tHOTP Info\n");
	DEBUG_LOG("\t\tCount : %d\n", cache->eventOTPHeader.eventTokens);
	if(inRam)
	{
		DEBUG_LOG("Checking Version : ");
		bool IsV4Cache = false;
		if(cache->eventOTPHeader.eventTokens > 0)
		{
			//there is a token
			DEBUG_LOG("Token Exists : ");
			DEBUG_LOG("[%d] ", cache->eventOTPHeader.eventInfos[0].IDLength);
			if((cache->eventOTPHeader.eventInfos[0].IDLength < 0) || (cache->eventOTPHeader.eventInfos[0].IDLength > 12)) //max allowed IDLength is 12. V4 caches would have OTP data here and so should have values >> 12 (unless we are unlucky and the encryption made it <= 12)
			{
				//not a valid v5 cache
				IsV4Cache = true;
			}
		}
		DEBUG_LOG("%d\r\n", IsV4Cache ? 4 : 5);

		if(IsV4Cache)
		{
			DEBUG_LOG("V4 Cache Block Detected. No Internal Dump for HOTP tokens.\r\n");
		}
		else
		{
			DEBUG_LOG("\t\tInfo\n");
			for(i = 0 ; i < cache->eventOTPHeader.eventTokens ; ++i)
			{
				DEBUG_LOG("\t\t\tToken Length : %d\n", cache->eventOTPHeader.eventInfos[i].genInfo.tokenSize);
				DEBUG_LOG("\t\t\tToken Count : %d\n", cache->eventOTPHeader.eventInfos[i].genInfo.count);
				DEBUG_LOG("\t\t\tLast Used : %d\n", cache->eventOTPHeader.eventInfos[i].genInfo.lastUsed);
			}
		}
	}
	else
		DEBUG_LOG("\t\tInfo Block : +[%08lX]\n", cache->eventOTPHeader.eventInfos);


}

/*
Converts all pointers to offsets from the start of the structure
*/
void OTPOffline_ConvertToStorageMode(PTOKEN_CACHE ptokenCache)
{
	int i;
	void* start = ptokenCache;
	PTIME_OTP_INFO ptotp = ptokenCache->timeOTPHeader.timeInfos;
	PEVENT_OTP_INFO peotp = ptokenCache->eventOTPHeader.eventInfos;

	DEBUG_LOG("OTPOffline_ConvertToStorageMode\n");
	if(ptokenCache->curMode == storageMode)	
	{
		//already in storage mode - we don't need to change anything
		return;
	}

	ptokenCache->curMode = storageMode;
	ptokenCache->header.username = GET_OFFSET(ptokenCache->header.username, start);

	ptokenCache->header.keyInfo.convertToStorageMode(start);

	for(i = 0  ; i < ptokenCache->timeOTPHeader.timeTokens ; ++i)
	{
		ptotp[i].genInfo.values = GET_OFFSET(ptotp[i].genInfo.values, start);
	}

	ptokenCache->timeOTPHeader.timeInfos = (PTIME_OTP_INFO)GET_OFFSET(ptokenCache->timeOTPHeader.timeInfos, start);

	DEBUG_LOG("Checking Version : ");
	bool IsV4Cache = false;
	if(ptokenCache->eventOTPHeader.eventTokens > 0)
	{
		//there is a token
		if((ptokenCache->eventOTPHeader.eventInfos[0].IDLength < 0) || (ptokenCache->eventOTPHeader.eventInfos[0].IDLength > 12)) //max allowed IDLength is 12. V4 caches would have OTP data here and so should have values >> 12 (unless we are unlucky and the encryption made it <= 12)
		{
			//not a valid v5 cache
			IsV4Cache = true;
		}
	}
	DEBUG_LOG("%d\r\n", IsV4Cache ? 4 : 5);

	if(IsV4Cache)
	{
		//convert to v4 version.
		PEVENT_OTP_INFO_V4 v4hotp = (PEVENT_OTP_INFO_V4)peotp;
		//change offsets as normal
		for(i = 0 ; i < ptokenCache->eventOTPHeader.eventTokens ; ++i)
		{
			v4hotp[i].genInfo.values = GET_OFFSET(v4hotp[i].genInfo.values, start);
		}
	}
	else
	{
		for(i = 0 ; i < ptokenCache->eventOTPHeader.eventTokens ; ++i)
		{
			peotp[i].genInfo.values = GET_OFFSET(peotp[i].genInfo.values, start);
		}
	}

	ptokenCache->eventOTPHeader.eventInfos = (PEVENT_OTP_INFO)GET_OFFSET(ptokenCache->eventOTPHeader.eventInfos, start);
}

/*
Converts all pointers from offsets to real locations
*/
void OTPOffline_ConvertFromStorageMode(PTOKEN_CACHE ptokenCache)
{
	int i;
	void* start = ptokenCache;
	PTIME_OTP_INFO ptotp;
	PEVENT_OTP_INFO peotp;

	DEBUG_LOG("OTPOffline_ConvertFromStorageMode\n");
	if(ptokenCache->curMode == ramMode)
	{
		//already in RAM mode, don't need to change it
		return;
	}

	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode setMode (RAM)\n");
	ptokenCache->curMode = ramMode;
	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode REV_OFF (username)\n");
	ptokenCache->header.username = REV_OFFSET(ptokenCache->header.username, start);

	ptokenCache->header.keyInfo.converToRamMode(start);

	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode REV_OFF(timeInfos)\n");
	ptokenCache->timeOTPHeader.timeInfos = (PTIME_OTP_INFO)REV_OFFSET(ptokenCache->timeOTPHeader.timeInfos, start);
	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode REV_OFF(eventInfos) [%08lX]\n", ptokenCache->eventOTPHeader.eventInfos);
	ptokenCache->eventOTPHeader.eventInfos = (PEVENT_OTP_INFO)REV_OFFSET(ptokenCache->eventOTPHeader.eventInfos, start);

	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode get totp/eotp\n");
	ptotp = ptokenCache->timeOTPHeader.timeInfos;
	peotp = ptokenCache->eventOTPHeader.eventInfos;

	for(i = 0 ; i < ptokenCache->timeOTPHeader.timeTokens ; ++i)
	{
		DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode REV_OFF totp[%d / %d].value [%08lX]\n", i, ptokenCache->timeOTPHeader.timeTokens, ptotp[i].genInfo.values);
		ptotp[i].genInfo.values = REV_OFFSET(ptotp[i].genInfo.values, start);
	}

	DEBUG_LOG("Checking Version : ");
	bool IsV4Cache = false;
	if(ptokenCache->eventOTPHeader.eventTokens > 0)
	{
		//there is a token
		if((ptokenCache->eventOTPHeader.eventInfos[0].IDLength < 0) || (ptokenCache->eventOTPHeader.eventInfos[0].IDLength > 12)) //max allowed IDLength is 12. V4 caches would have OTP data here and so should have values >> 12 (unless we are unlucky and the encryption made it <= 12)
		{
			//not a valid v5 cache
			IsV4Cache = true;
		}
	}
	DEBUG_LOG("%d\r\n", IsV4Cache ? 4 : 5);

	if(IsV4Cache)
	{
		//convert to v4 version.
		PEVENT_OTP_INFO_V4 v4hotp = (PEVENT_OTP_INFO_V4)peotp;
		//change offsets as normal
		for(i = 0 ; i < ptokenCache->eventOTPHeader.eventTokens ; ++i)
		{
			v4hotp[i].genInfo.values = REV_OFFSET(v4hotp[i].genInfo.values, start);
		}
	}
	else
	{
		for(i = 0 ; i < ptokenCache->eventOTPHeader.eventTokens ; ++i)
		{
			DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode REV_OFF eotp[%d / %d].value [%08lX]\n", i, ptokenCache->eventOTPHeader.eventTokens, peotp[i].genInfo.values);
			peotp[i].genInfo.values = REV_OFFSET(peotp[i].genInfo.values, start);
		}
	}

	DEBUG_VERBOSE("OTPOffline_ConvertFromStorageMode done\n");
}

bool DisableCacheEncrypt()
{
#ifdef _DEBUG
	HKEY EncryptKey;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon", &EncryptKey) == ERROR_SUCCESS)
	{
		DEBUG_VERBOSE("Key Opened\r\n");
		DWORD regVal;
		DWORD size = sizeof(regVal);
		if(RegQueryValueEx(EncryptKey, L"DisableCacheEncrypt", nullptr, nullptr, (LPBYTE)&regVal, &size) == ERROR_SUCCESS)
		{
			DEBUG_VERBOSE("DisableCacheEncrypt key exists, checking value\r\n");
			if(regVal == 1)
			{
				DEBUG_VERBOSE("CAUTION: DisableCacheEncrypt Key is 1, Cache will not be decrypted");
				return true;
			}
		}
		else
		{
			DEBUG_VERBOSE("RegVal doesn't exist. This is not an error.\r\n");
		}

		RegCloseKey(EncryptKey);
		return false;
	}
	else
	{
		DEBUG_VERBOSE("Key didn't exist!\r\n We will encrypt the cache by default");
		return false;
	}
#else
	return false; //must always encrypt cache on release mode
#endif
}

//writes the entire cache to Storage
void OTPOffline_WriteCache(PTOKEN_CACHE ptokenCache)
{
	DEBUG_LOG("+OTPOffline_WriteCache\n");
    DEBUG_LOG("OTPOffline_WriteCache - header: [%02X][%02X][%02X][%02X]\n", ptokenCache->cacheHeader[0], ptokenCache->cacheHeader[1], ptokenCache->cacheHeader[2], ptokenCache->cacheHeader[3]);

	//convert to storage mode
	OTPOffline_ConvertToStorageMode(ptokenCache);

	ptokenCache->pointerSize = sizeof(void*);
	memcpy(ptokenCache->cacheHeader, &cacheHeader[0], 4);
	//simple fwrite for the moment

	//check if we should encrypt the data with DPAPI
	bool useDPAPI = true;

	if(DisableCacheEncrypt())
	{
		useDPAPI = false;
	}
	
	CRYPT_INTEGER_BLOB plaintextBlob, ciphertextBlob;

	
	if(useDPAPI)
	{
		DEBUG_VERBOSE("Encrypting Cache...");
		plaintextBlob.pbData = (BYTE*)ptokenCache;
		plaintextBlob.cbData = ptokenCache->totalSize;
		
		CryptProtectData(&plaintextBlob, L"Mi-Token OTP Cache", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &ciphertextBlob);
		DEBUG_VERBOSE("DONE\r\n");
	}
	else
	{
		DEBUG_VERBOSE("Using Unencrypted Cache...\r\n");
		ciphertextBlob.pbData = (BYTE*)ptokenCache;
		ciphertextBlob.cbData = ptokenCache->totalSize;
	}

	DEBUG_VERBOSE("Getting expression for MTC File\r\n");
	char* SDDLString;
	PSECURITY_DESCRIPTOR fileSD = nullptr;
	if(GetMTC_SDDL_String(&SDDLString, false))
	{
		DEBUG_VERBOSE("Converting expression to descriptor\r\n");
		if(ConvertStringSecurityDescriptorToSecurityDescriptorA(SDDLString, SDDL_REVISION_1, &fileSD, nullptr))
		{
			DEBUG_VERBOSE("Created Successfully\r\n");
		}
		else
		{
			DEBUG_VERBOSE("Failed to create descriptor, error = %d\r\n", GetLastError());
		}
	}

	DEBUG_VERBOSE("Creating fileSA\r\n");
	SECURITY_ATTRIBUTES fileSA;
	fileSA.nLength = sizeof(fileSA);
	fileSA.lpSecurityDescriptor = fileSD;
	fileSA.bInheritHandle = false;

	DEBUG_VERBOSE("Opening File\r\n");

	if(PathFileExistsA(FILE_LOCATION))
	{
		DeleteFileA(FILE_LOCATION);
	}

	HANDLE fileHandle = CreateFileA(FILE_LOCATION, GENERIC_READ | GENERIC_WRITE, 0, &fileSA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		DEBUG_VERBOSE("Writing to file\r\n");
		DWORD bytesWritten = 0, totalBytesWritten = 0;
		while(totalBytesWritten != ciphertextBlob.cbData)
		{
			WriteFile(fileHandle, ciphertextBlob.pbData + totalBytesWritten, ciphertextBlob.cbData - totalBytesWritten, &bytesWritten, nullptr);
			DEBUG_VERBOSE("Writing from %d, %d bytes written. Total size is %d\r\n", totalBytesWritten, bytesWritten, ciphertextBlob.cbData);
			totalBytesWritten += bytesWritten;
		}
		CloseHandle(fileHandle);
	}
	else
	{
		DEBUG_VERBOSE("Failed to open the file. GLE = %d\r\n", GetLastError());
	}

	
	DEBUG_VERBOSE("Freeing SD\r\n");
	if(fileSD != nullptr)
	{
		LocalFree(fileSD);
	}

	/*
	FILE* fp = NULL;
	if(fopen_s(&fp, FILE_LOCATION, "wb+") == 0)
	{
		//fwrite(ptokenCache, 1, ptokenCache->totalSize, fp);
		fwrite(ciphertextBlob.pbData, 1, ciphertextBlob.cbData, fp);
		fclose(fp);
	}
	*/

	if(useDPAPI)
	{
		DEBUG_VERBOSE("Freeing CipherData\r\n");
		LocalFree(ciphertextBlob.pbData);
	}

	//convert back out of storage mode
	OTPOffline_ConvertFromStorageMode(ptokenCache);
	DEBUG_LOG("-OTPOffline_WriteCache\n");
}



//Reads the entire cache from Storage
void OTPOffline_ReadCache(PTOKEN_CACHE * pptokenCache)
{
	DEBUG_LOG("OTPOffline_ReadCache\n");
	FILE* fp = NULL;
	int fsize;

	//check if we should encrypt the data with DPAPI
	bool useDPAPI = true;
	
	if(DisableCacheEncrypt())
	{
		useDPAPI = false;
	}

	DEBUG_VERBOSE("Starting File Read\r\n");

	CRYPT_INTEGER_BLOB plaintextBlob, ciphertextBlob;

	if(fopen_s(&fp, FILE_LOCATION, "rb+") == 0)
	{
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		DEBUG_VERBOSE("File is %d bytes\r\n", fsize);
		if(fsize < sizeof(TOKEN_CACHE))
		{
			DEBUG_LOG("fsize < sizeof(TOKEN_CACHE) ignoring cache file\n");
			fclose(fp);
			return;
		}
		fseek(fp, 0, SEEK_SET);
		//*pptokenCache = (PTOKEN_CACHE)malloc(fsize);
		DEBUG_VERBOSE("Allocating Data for file\r\n");
		ciphertextBlob.pbData = (BYTE*)LocalAlloc(0, fsize);
		ciphertextBlob.cbData = fsize;
		if(ciphertextBlob.pbData)
		{
			DEBUG_VERBOSE("Reading file data [%08lX]...", fp);
			fread(ciphertextBlob.pbData, 1, fsize, fp);
			DEBUG_VERBOSE("Done\r\nClosing file handle [%08lX]\r\n", fp);
			fclose(fp);

			DEBUG_VERBOSE("Starting Decryption\r\n");
			if(useDPAPI)
			{
				DEBUG_VERBOSE("Decrypting Cache...");
				if(!CryptUnprotectData(&ciphertextBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &plaintextBlob))
				{
					DEBUG_VERBOSE("Failed to decrypt the cache. Cache may not have been encrypted.\r\n");
					*pptokenCache = nullptr;
					DEBUG_VERBOSE("Freeing Cache Data\r\n");
					//free(ciphertextBlob.pbData);
					LocalFree(ciphertextBlob.pbData);
					DEBUG_VERBOSE("Returning\r\n");
					return;
				}
				else
				{
					*pptokenCache = (PTOKEN_CACHE)malloc(plaintextBlob.cbData);
					memcpy(*pptokenCache, plaintextBlob.pbData, plaintextBlob.cbData);
					LocalFree(plaintextBlob.pbData);
					LocalFree(ciphertextBlob.pbData);
				}
				DEBUG_VERBOSE("DONE\r\n");
			}
			else
			{
				DEBUG_VERBOSE("Cache is not encrypted\r\n");
				*pptokenCache = (PTOKEN_CACHE)malloc(ciphertextBlob.cbData);
				memcpy(*pptokenCache, ciphertextBlob.pbData, ciphertextBlob.cbData);
				LocalFree(ciphertextBlob.pbData);
			}
			

			if(memcmp(*pptokenCache, cacheHeader, 4) != 0)
			{
				DEBUG_LOG("cache file had an invalid header... expected %c%c%c[%02X]\n", cacheHeader[0], cacheHeader[1], cacheHeader[2], cacheHeader[3]);
				unsigned char *ptr = reinterpret_cast<unsigned char*>(*pptokenCache);
				DEBUG_LOG("header: [%02X][%02X][%02X][%02X]\n", *ptr, *(ptr+1), *(ptr+2), *(ptr+3));

				//invalid header code - delete cache
				free(*pptokenCache);
				*pptokenCache = NULL;
				DEBUG_LOG("Returning without cache\r\n");
				return;
			}
			debugTokenCache(*pptokenCache);
			OTPOffline_ConvertFromStorageMode(*pptokenCache);
			debugTokenCache(*pptokenCache);
		}
	}
}

#define OTP_CHANGE_BASE (0x01)
#define OTP_CHANGE_KEYS (0x02)
#define OTP_CHANGE_TIME (0x04)
#define OTP_CHANGE_EVENT (0x08)

PTOKEN_CACHE OTPOffline_ChangeCache(PTOKEN_CACHE pcache, int changeFlags, PTOKEN_CACHE pBaseCache, PRSA_BigEndian pKeyInfo, PTIME_OTP_INFO pTime, PEVENT_OTP_INFO pEvent)
{
	PTOKEN_CACHE ncache;
	PTOKEN_CACHE rcache;
	PRSA_BigEndian rkeys;
	PEVENT_OTP_INFO revent;
	PTIME_OTP_INFO rtime;

	void* scratch;
	int newLength;
	int i;
	int len;

	DEBUG_LOG("OTPOffline_ChangeCache\n");
	//we need everything in real locations so call the conversion just incase it is in storage mode
	OTPOffline_ConvertFromStorageMode(pcache);

	DEBUG_LOG("OTPOffline - Changing Cache Data\n");
	//default size
	newLength = sizeof(TOKEN_CACHE);

	//check the flags
	if(pBaseCache == NULL)
	{
		DEBUG_VERBOSE("pBaseCache is null\nchangeFlags changed from %d to ", changeFlags);
		//if baseCache is null we CANNOT change the base, event or time sections
		changeFlags &= ~(OTP_CHANGE_BASE | OTP_CHANGE_EVENT | OTP_CHANGE_TIME);
		DEBUG_VERBOSE("%d", changeFlags);
	}

	if(pKeyInfo == NULL)
	{
		DEBUG_VERBOSE("pKeyInfo is null\nchangeFlags changed from %d to ", changeFlags);
		//if keyInfo is null, we CANNOT change the key section
		changeFlags &= ~(OTP_CHANGE_KEYS);
		DEBUG_VERBOSE("%d", changeFlags);
	}

	if(pTime == NULL)
	{
		DEBUG_VERBOSE("pTime is null\nchangeFlags changed from %d to ", changeFlags);
		//if pTime is null, we CANNOT change the time section
		changeFlags &= ~(OTP_CHANGE_TIME);
		DEBUG_VERBOSE("%d", changeFlags);
	}

	if(pEvent == NULL)
	{
		DEBUG_VERBOSE("pEvent is null\nchangeFlags changed from %d to ", changeFlags);
		//if pEvent is null, we CANNOT change the event section
		changeFlags &= ~(OTP_CHANGE_EVENT);
		DEBUG_VERBOSE("%d", changeFlags);
	}


	DEBUG_VERBOSE("changeFlags is %d\n", changeFlags);
	if(changeFlags == 0)
	{
		//if there are no change flags the copy is simpler
		//we will instead do : Convert To Storage mode -> Allocate a new cache -> memcpy -> change both to RAM Mode -> return new cache
		OTPOffline_ConvertToStorageMode(pcache);
		ncache = (PTOKEN_CACHE)malloc(pcache->totalSize);
		memcpy(ncache, pcache, pcache->totalSize);
		OTPOffline_ConvertFromStorageMode(pcache);
		OTPOffline_ConvertFromStorageMode(ncache);

		return ncache;
	}

	//from here on, NULL checks are not required as we have already disabled flags that would cause an issue
	DEBUG_VERBOSE("OTP_CHANGE_BASE Check\n");
	if(changeFlags & OTP_CHANGE_BASE)
	{
		newLength += pBaseCache->header.userlen;
	}
	else
	{
		newLength += pcache->header.userlen;
	}

	DEBUG_VERBOSE("OTP_CHANGE_KEYS Check\n");
	if(changeFlags & OTP_CHANGE_KEYS)
	{
		newLength += pKeyInfo->totalKeySize();
	}
	else
	{
		newLength += pcache->header.keyInfo.totalKeySize();
	}

	DEBUG_VERBOSE("OTP_CHANGE_TIME Check\n");
	if(changeFlags & OTP_CHANGE_TIME)
	{
		newLength += (pBaseCache->timeOTPHeader.timeTokens * sizeof(TIME_OTP_INFO));

		for(i = 0 ; i < pBaseCache->timeOTPHeader.timeTokens ; ++i)
		{
			newLength += (pTime[i].genInfo.tokenSize * pTime[i].genInfo.count);
		}
	}
	else
	{
		newLength += (pcache->timeOTPHeader.timeTokens * sizeof(TIME_OTP_INFO));

		for(i = 0 ; i < pcache->timeOTPHeader.timeTokens ; ++i)
		{
			newLength += (pcache->timeOTPHeader.timeInfos[i].genInfo.tokenSize * pcache->timeOTPHeader.timeInfos[i].genInfo.count);
		}
	}

	DEBUG_VERBOSE("OTP_CHANGE_EVENT Check\n");
	if(changeFlags & OTP_CHANGE_EVENT)
	{
		newLength += (pBaseCache->eventOTPHeader.eventTokens * sizeof(EVENT_OTP_INFO));

		for(i = 0 ; i < pBaseCache->eventOTPHeader.eventTokens ; ++i)
		{
			newLength += (pEvent[i].genInfo.tokenSize * pEvent[i].genInfo.count);
		}
	}
	else
	{
		newLength += (pcache->eventOTPHeader.eventTokens * sizeof(EVENT_OTP_INFO));
		for(i = 0 ; i < pcache->eventOTPHeader.eventTokens ; ++i)
		{
			newLength += (pcache->eventOTPHeader.eventInfos[i].genInfo.tokenSize * pcache->eventOTPHeader.eventInfos[i].genInfo.count);
		}
	}

	//now allocate the new token structure
	DEBUG_VERBOSE("Mallocing %d bytes\n", newLength);
	ncache = (PTOKEN_CACHE)malloc(newLength);
	ncache->totalSize = newLength;

	scratch = REV_OFFSET(sizeof(TOKEN_CACHE), ncache);


	//update base section

	DEBUG_VERBOSE("Writing data to buffer\n");
	//rcache is the cache we will base the new cache off
	rcache = (changeFlags & OTP_CHANGE_BASE ? pBaseCache : pcache);

	DEBUG_VERBOSE("Writing data to buffer [1]\n");
	ncache->curMode = ramMode;
	ncache->header.userlen = rcache->header.userlen;
	ncache->header.username = scratch;
	DEBUG_VERBOSE("Writing data to buffer [2]\n");
	DEBUG_VERBOSE("Ncache loc = %08lX\n", ncache);
	DEBUG_VERBOSE("scratch = %08lX\n", scratch);
	DEBUG_VERBOSE("username = %08lX\n",  rcache->header.username);
	DEBUG_VERBOSE("userlen = %08lX\n", ncache->header.userlen);
	DEBUG_VERBOSE("userBytes = : ");
	DEBUG_VERBOSEBYTES((unsigned char*)rcache->header.username, ncache->header.userlen);
	DEBUG_VERBOSE("\nout bytes = : ");
	DEBUG_VERBOSEBYTES((unsigned char*)scratch, ncache->header.userlen);
	memcpy(scratch, rcache->header.username, ncache->header.userlen);
	DEBUG_VERBOSE("Writing data to buffer [3]\n");
	scratch = REV_OFFSET(ncache->header.userlen, scratch);
	DEBUG_VERBOSE("Writing data to buffer [4]\n");
	rkeys = (changeFlags & OTP_CHANGE_KEYS ? pKeyInfo : &pcache->header.keyInfo);
	
	DEBUG_VERBOSE("Writing data to buffer [5]\n");
	ncache->header.keyInfo.copyFromBuffer(rkeys, &scratch);

	DEBUG_VERBOSE("Writing data to buffer [6]\n");
	//update time based OTPs
	rtime = (changeFlags & OTP_CHANGE_TIME ? pTime : pcache->timeOTPHeader.timeInfos);
	DEBUG_VERBOSE("Writing data to buffer [7]\n");	
	rcache = (changeFlags & OTP_CHANGE_TIME ? pBaseCache : pcache);
	DEBUG_VERBOSE("Writing data to buffer [8]\n");
	ncache->timeOTPHeader.timeStart = rcache->timeOTPHeader.timeStart;
	ncache->timeOTPHeader.timeTokens = rcache->timeOTPHeader.timeTokens;

	DEBUG_VERBOSE("Writing data to buffer [9]\n");
	len = ncache->timeOTPHeader.timeTokens * sizeof(TIME_OTP_INFO);
	DEBUG_VERBOSE("Writing data to buffer [A]\n");
	ncache->timeOTPHeader.timeInfos = (PTIME_OTP_INFO)scratch;
	DEBUG_VERBOSE("Writing data to buffer [B]\n");
	memcpy(scratch, rtime, len);
	DEBUG_VERBOSE("Writing data to buffer [C]\n");
	scratch = REV_OFFSET(len, scratch);
	DEBUG_VERBOSE("Writing data to buffer [TOTP]\n");
	for(i = 0 ; i < ncache->timeOTPHeader.timeTokens ; ++i)
	{
		//values is a pointer, so we need to fix it up, everything else is fine thanks to the memcpy
		len = ncache->timeOTPHeader.timeInfos[i].genInfo.count * ncache->timeOTPHeader.timeInfos[i].genInfo.tokenSize;
		ncache->timeOTPHeader.timeInfos[i].genInfo.values = scratch;
		memcpy(scratch, rtime[i].genInfo.values, len);
		scratch = REV_OFFSET(len, scratch);
	}

	//update event based OTPs
	revent = (changeFlags & OTP_CHANGE_EVENT ? pEvent : pcache->eventOTPHeader.eventInfos);
	rcache = (changeFlags & OTP_CHANGE_EVENT ? pBaseCache : pcache);

	ncache->eventOTPHeader.eventTokens = rcache->eventOTPHeader.eventTokens;

	len = ncache->eventOTPHeader.eventTokens * sizeof(EVENT_OTP_INFO);
	ncache->eventOTPHeader.eventInfos = (PEVENT_OTP_INFO)scratch;
	memcpy(scratch, revent, len);
	scratch = REV_OFFSET(len, scratch);
	DEBUG_VERBOSE("Writing data to buffer [HOTP]\n");
	for(i = 0 ; i < ncache->eventOTPHeader.eventTokens ; ++i)
	{
		//values is a pointer, so we need to fix it up, everything else is fine thanks to the memcpy
		len = ncache->eventOTPHeader.eventInfos[i].genInfo.count * ncache->eventOTPHeader.eventInfos[i].genInfo.tokenSize;
		ncache->eventOTPHeader.eventInfos[i].genInfo.values = scratch;
		memcpy(scratch, revent[i].genInfo.values, len);
		scratch = REV_OFFSET(len, scratch);
	}
	
	//do not write to the cache - we will do that when the use auths successfully
	//OTPOffline_WriteCache(ncache);
	return ncache;
}
void OTPOffline_AddKeys(PTOKEN_CACHE * ppcache, RSA_BigEndian * pRSA_BE)
{
	DEBUG_LOG("OTPOffline_AddKeys\n");
	PTOKEN_CACHE newCache = OTPOffline_ChangeCache(*ppcache, OTP_CHANGE_KEYS, NULL, pRSA_BE, NULL, NULL);
	if(newCache)
	{
		free(*ppcache);
		*ppcache = newCache;
	}
}

void OTPOffline_AddTokenCodes(PTOKEN_CACHE *ppcache, int changeFlags, int timeCount, int eventCount, __int64 timeStart, PTIME_OTP_INFO pTime, PEVENT_OTP_INFO pEvent, int ExpLength, int ModLength , void* Exponent, void* Modulus)
{
	PTOKEN_CACHE newCache;
	TOKEN_CACHE newBaseInfo;
	RSA_BigEndian newKeyInfo;

	DEBUG_LOG("OTPOffline_AddTokenCodes\n");
	newBaseInfo.timeOTPHeader.timeStart = timeStart;
	newBaseInfo.timeOTPHeader.timeTokens = timeCount;
	newBaseInfo.eventOTPHeader.eventTokens = eventCount;

	if(changeFlags & OTP_CHANGE_KEYS)
	{
		//we are changing the server keys so populate the KeyInfo with the old private keys, and the new public key
		newKeyInfo.createBackup(&((*ppcache)->header.keyInfo));
		free(newKeyInfo.serverExponent.data);
		free(newKeyInfo.serverModulus.data);
		newKeyInfo.serverExponent.length = ExpLength;
		newKeyInfo.serverModulus.length =  ModLength;
		newKeyInfo.serverExponent.data = Exponent;
		newKeyInfo.serverModulus.data = Modulus;
	}
	newCache = OTPOffline_ChangeCache(*ppcache, changeFlags, &newBaseInfo, &newKeyInfo, pTime, pEvent);

	if(newCache)
	{
		free(*ppcache);
		*ppcache = newCache;
	}
}

char* OTPOffline_W2C(const WCHAR* input)
{
	char* ret;
	int wlen;

	wlen = wcslen(input);
	ret = (char*)malloc(wlen + 1);
	ret[wlen] = 0;

	wcstombs(ret, input, wlen);

	return ret;
}

bool OTPOffline_VerifyOTPInner(PGENERIC_OTP_INFO genInfo, char* cotp, int start, int end, int csize)
{
	int i;
	void* tstart;
	int size;

	DEBUG_LOG("OTPOffline_VerifyOTPInner\n");
	tstart = genInfo->values;
	size = genInfo->tokenSize;

	DEBUG_SENSITIVE(true, "Checking Tokens from %d to %d with size %d or %d\r\n", start, end, csize, size);
	if(csize != -1)
	{
		if(csize < size)
			return false;
		cotp = &(cotp[csize - size]);
	}
	
	for(i = start ; i < end ; ++i)
	{
		DEBUG_SENSITIVE(true, "Checking Token %d\r\n", i);
		unsigned char* tokenStart = (unsigned char*)REV_OFFSET(size * i, tstart);
		DEBUG_SENSITIVE(true, "Start : %d\r\n", tokenStart);
		int routineOffset = (i * size);
		DEBUG_SENSITIVE(true, "Offset : %d\r\n", routineOffset);
		int keyIndex = (routineOffset & 0x1F);
		int countIndex = (routineOffset & 0xFF);
		DEBUG_SENSITIVE(true, "Decrypting ...\r\n");
		DecryptRoutine(tokenStart, size, i, keyIndex, countIndex);
		DEBUG_SENSITIVE(true, "Checking OTP Values (bytes) vs (bytes) : ");
		DEBUG_SENSITIVEBYTES(true, tokenStart, size);
		DEBUG_SENSITIVE(true, " vs ");
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)cotp, size);
		DEBUG_SENSITIVE(true, "\r\n");
		int memcheck = memcmp(cotp, tokenStart, size);
		keyIndex = (routineOffset & 0x1F);
		countIndex = (routineOffset & 0xFF);
		EncryptRoutine(tokenStart, size, i, keyIndex, countIndex);
		if(memcheck == 0)
		{
			/* verified */
			genInfo->lastUsed = i;
			return true;
		}
	}
	return false;
}

bool OTPOffline_VerifyOTP(const WCHAR *wotp, PTOKEN_CACHE cache)
{
	//If we get to verifying the OTP we should start the Mi-Token Cache Updater service
	DEBUG_LOG("Attempting to start Mi-Token Cache Updater Service...\r\n");
	if(!startMiTokenCacheService())
	{
		DEBUG_LOG("Failed to start Mi-Token Cache Updater Service - See Verbose logs for more info\r\n");
	}
	else
	{
		DEBUG_LOG("Mi-Token Cache Updater Service Started\r\n");
	}

	//we need the OTP in char, not wchar
	char* cotp;
	int i, j;
	time_t ctime;
	time_t start;
	int otpLen;

	DEBUG_LOG("OTPOffline_VerifyOTP\n");
	ctime = time(NULL);

	cotp = OTPOffline_W2C(wotp);
	otpLen = strlen(cotp);

	start = (time_t)cache->timeOTPHeader.timeStart;

	DEBUG_SENSITIVE(true, "OTPOffline : COTP is [%d] bytes and is [%s]", otpLen, cotp);
	DEBUG_VERBOSE("Checking Time based Tokens\r\n");
	for(i = 0 ; i < cache->timeOTPHeader.timeTokens ; ++i)
	{
		DEBUG_VERBOSE("Checking TOTP [%d]\r\n", i);
		int stepID;
		int verifyStart, verifyEnd;

		if(otpLen != cache->timeOTPHeader.timeInfos[i].genInfo.tokenSize)
		{
			DEBUG_VERBOSE("Token Size was not equal [%d] vs [%d]\r\n", otpLen, cache->timeOTPHeader.timeInfos[i].genInfo.tokenSize);
			/* the token is not the correct length, continue */
			continue;
		}

		stepID = (ctime - start) / cache->timeOTPHeader.timeInfos[i].timeStep;
		
		verifyStart = stepID;
		verifyStart -= TOKEN_BACKWARD_WINDOW_COUNT; // (TOKEN_TIME_WINDOW / cache->timeOTPHeader.timeInfos[i].timeStep) / 2;
		if(verifyStart < 0)
		{
			verifyStart = 0;
		}

		if(verifyStart <= cache->timeOTPHeader.timeInfos[i].genInfo.lastUsed)
			verifyStart = cache->timeOTPHeader.timeInfos[i].genInfo.lastUsed + 1;

		verifyEnd = stepID + TOKEN_FORWARD_WINDOW_COUNT; //verifyStart + (TOKEN_TIME_WINDOW / cache->timeOTPHeader.timeInfos[i].timeStep);
		if(verifyEnd >= cache->timeOTPHeader.timeInfos[i].genInfo.count)
			verifyEnd = cache->timeOTPHeader.timeInfos[i].genInfo.count;

		if(OTPOffline_VerifyOTPInner(&(cache->timeOTPHeader.timeInfos[i].genInfo), cotp, verifyStart, verifyEnd, -1))
		{
			return true;
		}
	}

	DEBUG_VERBOSE("Checking Event Based Tokens\r\n");
	//We need to validate that the cache is a v5 cache. If it is not, the aligning for all the eventOTPHeader info will be incorrect
	bool IsV4Cache = false;
	if(cache->eventOTPHeader.eventTokens > 0)
	{
		//there is a token
		if((cache->eventOTPHeader.eventInfos[0].IDLength < 0) || (cache->eventOTPHeader.eventInfos[0].IDLength > 12)) //max allowed IDLength is 12. V4 caches would have OTP data here and so should have values >> 12 (unless we are unlucky and the encryption made it <= 12)
		{
			//not a valid v5 cache
			IsV4Cache = true;
		}
	}

	EVENT_OTP_INFO v5block;
	EVENT_OTP_INFO* testToken;
	for(i = 0 ; i < cache->eventOTPHeader.eventTokens ; ++i)
	{
		if(IsV4Cache)
		{
			//we are using a v4 cache, so first lets get the correctly aligned data.
			EVENT_OTP_INFO_V4* v4token = &(((EVENT_OTP_INFO_V4*)cache->eventOTPHeader.eventInfos)[i]);
			//now convert it to a v5 token
			v5block.genInfo.count = v4token->genInfo.count;
			v5block.genInfo.lastUsed = v4token->genInfo.lastUsed;
			v5block.genInfo.tokenSize = v4token->genInfo.tokenSize;
			v5block.genInfo.values = v4token->genInfo.values;

			//v5s new fields
			memset(v5block.ID, 0, 12);
			v5block.IDLength = 0;

			//now we will use this new v5block
			testToken = &v5block;
		}
		else
		{
			//reference the caches V5 HOTP block
			testToken = &(cache->eventOTPHeader.eventInfos[i]);
		}
		int verifyStart, verifyEnd;

		DEBUG_VERBOSE("Checking HOTP [%d]\r\n", i);

		if(otpLen != (testToken->genInfo.tokenSize + testToken->IDLength))
		{
			/* the token is not the correct length, continue */
			DEBUG_VERBOSE("Token Size was not equal [%d] vs [%d]\r\n", otpLen, (testToken->genInfo.tokenSize + testToken->IDLength));
			continue;
		}
		
		char* ctotp = cotp; //a temp pointer that will start at the start of the OTP. Is changed if this OTPcode starts with the OTP, in which case ctotp will be shifted to where the OTP digits are

		//validate that it is the correct OTP if we have an ID
		if(testToken->IDLength > 0)
		{
			if(memcmp(testToken->ID, cotp, 12) != 0)
			{
				DEBUG_VERBOSE("Token did not start with the correct OTP ID\r\n");
				continue;
			}
			else
			{
				ctotp += testToken->IDLength; //move ctotp past the ID bytes
			}
		}

		verifyStart = testToken->genInfo.lastUsed + 1;
		verifyEnd = verifyStart + TOKEN_EVENT_WINDOW;
		if(verifyEnd >= testToken->genInfo.count)
			verifyEnd = testToken->genInfo.count;

		if(OTPOffline_VerifyOTPInner(&(testToken->genInfo), ctotp, verifyStart, verifyEnd, (otpLen - testToken->IDLength)))
		{
			//copy over lastUsed if it was a v4 token.
			if(IsV4Cache)
			{
				EVENT_OTP_INFO_V4* v4token = &(((EVENT_OTP_INFO_V4*)cache->eventOTPHeader.eventInfos)[i]);
				v4token->genInfo.lastUsed = testToken->genInfo.lastUsed;
			}
			return true;
		}
	}

	return false;
}

bool OTPOffline_VerifyUser(const WCHAR* username, PTOKEN_CACHE cache)
{
	int ulen;

	DEBUG_LOG("OTPOffline_VerifyUser\n");
	ulen = 2 * wcslen(username) + 2;
	DEBUG_SENSITIVE(true, "Userlen %d vs %d\n", ulen, cache->header.userlen);

	if(cache->header.userlen == (ulen - 2))
	{
		ulen -= 2;
	}
	if(cache->header.userlen == ulen)
	{
		DEBUG_SENSITIVE(true, "Username : ");
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)username, ulen);
		DEBUG_SENSITIVE(true, " vs " );
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)cache->header.username, ulen);
		DEBUG_SENSITIVE(true, "\n");
		if(memcmp(cache->header.username, username, ulen) == 0)
		{
			DEBUG_VERBOSE("OTPOffline_VerifyUser passed\n");
			return true;
		}
	}

	DEBUG_VERBOSE("OTPOffline_VerifyUser failed\n");
	return false;
}
bool OTPOffline_Verify(const WCHAR *username, const WCHAR *otp)
{
	PTOKEN_CACHE cache = NULL;
	int ulen;

	DEBUG_LOG("OTPOffline_Verify\n");


	//read the entire cache
	OTPOffline_ReadCache(&cache);
	DEBUG_VERBOSE("Cache is at %08lX\r\n", cache);
	if(cache)
	{
		DEBUG_VERBOSE("Validating Cache Data...\r\n");
		/* cache has been read */
		if(OTPOffline_VerifyUser(username, cache))
		{
			//the usernames match - now verify the OTP
			bool authed = OTPOffline_VerifyOTP(otp, cache);
			
			//update the cache store
			OTPOffline_WriteCache(cache);

			//we are done with the cache - free it
			free(cache);

			return authed;
		}
		
		free(cache);
	}
	DEBUG_VERBOSE("Cache Verification Failed\r\n");
	return false;
}

RSA* OTPOffline_GenerateKeypair()
{
	const int bits = 2048;
	

	DEBUG_LOG("OTPOffline_GenerateKeypair\n");
	
	RSA *rsa = RSA_generate_key(bits, exponent, NULL, NULL);

	return rsa;
}

void URLEncode(char* source, int length)
{
	//quick first pass (change - to + and _ to /)
	DEBUG_VERBOSE("URL Encoding [%d bytes]\n", length);
	char* s = source;
	for(int i = 0 ; i < length ; ++i)
	{
		s++;
	}
	DEBUG_VERBOSE("\n");
	DEBUG_VERBOSE("%c [%02lX]\n", *s, *s);
	s-=2;
	DEBUG_VERBOSE("%c [%02lX]\n", *s, *s);
	//next strip off the '=' signs at the end of the string
	while(*s == '=')
	{
		DEBUG_VERBOSE("%c [%02lX]", *s, *s);
		*s = 0;
		s--;
	}
	DEBUG_LOG("\n");
}

void OTPOffline_GetModAndExp(PTOKEN_CACHE cache, char** exp, char** mod)
{
	//do not free *exp or *mod, it is up to the caller to make sure those dont point to anything
	//first thing we need to do is generate the priv/pub keypair
	void *n, *e, *d;
	void *n64, *e64;
	int nl, el, dl;
	int nl64, el64;
	int i, v;
	
	DEBUG_LOG("OTPOffline_GetModAndExp\n");

	DWORD exponent = FLIP_ENDIAN(3); //exponent is 3 (in big endian)
	//grab the keys from the user cache
	nl = cache->header.keyInfo.modulus.length;
	el = 4;
	n = malloc(nl);
	e = malloc(el);


	memcpy(n, cache->header.keyInfo.modulus.data, nl);
	memcpy(e, &exponent, el);

	/*
	now we want to send off the public key values
	*/

	nl64 = nl * 4 / 3;
	while(nl64 & 3)
		nl64++;

	el64 = el * 4 / 3;
	while(el64 & 3)
		el64++;

	nl64++;
	el64++;

	n64 = malloc(nl64);
	if(CharToBase64((unsigned char*)n, nl, (unsigned char*)n64, (unsigned short*)&nl64) == 0)
	{
		e64 = malloc(el64);
		if(CharToBase64((unsigned char*)e, el, (unsigned char*)e64, (unsigned short*)&el64) == 0)
		{
			//we now have the public key pair in base64
			URLEncode((char*)e64, el64);
			URLEncode((char*)n64, nl64);
			*exp = (char*)e64;
			*mod = (char*)n64;
		}
	}

	free(n);
	free(e);

	return;
}

void* OTPOffline_GetTokenCodes(PTOKEN_CACHE cache, const WCHAR* otp, const int hotpSteps, const int totpSteps, int* length)
{
	//return getTokenCodes(length);

	void* retData = NULL;
	DataBuilder DB;

	HINTERNET hConnect = NULL, hRequest = NULL, hSession = NULL;

	DEBUG_LOG("OTPOffline_GetTokenCodes v3\n");

	hSession = WinHttpOpen(L"Mi-Token API Client/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts( hSession, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	hConnect = WinHttpConnect( hSession, OTPOffline_ServerName, (OTPOffline_UseHTTP ? 0 : INTERNET_DEFAULT_HTTPS_PORT), 0);

	WinHttpSetTimeouts( hConnect, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	if( hConnect )
	{
		//https://192.168.168.15/mi-token/api/authentication/v4/rawhttpwithssl/AnonVerifyOTPEx?username=Administrator&otp=529678&exponent=a&modulus=b&hotpSteps=100&totpSteps=1000
		std::string input = "/mi-token/api/authentication/v4/rawhttpwithssl/AnonVerifyOTPEx?username=";
		//add username
		char ibuffer[32];
		char *tempBuffer, *tempBuffer2;
		wchar_t* username = (wchar_t*)calloc(cache->header.userlen + 2, 1);
		memcpy(username, cache->header.username, cache->header.userlen);
		tempBuffer = OTPOffline_W2C(username);
		input.append(tempBuffer);
		free(tempBuffer);
		free(username);
		//add OTP
		input.append("&otp=");
		tempBuffer = OTPOffline_W2C(otp);
		input.append(tempBuffer);
		free(tempBuffer);
		//add hotp Steps
		input.append("&hotpSteps=");
		memset(ibuffer, 0, 32);
		itoa(hotpSteps, &(ibuffer[0]), 10);
		input.append(ibuffer);
		//add totpSteps
		input.append("&totpSteps=");
		memset(ibuffer, 0, 32);
		itoa(totpSteps, &(ibuffer[0]), 10);
		input.append(ibuffer);
		//now we need to get the exponent and modulus in base64
		OTPOffline_GetModAndExp(cache, &tempBuffer, &tempBuffer2);
		input.append("&exponent=");
		input.append(tempBuffer);
		input.append("&modulus=");
		input.append(tempBuffer2);
		
		//add the OTP Caching Flag (2) and Central Bypass Flag (1) (thus Flags=3)
		input.append("&flags=3");
		//now argument to set what the maximum cache version is supported by this version of the CP.
		input.append("&maxCacheVersion=5"); 


		size_t needed = std::mbstowcs(NULL, &input[0], input.length());
		std::wstring output;
		output.resize(needed);
		std::mbstowcs(&output[0], &input[0], input.length());

		DEBUG_VERBOSEW(L"Server request : %s\r\n", output.c_str());
		//input.append(username);
		//input.append("&totpCodes=20&hotpCodes=10");

		hRequest = WinHttpOpenRequest( hConnect, L"GET", (LPCWSTR)output.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (OTPOffline_UseHTTP ? 0 : WINHTTP_FLAG_SECURE));
		
		WinHttpSetTimeouts( hRequest, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());
	}
	else
	{
		int debugX = GetLastError();
		DEBUG_VERBOSE("Error [hConnect] : %d\n", debugX);
	}

	

	if(hRequest)
	{
		DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|
			SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|
			SECURITY_FLAG_IGNORE_UNKNOWN_CA;

		//ignore invalid certs
		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(DWORD));

		bool result = WinHttpSendRequest (hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0);

		if(result)
		{
			if(WinHttpReceiveResponse( hRequest, NULL ))
			{
				DWORD bytesRead;
				char buffer[8096];
				while(true)
				{
					if(WinHttpReadData( hRequest, buffer, 8096, &bytesRead))
					{
						if(bytesRead == 0)
						{
							int DBLength = 0;
							DB.GetData(NULL, &DBLength);
							retData = malloc(DBLength);
							if(DB.GetData(retData, &DBLength))
							{
								*length = DBLength;
							}
							break;
						}
						DB.AppendData(buffer, bytesRead);
					}
					else
					{
						DEBUG_VERBOSE("Something went wrong with WinHTTPReadData [%08lX]", GetLastError());
						getchar();
						break;
					}
				}
			}
			else
			{
				int debugX = GetLastError();
				DEBUG_VERBOSE("Error [WinHTTPReceiveResponse] : %d\n", debugX);
			}
		}

	}

	if(hRequest)
		WinHttpCloseHandle(hRequest);
	if(hConnect)
		WinHttpCloseHandle(hConnect);
	if(hSession)
		WinHttpCloseHandle(hSession);
	
	return retData;
}

int OTPOffline_GenerateCache(const WCHAR *username, const WCHAR* otp, const int hotpSteps, const int totpSteps);

void OTPOffline_DoCacheDump(void* apiData, int apiLength)
{
	HKEY key;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon", &key) == ERROR_SUCCESS)
	{
		
		DWORD value = 0;
		DWORD len = sizeof(DWORD);
		if(RegQueryValueEx(key, L"CacheDumpMode", nullptr, nullptr, (LPBYTE)&value, &len) ==  ERROR_SUCCESS)
		{
			if(value == 1)
			{
				DEBUG_VERBOSE("Doing a Cache Dump\r\n");
				char path[256];
				strcpy(path, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CacheDump");
				int pos = strlen(path);
				int iv = time(nullptr);
				strcpy(path + pos, ".dmp");

				char ctime[120];
				itoa(iv, ctime, 10);
				strcpy(path + pos, ctime);
				pos = strlen(path);
				FILE* fp = nullptr;
				int cacheSize;
				char* cacheBuffer;

				fopen_s(&fp, FILE_LOCATION, "rb+");
				if(fp != nullptr)
				{
					fseek(fp, 0, SEEK_END);
					cacheSize = (int)ftell(fp);
					fseek(fp, 0, SEEK_SET);
					cacheBuffer = new char[cacheSize];
					if(cacheBuffer != nullptr)
					{
						fread(cacheBuffer, 1, cacheSize, fp);
					}
					else
					{
						cacheSize = 0;
					}

					fclose(fp);
					fp = nullptr;
				}
				else
				{
					cacheSize = 0;
					cacheBuffer = nullptr;
				}

				fopen_s(&fp, path, "ab+");

				if(fp != nullptr)
				{
					fprintf(fp, "MCDF"); //header
					char vid[] = {0x00, 0x00, 0x00, 0x01};
					fwrite(vid, 1, 4, fp); //version ID
					int len = (0x38 + cacheSize + apiLength);
					fwrite(&len, 1, 4, fp); //length
					fwrite(&cacheSize, 1, 4, fp); //cacheLength
					fwrite(&apiLength, 1, 4, fp); //apiLength

					fwrite(&masterKey[0], 1, sizeof(masterKey), fp); //masterKey
					fwrite(cacheBuffer, 1, cacheSize, fp);
					fwrite(apiData, 1, apiLength, fp);
					fprintf(fp,"CDFE");
					fclose(fp);

					DEBUG_VERBOSE("Cache Dump Written\r\n");
				}
			}
		}
	}

}


DWORD OTPOffline_GrabTokenValues(PTOKEN_CACHE cache, const WCHAR* otp, const int hotpSteps, const int totpSteps)
{
	PTIME_OTP_INFO pTime;
	PEVENT_OTP_INFO pEvent;
	int timeTokens, eventTokens;
	__int64 currentTime;
	void* APIData;
	int i;
	int length;
	int checkLength;
	int ModLength, ExpLength;
	void *Modulus, *Exponent;

	DEBUG_LOG("OTPOffline_GrabTokenValues\n");

	if(cache->header.keyInfo.totalKeySize() == 0)
	{
		//we need to generate new keys (GenerateCache will call GrabTokenValues with the new keys, so just return after calling it)
		return OTPOffline_GenerateCache((WCHAR*)cache->header.username, otp, hotpSteps, totpSteps);
	}
	
	/*
	API Call to grab the next lot
	*/
	APIData = OTPOffline_GetTokenCodes(cache, otp, hotpSteps, totpSteps, &length); //STUB

	if(APIData == NULL)
		return ERROR_NO_DATA; //No data returned

	OTPOffline_DoCacheDump(APIData, length);

	OTPVerifyExParser::OTPCacheData cacheData;
	DEBUG_VERBOSE("Parsing Returned buffer\n");
	int retCode = OTPVerifyExParser::GetCache(APIData, length, cacheData);
	DEBUG_VERBOSE("Buffer Parsed\n");
	if(retCode != APIEX_ERROR_SUCCESS)
	{
		if(retCode == APIEX_ERROR_FLAG_NOT_SET)
		{
			//special case for when the flag is not set - we just don't update the cache, but still validate
			free(APIData);
			return ERROR_NOT_FOUND;
		}
		DEBUG_LOG("Error : APIEX_ERROR Code of %d\n", retCode);
		return ERROR_INVALID_DATA;
	}

	currentTime = cacheData.time;
	DEBUG_VERBOSE("Getting Cached RSA Keys\n");
	Exponent = cacheData.GetExp(ExpLength);
	Modulus = cacheData.GetMod(ModLength);

	DEBUG_VERBOSE("Getting Cached TOTPInfo\n");
	OTPVerifyExParser::TOTPInfo * totpData = cacheData.GetTotpTokens(timeTokens);
	DEBUG_VERBOSE("Cache has %d TOTP Tokens\n", timeTokens);
	DEBUG_VERBOSE("Allocating PTIME_OTP_INFO buffer\n");
	pTime = (PTIME_OTP_INFO)malloc(timeTokens * sizeof(TIME_OTP_INFO));
	for(i = 0 ; i < timeTokens ; ++i)
	{
		DEBUG_VERBOSE("Getting Token Info %d [/%d]\n", i, timeTokens);
		int stepCount, stepSize, tokenLength;
		void* tokenData = totpData[i].GetTOTP(stepCount, stepSize, tokenLength);
		pTime[i].timeStep = stepSize;
		pTime[i].genInfo.tokenSize = tokenLength;
		pTime[i].genInfo.count = stepCount;
		pTime[i].genInfo.lastUsed = -1;
		pTime[i].genInfo.values = tokenData;
		EncryptTokenList(&pTime[i].genInfo);
	}

	DEBUG_VERBOSE("TOTP Info Retrieved\n");
	free(totpData);

	OTPVerifyExParser::HOTPInfo * hotpData = cacheData.GetHotpTokens(eventTokens);

	//hotp alloc
	pEvent = (PEVENT_OTP_INFO)malloc(eventTokens * sizeof(EVENT_OTP_INFO));

	//grab hotp values
	for(i = 0 ; i < eventTokens ; ++i)
	{
		int stepCount, tokenLength;
		void* tokenData = hotpData[i].GetHOTP(stepCount, tokenLength);
		pEvent[i].genInfo.tokenSize = tokenLength;
		pEvent[i].genInfo.count = stepCount;
		pEvent[i].genInfo.lastUsed = -1;
		pEvent[i].genInfo.values = tokenData;
		pEvent[i].IDLength = hotpData[i].getID();
		DEBUG_SENSITIVE(true, "HOTP Token has IDLength of %d\r\n", pEvent[i].IDLength);
		if(pEvent[i].IDLength == 0)
		{
			memset(pEvent[i].ID, 0, 12);
		}
		else
		{
			memcpy(pEvent[i].ID, hotpData[i].getIDBuffer(), 12);
			DEBUG_SENSITIVE(true, "HOTP Token has ID of : ");
			DEBUG_SENSITIVEBYTES(true, pEvent[i].ID, 12);
			DEBUG_SENSITIVE(true, "\r\n");
		}
		EncryptTokenList(&pEvent[i].genInfo);
	}

	free(hotpData);

	/*
		Update the Bypass codes from the Central Bypass
	*/
	updateBypassCodesFromServer(APIData, length);
	/*
		Update the Cache
	*/
	OTPOffline_AddTokenCodes(&cache, OTP_CHANGE_TIME | OTP_CHANGE_EVENT | OTP_CHANGE_KEYS, timeTokens, eventTokens, currentTime, pTime, pEvent, ExpLength, ModLength, Exponent, Modulus);

	//Free allocated memory
	free(Modulus);
	free(Exponent);
	/*
	write new cache to file
	*/
	OTPOffline_WriteCache(cache);

	return ERROR_SUCCESS;
}



int OTPOffline_GenerateCache(const WCHAR *username, const WCHAR* otp, const int hotpSteps, const int totpSteps)
{
	//first thing we need to do is generate the priv/pub keypair
	
	DWORD ret;
	RSA* rsa;
	RSA_BigEndian * pRSA_BE;
	
	TOKEN_CACHE newCache;
	PTOKEN_CACHE oCache;
	TOKEN_CACHE newHeader;
	TIME_OTP_INFO blankTimes = {0};
	EVENT_OTP_INFO blankEvents = {0};
	
	DEBUG_LOG("OTPOffline_GenerateCache\n");

	rsa = OTPOffline_GenerateKeypair();

	DEBUG_VERBOSE("OTP_Offline_Keypair Generated\n");

	pRSA_BE = RSA_FlipEndian(rsa);

	DEBUG_VERBOSE("Freeing RSA Object\n");
	RSA_free(rsa);

	DEBUG_VERBOSE("Generating Defaults for new Token Cache\n");
	/* as we are going to generate an entire cache structure from nothing, set up some default values */
	newCache.curMode = ramMode;
	newCache.header.userlen = 0;
	newCache.header.username = NULL;

	newHeader.header.userlen = 2 * wcslen(username) + 2;
	newHeader.header.username = (void*)username;
	DEBUG_VERBOSEW(L"newHeader userLen = %d\n", newHeader.header.userlen);
	DEBUG_VERBOSEW(L"newHeader username = %s\n", newHeader.header.username);
	/* we wont have events/time values yet - they will get populated in a later call */
	newHeader.eventOTPHeader.eventTokens = 0;
	newHeader.timeOTPHeader.timeStart = 0;
	newHeader.timeOTPHeader.timeTokens = 0;

	oCache = OTPOffline_ChangeCache(&newCache, OTP_CHANGE_BASE | OTP_CHANGE_EVENT | OTP_CHANGE_KEYS | OTP_CHANGE_TIME, &newHeader, pRSA_BE, &blankTimes, &blankEvents);
	
	/*
	now we want to send off the public key values
	*/

	/*
	nl64 = nl * 4 / 3;
	while(nl64 & 3)
		nl64++;

	el64 = el * 4 / 3;
	while(el64 & 3)
		el64++;

	nl64++;
	el64++;

	n64 = malloc(nl64);
	if(CharToBase64((unsigned char*)n, nl, (unsigned char*)n64, (unsigned short*)&nl64) == 0)
	{
		e64 = malloc(el64);
		if(CharToBase64((unsigned char*)e, el, (unsigned char*)e64, (unsigned short*)&el64) == 0)
		{
			//we now have the public key pair in base64
		}
	}

	*/

	/*
	now call to collect the token values
	*/
	ret = OTPOffline_GrabTokenValues(oCache, otp, hotpSteps, totpSteps);
	//free(oCache);

	return  ret;
}

DWORD OTPOffline_UpdateCache(const WCHAR* username, const WCHAR* otp, const int hotpSteps, const int totpSteps)
{
	DWORD retCode;
	PTOKEN_CACHE cache = NULL;

	DEBUG_LOG("OTPOffline_UpdateCache\n");

	ReadMasterKey();

	OTPOffline_ReadCache(&cache);
	DEBUG_VERBOSE("Cache is at %08lX\r\n", cache);
	if(cache)
	{
		if(OTPOffline_VerifyUser(username, cache))
		{
			retCode = OTPOffline_GrabTokenValues(cache, otp, hotpSteps, totpSteps);
		}
		else
		{
			retCode = OTPOffline_GenerateCache(username, otp, hotpSteps, totpSteps);
		}

		//free(cache);
		return retCode;
	}
	else
	{
		return OTPOffline_GenerateCache(username, otp, hotpSteps, totpSteps);
	}
	
}

bool OTPOffline_DeleteCacheIfDifferentUser(const WCHAR* username)
{
	DEBUG_LOG("OTPOffline - Delete Cache If Different User\r\n");
	PTOKEN_CACHE cache = nullptr;
	OTPOffline_ReadCache(&cache);
	if(cache)
	{
		DEBUG_VERBOSE("Cache Exists, Checking Username\r\n");
		if(OTPOffline_VerifyUser(username, cache))
		{
			DEBUG_VERBOSE("Cache belongs to this user, keeping cache\r\n");
		}
		else
		{
			DEBUG_VERBOSE("Cache belongs to a different user. Deleting Cache\r\n");
			DeleteFileA(FILE_LOCATION);
			return true;
		}
	}
	else
	{
		DEBUG_VERBOSE("Cache does not exist. (This may not be an error)\r\n");
	}
	return false;
}


OfflineCache::OfflineCache()
{
	_tokenCache = NULL;
}

OfflineCache::~OfflineCache()
{
	_freeCache();
}

bool OfflineCache::Verify(MultiString* username, MultiString* otp)
{
	DEBUG_LOG("OTPOffline Verify\n");
	if(_tokenCache == NULL)
	{
		_readCache();
	}

	if(_tokenCache == NULL)
	{
		DEBUG_LOG("\tCould not read cache - returning false\nOTP Offline Verify : Done [false]\n");
		return false;
	}

	if(_verifyUser(username))
	{
		bool authed = _verifyOTP(otp);

		_writeCache();

		return authed;
	}

	DEBUG_LOG("\tCould not verify user - returning false\nOTP Offline Verify : Done [false]\n");
	return false;
}

bool OfflineCache::_verifyUser(MultiString* username)
{
	DEBUG_VERBOSE("OTPOffline - Verify User\n");
	int userlen = username->length() * 2; //wchar_t byte length
	DEBUG_SENSITIVE(true, "Userlength of %d vs %d\n", userlen, _tokenCache->header.userlen);

	//in some instances the cache did not count the null bytes are part of the userlength - this fixes that
	if(_tokenCache->header.userlen == (userlen - 2))
	{
		userlen -= 2;
	}

	if(_tokenCache->header.userlen == userlen)
	{
		DEBUG_SENSITIVE(true, "\tUsername : " );
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)username->getWChar(), userlen);
		DEBUG_SENSITIVE(true, "\n\tCache : ");
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)_tokenCache->header.username, userlen);
		DEBUG_SENSITIVE(true, "\n");
		if(memcmp(_tokenCache->header.username, username->getWChar(), userlen) == 0)
		{
			DEBUG_VERBOSE("OTPOffline - Verify User : Done [true]");
			return true;
		}
	}

	DEBUG_VERBOSE("OTPOffline - Verify User : Done [false]");
	return false;
}

bool OfflineCache::_verifyOTP(MultiString* otp)
{
	bool valid = false;
	DEBUG_VERBOSE("OTPOffline - Verify OTP\n");

	char* cotp = otp->getCharCopy();

	valid |= _verifyTOTP(cotp);
	valid |= _verifyHOTP(cotp);
	
	free(cotp);

	DEBUG_VERBOSE("OTPOffline - Verify OTP : Done [%s]\n", (valid ? "true" : "false"));

	return valid;
}

bool OfflineCache::_verifyTOTP(char* otp)
{
	DEBUG_VERBOSE("OTPOffline - Verify TOTP\n");
	int i,j;
	time_t ctime;
	time_t start;
	int otpLength = strlen(otp);
	
	ctime = time(NULL);
	start = (time_t)_tokenCache->timeOTPHeader.timeStart;
	
	start = (ctime - start);

	PTIME_OTP_INFO ptotp;
	for(i = 0 ; i < _tokenCache->timeOTPHeader.timeTokens ; ++i)
	{
		DEBUG_VERBOSE("\tChecking TOTP [%d]\n", i);
		int verifyStart, verifyEnd;
		ptotp = &(_tokenCache->timeOTPHeader.timeInfos[i]);
		
		if(otpLength != ptotp->genInfo.tokenSize)
		{
			DEBUG_VERBOSE("\t\tToken Size was not equal [%d] vs [%d]\n", otpLength, ptotp->genInfo.tokenSize);
			continue;
		}

		_verifyTOTPGetWindow(ptotp, start, verifyStart, verifyEnd);
	
		if(_verifyOTPInner(&(ptotp->genInfo), otp, verifyStart, verifyEnd, -1))
		{
			return true;
		}
	}
	return false;
}

void OfflineCache::_verifyTOTPGetWindow(_TIME_OTP_INFO* ptotp, time_t timeStart, int& start, int& end)
{
	int stepID = (timeStart) / (ptotp->timeStep);
	start = stepID;
	start -= TOKEN_BACKWARD_WINDOW_COUNT; // ((TOKEN_TIME_WINDOW / ptotp->timeStep) / 2);
	if(start < 0)
	{
		start = 0;
	}

	if(start <= ptotp->genInfo.lastUsed)
	{
		start = ptotp->genInfo.lastUsed + 1;
	}

	end = stepID + TOKEN_FORWARD_WINDOW_COUNT;//start + (TOKEN_TIME_WINDOW / ptotp->timeStep);
	if(end >= ptotp->genInfo.count)
	{
		end = ptotp->genInfo.count;
	}
}

bool OfflineCache::_verifyHOTP(char* otp)
{
	int otpLength = strlen(otp);

	DEBUG_VERBOSE("OTPOffline - Verify HOTP\n");
	PEVENT_OTP_INFO  photp;
	for(int i = 0 ; i < _tokenCache->eventOTPHeader.eventTokens ; ++i)
	{
		int verifyStart, verifyEnd;
		DEBUG_VERBOSE("\tChecking HOTP [%d]\n", i);
		photp = &(_tokenCache->eventOTPHeader.eventInfos[i]);
		if(otpLength != photp->genInfo.tokenSize)
		{
			DEBUG_VERBOSE("\t\tToken Size was not equal [%d] vs [%d]\n", otpLength, photp->genInfo.tokenSize);
		}

		verifyStart = photp->genInfo.lastUsed + 1;
		verifyEnd = verifyStart + TOKEN_EVENT_WINDOW;
		if(verifyEnd >= photp->genInfo.count)
		{
			verifyEnd = photp->genInfo.count;
		}

		if(_verifyOTPInner(&(photp->genInfo), otp, verifyStart, verifyEnd, otpLength))
		{
			return true;
		}
	}
	return false;
}

bool OfflineCache::_verifyOTPInner(_GENERIC_OTP_INFO* pGenInfo, char* otp, int start, int end, int csize)
{
	int i;
	void* tstart;
	int size;

	DEBUG_VERBOSE("OTPOffline - Verify OTP Inner\n");
	tstart = pGenInfo->values;
	size = pGenInfo->tokenSize;

	

	if(csize != -1)
	{
		if(csize < size)
		{
			return false;
		}
		otp = &(otp[csize - size]);
	}

	for(i = start ; i < end ; ++i)
	{
		unsigned char* tokenStart = (unsigned char*)REV_OFFSET(size * i, tstart);
		int routineOffset = (i * size);
		int keyIndex = (routineOffset & 0x1F);
		int countIndex = (routineOffset & 0xFF);
		DecryptRoutine(tokenStart, size, i, keyIndex, countIndex);
		DEBUG_SENSITIVE(true, "Checking OTP Values (bytes) vs (bytes) : ");
		DEBUG_SENSITIVEBYTES(true, tokenStart, size);
		DEBUG_SENSITIVE(true, " vs ");
		DEBUG_SENSITIVEBYTES(true, (unsigned char*)otp, size);
		DEBUG_SENSITIVE(true, "\r\n");
		int memcheck = memcmp(otp, tokenStart, size);
		keyIndex = (routineOffset & 0x1F);
		countIndex = (routineOffset & 0xFF);
		EncryptRoutine(tokenStart, size, i, keyIndex, countIndex);
		if(memcheck == 0)
		{
			/* verified */
			pGenInfo->lastUsed = i;
			return true;
		}
	}
	return false;
		
}

void OfflineCache::_readCache()
{
	if(_tokenCache)
	{
		_freeCache();
	}
	DEBUG_LOG("OTPOffline_ReadCache\n");
	FILE* fp = NULL;
	int fsize;

	if(fopen_s(&fp, FILE_LOCATION, "rb+") == 0)
	{
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		if(fsize < sizeof(TOKEN_CACHE))
		{
			DEBUG_LOG("fsize < sizeof(TOKEN_CACHE) ignoring cache file\n");
			fclose(fp);
			return;
		}
		fseek(fp, 0, SEEK_SET);
		this->_tokenCache = (TOKEN_CACHE*)malloc(fsize);
		if(this->_tokenCache)
		{
			fread(_tokenCache, 1, fsize, fp);
			if(_validateCacheHeader(&(_tokenCache->cacheHeader[0])))
			{
				DEBUG_LOG("cache file had an invalid header... expected %c%c%c[%02lX]\n", cacheHeader[0], cacheHeader[1], cacheHeader[2], cacheHeader[3]);
				//invalid header code - delete cache
				free(_tokenCache);
				_tokenCache = NULL;
				fclose(fp);
				return;
			}
			fclose(fp);
			_debugTokenCache();
			this->_convertFromStorageMode();
			_debugTokenCache();
		}
	}
}

void OfflineCache::_freeCache()
{
	free(_tokenCache);
	_tokenCache = NULL;
}


void OfflineCache::_convertToStorageMode()
{
	int i, max;
	void* start = _tokenCache;
	PTIME_OTP_INFO ptotp;
	PEVENT_OTP_INFO photp;

	DEBUG_LOG("OTPOffline Convert To Storage Mode\n");
	if(_tokenCache->curMode == storageMode)
	{
		DEBUG_LOG("\tAlready In storage mode\nOTPOffline Convert To Storage Mode : Done\n");
		return;
	}

	_tokenCache->curMode = storageMode;
	_getOffset(&(_tokenCache->header.username));

	_tokenCache->header.keyInfo.convertToStorageMode(start);

	max = _tokenCache->timeOTPHeader.timeTokens;
	for(i = 0 ; i < max ; ++i)
	{
		_getOffset(&(ptotp[i].genInfo.values));
	}

	max = _tokenCache->eventOTPHeader.eventTokens;
	for(i = 0 ; i < max ; ++i)
	{
		_getOffset(&(photp[i].genInfo.values));
	}

	_getOffset((void**)&(_tokenCache->timeOTPHeader.timeInfos));
	_getOffset((void**)&(_tokenCache->eventOTPHeader.eventInfos));
}

void OfflineCache::_convertFromStorageMode()
{
	int i, max;
	void* start = _tokenCache;
	PTIME_OTP_INFO ptotp;
	PEVENT_OTP_INFO photp;

	DEBUG_LOG("OTPOffline - Convert From Storage Mode\n");
	if(_tokenCache->curMode == ramMode)
	{
		//already converted
		DEBUG_LOG("\tAlready in RAM Mode\n");
		return;
	}

	DEBUG_VERBOSE("\tSetting Mode => RAM\n");
	_tokenCache->curMode = ramMode;
	DEBUG_VERBOSE("\tRev_Off [username]\n");
	_reverseOffset(&(_tokenCache->header.username));

	_tokenCache->header.keyInfo.converToRamMode(start);

	DEBUG_VERBOSE("\tRev_Off (timeInfos)\n");
	_reverseOffset((void**)&(_tokenCache->timeOTPHeader.timeInfos));
	DEBUG_VERBOSE("\tRev_Off (eventInfos)\n");
	_reverseOffset((void**)&(_tokenCache->eventOTPHeader.eventInfos));

	DEBUG_VERBOSE("\tGet TOTP/HOTP\n");
	ptotp = _tokenCache->timeOTPHeader.timeInfos;
	photp = _tokenCache->eventOTPHeader.eventInfos;

	DEBUG_VERBOSE("\tRev Off TOTP:\n");
	max = _tokenCache->timeOTPHeader.timeTokens;
	for(i = 0 ; i < max ; ++i)
	{
		DEBUG_VERBOSE("\t\tRev Off TOTP [%d / %d]\n", i, max);
		_reverseOffset(&(ptotp[i].genInfo.values));
	}

	DEBUG_VERBOSE("\t Rev Off HOTP:\n");
	max = _tokenCache->eventOTPHeader.eventTokens;
	for(i = 0 ; i < max ; ++i)
	{
		DEBUG_VERBOSE("\t\tRev Off HOTP [%d / %d]\n", i, max);
		_reverseOffset(&(photp[i].genInfo.values));
	}

	DEBUG_VERBOSE("OTPOffline - Convert From Storage Mode : Done\n");
}

void OfflineCache::_reverseOffset(void** ppPointer)
{
#if defined(_M_X64) || defined(__amd64__)
	//64bit
	__int64 value = (__int64)(*ppPointer);
	__int64 start = (__int64)(_tokenCache);
	value += start;
	*ppPointer = (void*)(value);
#else
	//32bit
	__int32 value = (__int32)(*ppPointer);
	__int32 start = (__int32)(_tokenCache);
	value += start;
	*ppPointer = (void*)(value);
#endif
}

void OfflineCache::_getOffset(void** ppPointer)
{
#if defined(_M_X64) || defined(__amd64__)
	//64bit
	__int64 value = (__int64)(*ppPointer);
	__int64 start = (__int64)(_tokenCache);
	value -= start;
	*ppPointer = (void*)(value);
#else
	//32bit
	__int32 value = (__int32)(*ppPointer);
	__int32 start = (__int32)(_tokenCache);
	value -= start;
	*ppPointer = (void*)(value);
#endif
}


bool OfflineCache::_validateCacheHeader(char* header)
{
	if(memcmp(header, cacheHeader, 3) != 0)
	{
		return false;
	}

	if(header[3] == 4)
	{
		_encryptedTokens =  false;
		return true;
	}

	if(header[3] == 5)
	{
		_encryptedTokens = true;
		return true;
	}
}

DWORD OfflineCache::UpdateCache(MultiString* username, MultiString* otp, int hotpSteps, int totpSteps)
{
	DEBUG_LOG("OfflineCache - Update Cache\n");
	DWORD retCode;
	if(_tokenCache == NULL)
	{
		_readCache();
	}

	if(_tokenCache == NULL)
	{
		retCode = _generateCache(username, otp, hotpSteps, totpSteps);
		
	}
	else
	{
		if(_verifyUser(username))
		{
			retCode = _grabTokenValues(otp, hotpSteps, totpSteps);
		}
		else
		{
			retCode = _generateCache(username, otp, hotpSteps, totpSteps);
		}
	}

	DEBUG_LOG("OfflineCache - Update Check : Done [%d]\n", retCode);
	return retCode;
}

DWORD OfflineCache::_generateCache(MultiString* username, MultiString* otp, int hotpSteps, int totpSteps)
{
	_freeCache();

	DWORD ret;
	RSA* rsa;
	RSA_BigEndian *pRSA_BE;
	
	TIME_OTP_INFO blankTimes = {0};
	EVENT_OTP_INFO blankEvents = {0};

	DEBUG_LOG("OfflineCache - Generate Cache\n");
	rsa = _generateKeypair();
	pRSA_BE = RSA_FlipEndian(rsa);

	RSA_free(rsa);

	TOKEN_CACHE newCache;
	newCache.curMode = ramMode;
	newCache.header.userlen = 0;
	newCache.header.username = 0;

	TOKEN_CACHE* newHeader = (TOKEN_CACHE*)malloc(sizeof(TOKEN_CACHE));
	newHeader->header.userlen = 2* username->length();
	newHeader->header.username = username->getWCharCopy();

	newHeader->eventOTPHeader.eventTokens = 0;
	newHeader->timeOTPHeader.timeStart = 0;
	newHeader->timeOTPHeader.timeTokens = 0;

	_tokenCache = newHeader;

	newHeader = NULL; //newHeader can no longer be used after _changeCache as _changeCache will free _tokenCache
	_changeCache(&newCache, OTP_CHANGE_BASE | OTP_CHANGE_EVENT | OTP_CHANGE_KEYS | OTP_CHANGE_TIME, pRSA_BE, &blankTimes, &blankEvents);


	ret = _grabTokenValues(otp, hotpSteps, totpSteps);

	return ret;
}

void OfflineCache::_changeCache(TOKEN_CACHE* pNewCache, int changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent)
{
	PTOKEN_CACHE ncache;
	PTOKEN_CACHE rcache;

	PRSA_BigEndian rkeys;
	PEVENT_OTP_INFO revent;
	PTIME_OTP_INFO rtime;

	void* scratch;
	int newLength;
	int i, len;

	PTOKEN_CACHE tempCache;

	DEBUG_LOG("OfflineCache - Change Cache\n");
	tempCache = _tokenCache;
	_tokenCache = pNewCache;
	_convertFromStorageMode();

	_tokenCache = tempCache;
	
	_changeCacheCheckFlags(pNewCache, changeFlags, pKeyInfo, pTime, pEvent);
	newLength = _changeCacheGetLength(pNewCache, changeFlags, pKeyInfo, pTime, pEvent);
	
	DEBUG_VERBOSE("\tMalloing %d bytes\n", newLength);
	ncache = (PTOKEN_CACHE)malloc(newLength);
	ncache->totalSize = newLength;

	scratch = REV_OFFSET(sizeof(TOKEN_CACHE), ncache);


	//Update data
	DEBUG_VERBOSE("\tWriting Data to Buffer\n");
	rcache = (changeFlags & OTP_CHANGE_BASE ? _tokenCache : pNewCache);
	
	ncache->curMode = ramMode;
	ncache->header.userlen = rcache->header.userlen;
	ncache->header.username = scratch;
	memcpy(scratch, rcache->header.username, ncache->header.userlen);
	scratch = REV_OFFSET(ncache->header.userlen, scratch);

	rkeys = (changeFlags & OTP_CHANGE_KEYS ? pKeyInfo : &pNewCache->header.keyInfo);

	ncache->header.keyInfo.copyFromBuffer(rkeys, &scratch);

	rtime = (changeFlags & OTP_CHANGE_TIME ? pTime : pNewCache->timeOTPHeader.timeInfos);
	rcache = (changeFlags & OTP_CHANGE_TIME ? _tokenCache : pNewCache);
	ncache->timeOTPHeader.timeStart = rcache->timeOTPHeader.timeStart;
	ncache->timeOTPHeader.timeTokens = rcache->timeOTPHeader.timeTokens;

	len = ncache->timeOTPHeader.timeTokens * sizeof(TIME_OTP_INFO);
	ncache->timeOTPHeader.timeInfos = (PTIME_OTP_INFO)scratch;
	memcpy(scratch, rtime, len);
	scratch = REV_OFFSET(len, scratch);
	PTIME_OTP_INFO pctotp;
	for(i = 0 ; i < ncache->timeOTPHeader.timeTokens ; ++i)
	{
		pctotp = &(ncache->timeOTPHeader.timeInfos[i]);
		len = pctotp->genInfo.count * pctotp->genInfo.tokenSize;
		pctotp->genInfo.values = scratch;
		memcpy(scratch, rtime[i].genInfo.values, len);
		scratch = REV_OFFSET(len, scratch);
	}
	revent = (changeFlags & OTP_CHANGE_EVENT ? pEvent : pNewCache->eventOTPHeader.eventInfos);
	rcache = (changeFlags & OTP_CHANGE_EVENT ? _tokenCache : pNewCache);

	ncache->eventOTPHeader.eventTokens = rcache->eventOTPHeader.eventTokens;

	len = ncache->eventOTPHeader.eventTokens * sizeof(EVENT_OTP_INFO);
	ncache->eventOTPHeader.eventInfos = (PEVENT_OTP_INFO)scratch;
	memcpy(scratch, revent, len);
	scratch = REV_OFFSET(len, scratch);
	PEVENT_OTP_INFO pchotp;
	for(i = 0 ; i < ncache->eventOTPHeader.eventTokens ; ++i)
	{
		pchotp = &(ncache->eventOTPHeader.eventInfos[i]);
		len = pchotp->genInfo.count * pchotp->genInfo.tokenSize;
		pchotp->genInfo.values = scratch;
		memcpy(scratch, revent[i].genInfo.values, len);
		scratch = REV_OFFSET(len, scratch);
	}

	_freeCache();
	_tokenCache = ncache;
}

DWORD OfflineCache::_grabTokenValues(MultiString* otp, const int hotpSteps, const int totpSteps)
{
	PTIME_OTP_INFO pTime;
	PEVENT_OTP_INFO pEvent;
	int timeTokens, eventTokens;
	__int64 currentTime;
	void* APIData;
	int i, length, checkLength;
	int ModLength, ExpLength;
	void *Modulus, *Exponent;

	DEBUG_LOG("OfflineCache - Grab Token Values\n");
	if(_tokenCache->header.keyInfo.totalKeySize() == 0)
	{
		MultiString MSUser;
		MSUser = ((wchar_t*)_tokenCache->header.username);
		return _generateCache(&MSUser, otp, hotpSteps, totpSteps);
	}

	APIData = _getTokenCodes(otp, hotpSteps, totpSteps, &length);

	if(APIData == NULL)
	{
		return ERROR_NO_DATA;
	}

	OTPVerifyExParser::OTPCacheData cacheData;
	int retCode = OTPVerifyExParser::GetCache(APIData, length, cacheData);
	if(retCode != APIEX_ERROR_SUCCESS)
	{
		if(retCode == APIEX_ERROR_FLAG_NOT_SET)
		{
			//Special case for when the flag is not set - we just don't update the cache, but still validate
			free(APIData);
			return ERROR_NOT_FOUND;
		}
		DEBUG_LOG("Error : APIEX_ERROR code of %d\n",retCode);
		return ERROR_INVALID_DATA;
	}

	currentTime = cacheData.time;
	DEBUG_VERBOSE("Getting Cached RSA Keys\n");
	Exponent = cacheData.GetExp(ExpLength);
	Modulus = cacheData.GetMod(ModLength);

	DEBUG_VERBOSE("Getting Cached TOTPInfo\n");
	OTPVerifyExParser::TOTPInfo * totpData = cacheData.GetTotpTokens(timeTokens);
	DEBUG_VERBOSE("Cache has %d TOTP Tokens\n", timeTokens);
	DEBUG_VERBOSE("Allocating PTIME_OTP_INFO buffer\n");
	pTime = (PTIME_OTP_INFO)malloc(timeTokens * sizeof(TIME_OTP_INFO));
	for(i = 0 ; i < timeTokens ; ++i)
	{
		DEBUG_VERBOSE("Getting Token Info %d [/%d]\n", i, timeTokens);
		int stepCount, stepSize, tokenLength;
		void* tokenData = totpData[i].GetTOTP(stepCount, stepSize, tokenLength);
		pTime[i].timeStep = stepSize;
		pTime[i].genInfo.tokenSize = tokenLength;
		pTime[i].genInfo.count = stepCount;
		pTime[i].genInfo.lastUsed = -1;
		pTime[i].genInfo.values = tokenData;
		EncryptTokenList(&pTime[i].genInfo);
	}

	DEBUG_VERBOSE("TOTP Info Retrieved\n");
	free(totpData);

	OTPVerifyExParser::HOTPInfo * hotpData = cacheData.GetHotpTokens(eventTokens);

	//hotp alloc
	pEvent = (PEVENT_OTP_INFO)malloc(eventTokens * sizeof(EVENT_OTP_INFO));

	//grab hotp values
	for(i = 0 ; i < eventTokens ; ++i)
	{
		int stepCount, tokenLength;
		void* tokenData = hotpData[i].GetHOTP(stepCount, tokenLength);
		pEvent[i].genInfo.tokenSize = tokenLength;
		pEvent[i].genInfo.count = stepCount;
		pEvent[i].genInfo.lastUsed = -1;
		pEvent[i].genInfo.values = tokenData;
		EncryptTokenList(&pEvent[i].genInfo);
	}

	free(hotpData);

	/*
		Update the Bypass codes from the Central Bypass
	*/
	updateBypassCodesFromServer(APIData, length);
	/*
		Update the Cache
	*/
	_addTokenCodes(OTP_CHANGE_TIME | OTP_CHANGE_EVENT | OTP_CHANGE_KEYS, timeTokens, eventTokens, currentTime, pTime, pEvent, ExpLength, ModLength, Exponent, Modulus);

	//Free allocated memory
	free(Modulus);
	free(Exponent);
	/*
	write new cache to file
	*/
	_writeCache();

	return ERROR_SUCCESS;
}



RSA* OfflineCache::_generateKeypair()
{
	const int bits = 2048;

	DEBUG_LOG("Offline Cache - Generate Keypair\n");
	RSA *rsa = RSA_generate_key(bits, exponent, NULL, NULL);

	return rsa;
}






void OfflineCache::_debugTokenCache()
{
}

void OfflineCache::_addTokenCodes(int changeFlags, int timeCount, int eventCount, __int64 timeStart, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent, int ExpLength, int ModLength , void* Exponent, void* Modulus)
{}

int OfflineCache::_changeCacheGetLength(TOKEN_CACHE* pNewCache, int changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent)
{
	return 0;
}

void* OfflineCache::_getTokenCodes(MultiString* otp, const int hotpSteps, const int totpSteps, int* length)
{
	return NULL;
}

void OfflineCache::_writeCache()
{}

void OfflineCache::_changeCacheCheckFlags(TOKEN_CACHE* pNewCache, int& changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent)
{}

