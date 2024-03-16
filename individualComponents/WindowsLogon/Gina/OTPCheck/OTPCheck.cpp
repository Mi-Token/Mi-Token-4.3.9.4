#pragma warning ( disable : 4101 )
#pragma warning ( disable : 4189 )
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )


#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif


#include "Credential.h"
#include "guid.h"
#include <assert.h>
#include <windows.h>
#include <winhttp.h>
#include <string.h>
#include <atlbase.h>
#include <malloc.h>
#include "oath.h"

#include <iostream>
#include <fstream>

#include <Sddl.h>

#pragma comment(lib, "netapi32.lib")
#include <lm.h>

#ifndef MAX_NAME
#define MAX_NAME 256
#endif
#pragma warning (push)
//#include <atlbase.h>
#include <tchar.h>
#include "freeradius-client.h"
#pragma warning (pop)
#include "OTPBypass.h"

#include "OTPCheck.h"
#include "OTPOffline.h"
#include "OTPVerifyExParser.h"
#include "DataBuilder.h"

#include <Dsgetdc.h>

#include "MultiString.h"

#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { ::free(p); p = NULL; }
#endif

long cacheUsed_g;

//#define POC TRUE

#define API 1

using namespace std;

DWORD getNumServers(DWORD maxValue, DWORD maxData, TCHAR* serverCheckLocation) {
	
	DWORD    cValues = 0;              // number of values for key 
	HKEY key;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, serverCheckLocation,0,KEY_READ, &key);
	
	RegQueryInfoKey(key,           // key handle 
			NULL,                   // buffer for class name 
			NULL,                   // size of class string 
			NULL,                   // reserved 
			NULL,                   // number of subkeys 
			NULL,                   // longest subkey size 
			NULL,                   // longest class string 
			&cValues,               // number of values for this key 
			&maxValue,				// longest value name 
			&maxData,				// longest value data 
			NULL,			        // security descriptor 
			NULL);	
									
	RegCloseKey(key);
	return cValues;
}


std::vector<BYTE> ReadValue(CString keyPath, CString valueName, DWORD type)
{
	HKEY hKey = NULL;
	DWORD length = 0;

	try
	{
		LONG result = RegOpenKeyEx (HKEY_LOCAL_MACHINE,keyPath,0,KEY_READ,&hKey);

		if(result == ERROR_SUCCESS)
		{
			/*Get the size of the registry value.*/
			result = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &length);

		//	if(result != ERROR_SUCCESS)
		//		ThrowCustomException(result);

			std::vector<BYTE> regVal(length);

			result = RegQueryValueEx(hKey, valueName, NULL, &type, &regVal[0], &length);

			if(result == ERROR_SUCCESS)
			{
				/*Succes*/
				RegCloseKey(hKey);
				return regVal;
			}
		}
		else
		{
			std::vector<BYTE> regVal(0);
			return regVal;
		}

//		ThrowCustomException(result);

		
	}	catch(const exception &ex)
	{
		if(hKey != NULL)
			RegCloseKey(hKey);

		std::vector<BYTE> regVal(0);
			return regVal;
	}
	std::vector<BYTE> regVal(0);
			return regVal;
}



std::vector<BYTE> ReadSingleStringValue(CString keyPath, CString valueName)
{
	return ReadValue(keyPath, valueName, REG_SZ);
}

void* _dynamicPassword = NULL;

