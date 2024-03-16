#include "MiTokenAPI.h"
#include "DataBuilder.h"

#include <atlstr.h>
#include <atlconv.h>
#include <fstream>

using namespace std;

int webResolveTimeAPI();
int webConnectTimeAPI();
int webSendTimeAPI();
int webReceiveTimeAPI();

MiTokenAPI::MiTokenAPI()
{
	HasClientCert = false;
	currentSession = NULL;
	useHTTP = false;
	server = NULL;
	debugMessage = NULL;
}

unsigned char hexCharToByte(char value)
{
	if((value >= '0') && (value <= '9'))
	{
		return value - '0';
	}

	if((value >= 'A') && (value <= 'F'))
	{
		return value - 'A' + 10;
	}
	if((value >= 'a') && (value <= 'f'))
	{
		return value - 'a' + 10;
	}

	return 0;
}

MiTokenAPI::MiTokenAPI(const char* clientConfigurationFile)
{
	HasClientCert = false;
	currentSession = NULL;
	useHTTP = false;
	server = NULL;

	fstream fileStream;
	fileStream.open(clientConfigurationFile, std::fstream::in);
	if(fileStream.is_open())
	{
		fileStream.seekg(0, fileStream.end);
		int length = fileStream.tellg();
		fileStream.seekg(0, fileStream.beg);

		char* lineBuffer = new char[length];

		while(!fileStream.eof())
		{
			fileStream.getline(lineBuffer, length);
			string lineData = lineBuffer;
			int pipePos = lineData.find('|');
			if(pipePos == string::npos)
			{
				//we only care about lines with pipes
				continue;
			}
			string key, value;
			key = lineData.substr(0, pipePos);
			value = lineData.substr(pipePos + 1);

			if(strcmp("Key", key.c_str()) == 0)
			{
				int valLength = value.length();
				clientKey = new unsigned char[valLength / 2];
				for(int i = 0  ; i < valLength ; i += 2)
				{
					clientKey[i / 2] = 0;
					clientKey[i / 2] += hexCharToByte(value.c_str()[i]) << 4;
					clientKey[i / 2] += hexCharToByte(value.c_str()[i + 1]);
				}
			}
			else if(strcmp("HostNames", key.c_str()) == 0)
			{
				server = new wchar_t[value.length() + 1];
				mbstowcs(server, value.c_str(), value.length());
				server[value.length()] = 0;
			}
		}
	}
}

bool MiTokenAPI::GetAPICallResult(std::string input)
{

	if(debugMessage != NULL)
	{
		debugMessage("Getting API Call (v1.0) Result of %s\r\n", input.c_str());
	}

	bool bResults;
	
	DWORD dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA;

	hSession = WinHttpOpen(L"Mi-Token API Client CPP/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts( hSession, webResolveTimeAPI(), webConnectTimeAPI(), webSendTimeAPI(), webReceiveTimeAPI());

	if(debugMessage != NULL)
	{
		debugMessage("Connecting\r\n");
	}
	hConnect = WinHttpConnect(hSession, server, (useHTTP ? 0 : INTERNET_DEFAULT_HTTPS_PORT /*18998*/), 0);
	WinHttpSetTimeouts(hConnect, webResolveTimeAPI(), webConnectTimeAPI(), webSendTimeAPI(), webReceiveTimeAPI());

	size_t needed = std::mbstowcs(NULL, &input[0], input.length());

	std::wstring output;
	output.resize(needed);

	std::mbstowcs(&output[0], &input[0], input.length());

	if(hConnect)
	{
		if(debugMessage != NULL)
		{
			debugMessage("Doing GET Request\r\n");
		}
		hRequest = WinHttpOpenRequest(hConnect, L"GET", (LPCWSTR)output.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (useHTTP ? 0 : WINHTTP_FLAG_SECURE) );
		WinHttpSetTimeouts(hRequest, webResolveTimeAPI(), webConnectTimeAPI(), webSendTimeAPI(), webReceiveTimeAPI());
	}
	else
	{
		if(debugMessage != nullptr)
		{
			debugMessage("Failed to Connect, GLE = %d\r\n",GetLastError());
		}
	}

	BOOL bRet = WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(DWORD));

	if( hRequest )
	{
		if(debugMessage != NULL)
		{
			debugMessage("Sending Request\r\n");
		}
		bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );
	}
	else
	{
		if(debugMessage != nullptr)
		{
			debugMessage("Failed to GET Request, GLE = %d\r\n",GetLastError());
		}
	}

	if(bResults)
	{
		if(debugMessage != NULL)
		{
			debugMessage("Reading Response\r\n");
		}
		bResults = WinHttpReceiveResponse( hRequest, NULL );
	}
	else
	{
		if(debugMessage != nullptr)
		{
			debugMessage("Failed to send Request, GLE = %d\r\n",GetLastError());
		}
	}

	return bResults;
}

