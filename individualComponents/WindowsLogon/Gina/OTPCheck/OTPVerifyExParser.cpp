/*
	Unpacks an OTPVerifyEx packet and returns either the OTPCache info 
		or the OTPBypass info depending on which function was called
*/

#include "OTPVerifyExParser.h"
#include <string.h>
#include <stdlib.h>
#include "DebugLogging.h"
#include "oath.h"

#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { ::free(p); (p) = NULL; }
#endif

using namespace OTPVerifyExParser;

//Expected API Version
int expectedVersion = 4;

int version4ID = 4;
int version5ID = 5;



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
//Structure with more info about the APIData

enum HOTP_VersionID
{
	HOTP_NoID,
	HOTP_WithID,
};

struct APIDataEx
{
public:
	void* startAddress;
	int length;
	
	bool valid;
	int _flags;
	void* CacheStart;
	int CachePos;
	void* BypassStart;
	int BypassPos;
	void* DynamicPassStart;
	int DynamicPassPos;
	int lastError;

	int versionID;
	HOTP_VersionID hotp_version;

	APIDataEx()
	{
		startAddress = 0;
		length = 0;
		valid = false;
		CacheStart = BypassStart = 0;
		lastError = APIEX_ERROR_SUCCESS;
	}
};

struct Version4Header
{
public:
	int Version;
	int Size;
	int Flags;
	int CacheOffset;
	int BypassOffset;
	int DynamicPassOffset;
};

APIDataEx ParseHeader(void* APIData, int length)
{
	DEBUG_VERBOSE("\n----------\n  DEBUG API RESULT  \n----------\n");
	DEBUG_VERBOSEBYTES((unsigned char*)APIData, length);
	DEBUG_VERBOSE("\n----------\nEND DEBUG API RESULT\n----------\n");
	
	APIDataEx ret;
	
	DEBUG_VERBOSE("Parse Header\n");
	Version4Header header;

	int lengthToCopy = (length < sizeof(header) ? length : sizeof(header));
	if(length < 12) //we MUST have at least 12 bytes (Version/Size/Flags) otherwise there is an issue
	{
		DEBUG_VERBOSE("Length < minimum 12 byte length\n");
		//invalid length - should be at least sizeof(header) bytes
		ret.lastError = APIEX_ERROR_INVALID_LENGTH;
	}

	DEBUG_VERBOSE("Copying header across [%d bytes]\n", lengthToCopy);
	//copy over the header
	memcpy(&header, APIData, lengthToCopy);
	
	if(header.Version == version4ID)
	{
		ret.versionID = 4;
		ret.hotp_version = HOTP_NoID;
	}
	else if(header.Version == version5ID)
	{
		ret.versionID = 5;
		ret.hotp_version = HOTP_WithID;
	}
	else
	{
		DEBUG_VERBOSE("Invalid Version, Got %d\n", header.Version);
		//Invalid Version ID
		ret.lastError = APIEX_ERROR_INVALID_VERSION;
		return ret;
	}
	DEBUG_VERBOSE("Valid header version\n");

	if(header.Size != length)
	{
		DEBUG_VERBOSE("Invalid Length, Got %d expected %d\n", header.Size, length);
		//Invalid Length
		ret.lastError = APIEX_ERROR_INVALID_LENGTH;
		return ret;
	}

	DEBUG_VERBOSE("Embedded size is the expected size\n");

	ret._flags = header.Flags;
	if(ret._flags & OTPEX_FLAG_CENTRAL_BYPASS)
	{
		DEBUG_VERBOSE("Flag CENTRAL_BYPASS Set\n");
		if(header.BypassOffset >= length)
		{
			DEBUG_VERBOSE("CENTRAL_BYPASS location beyond end of stream\n");
			ret.lastError = APIEX_ERROR_INVALID_BYPASS_OFFSET;
			return ret;
		}

		DEBUG_VERBOSE("CENTRAL_BYPASS in valid location\n");
		ret.BypassStart = REV_OFFSET(header.BypassOffset, APIData);
		ret.BypassPos = header.BypassOffset;
	}

	if(ret._flags & OTPEX_FLAG_OTP_CACHING)
	{
		DEBUG_VERBOSE("Flag OTP CACHING Set\n");
		if(header.CacheOffset >= length)
		{
			DEBUG_VERBOSE("OTP CACHING location beyond end of stream\n");
			ret.lastError = APIEX_ERROR_INVALID_CACHE_OFFSET;
			return ret;
		}
		ret.CachePos = header.CacheOffset;
		ret.CacheStart = REV_OFFSET(header.CacheOffset, APIData);
	}

	if(ret._flags & OTPEX_FLAG_DYNAMIC_PASSWORD)
	{
		DEBUG_VERBOSE("Flag DYNAMIC PASSWORD Set\n");
		if(header.DynamicPassOffset >= length)
		{
			DEBUG_VERBOSE("DYNAMIC PASSWORD location beyond end of stream\n");
			ret.lastError = APIEX_ERROR_INVALID_SECTION_OFFSET;
			return ret;
		}
		ret.DynamicPassPos = header.DynamicPassOffset;
		ret.DynamicPassStart = REV_OFFSET(header.DynamicPassOffset, APIData);
	}

	DEBUG_VERBOSE("Setting return struct\n");
	ret.startAddress = APIData;
	ret.length = length;
	ret.valid = true;
	
	DEBUG_VERBOSE("Returning valid struct\n");
	return ret;
}