bool doApiCall(const std::string APICall, const wchar_t* serverName, const bool useHTTP, const bool getOutput, char** ppOutput, int* pLength)
{	
	DEBUG_LOG("OTPCheck::In doAPICall");
	HINTERNET hConnect = NULL, hRequest = NULL, hSession = NULL;

	DWORD dwStatusCode[11];
	DWORD dwSize = 10;
	DWORD headernonsense = WINHTTP_NO_HEADER_INDEX;

	DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|
		SECURITY_FLAG_IGNORE_UNKNOWN_CA;
	
	{
		//const cast because the 4th parameter should really have been defined const anyway - in its own scope because we don't want anything being able to hit the non-const servername variable
		wchar_t * ncServerName = const_cast<wchar_t*>(serverName);
		rc_event_log(2,1,1,ncServerName);
		ncServerName = NULL;
	}

	DEBUG_LOGW(L"OTPCheck::Server Name : %s\n", serverName);

	hSession = WinHttpOpen(L"Mi-Token API Client/1.0",WINHTTP_ACCESS_TYPE_NO_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts( hSession, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	hConnect = WinHttpConnect(hSession, serverName, (useHTTP ? 0 : INTERNET_DEFAULT_HTTPS_PORT), 0); 
	WinHttpSetTimeouts( hConnect, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	// null-call to get the size
	size_t needed = std::mbstowcs(NULL,&APICall[0],APICall.length());

	// allocate
	std::wstring output;
	output.resize(needed);

	bool bResults = true;

	// real call
	std::mbstowcs(&output[0],&APICall[0],APICall.length());

	if( hConnect ){
		DEBUG_LOG("OTPCheck::Opening Request\n");
		hRequest = WinHttpOpenRequest( hConnect, L"GET", (LPCWSTR)output.c_str(),NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (useHTTP ? 0 : WINHTTP_FLAG_SECURE ) );
		WinHttpSetTimeouts( hRequest, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());
	} 
	// important - set security to ignore 'bad' certs (basically the api service is self signed which is bad)
	BOOL bRet = WinHttpSetOption(hRequest,WINHTTP_OPTION_SECURITY_FLAGS,&dwFlags,sizeof(DWORD));

	if( hRequest ){
		DEBUG_LOG("OTPCheck::Sending Request\n");
		bResults = WinHttpSendRequest( hRequest,WINHTTP_NO_ADDITIONAL_HEADERS, 0,WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );
	} 

	// End the request.
	if( bResults ) {
		DEBUG_LOG("OTPCheck::Receiving Response\n");
		bResults = WinHttpReceiveResponse( hRequest, NULL );
	} 

	if( bResults ){
		DEBUG_LOG("OTPCheck::Querying Headers\n");
		bResults = WinHttpQueryHeaders( hRequest, WINHTTP_QUERY_STATUS_CODE |WINHTTP_QUERY_FLAG_NUMBER,NULL, dwStatusCode, &dwSize, &headernonsense );
	} 

	bool result = false;
	if (bResults)
	{
		DEBUG_LOG("OTPCheck::Status code is %d\n", dwStatusCode[0]);
		if (dwStatusCode[0] == HTTP_STATUS_OK )
		{
			result = true;
		}
	} 

	if( hRequest ) WinHttpCloseHandle( hRequest );
	if( hConnect ) WinHttpCloseHandle( hConnect );
	if( hSession ) WinHttpCloseHandle( hSession );

	return result;
}

bool validateTokenlessUsers(const char* UserName, const wchar_t* serverName, const bool useHTTP)
{
	std::string input;

	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyBlankOTP?username=";
	input.append(UserName);

	return doApiCall(input, serverName, useHTTP, false, NULL, NULL);
}

bool ReadRegistryInt(const HKEY key, const wchar_t* keyName, int& value)
{
	DWORD len = sizeof(value);
	return (RegQueryValueEx(key, keyName, NULL, NULL, (unsigned char*)&value, &len) == ERROR_SUCCESS);
}

bool ReadRegistryIntAsBool(const HKEY key, const wchar_t* keyName, int trueValue)
{
	int value;
	return ((ReadRegistryInt(key, keyName, value)) && (value == trueValue));
}

void GetAPISettings(bool& otpCacheMode, bool& dynamicPasswordMode, int& totpCount, int& hotpCount, bool& useHTTP)
{
	HKEY subKey;
	DWORD result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Config", 0, KEY_READ, &subKey);
	if(result == ERROR_SUCCESS)
	{
		int value;

		otpCacheMode = ReadRegistryIntAsBool(subKey, L"EnableOTPCaching", 1);
		dynamicPasswordMode = ReadRegistryIntAsBool(subKey, L"DynamicPassword", 1);
		useHTTP = ReadRegistryIntAsBool(subKey, L"APIUseHTTP", 1);

		if(ReadRegistryInt(subKey, L"OTPCacheTOTP", value))
		{
			totpCount = value;
		}

		if(ReadRegistryInt(subKey, L"OTPCacheHOTP", value))
		{
			hotpCount = value;
		}

		RegCloseKey(subKey);
	}

	if((otpCacheMode) && (dynamicPasswordMode))
	{
		//we are using both Cache and Dynamic password mode, these modes are mutually exclusive (cache allows login without DC access, Dynamic requires DC access for each login) - so warn the user, and disable dynamic pass
		dynamicPasswordMode = false;
		DEBUG_LOG("ERROR!!! : Both Cache and Dynamic Password mode are on. These modes are mutually exclusive. DISABLING Dynamic Password Mode - Keeping Cache Mode Enabled\n");
	}

	return;
}

bool VerifyAgainstCache(const wchar_t* server, const bool useHTTP, const int totpCount, const int hotpCount, const wchar_t* wUser, const wchar_t* wOTP, bool& authenticated, bool& serverHit)
{
	DEBUG_VERBOSE("Getting Server Length [");
	DEBUG_VERBOSEW(L"%s]\n", server);
	int wszlen = wcslen(server);
	DEBUG_VERBOSE("SAFE FREEing Server Name\n");
	SAFE_FREE(OTPOffline_ServerName);
	
	DEBUG_VERBOSE("Setting USE HTTP Mode\n");
	OTPOffline_UseHTTP = useHTTP;

	DEBUG_VERBOSE("Allocating Server Name Space\n");
	OTPOffline_ServerName = (wchar_t*)calloc(wszlen * 2 + 2, 1);
	DEBUG_VERBOSE("Setting Server Name\n");
	memcpy(OTPOffline_ServerName, server, wszlen * 2);

	DEBUG_VERBOSE("Calling Update cache\n");
	DWORD retStatus = OTPOffline_UpdateCache(wUser, wOTP, hotpCount, totpCount);
	switch(retStatus)
	{
	case ERROR_SUCCESS:
		DEBUG_LOG("User Authenticated Successfully\n");
		return true;
	case ERROR_NO_DATA:
		DEBUG_LOG("The Server did not respond... checking other servers for a responce.\n");
		break;
	case ERROR_ACCESS_DENIED:
		serverHit = true;
		DEBUG_LOG("User authentication failed, error code %08lX\n", retStatus);
		authenticated = false;
		break;
	case ERROR_NOT_FOUND:
		serverHit = true;
		DEBUG_LOG("User was authenticated, However Cached OTPs were not sent\n");
		authenticated = true;
		break;
	case ERROR_INVALID_DATA:
		DEBUG_LOG("Server sent back an invalid response. [%08lX], Not counting server as a hit\r\n", retStatus);
		authenticated = false;
		break;
	case ERROR_MORE_DATA:
		DEBUG_LOG("Server sent some invalid data (More Data) [%08lX]\n", retStatus);
		authenticated = false;
		serverHit = true;
		break;
	}

	DEBUG_VERBOSE("Cache Login Failed\n");
	return false;
}

bool UserRequires2FA(const bool dynamicPasswordMode, const wchar_t* server, const bool useHTTP, MultiString* msUsername)
{
	if(dynamicPasswordMode)
	{
		if(validateTokenlessUsers(msUsername->getChar(), server, useHTTP))
		{
			return false;
		}
	}
	return true;
}

//Note: GetAPICallString presumes the user requires 2FA. Always call UserRequires2FA prior to GetAPICallString to check if this is the case
char* FixedCW2A(const wchar_t* input)
{
	int inputLength;
	char* output;

	inputLength = wcslen(input);
	output = (char*)malloc(inputLength + 1);
	output[inputLength] = 0;
	wcstombs(output, input, inputLength);

	return output;

}

std::string GetAPICallString(const bool dynamicPasswordMode, const wchar_t* server, const bool useHTTP, MultiString* msUsername, MultiString* msOTP)
{
	std::string input;

	DEBUG_VERBOSE("OTPCheck::GetAPICallString()");
	DEBUG_SENSITIVEW(true, L"OTPCheck::GetAPICallString() WServer = %s\n", server);
	DEBUG_SENSITIVEW(true, L"OTPCheck::GetAPICallString() WUser = %s\n", msUsername->getWChar());
	DEBUG_SENSITIVEW(true, L"OTPCheck::GetAPICallString() WOTP = %s\n", msOTP->getWChar());

	DEBUG_SENSITIVE(true, "OTPCheck::GetAPICallString() User = %s\n", msUsername->getChar());
	DEBUG_SENSITIVE(true, "OTPCheck::GetAPICallString() OTP = %s\n", msOTP->getChar());

	int flagID = (OTPEX_FLAG_CENTRAL_BYPASS);
	if(dynamicPasswordMode)
	{
		DEBUG_LOG("OTPCheck::GetAPICallString() Tokenless Users Not Validated - Presuming OTP Field contained users OTP");
		flagID |= OTPEX_FLAG_DYNAMIC_PASSWORD;
	}

	char cFlagID[32];
	itoa(flagID, cFlagID, 10);
	
	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyOTPEx?username="; 
			input.append(msUsername->getChar());
			input.append("&otp=");
			input.append(msOTP->getChar());
			input.append("&flags=");
			input.append(cFlagID);

	return input;
}

bool GetAPICallResult(const std::string input, const wchar_t* server, const bool useHTTP, HINTERNET& hSession, HINTERNET& hConnect, HINTERNET& hRequest)
{
	bool bResults;

	DEBUG_LOG("OTPCheck::GetAPICallResult() on %s\r\n", input.c_str());

	DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA;

	hSession = WinHttpOpen(L"Mi-Token API Client/1.0",WINHTTP_ACCESS_TYPE_NO_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts( hSession, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	hConnect = WinHttpConnect(hSession, server, (useHTTP ? 0 : INTERNET_DEFAULT_HTTPS_PORT), 0); 
	WinHttpSetTimeouts( hConnect, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());

	// null-call to get the size
	size_t needed = std::mbstowcs(NULL,&input[0],input.length());

	// allocate
	std::wstring output;
	output.resize(needed);

	// real call
	std::mbstowcs(&output[0],&input[0],input.length());

	if( hConnect ){
		DEBUG_LOG("OTPCheck::GetAPICallResult() Opening Request\n");
		hRequest = WinHttpOpenRequest( hConnect, L"GET", (LPCWSTR)output.c_str(),NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (useHTTP ? 0 : WINHTTP_FLAG_SECURE ) );
		WinHttpSetTimeouts( hRequest, webResolveTime(), webConnectTime(), webSendTime(), webReceiveTime());
	} 
	// important - set security to ignore 'bad' certs (basically the api service is self signed which is bad)
	BOOL bRet = WinHttpSetOption(hRequest,WINHTTP_OPTION_SECURITY_FLAGS,&dwFlags,sizeof(DWORD));

	if( hRequest ){
		DEBUG_LOG("OTPCheck::GetAPICallResult() Sending Request\n");
		bResults = WinHttpSendRequest( hRequest,WINHTTP_NO_ADDITIONAL_HEADERS, 0,WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );
	} 

	// End the request.
	if( bResults ) {
		DEBUG_LOG("OTPCheck::GetAPICallResult() Receiving Response\n");
		bResults = WinHttpReceiveResponse( hRequest, NULL );
	} 

	if(!bResults)
	{
		DEBUG_LOG("OTPCheck::GetAPICallResult() Step failed with Error ID of %d\r\n", GetLastError());
	}

	return bResults;
}

void* GetAPICallData(HINTERNET hRequest, int& DBLength)
{
	void* retData = NULL;
	DataBuilder DB;
	DWORD bytesRead;
	char buffer[8096];
	while(true)
	{
		if(WinHttpReadData( hRequest, buffer, 8096, &bytesRead))
		{
			if(bytesRead == 0)
			{
				DB.GetData(NULL, &DBLength);
				retData = malloc(DBLength);
				if(DB.GetData(retData, &DBLength))
					break;
			}
			DB.AppendData(buffer, bytesRead);
		}
		else
		{
			break;
		}
	}
	return retData;
}


bool VerifyAPIResultsNormalMode(HINTERNET hRequest)
{
	//Due to the fact we will now always request with OTPEX_FLAG_CENTRAL_BYPASS set, this is going to look more similar to VerifyAPIResultsDynamicPasswordMode
	DEBUG_VERBOSE("OTPCheck::VerifyAPIResultsNormalMode() Grabbing data from API server\n");
	int DBLength;
	void* retData  = GetAPICallData(hRequest, DBLength);

	bool OTPResult = false;

	if(retData)
	{
		if(OTPVerifyExParser::ParseValidData(retData, DBLength))
		{
			updateBypassCodesFromServer(retData, DBLength);
			OTPResult = true;
		}

		free(retData);
	}

	return OTPResult;
}

//Returns true when GetAPICallData returned data - ie. result contains a valid value.
bool VerifyAPIResultsDynamicPasswordMode(HINTERNET hRequest, bool& result)
{
	DEBUG_VERBOSE("OTPCheck::VerifyAPIResultsDynamicPasswordMode() Grabbing data from API server\n");
	int DBLength;
	void* retData  = GetAPICallData(hRequest, DBLength);

	if(retData)
	{
		OTPVerifyExParser::DynamicPasswordData dynamicPass;
		int retCode = OTPVerifyExParser::GetDynamicPassword(retData, DBLength, dynamicPass);
		if(retCode != APIEX_ERROR_SUCCESS)
		{
			DEBUG_LOG("OTPCheck::VerifyAPIResultsDynamicPasswordMode() ERROR : RetCode != APIEX_ERROR_SUCCESS - OTP was NOT checked");
			free(retData);
			result = false;
			return true;
		}
		else
		{
			int passLen = dynamicPass.passwordLength();
			_dynamicPassword = calloc(passLen + 1, 1);
			dynamicPass.copyPassword(_dynamicPassword);
			//and lastly lets attempt to update the bypass codes
			updateBypassCodesFromServer(retData, DBLength);
		}
		free(retData);

		result = true;
		return true;
	}

	DEBUG_LOG("OTPCheck::VerifyAPIResultsDynamicPasswordMode( VerifyAPIResultsDynamicPasswordMode : Failed to grab data from GetAPICallData\n");
	return false;
}

//bool queryapi(const wchar_t* wotp, const wchar_t* wuser, const wchar_t* wDomainAndUser, DWORD* Error_Code, LPCSTR ServerR);
/*
bool queryapi (char* OTP,char* UserName,const wchar_t* wDomainAndUser, DWORD* Error_Code, LPCSTR ServerR)
{
	DEBUG_LOG("Query API (c)\n");

	wchar_t * wuser, * wotp;
	wuser = (wchar_t*)calloc(strlen(UserName) * 2 + 2, 1);
	wotp = (wchar_t*)calloc(strlen(OTP) * 2 + 2, 1);
	mbstowcs(wuser, UserName, strlen(UserName));
	mbstowcs(wotp, OTP, strlen(OTP));

	bool result = queryapi(wotp, wuser, wDomainAndUser, Error_Code, ServerR);
	free(wuser);
	free(wotp);

	return result;
}
*/
//bool queryapi(const wchar_t* wotp, const wchar_t* wuser, const wchar_t* wDomainAndUser, DWORD* Error_Code, LPCSTR ServerR)
bool queryapi(MultiString* msOTP, MultiString* msUser, MultiString* msDomainAndUser, DWORD* Error_Code, LPCSTR ServerR)
{
	DEBUG_LOG("OTPCheck::queryapi() Querying API (v17)\n");

	DEBUG_SENSITIVEW(true, L"OTPCheck::queryapi() OTP = %s\n", msOTP->getWChar());
	DEBUG_SENSITIVEW(true, L"OTPCheck::queryapi() User = %s\n", msUser->getWChar());

	::InterlockedExchange(&cacheUsed_g, 0L); //default to having not used the cache.

	//debug only
	/*if (*OTP == "123456789")
		return true;		   */
	char junk[10];
	bool authenticated =false;
	bool serverHit = false;
	Error_Code = 0;
	
	BOOL bResults = FALSE;
	DWORD headernonsense = WINHTTP_NO_HEADER_INDEX;
	DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|
	SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|
	SECURITY_FLAG_IGNORE_UNKNOWN_CA;
	
	DWORD dwStatusCode[11];
		DWORD dwSize = 10;
	LPSTR pszOutBuffer;
	BOOL  bDone = FALSE;


	
	
	TCHAR* serverLocationList[] = {TEXT("Software\\Mi-Token\\Logon\\Servers"), TEXT("Software\\Mi-Token\\Logon")};
	
	int serverLocationCount = (sizeof(serverLocationList) / sizeof(TCHAR*));

	DEBUG_LOG("OTPCheck::queryapi() Server Location Count = %d\n", serverLocationCount);

	bool otpCacheMode = false;
	for(int serverCheckLocations = 0 ; serverCheckLocations <  serverLocationCount; ++serverCheckLocations)
	{
		TCHAR* serverLocation = serverLocationList[serverCheckLocations];

		DWORD cValues = getNumServers(MAX_VALUE_NAME, MAX_KEY_LENGTH, serverLocation);	
	
		DEBUG_LOG("OTPCheck::queryapi() Server Count = %d\n", cValues);

		
		int totpCount = 1000, hotpCount = 100;
		bool useHTTP = false;
		bool useDynamicPassword = false;
		
		GetAPISettings(otpCacheMode, useDynamicPassword, totpCount, hotpCount, useHTTP);

		DEBUG_LOGW(_T("OTPCheck::queryapi() Server Location = %s\n"), serverLocation);
		for (DWORD i = 0;i <cValues;	i++) 
		{
			//just make a request to x.89.226
			ServerR = "192.168.89.226";
			HKEY key;
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, serverLocation,0,KEY_READ, &key);

			TCHAR  achValue[MAX_VALUE_NAME]; 
			DWORD cchValue = MAX_VALUE_NAME; 
			cchValue = MAX_VALUE_NAME; 
			memset (achValue,'\0',MAX_VALUE_NAME * sizeof(TCHAR));
			DWORD retCode; 
			DWORD valType;

			retCode = RegEnumValue(
				key, 
				i,			//The index of the value to be retrieved. 
				achValue,   //A pointer to a buffer that receives the name of the value as a null-terminated string.
				&cchValue,  //A pointer to a variable that specifies the size of the buffer 
				NULL,		//This parameter is reserved and must be NULL.
				&valType,		//A pointer to a variable that receives a code indicating the type of data stored in the specified value.
				NULL,		//A pointer to a buffer that receives the data for the value entry.
				NULL		//A pointer to a variable that specifies the size of the buffer 
				);
			
			if(valType != REG_SZ)
			{
				//its not a string, so don't even try it.
				continue;
			}
			RadiusServer *server = new RadiusServer();
			std::vector<BYTE> value = ReadSingleStringValue(serverLocation,achValue);
			server->serverName = reinterpret_cast< TCHAR *>(&value[0]);

		
			
			MultiString wsz;
			wsz = server->serverName.GetBuffer();
			server->serverName.ReleaseBuffer();

			if(otpCacheMode)
			{
				/* Code for Offline caching*/

				
				DEBUG_LOG("OTPCheck::queryapi() Doing a cache enabled login\n");

				if(VerifyAgainstCache(wsz.getWChar(), useHTTP, totpCount, hotpCount, msUser->getWChar(), msOTP->getWChar(), authenticated, serverHit))
				{

					DEBUG_LOG("OTPCheck::queryapi() Stopping Mi-Token Cache Updater service as we managed to get a local connection\r\n");
					if(stopMiTokenCacheService())
					{
						DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was successfully stopped\r\n");
					}
					else
					{
						DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was not successfully stopped - see Verbose logs for more details\r\n");
					}
					OTPOffline_DeleteCacheIfDifferentUser(msUser->getWChar());
					return true;
				}
			}

			else
			{

				/* Verify against the server without any offline caching*/
				HINTERNET hConnect = NULL,
					hRequest = NULL, hSession = NULL;
				bool returnValue = false;

				
				std::string input;

				if(!UserRequires2FA(useDynamicPassword, wsz.getWChar(), useHTTP, msUser))
				{
					//user does not require 2FA, so return true
					return true;
				}
				//GetAPICallString will return true IFF we are done with everything.
				input = GetAPICallString(useDynamicPassword, wsz.getWChar(), useHTTP, msUser, msOTP);

				wchar_t* wszCopy = wsz.getWCharCopy(); //we need a non const wchar_t* for rc_event_log
				rc_event_log(2,1,1,wszCopy);
				free(wszCopy); //free the copy we just made

				DEBUG_LOGW(L"OTPCheck::queryapi() Server Name : %s\n", wsz.getWChar());

				bResults = GetAPICallResult(input, wsz.getWChar(), useHTTP, hSession, hConnect, hRequest);
				
				if(!useDynamicPassword)
				{
					if(bResults && VerifyAPIResultsNormalMode(hRequest))
					{
						bDone = true;
						returnValue = true;
					}
				}
				else
				{
					bool dynamicModeResult = false;
					if(bResults && VerifyAPIResultsDynamicPasswordMode(hRequest, dynamicModeResult))
					{
						//We still need to check dynamic result to see if this worked or not
						if(dynamicModeResult)
						{
							bDone = true;
							returnValue = true;
						}
						else
						{
							bDone = true;
							returnValue = false;
						}
					}
					
				}

				if( hRequest ) WinHttpCloseHandle( hRequest );
				if( hConnect ) WinHttpCloseHandle( hConnect );
				if( hSession ) WinHttpCloseHandle( hSession );

				if(bDone)
				{
					RegCloseKey(key);
					DEBUG_LOG("OTPCheck::queryapi() Stopping Mi-Token Cache Updater service as we managed to get a local connection\r\n");
					if(stopMiTokenCacheService())
					{
						DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was successfully stopped\r\n");
					}
					else
					{
						DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was not successfully stopped - see Verbose logs for more details\r\n");
					}

					if(returnValue)
					{
						OTPOffline_DeleteCacheIfDifferentUser(msUser->getWChar());
					}
					return returnValue;
				}
			}


			RegCloseKey(key);
		}

	}
	if((!serverHit) && (otpCacheMode))
	{	
		DEBUG_LOG("OTPCheck::queryapi() No server was ever hit, now checking the cache\n");
		::InterlockedExchange(&cacheUsed_g, 1L); //we are using the cache to verify.
		if(OTPOffline_Verify(msUser->getWChar(), msOTP->getWChar()))
		{
			DEBUG_LOG("OTPCheck::queryapi() User token existed in the cache, user is now authenticated\n");
			return true;
		}
		else
		{
			DEBUG_LOG("OTPCheck::queryapi() User token did not exist in the cache\n");
			authenticated = false;
		}
	}
	else
	{
		DEBUG_LOG("OTPCheck::queryapi() Stopping Mi-Token Cache Updater service as we managed to get a local connection or otpCacheMode is disabled.\r\n");
		if(stopMiTokenCacheService())
		{
			DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was successfully stopped\r\n");
		}
		else
		{
			DEBUG_LOG("OTPCheck::queryapi() Mi-Token Cache Updater service was not successfully stopped - see Verbose logs for more details\r\n");
		}
		DEBUG_LOG("OTPCheck::queryapi() One or more servers were hit (or not set to cache check mode), not checking the cache\n");
	}
	
	return authenticated;
	//return true;

}

bool convertToMultibyte(const WCHAR* src, char* dest, int length)
{
	errno_t err;
	size_t charsConverted;

	err = wcstombs_s(&charsConverted, dest, length, src, length);
	if((err != 0) || (charsConverted != length))
	{
		DEBUG_LOG("OTPCheck::convertToMultibyte() Error converting to multibyte\n");
		return false;
	}
	return true;
}

bool getMultibyteValue(const WCHAR* wchar, char** cstrout)
{
	if(wchar == NULL)
	{
		DEBUG_LOG("OTPCheck::getMultibyteValue() Get Multibyte Value error : wchar value is null\r\n");
		return false;
	}

	if(cstrout == NULL)
	{
		DEBUG_LOG("OTPCheck::getMultibyteValue() Get Multibyte Value error : cstrout value is null\r\n");
		return false;
	}

	int strlen = wcslen(wchar) + 1;
	*cstrout = new char[strlen];

	if(!convertToMultibyte(wchar, *cstrout, strlen))
	{
		DEBUG_LOG("OTPCheck::getMultibyteValue() Converting wchar to mbstr failed\n");
		free(*cstrout);
		return false;
	}

	return true;

}

bool getMultibyteValues(const WCHAR* username, const WCHAR* otp, char** mbUsername, char** mbOTP)
{
	DEBUG_VERBOSE("OTPCheck::getMultibyteValues() In GetMultiByteValues\n");
	int userlen, otplen;

	if((mbUsername == NULL) || (mbOTP == NULL))
	{
		DEBUG_LOG("OTPCheck::getMultibyteValues() Get Multibyte Values error : mbUsername or mbOTP is NULL\n");
		return false;
	}
	if((username == NULL) || (otp == NULL))
	{
		DEBUG_LOG("OTPCheck::getMultibyteValues() Get Multibyte Values error : Username or OTP is NULL\n");
		return false;
	}

	userlen = wcslen(username) + 1;
	otplen = wcslen(otp) + 1;

	*mbOTP = new char[otplen];
	*mbUsername = new char[userlen];

	if(!convertToMultibyte(username, *mbUsername, userlen))
	{
		DEBUG_LOG("OTPCheck::getMultibyteValues() Converting username to multibyte failed");
		//Dealloc this data - on fails getMultibyteValues should have not allocated anything
		free(*mbOTP);
		free(*mbUsername);
		return false;
	}

	if(!convertToMultibyte(otp, *mbOTP, otplen))
	{
		DEBUG_LOG("OTPCheck::getMultibyteValues() Convert OTP to multibyte failed");
		//Dealloc this data - on fails getMultibyteValues should have not allocated anything
		free(*mbOTP);
		free(*mbUsername);
		return false;
	}

	return true;
}

WCHAR* mergeStrings(WCHAR* str1, WCHAR* str2)
{
	int slen1, slen2;
	slen1 = wcslen(str1);
	slen2 = wcslen(str2);

	WCHAR* comb = (WCHAR*)calloc((slen1 + slen2) * 2 + 2, 1);
	memcpy(comb, str1, slen1 * 2);
	memcpy(&(comb[slen1]), str2, slen2 * 2);

	return comb;
}
WCHAR* properDomainName(WCHAR* domainAndUser)
{
	DEBUG_VERBOSEW(L"OTPCheck::properDomainName() domainAndUser = %s\r\n", domainAndUser);
	int slen = wcslen(domainAndUser);
	bool isDNS = false;
	WCHAR* ret = NULL;
	for(int i = 0 ; i < slen ; ++i)
	{
		if(domainAndUser[i] == L'\\')
		{
			DEBUG_VERBOSE("OTPCheck::properDomainName() Found '\\'. Not a DNS\r\n");
			break;
		}

		if(domainAndUser[i] == L'.')
		{
			DEBUG_VERBOSE("OTPCheck::properDomainName() Found '.'. A DNS\r\n");
			isDNS = true;
			break;
		}
	}

	if(isDNS)
	{
		DEBUG_VERBOSE("OTPCheck::properDomainName() Is a DNS - Finding Length");
		int dnsLen = 0;
		for(; dnsLen < slen ; ++dnsLen)
		{
			if(domainAndUser[dnsLen] == L'\\')
			{
				DEBUG_VERBOSE("OTPCheck::properDomainName() '\\' Found at position %d\r\n", dnsLen);
				break;
			}
		}
		WCHAR* domain = (WCHAR*)calloc(dnsLen * 2 + 2, 1);
		memcpy(domain, domainAndUser, dnsLen * 2);
		DOMAIN_CONTROLLER_INFO* pDCI;

		DEBUG_VERBOSEW(L"OTPCheck::properDomainName() Domain Name is : %s\r\n", domain);

		
		if(DsGetDcName(NULL, domain, NULL, NULL, DS_IS_DNS_NAME | DS_RETURN_FLAT_NAME, &pDCI) == ERROR_SUCCESS)
		{
			DEBUG_VERBOSE("OTPCheck::properDomainName() Found Flat Name from Domain Name\r\n");
			int flatlen = wcslen(pDCI->DomainName);
			DEBUG_VERBOSE("OTPCheck::properDomainName() Flat Name length = %d\r\n", flatlen);
			WCHAR* dom = (WCHAR*)calloc(flatlen * 2 + 2, 1);
			memcpy(dom, pDCI->DomainName, flatlen * 2);
			DEBUG_VERBOSE("OTPCheck::properDomainName() Domain Copied\r\n");
			DEBUG_VERBOSEW(L"OTPCheck::properDomainName() Domain = %s\r\n", dom);
			ret = mergeStrings(dom, &(domainAndUser[dnsLen]));

		}

		free(domain);
		DEBUG_VERBOSE("OTPCheck::properDomainName() DNS Freed\r\n");
	}

	if(ret == NULL)
	{
		DEBUG_VERBOSE("OTPCheck::properDomainName() Ret == NULL, Setting to DomainAndUser\r\n");
		ret = (WCHAR*)calloc(slen * 2 + 2, 1);
		memcpy(ret, domainAndUser, slen * 2);
	}

	DEBUG_VERBOSEW(L"OTPCheck::properDomainName() Domain Name : %s\r\n", ret);
	return ret;
}

bool checkOTP(WCHAR *username, WCHAR *otp, WCHAR* bypassCode, LPWSTR wMessage, int wMessageLength, WCHAR* domainAndUser)
{
	DEBUG_MESSAGE("checkOTP v1.0");

	DEBUG_SENSITIVEW(true, L"OTPCheck::checkOTP() Username Loc = %08lX, otp Loc = %08lX, bypass Loc = %08lX\n", username, otp, bypassCode);
	DEBUG_SENSITIVEW(true, L"OTPCheck::checkOTP() Username = %s\n", username);
	DEBUG_SENSITIVEW(true, L"OTPCheck::checkOTP() domainAndUser = %s\n", domainAndUser);
	DEBUG_SENSITIVEW(true, L"OTPCheck::checkOTP() otp = %s\n", otp);
	DEBUG_SENSITIVEW(true, L"OTPCheck::checkOTP() bypassCode = %s\n", bypassCode);

	UNREFERENCED_PARAMETER(wMessage);
	UNREFERENCED_PARAMETER(wMessageLength);

	//We will use the MultiString class for username / otp / domainAndUser do we don't have to worry about doing char*/wchar_t* conversions
	MultiString msUsername, msOTP, msDomainAndUser;
	msUsername = username;
	msOTP = otp;
	msDomainAndUser = domainAndUser;

	//Check if the otp is "1234" (Debug passthrough only)
	#ifdef POC	
		if(wcscmp(otp,L"123456789") == 0)
			return true;
	#endif
	
	if(checkOTPBypass(username, bypassCode, _T("Software\\Mi-Token\\Logon\\Bypass"), domainAndUser))
	{
		DEBUG_LOG("OTPCheck::checkOTP() bypass successful\n");
		return true;
	}
	
	//put actual radius call here
	bool OTPValid = false;
	DEBUG_LOG("OTPCheck::checkOTP() not using bypass\n");
	
	//char message[4096] = {0};
	/*

	char *multiByteOTP, *multiByteUserName;
	if(!getMultibyteValues(username, otp, &multiByteUserName, &multiByteOTP))
	{
		return OTPValid;
	}
	

	DEBUG_SENSITIVEW(true, L"Multibyte conversion input [Username] = %s\n", username);
	DEBUG_SENSITIVEW(true, L"Multibyte conversion input [OTP] = %s\n", otp);

	DEBUG_SENSITIVE(true, "Multibyte conversion output [Username] = %s\n", multiByteUserName);
	DEBUG_SENSITIVE(true, "Multibyte conversion output [OTP] = %s\n", multiByteOTP);
	
	DEBUG_SENSITIVE(true, "OTPCheck::checkOTP() Using MultiString for OTP/Username\n");
	*/
	
#ifdef API

	
	/*******************************************************
	API INTEGRATION:	
	*******************************************************/

	WCHAR* domName = properDomainName(domainAndUser);
	DEBUG_SENSITIVEW("OTPCheck::checkOTP() properDomainName(%s) returned domName[%s] **BUT ITS NOT USING THE RESULT FROM THIS CALL TO QUERY API****\n", domainAndUser, domName);

	DWORD ErrorCode = 0;
	LPCSTR ServerName = NULL;
	DEBUG_SENSITIVE(false, "OTPCheck::checkOTP() queryAPI \n");
	OTPValid = queryapi(&msOTP, &msUsername, &msDomainAndUser, &ErrorCode, ServerName); //(multiByteOTP,multiByteUserName, domName, &ErrorCode, ServerName);
	
	free(domName);
		
	
#else
	/*******************************************************
	RADIUS INTEGRATION:
	*******************************************************/
	int r = get_radius_reply(multiByteUserName, multiByteOTP);


	
	if(r == 0)
		OTPValid = true;
	else
	{
		decode_message(message, 4096, r);
		MultiByteToWideChar(CP_ACP, 0, message, -1, wMessage, wMessageLength);
	
	}


#endif

	if (!OTPValid)
	{
		DEBUG_LOG("OTPCheck::checkOTP() invalid OTP\n");
		char *message = "Failed to Authenticate, Bad token";
		MultiByteToWideChar(CP_ACP,0,message  ,-1,wMessage,wMessageLength);
	}

	//free allocated data - not needed, MultiStrings are freed in the class destructor
	return OTPValid;
}

SC_HANDLE getServiceHandle()
{
	SC_HANDLE scManager;
	scManager = OpenSCManager(NULL, NULL, SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
	if(scManager)
	{
		SC_HANDLE ret = OpenService(scManager, L"Mi-Token Client Cache Updater", SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
		CloseServiceHandle(scManager);
		return ret;
	}
	return NULL;

}

bool validateServiceState(SC_HANDLE handle, DWORD expectedState)
{
	SERVICE_STATUS_PROCESS data;
	DWORD bytesNeeded = 0;
	if(!QueryServiceStatusEx(handle, SC_STATUS_PROCESS_INFO, (LPBYTE)&data, sizeof(data), &bytesNeeded))
	{
		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			DEBUG_VERBOSE("OTPCheck::validateServiceState() Failed to Validate Service State - Returned INSUFFICIENT_BUFFER.\r\n\tBuffer Size was : %d\r\n\tNeeded Size was : %d\r\n", sizeof(data), bytesNeeded);
			return false;
		}
		DEBUG_VERBOSE("OTPCheck::validateServiceState() Failed to Validate Service State - GLE is %d\r\n", GetLastError());
		return false;
	}
	if(data.dwCurrentState == expectedState)
	{
		DEBUG_VERBOSE("OTPCheck::validateServiceState() Service State was the expected state");
		return true;
	}
	else
	{
		DEBUG_VERBOSE("OTPCheck::validateServiceState() Service State was not the expected state");
		return false;
	}

}

bool stopMiTokenCacheService()
{
	bool success = false;
	DEBUG_VERBOSE("OTPCheck::stopMiTokenCacheService() Stopping MiToken Cache Service\r\n");
	SC_HANDLE handle = getServiceHandle();
	if(handle == NULL)
	{
		DEBUG_VERBOSE("OTPCheck::stopMiTokenCacheService( Service could not be found. GLE is %d\r\n", GetLastError());
		return false;
	}
	if(validateServiceState(handle, SERVICE_RUNNING))
	{
		SERVICE_CONTROL_STATUS_REASON_PARAMS reasons;
		reasons.dwReason = SERVICE_STOP_REASON_FLAG_PLANNED | SERVICE_STOP_REASON_MAJOR_NONE | SERVICE_STOP_REASON_MINOR_NONE;
		reasons.pszComment = NULL;
		if(ControlServiceEx(handle, SERVICE_CONTROL_STOP, SERVICE_CONTROL_STATUS_REASON_INFO, (PVOID)&reasons))
		{
			DEBUG_VERBOSE("OTPCheck::stopMiTokenCacheService( Service has been stopped\r\n");
			success = true;
		}
		else
		{
			DEBUG_VERBOSE("OTPCheck::stopMiTokenCacheService( Service could not be stopped. GLE is %d\r\n", GetLastError());
		}
	}
	else
	{
		DEBUG_VERBOSE("OTPCheck::stopMiTokenCacheService( The service was not in a started state, and thus could not be stopped\r\n");
	}

	CloseServiceHandle(handle);
	return success;
}

bool cacheServiceEnabled()
{
	DEBUG_VERBOSE("OTPCheck::cacheServiceEnabled() Checking if Cache Service is enabled in the registry\r\n");
	bool bEnabled = false;
	HKEY RegKey;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon\\Config", &RegKey) == ERROR_SUCCESS)
	{
		DEBUG_VERBOSE("\tOTPCheck::cacheServiceEnabled() RegKey Opened\r\n");
		DWORD enabled;
		DWORD length = sizeof(enabled);
		if(RegQueryValueEx(RegKey, L"EnableOTPCaching", NULL, NULL, (LPBYTE)&enabled, &length) ==  ERROR_SUCCESS)
		{
			DEBUG_VERBOSE("\tOTPCheck::cacheServiceEnabled() Value Read\r\n");
			if(enabled == 1)
			{
				DEBUG_VERBOSE("\tOTPCheck::cacheServiceEnabled() Value Enabled\r\n");
				bEnabled = true;
			}
			else
			{
				DEBUG_VERBOSE("\tOTPCheck::cacheServiceEnabled() Value Disabled\r\n");
			}
		}

		RegCloseKey(RegKey);
	}

	DEBUG_VERBOSE("OTPCheck::cacheServiceEnabled() Returning %s\r\n", (bEnabled ? "TRUE" : "FALSE"));
	return bEnabled;
}

bool startMiTokenCacheService()
{
	if(!cacheServiceEnabled())
	{
		return false;
	}
	bool success = false;
	DEBUG_VERBOSE("OTPCheck::startMiTokenCacheService() Starting MiToken Cache Service\r\n");
	SC_HANDLE handle = getServiceHandle();
	if(handle == NULL)
	{
		DEBUG_VERBOSE("OTPCheck::startMiTokenCacheService() Service could not be found. GLE is %d\r\n", GetLastError());
		return false;
	}
	if(validateServiceState(handle, SERVICE_STOPPED))
	{
		if(!StartService(handle, 0, NULL))
		{
			DEBUG_VERBOSE("OTPCheck::startMiTokenCacheService() Could not start the service. GLE is %d\r\n", GetLastError());
			
		}
		else
		{
			DEBUG_VERBOSE("OTPCheck::startMiTokenCacheService() MiToken Cache Service Started\r\n");
			success = true;
		}
	}
	else
	{
		DEBUG_VERBOSE("OTPCheck::startMiTokenCacheService() The service was not in a stopped state, and thus could not be started");
	}

	CloseServiceHandle(handle);
	return success;

}


int _webResolveTime = 0, _webConnectTime = 0, _webSendTime = 0, _webReceiveTime = 0;

int webGetTimes()
{
	DEBUG_VERBOSE("OTPCheck:webGetTimes() Reading Web Times\n");

	DEBUG_VERBOSE("OTPCheck:webGetTimes() Setting to default times\n");
	//Defaults are 1000 for Resolve/Send/Receive, and 2000 for Connect
	_webResolveTime = _webSendTime = 1000;
	_webReceiveTime = 3000;
	_webConnectTime = 2000;


	HKEY regKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ, &regKey) == ERROR_SUCCESS)
	{
		DEBUG_VERBOSE("OTPCheck:webGetTimes() Key Opened\n");

		int* times[] = {&_webResolveTime, &_webConnectTime, &_webSendTime, &_webReceiveTime};
		LPWSTR values[] = {L"WebResolveTime", L"WebConnectTime", L"WebSendTime", L"WebReceiveTime" };
		

		for(int i = 0 ; i < 4  ; ++i)
		{
			DEBUG_VERBOSEW(L"OTPCheck:webGetTimes() Checking for value %s\n", values[i]);
			DWORD val = 0;
			DWORD valSize = sizeof(val);
			if(RegQueryValueEx(regKey, values[i], NULL, NULL, (LPBYTE)&val, &valSize) == ERROR_SUCCESS)
			{
				DEBUG_VERBOSEW(L"OTPCheck:webGetTimes() Overwriting time for %s with %d\n", values[i], val);
				*times[i] = val;
			}
		}

		RegCloseKey(regKey);
	}
	DEBUG_VERBOSE("OTPCheck:webGetTimes() Finished Reading Web Times\n");

	return 0;
}

int webResolveTime()
{
	if(_webResolveTime == 0)
	{
		webGetTimes();
	}
	return _webResolveTime;
}

int webConnectTime()
{
	if(_webConnectTime == 0)
	{
		webGetTimes();
	}
	return _webConnectTime;
}

int webSendTime()
{
	if(_webSendTime == 0)
	{
		webGetTimes();
	}
	return _webSendTime;
}

int webReceiveTime()
{
	if(_webReceiveTime == 0)
	{
		webGetTimes();
	}
	return _webReceiveTime;
}

bool cacheUsed()
{
	return ::InterlockedExchangeAdd(&cacheUsed_g, 0L) > 0;
}