void* MiTokenAPI::GetAPICallData(int& DBLength)
{
	void* retData = NULL;
	DataBuilder DB;
	DWORD bytesRead;
	char buffer[8096];

	while(true)
	{
		if(WinHttpReadData( hRequest, buffer, 8096, &bytesRead ))
		{
			if(bytesRead == 0)
			{
				DB.GetData(NULL, &DBLength);
				retData = malloc(DBLength);
				if(retData)
				{
					if(DB.GetData(retData, &DBLength))
					{
					}
					
				}
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

void MiTokenAPI::setServer(wchar_t* server)
{
	this->server = server;
}

void MiTokenAPI::setProtocolMode(bool useHTTP)
{
	this->useHTTP = useHTTP;
}

void* MiTokenAPI::AnonGetGroupSID(const char* domain, const char* username, int& dataLength)
{
	std::string input;
	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonGetGroupSID?domain=";

	

	input.append(domain);
	input.append("&username=");
	input.append(username);
	input.append("&debugMode=verbose");
	
	GetAPICallResult(input);

	dataLength = 0;
	void* data = GetAPICallData(dataLength);

	return data;
}

void* MiTokenAPI::AnonGetGroupSID(const wchar_t* domain, const wchar_t* username, int& dataLength)
{
	const char* cDomain = CW2A(domain);
	const char* cUsername = CW2A(username);

	return AnonGetGroupSID(cDomain, cUsername, dataLength);
}

char* MiTokenAPI::Assign(char* username, char* tokenGUID){ return NULL;}
char* MiTokenAPI::CloseSession(){ return NULL;}
char* MiTokenAPI::CreateSoftToken(){ return NULL;}
char* MiTokenAPI::CreateSoftToken(char* version){ return NULL;}
char* MiTokenAPI::Delete(char* tokenGUID){ return NULL;}
char* MiTokenAPI::Disable(char* tokenGUID){ return NULL;}
char* MiTokenAPI::Enable(char* tokenGUID){ return NULL;}
char* MiTokenAPI::List(char* username){ return NULL;}
char* MiTokenAPI::Unassign(char* tokenGUID){ return NULL;}
std::string MiTokenAPI::Verify(char* username, char* password, char* validate, char* usertype)
{
	std::string input;
	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/Verify?username=";
	input.append(username);
	input.append("&password=");
	input.append(password);
	input.append("&validate=");
	input.append(validate);
	input.append("&usertype=");
	input.append(usertype);

	std::string sessionKey = currentSession;
	std::string data;
	doSessionAPICall(input, sessionKey, data);


	if(!sessionWasValid(data, sessionKey.c_str()))
	{
		doSessionAPICall(input, sessionKey, data);
	}


	return data;

}

void* MiTokenAPI::AnonGenerateBLEHash(unsigned char* macAddress, int& dataLength)
{
	if(debugMessage != NULL)
	{
		debugMessage("In Anon Generate BLE Hash\r\n");
	}
	const unsigned char hexLut[] = {'0', '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	std::string input;
	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonGenerateBLEHash?macAddress=";
	char hexDigit[4] = {0};
	for(int i = 0;  i < 6 ; ++i)
	{
		unsigned char v = macAddress[i];
		unsigned char v1 = (v & 0xF0) >> 4;
		unsigned char v2 = (v & 0x0F);
		
		hexDigit[0] = hexLut[v1];
		hexDigit[1] = hexLut[v2];
		hexDigit[2] = (i != 5 ? ':' : '\0');
		hexDigit[3] = '\0';
		input.append(hexDigit);
	}

	if(debugMessage != NULL)
	{
		debugMessage("Getting API call result\r\n");
	}
	dataLength = 0;
	GetAPICallResult(input);

	if(debugMessage != NULL)
	{
		debugMessage("Getting API Call Data\r\n");
	}

	void* data = GetAPICallData(dataLength);


	if(debugMessage != NULL)
	{
		debugMessage("Returning\r\n");
	}
	return data;
}

void* MiTokenAPI::AnonVerifyBLEHashResponse(unsigned char* macAddress, unsigned int UID, const unsigned char* hash, int& dataLength)
{
	const unsigned char hexLut[] = {'0', '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	std::string input;
	input = "/mi-token/api/authentication/v4/RawHttpWithSSL/AnonVerifyBLEHashResponse?mac=";
	char hexDigit[4] = {0};
	for(int i = 0;  i < 6 ; ++i)
	{
		unsigned char v = macAddress[i];
		unsigned char v1 = (v & 0xF0) >> 4;
		unsigned char v2 = (v & 0x0F);
		
		hexDigit[0] = hexLut[v1];
		hexDigit[1] = hexLut[v2];
		hexDigit[2] = (i != 5 ? ':' : '\0');
		hexDigit[3] = '\0';
		input.append(hexDigit);
	}
	input.append("&UID=");
	char dest[30] = {0};
	itoa(UID, &(dest[0]), 10);
	input.append(dest);
	input.append("&hash=");
	input.append((char*)hash);


	GetAPICallResult(input);
	void* data = GetAPICallData(dataLength);

	return data;

}

char* MiTokenAPI::SendSMS(char* phone){ return NULL;}
char* MiTokenAPI::PhoneVerify(char* phone, char* otp){ return NULL;}

void* MiTokenAPI::AnonVerifyOTPEx(char* username, char* otp, char* exponent, char* modulus, char* hotpSteps, char* totpSteps, char* flags) { return NULL; }


string MiTokenAPI::parseJSONKey(string jsonString, int& readIndex)
{
	int sectionStart, sectionEnd;
	int bracketIndex, braceIndex;

	sectionStart = jsonString.find('\"', readIndex);
	braceIndex = jsonString.find('{', readIndex);
	bracketIndex = jsonString.find('[', readIndex);

	if(sectionStart == string::npos)
	{
		throw "Invalid JSON String - No Key Start";
	}
	if((braceIndex != string::npos) && (braceIndex < sectionStart))
	{
		throw "Invalid JSON String - Brace found before Key Start";
	}
	if((bracketIndex != string::npos) && (bracketIndex < sectionStart))
	{
		throw "Invalid JSON String - Bracket found before Key Start";
	}

	sectionStart++;
	sectionEnd = jsonString.find('\"', sectionStart);
	braceIndex = jsonString.find('{', readIndex);
	bracketIndex = jsonString.find('[', readIndex);

	if(sectionEnd == string::npos)
	{
		throw "Invalid JSON String - No Key End";
	}
	if((braceIndex != string::npos) && (braceIndex < sectionEnd))
	{
		throw "Invalid JSON String - Brace inside key";
	}
	if((bracketIndex != string::npos) && (bracketIndex < sectionEnd))
	{
		throw "Invlaid JSON String - Bracket inside key";
	}

	string key = jsonString.substr(sectionStart, sectionEnd - sectionStart);
	readIndex = sectionEnd + 1;

	return key;
}

std::string parseJSONValue(std::string jsonString,  int& readIndex)
{
	int sectionStart, sectionEnd;
	int bracketIndex, braceIndex;
	std::string value;

	sectionStart = jsonString.find('\"', readIndex);
	braceIndex = jsonString.find('{', readIndex);
	bracketIndex = jsonString.find('[', readIndex);

	if(sectionStart == string::npos)
	{
		throw "Invalid JSON String - No Value Start";
	}

	if(((bracketIndex != string::npos) && (bracketIndex < sectionStart)) || ((braceIndex != string::npos) && (braceIndex < sectionStart)))
	{
		char searchOpen, searchClose;
		int valueStart;
		if((bracketIndex != string::npos) && ((bracketIndex < braceIndex) || (braceIndex == string::npos)))
		{
			searchOpen = '[';
			searchClose = ']';
			valueStart = bracketIndex;
		}
		else
		{
			searchOpen = '{';
			searchClose = '}';
			valueStart = braceIndex;
		}

		int innerKeyStart, innerKeyEnd;
		int scopeLevel = 1;
		int searchStart = valueStart + 1;

		while(scopeLevel > 0)
		{
			innerKeyStart = jsonString.find(searchOpen, searchStart);
			innerKeyEnd = jsonString.find(searchClose, searchStart);
			if((innerKeyStart != string::npos) && ((innerKeyStart < innerKeyEnd) || (innerKeyEnd == string::npos)))
			{
				scopeLevel++;
				searchStart = innerKeyStart + 1;
			}
			else if(innerKeyEnd != string::npos)
			{
				scopeLevel--;
				searchStart = innerKeyEnd + 1;
			}
			else
			{
				throw "Invalid JSON String - Missing scope ending character";
			}
		}

		sectionEnd = searchStart;
		sectionStart = valueStart;
		readIndex = sectionEnd;
	}
	else
	{
		sectionStart++;
		sectionEnd = jsonString.find('\"', sectionStart + 1);
		if(sectionEnd == string::npos)
		{
			throw "Invalid JSON String - No Value End";
		}
	}

	value = jsonString.substr(sectionStart, sectionEnd - sectionStart);

	return value;
}

std::map<std::string, std::string> MiTokenAPI::basicJSONParser(std::string jsonString)
{
	int readIndex = 0;
	if(jsonString[readIndex] != '{')
	{
		throw "Invalid JSON Block - Expected \"{\"";
	}
	std::map<string, string> keyval;
	readIndex++;
	while(jsonString[readIndex] != '}')
	{
		string key, value;
		key = parseJSONKey(jsonString, readIndex);
		value = parseJSONValue(jsonString, readIndex);
		keyval.insert( std::pair<string, string> (key, value) );
		while(jsonString[readIndex] == ' ')
		{
			readIndex++;
		}
	}

	return keyval;
}

void MiTokenAPI::UpdateSessionValue(const char* testedSessionValue)
{
	bool requireUpdate = false;
	if(testedSessionValue == NULL)
	{
		requireUpdate = true;
	}
	else
	{
		if(strcmp(testedSessionValue, currentSession) == 0)
		{
			requireUpdate = true;
		}
	}

	if(requireUpdate)
	{
		UpdateSessionInner();
	}
}

std::string MiTokenAPI::UpdateSessionComputeResponse(std::string challenge)
{
	return challenge;
}

void MiTokenAPI::UpdateSessionInner()
{
	std::string apiResult;
	apiResult = this->getChallenge();
	std::map<std::string, std::string> challengeKeyVal = MiTokenAPI::basicJSONParser(apiResult);
	if(challengeKeyVal.count("result") == 1)
	{
		if(strcmp(challengeKeyVal["result"].c_str(), "success") == 0)
		{
			if(challengeKeyVal.count("details") == 1)
			{
				std::string base64ChallengeString = challengeKeyVal["details"];
				std::string base64ResponseString = UpdateSessionComputeResponse(base64ChallengeString);
				apiResult = newSession(base64ResponseString.c_str());
				std::map<std::string, std::string> sessionKeyVal = MiTokenAPI::basicJSONParser(apiResult);
				if(sessionKeyVal.count("result") == 1)
				{
					if(strcmp(sessionKeyVal["result"].c_str(), "success") == 0)
					{
						if(sessionKeyVal.count("details") == 1)
						{
							if(currentSession)
							{
								free(currentSession);
							}
							int valLength = sessionKeyVal["details"].length();
							currentSession = (char*)calloc(valLength + 1, 1);
							memcpy(currentSession, sessionKeyVal["details"].c_str(), valLength);
							return;
						}
					}
				}
			}
		}
	}
}


bool MiTokenAPI::sessionWasValid(std::string apiResult, const char* testedSessionKey)
{
	std::map<std::string, std::string> keyval = MiTokenAPI::basicJSONParser(apiResult);
	if(keyval.count("result") == 1)
	{
		if(strcmp(keyval["result"].c_str(), "unauthorized") == 0)
		{
			//update the session and return false so the callee knows to retry the API call
			UpdateSessionValue(testedSessionKey);
			return false;
		}
	}
	return true;
}

void MiTokenAPI::doSessionAPICall(const std::string baseString, std::string& testedSession, std::string& data)
{}

std::string MiTokenAPI::parseJSONValue(std::string jsonString, int& readIndex)
{
	int sectionStart, sectionEnd;
	int bracketIndex, braceIndex;

	sectionStart = jsonString.find('\"', readIndex);
	braceIndex = jsonString.find('{', readIndex);
	bracketIndex = jsonString.find('[', readIndex);

	if(sectionStart == -1)
	{
		throw "Invalid JSON String - no value start";
	}

	if(((bracketIndex != -1) && (bracketIndex < sectionStart)) || ((braceIndex != -1) && (braceIndex < sectionStart)))
	{
		char searchOpen, searchClose;
		int valueStart;
		if((bracketIndex != -1) && ((bracketIndex < braceIndex) || (braceIndex == -1)))
		{
			searchOpen = '[';
			searchClose = ']';
			valueStart = bracketIndex;
		}
		else
		{
			searchOpen = '{';
			searchClose = '}';
			valueStart = braceIndex;
		}

		int innerKeyStart, innerKeyEnd;
		int scopeLevel = 1;
		int searchStart = valueStart + 1;

		while(scopeLevel > 0)
		{
			innerKeyStart = jsonString.find(searchOpen, searchStart);
			innerKeyEnd = jsonString.find(searchClose, searchStart);

			if((innerKeyStart != -1) && ((innerKeyStart < innerKeyEnd) || (innerKeyEnd == -1)))
			{
				scopeLevel++;
				searchStart = innerKeyStart + 1;
			}
			else if(innerKeyEnd != -1)
			{
				scopeLevel--;
				searchStart = innerKeyEnd + 1;
			}
			else
			{
				throw "Invalid JSON String - Mismatched number of scopes";
			}
		}

		sectionEnd = searchStart;
		sectionStart = valueStart;
		readIndex = sectionEnd;
	}
	else
	{
		sectionStart++;
		sectionEnd = jsonString.find('\"', sectionStart);
		if(sectionEnd == -1)
		{
			throw "Invalid JSON String - No Value End";
		}

		readIndex = sectionEnd + 1;
	}

	return jsonString.substr(sectionStart, sectionEnd - sectionStart);
}

std::string MiTokenAPI::newSession(const char* response)
{
	return "";
}

std::string MiTokenAPI::getChallenge()
{
	return "";
}


int _webResolveTimeAPI = 0, _webConnectTimeAPI = 0, _webSendTimeAPI = 0, _webReceiveTimeAPI = 0;

int webGetTimesAPI()
{
	//Defaults are 1000 for Resolve/Send/Receive, and 2000 for Connect
	_webResolveTimeAPI = _webSendTimeAPI = _webReceiveTimeAPI = 1000;
	_webConnectTimeAPI = 2000;


	HKEY regKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Mi-Token\\API\\Config", 0, KEY_READ, &regKey) == ERROR_SUCCESS)
	{
		int* times[] = {&_webResolveTimeAPI, &_webConnectTimeAPI, &_webSendTimeAPI, &_webReceiveTimeAPI};
		LPWSTR values[] = {L"WebResolveTime", L"WebConnectTime", L"WebSendTime", L"WebReceiveTime" };
		

		for(int i = 0 ; i < 4  ; ++i)
		{
			DWORD val = 0;
			DWORD valSize = sizeof(val);
			if(RegQueryValueEx(regKey, values[i], NULL, NULL, (LPBYTE)&val, &valSize) == ERROR_SUCCESS)
			{
				*times[i] = val;
			}
		}

		RegCloseKey(regKey);
	}

	return 0;
}

int webResolveTimeAPI()
{
	if(_webResolveTimeAPI == 0)
	{
		webGetTimesAPI();
	}
	return _webResolveTimeAPI;
}

int webConnectTimeAPI()
{
	if(_webConnectTimeAPI == 0)
	{
		webGetTimesAPI();
	}
	return _webConnectTimeAPI;
}

int webSendTimeAPI()
{
	if(_webSendTimeAPI == 0)
	{
		webGetTimesAPI();
	}
	return _webSendTimeAPI;
}

int webReceiveTimeAPI()
{
	if(_webReceiveTimeAPI == 0)
	{
		webGetTimesAPI();
	}
	return _webReceiveTimeAPI;
}