bool OTPVerifyExParser::ParseValidData(void* APIData, int length)
{
	APIDataEx value = ParseHeader(APIData, length);
	return value.valid;
}

//v4 cache header - unfortunently split into 2 sections in v4 due to where the RSA keys are written
//part1 has the header/section size/time/and RSA key info
//part2 has the totp/hotp steps, count and tokens
struct CacheHeader
{
public:
	int HeaderBytes;
	int sectionSize;
	__int64 time64;
	int RSAExpLen;
	int RSAModLen;
};

//v4 cache header 2nd part
struct CacheHeader2
{
public:
	int TotpCount;
	int TotpSteps;
	int HotpCount;
	int HotpSteps;
};

struct BypassHeader
{
public:
	int HeaderBytes;
	int sectionSize;
	int UniqueSIDBlocks;
};

int OTPVerifyExParser::GetCache(void* APIData, int length, OTPCacheData& retCache)
{
	DEBUG_VERBOSE("Get Cache Start\n");
	APIDataEx data = ParseHeader(APIData, length);
	if(!data.valid)
	{
		DEBUG_VERBOSE("Returned data was not valid\n");
		if(data.lastError != APIEX_ERROR_SUCCESS)
			return data.lastError;
		//data states it is invalid, however no error was logged - therefore -> unknown error
		return APIEX_ERROR_GETCACHE_UNKNOWN;
	}

	DEBUG_VERBOSE("Flags are %08lX\n", data._flags);

	if((data._flags & OTPEX_FLAG_OTP_CACHING) == 0)
	{
		DEBUG_VERBOSE("OTP CACHING flag is not set\n");
		//The OTP CACHING flag is not set.
		return APIEX_ERROR_FLAG_NOT_SET;
	}

	DEBUG_VERBOSE("Grabbing Token Cache [+%08lX]\n", data.CacheStart);
	void* cache = data.CacheStart;
	int cacheLen = length - data.CachePos;
	if(cacheLen < sizeof(CacheHeader))
	{
		DEBUG_VERBOSE("CacheLen is not large enough to fit the header\n");
		return APIEX_ERROR_INVALID_CACHE_LENGTH;
	}

	DEBUG_VERBOSE("Copying over cache header\n");
	//Copy the header over
	CacheHeader header;
	memcpy(&header, cache, sizeof(header));

	if((header.HeaderBytes != 0x4350544F) &&
		(header.HeaderBytes != 0x4450544F))//OTPC in little endian for V4, and OTPD in little endian for V5.
	{
		DEBUG_VERBOSE("Invalid Cache Header [%08lX]\n", header.HeaderBytes);
		DEBUG_VERBOSE("Cache Header:\n");
		DEBUG_VERBOSEBYTES((unsigned char*)&header, sizeof(header));
		DEBUG_VERBOSE("\nEnd Cache Header\n");
		//invalid header
		return APIEX_ERROR_INVALID_CACHE_HEADER;
	}

	if(header.sectionSize > cacheLen)
	{
		DEBUG_VERBOSE("Cache runs beyond the end of the stream\n");
		//the section size runs past the end of the stream
		return APIEX_ERROR_INVALID_CACHE_LENGTH;
	}

	DEBUG_VERBOSE("Getting Time and RSA Data\n");
	retCache.time = header.time64;

	void* RSAKeyLocation = REV_OFFSET(sizeof(header), cache);
	void* RSAExp, *RSAMod;

	RSAExp = RSAKeyLocation;
	RSAMod = REV_OFFSET(header.RSAExpLen, RSAExp);
	retCache.SetRSA(RSAExp, RSAMod, header.RSAExpLen, header.RSAModLen);

	//move the cache to the 2nd header location (RSAMod + RSAModLen)
	DEBUG_VERBOSE("Getting Cache Header (p2)\n");
	cache = REV_OFFSET(header.RSAModLen, RSAMod);

	CacheHeader2 header2;
	
	DEBUG_VERBOSE("Copying over header data\n");
	memcpy(&header2, cache, sizeof(header2));
	void* TOTP, *HOTP;

	DEBUG_VERBOSE("Getting TOTP Data\n");
	TOTP = REV_OFFSET(sizeof(header2), cache);
	HOTP = retCache.SetTOTP(TOTP, header2.TotpCount, header2.TotpSteps);
	DEBUG_VERBOSE("Getting HOTP Data\n");
	retCache.SetHOTP(HOTP, header2.HotpCount, header2.HotpSteps, (data.versionID >= 5));

	DEBUG_VERBOSE("Getting cache is completed\n");
	//and we are done
	return APIEX_ERROR_SUCCESS;
}

