/*
OTPOffline.cpp

Allows offline OTP verification using cached OTPs
*/

#pragma once

#include <iostream>
#include <Windows.h>
#include <winhttp.h>


#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "base64.h"

#include "MultiString.h"
/*
Verifies and updates a user's cache if the Authentication is successful

Return Codes:
	Uses Windows standard ones (eg, ERROR_SUCCESS, ERROR_NOT_ENOUGH_MEMORY etc)
*/


DWORD OTPOffline_UpdateCache(const WCHAR *username, const WCHAR* otp, const int hotpSteps, const int totpSteps);

bool OTPOffline_Verify(const WCHAR *username, const WCHAR *otp);
bool OTPOffline_DeleteCacheIfDifferentUser(const WCHAR *username);

extern wchar_t* OTPOffline_ServerName;
extern bool OTPOffline_UseHTTP;

struct _RSA_BigEndian;
struct _GENERIC_OTP_INFO;


enum tokenCacheMode
{
	storageMode,
	ramMode
};

struct RSA_BigEndian_Inner
{
	void* data;
	int length;
	void setFromRSABigNum(BIGNUM* source);
	void convertToRamMode(void* start);
	void convertToStorageMode(void* start);
	void DebugKeyInner(char* name, bool inRam);
	void copyFromBuffer(RSA_BigEndian_Inner* source, void** pScratch);
	void createBackup(RSA_BigEndian_Inner * source);
};

typedef struct _RSA_BigEndian
{
	RSA_BigEndian_Inner exponent, modulus, D, DP, DQ, inverseQ, P, Q, serverExponent, serverModulus;
	void converToRamMode(void* start);
	void convertToStorageMode(void* start);
	void debugPrint(bool inRAM);
	int totalKeySize();
	void copyFromBuffer(_RSA_BigEndian* source, void** pScratch);
	void createBackup(_RSA_BigEndian *source);
}RSA_BigEndian, *PRSA_BigEndian;

struct _TIME_OTP_INFO;
struct _EVENT_OTP_INFO;

#pragma pack(push)
#pragma pack(4)
struct TOKEN_CACHE
{
	char cacheHeader[4];
	int pointerSize;
	tokenCacheMode curMode;
	int totalSize;
	struct
	{
		int userlen;
		void* username;
		RSA_BigEndian keyInfo;
	} header;
	struct
	{
		int timeTokens;
		int _padding;
		__int64 timeStart;
		_TIME_OTP_INFO* timeInfos;
	} timeOTPHeader;
	struct
	{
		int eventTokens;
		_EVENT_OTP_INFO* eventInfos;
	} eventOTPHeader;
};
#pragma pack(pop)

class OfflineCache
{
public:
	OfflineCache();
	//We will use MultiStrings as some functions may require wchar_t* while others may require char*
	bool Verify(MultiString* username, MultiString* otp);
	DWORD UpdateCache(MultiString* username, MultiString* otp, int hotpSteps, int totpSteps);
	void UpdateServerName(char* newName);
	void UpdateServerName(wchar_t* newName);
	~OfflineCache();

private:
	OfflineCache(OfflineCache& other);
	TOKEN_CACHE* _tokenCache;
	void _convertToStorageMode();
	void _convertFromStorageMode();
	void _writeCache();
	void _readCache();
	void _addKeys(_RSA_BigEndian* keys);
	bool _verifyOTPInner(_GENERIC_OTP_INFO* otpInfo, char* otp, int start, int end, int csize);
	bool _verifyOTP(MultiString* otp);
	bool _verifyTOTP(char* otp);
	void _verifyTOTPGetWindow(_TIME_OTP_INFO* ptotp, time_t timeStart, int& start, int& end);
	bool _verifyHOTP(char* otp);
	bool _verifyUser(MultiString* username);
	RSA* _generateKeypair();
	void _urlEncode(char* source, int length);
	void _getModAndExp(char** exp, char** mod);
	void* _getTokenCodes(MultiString* otp, const int hotpSteps, const int totpSteps, int* length);
	DWORD _grabTokenValues(MultiString* otp, const int hoptSteps, const int totpSteps);
	DWORD _generateCache(MultiString* username, MultiString* otp, const int hotpSteps, const int totpSteps);
	void _updateCache(MultiString* username, MultiString* otp, const int hotpSteps, const int totpSteps);
	void _debugTokenCache();
	bool _validateCacheHeader(char* header);
	MultiString _serverName;
	bool _encryptedTokens;
	void _freeCache();
	void _reverseOffset(void** ppPointer);
	void _getOffset(void** ppPointer);
	void _changeCache(TOKEN_CACHE* pNewCache, int changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent);
	void _changeCacheCheckFlags(TOKEN_CACHE* pNewCache, int& changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent);
	int _changeCacheGetLength(TOKEN_CACHE* pNewCache, int changeFlags, PRSA_BigEndian pKeyInfo, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent);
	void _addTokenCodes(int changeFlags, int timeCount, int eventCount, __int64 timeStart, _TIME_OTP_INFO* pTime, _EVENT_OTP_INFO* pEvent, int ExpLength, int ModLength , void* Exponent, void* Modulus);
};

typedef TOKEN_CACHE* PTOKEN_CACHE;