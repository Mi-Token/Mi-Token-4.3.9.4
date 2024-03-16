#pragma once

//#include <string>
#include <windows.h>



namespace OTPVerifyExParser
{
	struct TOTPInfo
	{
		TOTPInfo();
		~TOTPInfo();
		void free();

		void* GetTOTP(int& steps, int& stepSize, int& length);
		void* SetTOTP(void* data, int steps);
	protected:
		int _steps;
		int _stepSize;
		int _tokenLength;
		void* _data;
	};

	struct HOTPInfo
	{
		HOTPInfo();
		~HOTPInfo();
		void free();

		int getID();
		void* getIDBuffer();

		void* GetHOTP(int& steps, int& length);
		void* SetHOTP(void* data, int steps, bool HOTPHasID);
		//std::string GetPublicIdentity() { return _publicIdentity? std::string().assign(*_publicIdentity,12): std::string(); }
	protected:
		int _steps;
		int _tokenLength;
		int _idLength;
		unsigned char _id[12];
		//char (* _publicIdentity)[12];
		void* _data;
	};

	struct OTPCacheData
	{
	public:
		OTPCacheData();
		~OTPCacheData();
		void free();

		void* GetExp(int& length);
		void* GetMod(int& length);
		TOTPInfo* GetTotpTokens(int& count);
		HOTPInfo* GetHotpTokens(int& count);
		void GetSteps(int& totpSteps, int& hotpSteps);
		__int64 time;

		void SetRSA(void* Exp, void* Mod, int ExpLen, int ModLen);
		void* SetTOTP(void* data, int count, int steps);
		void* SetHOTP(void* data, int count, int steps, bool HOTPHasID);
	protected:
		void *_RSAExp, *_RSAMod;
		TOTPInfo *_TOTPTokens;
		HOTPInfo *_HOTPTokens;
		int _TOTPCount, _HOTPCount; //how many tokens of this type
		int _TOTPSteps, _HOTPSteps; //how many token codes per token
		int _RSAExpLen, _RSAModLen;
		void _freeRSA();
		void _freeTOTP();
		void _freeHOTP();
	};

	struct BypassSidData
	{
	public:
		int SIDLength;
		void* SID;
		int BypassCount;
		void* BypassData;

		void* InitWithData(void* currentRead, void* endOfStream);
		int BypassLength();
		void CopyData(BypassSidData* reference);
		void free();
	protected:
		bool allocedData;
	};

	struct OTPBypassData
	{
	public:
		OTPBypassData();
		~OTPBypassData();
		void free();

		int GetSIDBlockCount();
		BypassSidData* GetSIDBlock(int index);
		void SetSIDBlocks(BypassSidData* SIDBlocks, int SIDBlockCount);
		BYTE* GenerateHash(int& length);
	protected:
		BypassSidData * _SIDBlocks;
		int _SIDBlockCount;
	};


	struct DynamicPasswordData
	{
		DynamicPasswordData();
		~DynamicPasswordData();
		void free();

		int passwordLength();
		void copyPassword(void* dest);

		void setPassword(void* src, int length);
	protected:
		void* _password;
		int _passLength;
	};

	bool ParseValidData(void* APIData, int length);

	int GetCache(void* APIData, int length, OTPCacheData& cache);

	int GetBypass(void* APIData, int length, OTPBypassData& bypass);

	int GetDynamicPassword(void* APIData, int length, DynamicPasswordData& dynamicPassword);
};

//Everything worked
#define APIEX_ERROR_SUCCESS (0)

//The struct is a version that cannot be deconstructed with this version of the CP
#define APIEX_ERROR_INVALID_VERSION (-1)

//The structs internal length does not match the data returned by the API call
#define APIEX_ERROR_INVALID_LENGTH (-2)

//The section offset is past the end of the stream
#define APIEX_ERROR_INVALID_SECTION_OFFSET (-3)

//The structs bypass offset is past the end of the struct
#define APIEX_ERROR_INVALID_BYPASS_OFFSET APIEX_ERROR_INVALID_SECTION_OFFSET

//The structs cache offset is past the end of the struct
#define APIEX_ERROR_INVALID_CACHE_OFFSET APIEX_ERROR_INVALID_SECTION_OFFSET

//Unknown error inside a Get(Section)
#define APIEX_ERROR_SECTION_UNKNOWN (-5)

//Unknown error inside GetCache
#define APIEX_ERROR_GETCACHE_UNKNOWN APIEX_ERROR_SECTION_UNKNOWN

//Unknown error inside GetBypass
#define APIEX_ERROR_GETBYPASS_UNKNOWN APIEX_ERROR_SECTION_UNKNOWN

//The sections internal length is past the end of the data stream
#define APIEX_ERROR_INVALID_SECTION_LENGTH (-7)
//The Caches internal length is past the end of the data stream
#define APIEX_ERROR_INVALID_CACHE_LENGTH APIEX_ERROR_INVALID_SECTION_LENGTH

//The sections header bytes are invalid
#define APIEX_ERROR_INVALID_SECTION_HEADER (-8)
//The Caches Header bytes are invalid
#define APIEX_ERROR_INVALID_CACHE_HEADER APIEX_ERROR_INVALID_SECTION_HEADER

//The flag for the requested operation is not set
#define APIEX_ERROR_FLAG_NOT_SET (-9)

//code that isn't yet written
#define APIEX_ERROR_NOT_IMPLIMENTED (-10)


//Flags for AnonVerifyOTPEx
#define OTPEX_FLAG_CENTRAL_BYPASS (0x01)
#define OTPEX_FLAG_OTP_CACHING (0x02)
#define OTPEX_FLAG_DYNAMIC_PASSWORD (0x04)