int OTPVerifyExParser::GetBypass(void* APIData, int length, OTPBypassData& bypass)
{
	DEBUG_VERBOSE("Get Bypass\n");
	APIDataEx data = ParseHeader(APIData, length);
	if(!data.valid)
	{
		DEBUG_VERBOSE("Returned data was not valid\n");
		if(data.lastError != APIEX_ERROR_SUCCESS)
			return data.lastError;
		//data states it is invalid, however no error was logged - therefore -> unknown error
		return APIEX_ERROR_GETBYPASS_UNKNOWN;
	}

	DEBUG_VERBOSE("Flags are %08lX\n", data._flags);

	if((data._flags & OTPEX_FLAG_CENTRAL_BYPASS) == 0)
	{
		DEBUG_VERBOSE("CENTRAL BYPASS flag is not set\n");
		//The OTP CACHING flag is not set.
		return APIEX_ERROR_FLAG_NOT_SET;
	}

	DEBUG_VERBOSE("Grabbing Bypass [+%08lX]\n", data.BypassStart);
	void* bypassData = data.BypassStart;
	int bypassLen = length - data.BypassPos;
	if(bypassLen < sizeof(BypassHeader))
	{
		DEBUG_VERBOSE("CacheLen is not large enough to fit the header\n");
		return APIEX_ERROR_INVALID_SECTION_LENGTH;
	}

	DEBUG_VERBOSE("Copying over cache header\n");
	//Copy the header over
	BypassHeader header;
	memcpy(&header, bypassData, sizeof(header));

	if(header.HeaderBytes != 0x53555042) // BPUS in little endian
	{
		DEBUG_VERBOSE("Invalid Bypass Header [%08lX]\n", header.HeaderBytes);
		DEBUG_VERBOSE("Bypass Header:\n");
		DEBUG_VERBOSEBYTES((unsigned char*)&header, sizeof(header));
		DEBUG_VERBOSE("\nEnd Bypass Header\n");
		//invalid header
		return APIEX_ERROR_INVALID_SECTION_HEADER;
	}

	if(header.sectionSize > bypassLen)
	{
		DEBUG_VERBOSE("Bypass runs beyond the end of the stream\n");
		//the section size runs past the end of the stream
		return APIEX_ERROR_INVALID_SECTION_LENGTH;
	}

	DEBUG_VERBOSE("Allocating Memory for the %d Unique SID Blocks\n", header.UniqueSIDBlocks);
	//Allocate memory for our SID Headers
	BypassSidData * pBSD = (BypassSidData*)calloc(header.UniqueSIDBlocks, sizeof(BypassSidData));
	void* readFrom = REV_OFFSET(bypassData, sizeof(BypassHeader));
	void* EOS = REV_OFFSET(APIData, length);
	for(int i = 0 ; i < header.UniqueSIDBlocks ; ++i)
	{
		DEBUG_VERBOSE("Initializing BSD[%d] with Data\n", i);
		readFrom = pBSD[i].InitWithData(readFrom, EOS);
		if(readFrom == NULL)
		{
			DEBUG_VERBOSE("Failed to Init BSD with Data - ran past EOS\n");
			return APIEX_ERROR_INVALID_SECTION_LENGTH;
		}
	}

	bypass.SetSIDBlocks(pBSD, header.UniqueSIDBlocks);

	return APIEX_ERROR_SUCCESS;
}



