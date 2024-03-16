#pragma once

#ifndef _MI_TOKEN_API_
#define _MI_TOKEN_API_

#include <Windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>


class MiTokenAPI
{
public:
	MiTokenAPI();
	MiTokenAPI(const char* clientConfigurationFile);

	char* Assign(char* username, char* tokenGUID);
	char* CloseSession();
	char* CreateSoftToken();
	char* CreateSoftToken(char* version);
	char* Delete(char* tokenGUID);
	char* Disable(char* tokenGUID);
	char* Enable(char* tokenGUID);
	char* List(char* username);
	char* Unassign(char* tokenGUID);
	std::string Verify(char* username, char* password, char* validate, char* usertype);
	char* SendSMS(char* phone);
	char* PhoneVerify(char* phone, char* otp);

	void* AnonGetGroupSID(const wchar_t* domain, const wchar_t* username, int& dataLength);
	void* AnonGetGroupSID(const char* domain, const char* username, int& dataLength);
	void* AnonVerifyOTPEx(char* username, char* otp, char* exponent, char* modulus, char* hotpSteps, char* totpSteps, char* flags);

	void* AnonGenerateBLEHash(unsigned char* macAddress, int& dataLength);
	void* AnonVerifyBLEHashResponse(unsigned char* macAddress, unsigned int UID, const unsigned char* hash, int& dataLength);


	void setServer(wchar_t* server);
	void setProtocolMode(bool useHTTP);

	static std::map<std::string, std::string> basicJSONParser(std::string jsonString);

	void (*debugMessage) (char* format, ...);

private:
	std::string getChallenge();
	std::string newSession(const char* response);

	bool HasClientCert;
	char* currentSession;
	unsigned char* clientKey;

	bool useHTTP;
	wchar_t* server;
	HINTERNET hSession;
	HINTERNET hConnect;
	HINTERNET hRequest;
	bool GetAPICallResult(const std::string input);
	void* GetAPICallData(int& DBLength);
	void UpdateSessionValue(const char* testedSessionKey);
	std::string UpdateSessionComputeResponse(std::string challenge);
	void UpdateSessionInner();
	
	static std::string parseJSONKey(std::string jsonString, int& readIndex);
	static std::string parseJSONValue(std::string jsonString, int& readIndex);
	bool sessionWasValid(std::string apiResult, const char* testedSessionKey);

	void doSessionAPICall(const std::string baseString, std::string& testedSession, std::string& data);

};
#endif