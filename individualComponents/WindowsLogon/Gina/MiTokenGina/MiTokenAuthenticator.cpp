#include <windows.h>
#include <atlbase.h>
#include "MiTokenAuthenticator.h"
#include "..\..\freeradius-client\include\freeradius-client.h"
#include <windows.h>
#include <winhttp.h>
#include <string.h>
#include <atlbase.h>
#include <vector>
#include <malloc.h>

#include <Sddl.h>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")
#define MAX_NAME 256
#include "..\..\..\RADIUS\MiTokenOTPLib\oath.h"
#include "..\..\CredProvider\MiTokenCredentialProvider\OTPBypass.h"
#include "OTPCheck.h"
#define API TRUE
//#define POC TRUE
using namespace std;

//bool checkOTPBypass(CString username, CString OTP, CString keyPath);

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
				/*Success*/
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
	}
	catch(const std::exception &ex)
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



DWORD getNumServers(DWORD maxValue, DWORD maxData) {
	
	DWORD    cValues;              // number of values for key 
	HKEY key;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Mi-Token\\Logon"),0,KEY_ALL_ACCESS, &key);
	
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




std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

bool queryapi (char* OTP,char* UserName,DWORD* Error_Code, LPCSTR ServerR)
{
	//debug only
	//if (*OTP == '9')
	//	return true;		   
	char junk[10];
	bool authenticated =false;
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

	
	DWORD cValues = getNumServers(MAX_VALUE_NAME, MAX_KEY_LENGTH);	
	
	
	for (DWORD i = 0;i <cValues;	i++) {
		//just make a request to x.89.226
		ServerR = "192.168.89.226";
		HKEY key;
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Mi-Token\\Logon"),0,KEY_ALL_ACCESS, &key);

		TCHAR  achValue[MAX_VALUE_NAME]; 
		DWORD cchValue = MAX_VALUE_NAME; 
		cchValue = MAX_VALUE_NAME; 
		memset (achValue,'\0',MAX_VALUE_NAME * sizeof(TCHAR));
		DWORD retCode; 
		
		retCode = RegEnumValue(
			key, 
			i,			//The index of the value to be retrieved. 
			achValue,   //A pointer to a buffer that receives the name of the value as a null-terminated string.
			&cchValue,  //A pointer to a variable that specifies the size of the buffer 
			NULL,		//This parameter is reserved and must be NULL.
			NULL,		//A pointer to a variable that receives a code indicating the type of data stored in the specified value.
			NULL,		//A pointer to a buffer that receives the data for the value entry.
			NULL		//A pointer to a variable that specifies the size of the buffer 
			);
		
		RadiusServer *server = new RadiusServer();
		std::vector<BYTE> value = ReadSingleStringValue("Software\\Mi-Token\\Logon",achValue);
		server->serverName = reinterpret_cast< TCHAR *>(&value[0]);



		HINTERNET hConnect = NULL,
			hRequest = NULL, hSession = NULL;


		std::string input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyOTP?username="; 
			input.append(UserName);
			input.append("&otp=");
			input.append(OTP);
		
		CT2W wsz(server->serverName);
		rc_event_log(2,1,1,wsz);
		hSession = WinHttpOpen(L"Mi-Token API Client/1.0",WINHTTP_ACCESS_TYPE_NO_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS, 0);
		
		WinHttpSetTimeouts( hSession, 1000, 2000, 2000, 2000);

		hConnect = WinHttpConnect(hSession, wsz, INTERNET_DEFAULT_HTTPS_PORT, 0); 
		WinHttpSetTimeouts( hConnect, 1000, 2000, 2000, 2000);

		// null-call to get the size
		size_t needed = std::mbstowcs(NULL,&input[0],input.length());

		// allocate
		std::wstring output;
		output.resize(needed);

		// real call
		std::mbstowcs(&output[0],&input[0],input.length());

		if( hConnect ){
			hRequest = WinHttpOpenRequest( hConnect, L"GET", (LPCWSTR)output.c_str(),NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE );
			WinHttpSetTimeouts( hRequest, 1000, 2000, 2000, 2000);

		}

		// important - set security to ignore 'bad' certs (basically the api service is self signed which is bad)
		BOOL bRet = WinHttpSetOption(hRequest,WINHTTP_OPTION_SECURITY_FLAGS,&dwFlags,sizeof(DWORD));

		if( hRequest ){
			bResults = WinHttpSendRequest( hRequest,WINHTTP_NO_ADDITIONAL_HEADERS, 0,WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );
		}
		// End the request.
		if( bResults ) {
			bResults = WinHttpReceiveResponse( hRequest, NULL );
		} 

		if( bResults ){
			bResults = WinHttpQueryHeaders( hRequest, WINHTTP_QUERY_STATUS_CODE |WINHTTP_QUERY_FLAG_NUMBER,NULL, dwStatusCode, &dwSize, &headernonsense );
		} 

		if (bResults)
		{
			if (dwStatusCode[0] == HTTP_STATUS_OK )
			{
				bDone = TRUE;
				return true;
			}
		} 

		if( hRequest ) WinHttpCloseHandle( hRequest );
		if( hConnect ) WinHttpCloseHandle( hConnect );
		if( hSession ) WinHttpCloseHandle( hSession );

		RegCloseKey(key);
	}

	return authenticated;
	//return true;

}

BOOL MiTokenAuthenticator::VerifyOTP(LPWSTR username, LPWSTR otp, LPWSTR wMessage, int wMessageLength)
{
	/*
	//Debug Logging Registry Key
	HKEY subKey = NULL;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon", 0, KEY_READ, &subKey);
	if(result == ERROR_SUCCESS)
	{
		long value;
		DWORD len = sizeof(value);
		result = RegQueryValueEx(subKey, L"CPDebugMode", NULL, NULL, (BYTE*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			debugLog = value;
			//if(value == 1)
			//	debugLog = true;
		}
		RegCloseKey(subKey);
	}
	DEBUG_MESSAGE("This should work");

	BOOL OTPValid = FALSE;
	char message[4096] = {0};
	size_t charsConverted;
	errno_t errorNumber;

	char* multiByteOTP = new char[wcslen(otp) + 1];
	char* multiByteUserName = new char[wcslen(username) + 1];
#ifdef POC
	if(wcscmp(otp,L"123456789") == 0)
			return true;
#endif
	//wcstombs(multiByteOTP, otp, wcslen(otp) + 1);
	//wcstombs(multiByteUserName, username, wcslen(username) + 1);

	if(checkOTPBypass(username, otp, L"Software\\Mi-Token\\Logon\\Bypass", username))
		return true;
	
	errorNumber = wcstombs_s(&charsConverted, multiByteOTP, wcslen(otp) + 1, otp, wcslen(otp) + 1);
	if(errorNumber != 0 || charsConverted != wcslen(otp) + 1)
	{
		wMessage = L"Internal error: Error while converting OTP"; /*Ensure length is < wMessageLength*//*
		return OTPValid;
	}

	wcstombs_s(&charsConverted, multiByteUserName, wcslen(username) + 1, username, wcslen(username) + 1);
	if(errorNumber != 0 || charsConverted != wcslen(username) + 1)
	{
		wMessage = L"Internal error: Error while converting user name.";/*Ensure length is < wMessageLength*//*
		return OTPValid;
	}


#ifdef API
	DWORD ErrorCode = 0;
	LPCSTR ServerName = NULL;

	OTPValid = queryapi(multiByteOTP,multiByteUserName, &ErrorCode, ServerName);
		if (OTPValid == FALSE)
		{
			MultiByteToWideChar(CP_ACP,0,"Failed to Authenticate, Bad token"   ,-1,wMessage,wMessageLength);
		}

	/*******************************************************
	RADIUS INTEGRATION:
	*******************************************************//*
#else

	if(otp[0] == '9')
	{
		OTPValid = TRUE;
	}
	else
	{ 
		int r = get_radius_reply(multiByteUserName, multiByteOTP);
		decode_message(message, 4096, r);
		
		if(r == 0)
		{
			OTPValid = TRUE;
			//MultiByteToWideChar(CP_ACP, 0, message, -1, cachedUserName, wcslen);
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, message, -1, wMessage, wMessageLength);
		}
	}
#endif
	return OTPValid;
	*/

	return checkOTP(username, otp, wMessage, wMessageLength, username);

}