struct DynamicPassHeader
{
	int HeaderBytes;
	int PasswordLength;
};

int OTPVerifyExParser::GetDynamicPassword(void* APIData, int length, DynamicPasswordData& dynamicPassword)
{
	DEBUG_VERBOSE("Get DynamicPassword Start\n");
	APIDataEx data = ParseHeader(APIData, length);
	if(!data.valid)
	{
		DEBUG_VERBOSE("Returned data was not valid\n");
		if(data.lastError != APIEX_ERROR_SUCCESS)
			return data.lastError;
		//data states it is invalid, however no error was logged - therefore -> unknown error
		return APIEX_ERROR_SECTION_UNKNOWN;
	}

	DEBUG_VERBOSE("Flags are %08lX\n", data._flags);

	if((data._flags & OTPEX_FLAG_DYNAMIC_PASSWORD) == 0)
	{
		DEBUG_VERBOSE("Dynamic Password flag is not set\n");
		//The OTP CACHING flag is not set.
		return APIEX_ERROR_FLAG_NOT_SET;
	}

	DEBUG_VERBOSE("Grabbing Dynamic Password [+%08lX]\n", data.DynamicPassStart);
	void* cache = data.DynamicPassStart;
	int cacheLen = length - data.DynamicPassPos;
	if(cacheLen < sizeof(DynamicPassHeader))
	{
		DEBUG_VERBOSE("CacheLen is not large enough to fit the header\n");
		return APIEX_ERROR_INVALID_CACHE_LENGTH;
	}

	DEBUG_VERBOSE("Copying over cache header\n");
	//Copy the header over
	DynamicPassHeader header;
	memcpy(&header, cache, sizeof(header));

	DEBUG_VERBOSE("Header Copied\n");
	if(header.HeaderBytes != 0x504E5944) //DYNP in little endian
	{
		DEBUG_VERBOSE("Invalid Section Header [%08lX]\n", header.HeaderBytes);
		DEBUG_VERBOSE("Section Header:\n");
		DEBUG_VERBOSEBYTES((unsigned char*)&header, sizeof(header));
		DEBUG_VERBOSE("\nEnd Cache Header\n");
		//invalid header
		return APIEX_ERROR_INVALID_SECTION_HEADER;
	}

	if((header.PasswordLength + 8) > cacheLen)
	{
		DEBUG_VERBOSE("Dynamic Password Section runs beyond the end of the stream\n");
		//the section size runs past the end of the stream
		return APIEX_ERROR_INVALID_SECTION_LENGTH;
	}

	DEBUG_VERBOSE("Dynamic Password [bin] : ");
	DEBUG_SENSITIVEBYTES(true, (unsigned char*)cache + 8, header.PasswordLength);
	DEBUG_VERBOSE("\n");
	dynamicPassword.setPassword((char*)cache + 8, header.PasswordLength);
	//and we are done
	return APIEX_ERROR_SUCCESS;
}
OTPCacheData::OTPCacheData()
{
	_RSAExp = _RSAMod = NULL;
	_TOTPTokens = NULL;
	_HOTPTokens = NULL;
	_RSAExpLen = _RSAModLen = _TOTPCount = _TOTPSteps = _HOTPCount = _HOTPSteps = 0;
}

OTPCacheData::~OTPCacheData()
{
	free();
}

void OTPCacheData::free()
{
	DEBUG_VERBOSE("OTPCACHE::Free\n");
	_freeRSA();
	_freeTOTP();
	_freeHOTP();
	DEBUG_VERBOSE("End OTPCACHE::Free\n");
}

void OTPCacheData::_freeRSA()
{
	DEBUG_VERBOSE("OTPC FreeRSA\n");
	SAFE_FREE(_RSAExp);
	SAFE_FREE(_RSAMod);
	_RSAExpLen = _RSAModLen = 0;
	DEBUG_VERBOSE("End OTPC FreeRSA\n");
}

void OTPCacheData::_freeTOTP()
{
	DEBUG_VERBOSE("OTPC Free TOTP\n");
	SAFE_FREE(_TOTPTokens);
	_TOTPCount = _TOTPSteps = 0;
	DEBUG_VERBOSE("End OTPC Free TOTP\n");
}

void OTPCacheData::_freeHOTP()
{
	DEBUG_VERBOSE("OTPC Free HOTP\n");
	SAFE_FREE(_HOTPTokens);
	_HOTPCount = _HOTPSteps = 0;
	DEBUG_VERBOSE("End OTPC Free HOTP\n");
}

void OTPCacheData::SetRSA(void* Exp, void* Mod, int	ExpLen, int ModLen)
{
	DEBUG_VERBOSE("Setting RSA\n");
	_freeRSA();
	DEBUG_VERBOSE("Mallocing RSA Exp/Mod\n");
	_RSAExp = malloc(ExpLen);
	_RSAMod = malloc(ModLen);
	DEBUG_VERBOSE("Result [%08lX and %08lX]\n", _RSAExp, _RSAMod);
	_RSAExpLen = ExpLen;
	_RSAModLen = ModLen;

	if((_RSAExp == NULL) || (_RSAMod == NULL))
		throw "Error : Not enough memory";

	DEBUG_VERBOSE("Copying over RSA memory\n");
	memcpy(_RSAExp, Exp, ExpLen);
	memcpy(_RSAMod, Mod, ModLen);
}

void* OTPCacheData::SetTOTP(void* data, int count, int steps)
{
	DEBUG_VERBOSE("Set TOTP\n");
	_freeTOTP();
	_TOTPCount = count;
	_TOTPSteps = steps;

	DEBUG_VERBOSE("Allocating Token Buffer : ");
	_TOTPTokens = (TOTPInfo*)calloc(count * sizeof(TOTPInfo), 1);
	DEBUG_VERBOSE("%08lX\n", _TOTPTokens);
	for(int i = 0 ; i < count ; ++i)
	{
		DEBUG_VERBOSE("Getting Token Data\n");
		data = _TOTPTokens[i].SetTOTP(data, steps);
	}

	DEBUG_VERBOSE("Finished Setting TOTP\n");
	return data;
}

void* OTPCacheData::SetHOTP(void* data, int count, int steps, bool HOTPHasID)
{
	DEBUG_VERBOSE("Set HOTP\n");
	_freeHOTP();
	_HOTPCount = count;
	_HOTPSteps = steps;

	DEBUG_VERBOSE("Allocating Token Buffer : ");
	_HOTPTokens = (HOTPInfo*)calloc(count * sizeof(HOTPInfo), 1);
	DEBUG_VERBOSE("%08lX\n", _HOTPTokens);
	for(int i = 0 ; i < count ; ++i)
	{
		DEBUG_VERBOSE("Getting Token Data\n");
		data = _HOTPTokens[i].SetHOTP(data, steps, HOTPHasID);
	}
	DEBUG_VERBOSE("Finished Setting HOTP\n");
	return data;
}

void* OTPCacheData::GetExp(int &length)
{
	DEBUG_VERBOSE("Getting EXP\n");
	if((_RSAExpLen == 0) || (_RSAExp == NULL))
	{
		DEBUG_VERBOSE("Invalid params to get Exp\n");
		length = 0;
		return NULL;
	}

	length = _RSAExpLen;
	DEBUG_VERBOSE("Allocating Data : ");
	void* ret = malloc(length);
	DEBUG_VERBOSE("%08lX\n", ret);
	DEBUG_VERBOSE("Copying memory\n");
	memcpy(ret, _RSAExp, length);
	DEBUG_VERBOSE("Done Getting EXP\n");
	return ret;
}

void* OTPCacheData::GetMod(int &length)
{
	DEBUG_VERBOSE("Getting MOD\n");
	if((_RSAModLen == 0) || (_RSAMod == NULL))
	{
		DEBUG_VERBOSE("Invalid params to get Mod\n");
		length = 0;
		return NULL;
	}

	length = _RSAModLen;
	DEBUG_VERBOSE("Allocating memory : ");
	void* ret = malloc(length);
	DEBUG_VERBOSE("%08lX\nCopying Mod\n", ret);
	memcpy(ret, _RSAMod, length);
	DEBUG_VERBOSE("Returning Mod\n");
	return ret;
}
TOTPInfo* OTPCacheData::GetTotpTokens(int &count)
{
	DEBUG_VERBOSE("Gettting TOTPTokens\n");
	if((_TOTPCount == 0) || (_TOTPTokens == NULL))
	{
		DEBUG_VERBOSE("Invalid params to get TOTPTokens\n");
		count = 0;
		return NULL;
	}

	count = _TOTPCount;
	DEBUG_VERBOSE("Allocating Memory [%d] : ", count);
	void* ret = malloc(count * sizeof(TOTPInfo));
	DEBUG_VERBOSE("%08lX\nCopying TOTP Tokens\n", ret);
	memcpy(ret, _TOTPTokens, count * sizeof(TOTPInfo));
	DEBUG_VERBOSE("Returning TOTPInfo\n");
	return (TOTPInfo*)ret;
}

HOTPInfo* OTPCacheData::GetHotpTokens(int &count)
{
	DEBUG_VERBOSE("Getting HOTPTokens\n");
	if((_HOTPCount == 0) || (_HOTPTokens == NULL))
	{
		DEBUG_VERBOSE("Invalid params to get HOTPTokens\n");
		count = 0;
		return NULL;
	}

	count = _HOTPCount;
	DEBUG_VERBOSE("Allocating Memory : ");
	void* ret = malloc(count * sizeof(HOTPInfo));
	DEBUG_VERBOSE("%08lX\nCopying over HOTPTokens\n", ret);
	memcpy(ret, _HOTPTokens, count * sizeof(HOTPInfo));
	DEBUG_VERBOSE("Returning HOTP Data\n");
	return (HOTPInfo*)ret;
}


TOTPInfo::TOTPInfo()
{
	_steps = _stepSize = _tokenLength = 0;
	_data = NULL;
}

TOTPInfo::~TOTPInfo()
{
	this->free();
}

void TOTPInfo::free()
{
	DEBUG_VERBOSE("TOTPInfo::Free\n");
	SAFE_FREE(_data);
	_steps = _stepSize = _tokenLength = 0;
	DEBUG_VERBOSE("End TOTPInfo::Free\n");
}

void* TOTPInfo::GetTOTP(int& steps, int& stepSize, int& length)
{
	steps = _steps;
	stepSize = _stepSize;
	length = _tokenLength;

	DEBUG_VERBOSE("TOTPInfo::GetTOTP [%d s, %d l\n", steps, length);
	void* ret = malloc(steps * length);
	DEBUG_VERBOSE("[%08lX]\n", ret);
	memcpy(ret, _data, steps * length);
	DEBUG_VERBOSE("End TOTPInfo::GetTOTP\n");
	return ret;
}

void* TOTPInfo::SetTOTP(void* data, int steps)
{
	DEBUG_VERBOSE("TOTPInfo::SetTOTP\n");
	this->free();
	memcpy(&_stepSize, data, 4);
	REV_OFFSET2(data, 4, void);
	memcpy(&_tokenLength, data, 4);
	REV_OFFSET2(data, 4, void);
	_steps = steps;
	DEBUG_VERBOSE("\tSS : %d\n\tTL : %d\n\tS : %d\n", _stepSize, _tokenLength, _steps);
	_data = malloc(steps * _tokenLength);
	memcpy(_data, data, steps * _tokenLength);
	REV_OFFSET2(data, steps * _tokenLength, void);
	DEBUG_VERBOSE("End TOTPInfo::SetTOTP\n");
	return data; //next read location
}

HOTPInfo::HOTPInfo()
{
	_steps = _tokenLength = 0;
	_data = NULL;
	//_publicIdentity = NULL;
}

HOTPInfo::~HOTPInfo()
{
	this->free();
}

void HOTPInfo::free()
{
	DEBUG_VERBOSE("HOTPInfo::Free\n");
	SAFE_FREE(_data);
	//SAFE_FREE(_publicIdentity);
	_steps = _tokenLength = 0;
	_idLength = 0;
	memset(_id, 0, sizeof(_id));
	DEBUG_VERBOSE("Done HOTPInfo::Free\n");
}

int HOTPInfo::getID()
{
	return _idLength;
}

void* HOTPInfo::getIDBuffer()
{
	return _id;
}

void* HOTPInfo::GetHOTP(int &steps, int &length)
{
	DEBUG_VERBOSE("HOTPInfo::GetHOTP\n");
	DEBUG_VERBOSE("Steps = %d and length = %d\r\n",_steps, _tokenLength);

	steps = _steps;
	length = _tokenLength;

	void *ret = malloc(steps * length);
	memcpy(ret, _data, steps * length);

	DEBUG_VERBOSE("End HOTPInfo::GetHOTP\n");
	return ret;
}

void* HOTPInfo::SetHOTP(void* data, int steps, bool HOTPHasID)
{
	DEBUG_VERBOSE("HOTPInfo::SetHOTP\n");
	this->free();
	memcpy(&_tokenLength, data, 4);
	REV_OFFSET2(data, 4, void);

	if(HOTPHasID)
	{
		DEBUG_VERBOSE("HOTP has ID\r\n");
		//get the ID Length
		memcpy(&_idLength, data, 4);
		REV_OFFSET2(data, 4, void);

		DEBUG_SENSITIVE(true, "HOTP has ID Length of %d\r\n", _idLength);
		//if the ID Length is 12, read the next 12 bytes into the ID
		if(_idLength == 12)
		{
			memcpy(_id, data, 12);
			REV_OFFSET2(data, 12, void);
			DEBUG_SENSITIVE(true, "HOTP has ID of : ");
			DEBUG_SENSITIVEBYTES(true, _id, 12);
		}
	}

	_steps = steps;
	_data = malloc(steps * _tokenLength);
	memcpy(_data, data, steps * _tokenLength);
	REV_OFFSET2(data, steps * _tokenLength, void);

	DEBUG_VERBOSE("HOTP Settings : \r\n\tSteps : %d\r\n\tToken Length : %d\r\n", steps, _tokenLength);
	DEBUG_VERBOSE("End HOTPInfo::SetHOTP\n");
	return data;
}

void* BypassSidData::InitWithData(void* currentRead, void* endOfStream)
{
	free();
	allocedData = false;
	unsigned char* rpos = (unsigned char*)currentRead;
	unsigned char* EOS = (unsigned char*)endOfStream;

	//check that we aren't about to read past the EOS
	if(rpos + 4 > EOS)
		return NULL;
	//it is fine to do this read
	memcpy(&SIDLength, rpos, 4);
	rpos += 4;
	DEBUG_VERBOSE("SID length of %d found\n", SIDLength);
	//check that the SID does not go beyond the EOS
	if(rpos + SIDLength >= EOS)
		return NULL;

	//SID is fine to read
	SID = (void*)rpos;
	rpos += SIDLength;

	DEBUG_VERBOSE("SID : ");
	DEBUG_VERBOSEBYTES((unsigned char*)SID, SIDLength);
	DEBUG_VERBOSE("\n");

	//check that we don't read past the EOS
	if(rpos + 4 > EOS)
		return NULL;
	//it is fine to do this read
	memcpy(&BypassCount, rpos, 4);
	rpos += 4;

	DEBUG_VERBOSE("Bypass Count is %d [Len : %d]\n", BypassCount, BypassLength());
	//Check the BypassData isn't beyond the EOS
	if(rpos + BypassLength() > EOS)
		return NULL;
	//we are not beyond the EOS so everything is fine
	BypassData = (void*)rpos;
	rpos += BypassLength();

	DEBUG_VERBOSE("Bypass Data : ");
	DEBUG_VERBOSEBYTES((unsigned char*)BypassData, BypassLength());
	DEBUG_VERBOSE("\n");
	return rpos;
}

int BypassSidData::BypassLength()
{
	return 21 + (20 * BypassCount);
}
void BypassSidData::CopyData(BypassSidData* reference)
{
	this->SIDLength = reference->SIDLength;
	this->BypassCount = reference->BypassCount;
	this->SID = malloc(this->SIDLength);
	this->BypassData = malloc(reference->BypassLength());
	memcpy(this->SID, reference->SID, this->SIDLength);
	memcpy(this->BypassData, reference->BypassData, reference->BypassLength());
	allocedData = true;
}
void BypassSidData::free()
{
	if(allocedData)
	{
		SAFE_FREE(SID);
		SAFE_FREE(BypassData);
		allocedData = false;
	}
}
OTPBypassData::OTPBypassData()
{
	_SIDBlocks = NULL;
	_SIDBlockCount = 0;
}
OTPBypassData::~OTPBypassData()
{
	free();
}
void OTPBypassData::free()
{
	SAFE_FREE(_SIDBlocks);
	_SIDBlockCount = 0;
}

int OTPBypassData::GetSIDBlockCount()
{
	return _SIDBlockCount;
}

BypassSidData* OTPBypassData::GetSIDBlock(int index)
{
	if(index >= _SIDBlockCount)
		return NULL; //no such SIDBlock

	BypassSidData* pRet = (BypassSidData*)malloc(sizeof(BypassSidData));
	pRet->CopyData(&(_SIDBlocks[index]));

	return pRet;
}
void OTPBypassData::SetSIDBlocks(OTPVerifyExParser::BypassSidData *SIDBlocks, int SIDBlockCount)
{
	free();
	_SIDBlockCount = SIDBlockCount;
	_SIDBlocks = (BypassSidData*)calloc(SIDBlockCount, sizeof(BypassSidData));
	for(int i = 0 ; i < SIDBlockCount ; ++i)
	{
		_SIDBlocks[i].CopyData(&(SIDBlocks[i]));
	}
}
BYTE* OTPBypassData::GenerateHash(int& length)
{
	/*
		We will use a 40 byte hash which will be
			Hash = Append([SID Hash],[Code Hash]);

		Where SID hash is generated via:
			hash = (static const)
			foreach(SID s in SIDs)
			{
				hash = hmac_sha1(hash, s);
			}

		And Code Hash is generated via:
			hash = (static const)
			foreach(BypassCode b in BypassCodes)
			{
				hash = hmac_sha1(hash, b);
			}

		Note: BypassCodes are the values directly from the server, not the values post PC Key hashing
	*/

	//Inital values were randomly generated via http://www.random.org/cgi-bin/randbyte?nbytes=20&format=h
	//They don't really need to be secret or random, so this will do.
	BYTE SIDHash[20] =  {0xfc, 0x20, 0xc3, 0xed, 0x22, 0x0d, 0x21, 0x67, 0x74, 0xbd, 0x22, 0x96, 0x47, 0x52, 0x67, 0xe8, 0x74, 0x54, 0xe5, 0xdb};
	BYTE CodeHash[20] = {0x81, 0xb7, 0x67, 0x7b, 0x8a, 0x77, 0x16, 0xe9, 0x12, 0x47, 0x7f, 0x8e, 0xbe, 0x0a, 0xe7, 0xb5, 0x3f, 0x4c, 0x63, 0x76};
	BYTE* totalHash = (BYTE*)malloc(40);

	//where the data of each hmac_sha1 will be written to (incase having source/dest as the same mem location stuffs it up)
	BYTE writeBuffer[20];

	for(int i = 0 ; i < _SIDBlockCount ; ++i)
	{
		hmac_sha1(writeBuffer, SIDHash, 20, (unsigned char*)_SIDBlocks[i].SID, _SIDBlocks[i].SIDLength);
		memcpy(SIDHash, writeBuffer, 20);

		hmac_sha1(writeBuffer, CodeHash, 20, (unsigned char*)_SIDBlocks[i].BypassData, _SIDBlocks[i].BypassLength());
		memcpy(CodeHash, writeBuffer, 20);
	}

	//now copy the hashes over to totalhash
	memcpy(totalHash, SIDHash, 20);
	memcpy(totalHash + 20, CodeHash, 20);
	length = 40;
	return totalHash;	
}
DynamicPasswordData::DynamicPasswordData()
{
	this->_password = NULL;
	this->_passLength = 0;
}
DynamicPasswordData::~DynamicPasswordData()
{
	this->free();
}
void DynamicPasswordData::free()
{
	if(this->_password)
	{
		::free(this->_password);
	}
	this->_password = NULL;
	this->_passLength = 0;
}
int DynamicPasswordData::passwordLength()
{
	return this->_passLength;
}
void DynamicPasswordData::copyPassword(void *dest)
{
	DEBUG_VERBOSE("Copying Dynamic Password\n");
	if(this->_password)
	{
		memcpy(dest, this->_password, this->_passLength);
	}
	DEBUG_VERBOSE("Copying Dynamic Password - Done\n");
}
void DynamicPasswordData::setPassword(void *src, int length)
{
	DEBUG_VERBOSE("Setting Dynamic Password\n");
	this->free();
	this->_password = (char*)malloc(length);
	memcpy(this->_password, src, length);
	this->_passLength = length;
	DEBUG_VERBOSE("Setting Dynamic Password - Done\n");
}