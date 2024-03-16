#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )


//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// Provider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we have decided to show two tiles, one for
// Administrator and one for Guest.  You will need to decide what
// tiles make sense for your situation.  Can you enumerate the
// users who will use your method to log on?  Or is it better
// to provide a tile where they can type in their username?
// Does the user need to interact with something other than the
// keyboard before you can recognize which user it is (such as insert 
// a smartcard)?  We call these "event driven" credential providers.  
// We suggest that such credential providers first provide one basic tile which
// tells the user what to do ("insert your smartcard").  Once the
// user performs the action, then you can callback into LogonUI to
// tell it that you have new tiles, and include a tile that is specific
// to the user that the user can then interact with if necessary.

#include <credentialprovider.h>
#include "OTPBypass.h"
#include "Provider.h"
#include "Credential.h"
#include "guid.h"
#include <wincred.h>
#include "CredentialFilter.h"
#include <time.h>

#include <BLE_API.h>
#include <MiTokenBLE_UUIDs.h>
#include "MiTokenAPI.h"
#include "base64_2.h"
#include "DebugLogging.h"


void MiTokenAPI_debugMessage (char* format, ...)
{
	va_list argp;
	va_start(argp, format);
	DEBUG_LOGLIST(format, argp);
	va_end(argp);
}


bool globalHasData = false;

bool allowRetryofBLE = true;

// Provider ////////////////////////////////////////////////////////

Provider::Provider():
    _cRef(1),
    _pkiulSetSerialization(NULL),
    _dwCredUIFlags(0),
    _bRecreateEnumeratedCredentials(true),
    _bAutoSubmitSetSerializationCred(false),
    _bDefaultToFirstCredential(false)
{
	filepath = "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Logon\\DebugCP.log";
	HKEY subKey = NULL;
	_pcpe = NULL;
	thread = INVALID_HANDLE_VALUE;
	hasData = false;
    DllAddRef();
	
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token", 0, KEY_READ, &subKey);
	if(result == ERROR_SUCCESS)
	{
		long value;
		DWORD len = sizeof(value);
		result = RegQueryValueEx(subKey, "CPDebugMode", NULL, NULL, (BYTE*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			debugLog = value;
			//if(value )
			//	debugLog = true;
		}
		RegCloseKey(subKey);
	}
	
	DEBUG_MESSAGE("Provder::Constructor\r\n");
	
	_readyMutex = CreateEvent(NULL, FALSE, FALSE, NULL);

#ifdef _DEBUG
	DEBUG_MESSAGE("---RUNNING DEBUG BUILD!---");
#endif
	DEBUG_MESSAGE("Provider constructor");
    ZeroMemory(_rgpCredentials, sizeof(_rgpCredentials));

	tokenControl = BLETokenLogin::getInstance();
	tokenControl->SetGotUserPassCallback(Provider::TokenFoundCallback, this);
}



Provider::~Provider()
{
	
	DEBUG_MESSAGE("Provider destructor");
    _ReleaseEnumeratedCredentials();
	delete tokenControl;
    DllRelease();
}

void Provider::_ReleaseEnumeratedCredentials()
{
	DEBUG_MESSAGE("Provider:Release Enumerated Credentials");
    for (int i = 0; i < ARRAYSIZE(_rgpCredentials); i++)
    {
        if (_rgpCredentials[i] != NULL)
        {
            _rgpCredentials[i]->Release();
        }
    }
}


// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
//
// This sample only handles the logon, unlock, and credui scenarios.
HRESULT Provider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags
    )
{
	DEBUG_MESSAGE("Provider: SetUsageScenario");
    HRESULT hr;

    _cpus = cpus;
    if (cpus == CPUS_CREDUI)
    {
		DEBUG_MESSAGE("Provider: SetUsageScenario CPUS_CREDUI");
        _dwCredUIFlags = dwFlags;  // currently the only flags ever passed in are only valid for the credui scenario
    }
    _bRecreateEnumeratedCredentials = true;

    // unlike SampleCredentialProvider, we're not going to enumerate here.  Instead, we'll store off the info
    // and then we'll wait for GetCredentialCount to enumerate.  That way we'll know at enumeration time
    // whether we have a SetSerialization cred to deal with.  That's a bit more important in the credUI case
    // than the logon case (although even in the logon case you could choose to only enumerate the SetSerialization
    // credential if there is one -- that's what the built-in password provider does).
    switch (cpus)
    {
    case CPUS_LOGON:
		DEBUG_MESSAGE("Provider: SetUsageScenario >LOGON");
    case CPUS_UNLOCK_WORKSTATION:
		DEBUG_MESSAGE("Provider: SetUsageScenario >UNLOCK");
    case CPUS_CREDUI:
		DEBUG_MESSAGE("Provider: SetUsageScenario >CPUS_CREDUI");
        hr = S_OK;
        break;

    case CPUS_CHANGE_PASSWORD:
		DEBUG_MESSAGE("Provider: SetUsageScenario >CPUS_CHANGE_PASS");
        hr = E_NOTIMPL;
        break;

    default:
		DEBUG_MESSAGE("Provider: SetUsageScenario >DEFAULT");
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//


void printCPCS(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn)
{
	DEBUG_MESSAGE("Provider: Print CPCS");
#ifdef ENABLE_LOGGING
	if(debugLog)
	{
		FILE* fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerialized.log", "a+");
		if(fp != NULL)
		{
			fprintf(fp, "printCPCS\r\n");
			fprintf(fp, "[%08lX] %08lX-%04lX-%04lX-", pcpcsIn->ulAuthenticationPackage, pcpcsIn->clsidCredentialProvider.Data1, pcpcsIn->clsidCredentialProvider.Data2, pcpcsIn->clsidCredentialProvider.Data3);
			for(unsigned int i = 0 ; i < 8;  ++i)
				fprintf(fp,"%02lX", pcpcsIn->clsidCredentialProvider.Data4[i]);
			fprintf(fp, "\r\n");
			fprintf(fp, "%d\t[%08lX]\r\n", pcpcsIn->cbSerialization, pcpcsIn->rgbSerialization);

			for(unsigned int i = 0 ; i < pcpcsIn->cbSerialization ; i++)
			{
				fprintf(fp, "%c", pcpcsIn->rgbSerialization[i]);
			}
			fprintf(fp,"\r\n\r\n");
			fclose(fp);
		}
	}
#endif
}
STDMETHODIMP Provider::SetSerialization(
    const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
	DEBUG_MESSAGE("Provider: SetSerialization");
    HRESULT hr = E_NOTIMPL;
	
	DEBUG_MESSAGE("Provider : SetSerialization");
    if ((CLSID_Provider == pcpcs->clsidCredentialProvider) || (CPUS_CREDUI == _cpus))
    {
        // Get the current AuthenticationPackageID that we are supporting
		DEBUG_MESSAGE("Provider : SetSerialization - CLSID = pcpcs");
        ULONG ulAuthPackage;
        hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

        if (SUCCEEDED(hr))
        {
            //if ((CPUS_CREDUI == _cpus) && (_dwCredUIFlags & CREDUIWIN_AUTHPACKAGE_ONLY) && 
             //   (ulAuthPackage == pcpcs->ulAuthenticationPackage))
			if( (CPUS_CREDUI == _cpus) )
            {
				DEBUG_MESSAGE("Provider : SetSerialization - CPUS_CREDUI = _cpus +more");
                // In the credui case, SetSerialization should only ever return S_OK if it is able to serialize the input cred.
                // Unfortunately, SetSerialization had to be overloaded to indicate whether or not it will be able to GetSerialization 
                // for the specific Auth Package that is being requested for CREDUIWIN_AUTHPACKAGE_ONLY to work, so when that flag is 
                // set, it should return S_FALSE unless it is ALSO able to serialize the input cred, then it can return S_OK.
                hr = S_FALSE;
				return hr;
            }
            if ((ulAuthPackage == pcpcs->ulAuthenticationPackage) &&
                (0 < pcpcs->cbSerialization && pcpcs->rgbSerialization))
            {
				DEBUG_MESSAGE("Provider : SetSerialization - pkil stuff [v2]");
				printCPCS(pcpcs);


                KERB_INTERACTIVE_UNLOCK_LOGON* pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*) pcpcs->rgbSerialization;
                if (KerbInteractiveLogon == pkil->Logon.MessageType)
                {
					DEBUG_MESSAGE("Provider : SetSerialization - KerbInteractiveLogon");
                    BYTE* rgbSerialization;
                    rgbSerialization = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pcpcs->cbSerialization);
                    HRESULT hrCreateCred = rgbSerialization ? S_OK : E_OUTOFMEMORY;

                    if (SUCCEEDED(hrCreateCred))
                    {
						DEBUG_MESSAGE("Provider : SetSerialization - hrCreateCred SUCCEED");
                        CopyMemory(rgbSerialization, pcpcs->rgbSerialization, pcpcs->cbSerialization);
                        KerbInteractiveLogonUnpackInPlace((KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization);

                        if (_pkiulSetSerialization)
                        {
                            HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
							DEBUG_MESSAGE("Provider : SetSerialization -> Let it be free");
                        }
						DEBUG_MESSAGE("Provider : SetSerialization - pre pkiulSetSerialization");
                        _pkiulSetSerialization = (KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization;
                        if (SUCCEEDED(hrCreateCred))
                        {
							DEBUG_MESSAGE("Provider : SetSerialization - hrCreateCted");
                            // we allow success to override the S_FALSE for the CREDUIWIN_AUTHPACKAGE_ONLY, but
                            // failure to create the cred shouldn't override that we can still handle
                            // the auth package
#ifdef ENABLE_LOGGING
							if(debugLog)
							{
								FILE* fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\RDC.log", "a+");
								DEBUG_MESSAGE("opening FP");
								if(fp != NULL)
								{
									char userstr[500];
									char passstr[500];
									char domainstr[500];
									memset(userstr, 0, 500);
									memset(passstr, 0, 500);
									memset(domainstr, 0, 500);

									DEBUG_MESSAGE("FP not null");
									DEBUG_MESSAGE("writing username");
									if((_pkiulSetSerialization->Logon.LogonDomainName.Length > 400) ||
										(_pkiulSetSerialization->Logon.Password.Length > 400) ||
										(_pkiulSetSerialization->Logon.UserName.Length > 400))
									{
										DEBUG_MESSAGE("WARNING : Length is over 400");
									}

									memcpy(userstr, _pkiulSetSerialization->Logon.UserName.Buffer, _pkiulSetSerialization->Logon.UserName.Length);
									memcpy(passstr, _pkiulSetSerialization->Logon.Password.Buffer, _pkiulSetSerialization->Logon.Password.Length);
									memcpy(domainstr, _pkiulSetSerialization->Logon.LogonDomainName.Buffer, _pkiulSetSerialization->Logon.LogonDomainName.Length);
									fprintf(fp, "Username [%d] : %ls\n", _pkiulSetSerialization->Logon.UserName.Length, userstr);
									fprintf(fp, "Password [%d] : %ls\n", _pkiulSetSerialization->Logon.Password.Length, passstr);
									fprintf(fp, "Domain [%d] : %ls\n", _pkiulSetSerialization->Logon.LogonDomainName.Length, domainstr);

									fclose(fp);

									
								}
							}
#endif

							if(_pkiulSetSerialization->Logon.Password.Length == 0)
							{
								HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
								_pkiulSetSerialization = NULL;
								isRDP = false;
							}
							//DO NOT FREE THE DATA - WinLogon does that without telling us...
							//Thank you Microsoft :/
							//HeapFree(GetProcessHeap(), 0, pcpcs->rgbSerialization);
							hr = hrCreateCred;
                        }
                    }
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

	DEBUG_MESSAGE("Exiting SetSerialization");
    return S_OK; //currently stops LogonUI from crashing, should look into
}


void BLECallback()
{
	globalHasData = true;
}

int minLoginRSSI = 0;

/*
bool NormalStageOutput(int response, int& oldResponse, const char* successMessage, const char* failMessage, int& stage, int nextStage)
{
	bool ret = false;
	if(response == BLE_CONN_SUCCESS)
	{
		printf("%s\r\n", successMessage);
		stage = nextStage;
		ret = true;
	}
	else
	{
		if(response != oldResponse)
		{
			printf("%s : error code [%d] [%s]\r\n", failMessage, response, "[ERR CODE TO STRING NOT IMPLEMENTED]");
		}
	}

	oldResponse = response;
	return ret;
}

int WriteDataToUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID, uint8 dataLength, uint8* dataPtr)
{
	int resp = BLE_CONN_SetAttribute(requestID, UUID, dataLength, dataPtr);
	NormalStageOutput(resp, lastResponse, "Writing data to UUID", "Cannot write to UUID", stage, nextStage);
	return resp;
}

int ScanServicesNearUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, UUID, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int StartReadAttribute(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID)
{
	int resp = BLE_CONN_StartGetAttribute(requestID, attributeUUID);
	NormalStageOutput(resp, lastResponse, "Start reading attribute", "Cannot start reading attribute yet", stage, nextStage);
	return resp;
}

int ReadAttributeBuffer(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID, uint8& readLength, uint8*& readBuffer)
{
	int resp = BLE_CONN_GetAttribute(requestID, attributeUUID, 0, readLength, readBuffer);
	if(NormalStageOutput(resp, lastResponse, "Read data length from attribute", "Cannot long read from attribute yet", stage, stage))
	{
		readBuffer = new uint8[readLength];
		resp = BLE_CONN_GetAttribute(requestID, attributeUUID, readLength, readLength, readBuffer);
		if(NormalStageOutput(resp, lastResponse, "Read data from attribute", "Error : 2nd attribute read failed : Weird ", stage, nextStage))
		{
			printf("Read %d bytes from handle [%04lX]\r\n", readLength, attributeUUID);
		}
	}
	return resp;
}

*/

bool deviceHash(unsigned char* macAddress, unsigned char*& hash){ return false; }
/*
bool deviceHash(unsigned char* macAddress, unsigned char*& hash)
{
	DEBUG_MESSAGE("Getting Device Hash\r\n");
	if(BLE_NP_RequestExclusiveAccess() != BLE_CONN_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to get ExclusiveAccess, failed\r\n");
		return false;
	}

	bool waitForExclusive = true;
	while(waitForExclusive)
	{
		Sleep(100);
		switch(BLE_NP_HasExclusiveAccess())
		{
		case BLE_API_EXCLUSIVE_ALLOWED:
		case BLE_API_NP_COM_FALLBACK:
			waitForExclusive = false;
			break;
		case BLE_API_EXCLUSIVE_REJECTED:
			DEBUG_MESSAGE("Exclusive Access was Rejected\r\n");
			return false;
		}
	}

	int requestID;
	mac_address ma;
	memcpy(&(ma.addr), macAddress, sizeof(ma));

	if(BLE_CONN_StartProfile(ma, requestID) != BLE_CONN_SUCCESS)
	{
		DEBUG_MESSAGE("Failed to start profile\r\n");
		BLE_NP_ReleaseExclusiveAccess();
		return false;
	}

	bool runMainLoop = true;
	int oldreq = requestID;

	int stage = 0;

	DEBUG_MESSAGE("Starting main loop");
	while(runMainLoop)
	{
		DEBUG_LOG(".");
		if(oldreq == BLE_CONN_BAD_REQUEST)
		{
			break;
		}

		Sleep(100);
		switch(stage)
		{
		case 0 :
			//Scan services in HASH block
			ScanServicesNearUUID(requestID, stage, oldreq, 1, BLE_UUID_SERVICE_HASH );
			break;

		case 1:
			WriteDataToUUID(requestID, stage, oldreq, 2, BLE_UUID_ATTRIBUTE_HASH_BUFFER, 20, hash);
			break;

		case 2:
			StartReadAttribute(requestID, stage, oldreq, 3, BLE_UUID_ATTRIBUTE_HASH_STATUS);
			break;

		case 3:
			{
				uint8 len = 0;
				uint8* readPtr = NULL;
				if(ReadAttributeBuffer(requestID, stage, oldreq, 2, BLE_UUID_ATTRIBUTE_HASH_STATUS, len, readPtr) == BLE_CONN_SUCCESS)
				{
					if(len == 1)
					{
						if(readPtr[0] == 0)
						{
							stage = 4;
						}
					}
					delete[] readPtr;
				}
			}
			break;

		case 4:
			StartReadAttribute(requestID, stage, oldreq, 5, BLE_UUID_ATTRIBUTE_HASH_BUFFER);
			break;

		case 5:
			{
				uint8 len = 0;
				uint8* ptr = NULL;
				if(ReadAttributeBuffer(requestID, stage, oldreq, 4, BLE_UUID_ATTRIBUTE_HASH_BUFFER, len, ptr) == BLE_CONN_SUCCESS)
				{
					if(len == 20)
					{
						DEBUG_MESSAGE("Hash is:\r\n");
						for(int i = 0 ; i < 20 ; ++i)
						{
							DEBUG_LOG("%02lX ", ptr[i]);
						}
						DEBUG_LOG("\r\n");
						memcpy(hash, ptr, 20);
						stage = 6;
					}
					delete[] ptr;
				}
			}
			break;

		case 6:
			runMainLoop = false;
			break;
		}

	}

	BLE_CONN_Disconnect(requestID);

	BLE_NP_ReleaseExclusiveAccess();

	return true;
	
}
*/



bool validateDevice(unsigned char* macAddress)
{
	static int hashSize = 20;

	//Get random hash from API servera
	DEBUG_MESSAGE("Getting Random hash from API Server\r\n");
	
	//TODO : Make actual API call
	unsigned char* hash = new unsigned char[hashSize];
	for(int i = 0 ; i < hashSize ; ++i)
	{
		hash[i] = rand();
	}

	
	MiTokenAPI API = MiTokenAPI();
	//API.debugMessage = MiTokenAPI_debugMessage;
	int respLength;

	bool useAPI = false;
	unsigned int UID = 0;

	//Set the server and use HTTPS
	DEBUG_LOG("CAUTION : Using Static Server IP Address, This will not work outside the dev environment!\r\n");
	API.setServer(L"192.168.112.45");
	API.setProtocolMode(false);
	
	
	DEBUG_LOG("Calling AnonGenerateBLEHash\r\n");
	void* resp = API.AnonGenerateBLEHash(macAddress, respLength);
	DEBUG_LOG("Responded with %d length, ptr @ 0x%08lX\r\n", respLength, resp);
	if(respLength != 0)
	{
		DEBUG_LOG("Recieved a response from the Mi-Token API, of length %d\r\n\t", respLength);
		DEBUG_BYTES((uint8*)resp, respLength);
		DEBUG_LOG("\r\n");

		std::string strresp = (char*)resp;
		
		std::map<std::string, std::string> keyval = API.basicJSONParser(strresp);

		//Debug MAP
		for(std::map<std::string, std::string>::iterator it = keyval.begin(); it != keyval.end(); ++it)
		{
			DEBUG_LOG("%s : %s\r\n", it->first.c_str(), it->second.c_str());
		}

		if(keyval.count("result") == 1)
		{
			if(keyval["result"] == "success")
			{
				if((keyval.count("hash") == 1) && (keyval.count("UID") == 1))
				{
					std::string hashB64 = keyval["hash"];
					std::string suid = keyval["UID"];

					//need to de-base64 the hashB64.
					DEBUG_LOG("Hash Info : \r\n\tUID = %s\r\n\tHash B64 = %s\r\n",
						suid.c_str(), hashB64.c_str());
					
					
					size_t pos;
					while(pos = hashB64.find("-"), pos != std::string::npos)
					{
						hashB64[pos] = '+';
					}
					while(pos = hashB64.find("_"), pos != std::string::npos)
					{
						hashB64[pos] = '/';
					}

					std::string binString = base64_decode(hashB64);

					memcpy(hash, binString.c_str(), hashSize);

					DEBUG_BYTES(hash, hashSize);
					
					useAPI = true;

					UID = 0;
					for(int i = 0 ; i < suid.length() ; ++i)
					{
						UID *= 10;
						UID += (suid[i] - '0');
						if((suid[i] < '0') || (suid[i] > '9'))
						{
							DEBUG_LOG("Error : Invalid SUID value at position %d, expected >= '0' and  <= '9', got %c\r\n", i, suid[i]);
						}
					}
				}
				
			}
		}
	}
	

	

	//Print hash
	DEBUG_MESSAGE("Hashing the following data\r\n\t");
	for(int i = 0 ; i < hashSize; ++i)
	{
		DEBUG_LOG("%02lX%s", hash[i], (i == (hashSize - 1) ? "" : " "));
	}

	//Ask device to perform hash
	if(!deviceHash(macAddress, hash))
	{
		return false;
	}

	

	DEBUG_MESSAGE("Data Hashed\r\nHash is as follows\r\n\t");
	//Print hash
	for(int i = 0 ; i < hashSize; ++i)
	{
		DEBUG_LOG("%02lX%s", hash[i], (i == (hashSize - 1) ? "" : " "));
	}

	if(useAPI)
	{
		DEBUG_LOG("Validating Hash with the Mi-Token API\r\n");
		std::string hashToValidateB64 = base64_encode(hash, hashSize);
		
		size_t pos;
		while(pos = hashToValidateB64.find('/'), pos != std::string::npos)
		{
			hashToValidateB64[pos] = '_';
		}
		while(pos = hashToValidateB64.find('+'), pos != std::string::npos)
		{
			hashToValidateB64[pos] = '-';
		}

		void* apiRespPtr = NULL;
		int apiRespLen = 0;
		apiRespPtr = API.AnonVerifyBLEHashResponse(macAddress, UID, (const unsigned char*)hashToValidateB64.c_str(), apiRespLen);
		DEBUG_LOG("Recieved a response from the Mi-Token API, of length %d\r\n\t", respLength);
		DEBUG_BYTES((uint8*)resp, respLength);
		DEBUG_LOG("\r\n");

		std::string strresp = (char*)apiRespPtr;
		
		std::map<std::string, std::string> keyval = API.basicJSONParser(strresp);

		//Debug MAP
		for(std::map<std::string, std::string>::iterator it = keyval.begin(); it != keyval.end(); ++it)
		{
			DEBUG_LOG("%s : %s\r\n", it->first.c_str(), it->second.c_str());
		}

	}


	return true;
	
}

void v2Poll(void* filterObject, int expectedPoll)
{
#ifndef REQUIRE_BLE_API_V2_1

	int devCount = BLE_GetDeviceFoundCount();
	DEBUG_LOG("V2 Poll : GDFC = %d\r\n", devCount);
	DeviceInfo devInfo;
	for(int i = 0 ; i < devCount ; ++i)
	{
		BLE_GetDeviceInfo(i, &devInfo);
		DEBUG_LOG("MAC : ");
		for(int j = 0 ; j < 6 ; ++j)
		{
			DEBUG_LOG("%02lX%s", devInfo.address[j], (j == 5 ? "" : ":"));
		}
		DEBUG_LOG("\r\n");
	}


	DEBUG_MESSAGE("Restart scan\r\n");
	BLE_RestartFilteredSearch(filterObject);
	bool found = false;
	unsigned char macAddressToCheck[6];
	char bestRSSI = -127;
	while(BLE_ContinueFilteredSearch(filterObject, &devInfo) == 0)
	{
		found = true;
		if((devInfo.RSSI > minLoginRSSI) && (devInfo.lastSeen == expectedPoll))
		{
			DEBUG_LOG("Device found, checking against current best\r\n");
			if(devInfo.RSSI > bestRSSI)
			{
				DEBUG_LOG("New best device found @ %02lX:%02lX:%02lX:%02lX:%02lX:%02lX\r\n", devInfo.address[0], devInfo.address[1], devInfo.address[2]
																							, devInfo.address[3], devInfo.address[4], devInfo.address[5]);
				memcpy(macAddressToCheck, devInfo.address, sizeof(devInfo.address));
				bestRSSI = devInfo.RSSI;
			}
		
		}
		else
		{
			DEBUG_LOG("Found device however the RSSI or LastSeen value is incorrect (RSSI, LastSeen) : (%d, %d) vs (%d, %d)\r\n", (unsigned int)devInfo.RSSI, (unsigned int)devInfo.lastSeen, (unsigned int)minLoginRSSI, (unsigned int)expectedPoll);
		}
	}

	if(found)
	{
		DEBUG_MESSAGE("Device Found - checking hash\r\n");
		if(!validateDevice(macAddressToCheck))
		{
			DEBUG_MESSAGE("Device failed validation\r\n");
		}
		else
		{
			DEBUG_MESSAGE("Device passed validation\r\n");
			globalHasData = true;

			//Send the message to the watcher so it knows which device to watch
			uint8 bdata[7] = { 0x03 , 0x00};
			memcpy(bdata + 1, macAddressToCheck, 6);
			BLE_NP_SendMessage(bdata, sizeof(bdata), -1);
		}
		
	}
	if(!found)
	{
		DEBUG_MESSAGE("No Devices Were Found\r\n");
	}
#endif

}



DWORD WINAPI ThreadFunction(void* vpData)
{
#ifndef REQUIRE_BLE_API_V2_1
	static int pollID = 0;
	static bool closeCOMIfOpen = true;

	Sleep(1000);
	//WaitForSingleObject(((Provider*)vpData)->_readyMutex, INFINITE);

	DEBUG_MESSAGE("Starting Thread (TL)\r\n");
	while(true)
	{
		pollID = time(NULL);
		BLE_SetPollID(pollID);

		Sleep(1000);
		DEBUG_MESSAGE("In Thread Loop (TL)\r\n");
		v2Poll(((Provider*)vpData)->BLEFilterObject, pollID);
		if(globalHasData)
		{
			((Provider*)vpData)->hasData = true;
			((Provider*)vpData)->OnConnectStatusChanged();
			break;
		}
		else
		{
			//Disconnect from the COM port if possible
			if(closeCOMIfOpen)
			{
				DEBUG_MESSAGE("Attempting to close COM port\r\n");
				int ret = BLE_ReleaseCOMPort();
				if(ret == BLE_API_NP_SUCCESS)
				{
					//COM was just closed - or was never connected
					closeCOMIfOpen = false;
					DEBUG_MESSAGE("COM Port closed - Using NP For comms\r\n");
					Sleep(1000); //wait a second to allow the server to setup the COM port again
					BLE_RestartScanner();
				}
				else if(ret == BLE_API_NP_ERR_NOT_CONNECTED)
				{
					DEBUG_MESSAGE("Cannot close COM Port : NP_ERR_NOT_CONNECTED\r\n");
				}
				else
				{
					DEBUG_LOG("Tried to close COM, Got unknown error code %d\r\n", ret);
				}
			}
		}
	}

#endif
	return 0;

}

void GetBLEInfo(char** ppCOMPort, unsigned char** authAddress, int& authSize, char& minRSSI, unsigned char** filterData, int& filterSize)
{
	DEBUG_LOG("Getting BLE Info\r\n");
	HKEY RegKey;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mi-Token\\BLE", &RegKey) == ERROR_SUCCESS)
	{
		DWORD addLength;
		DWORD byteLen;
		DEBUG_LOG("Getting COMPort Length\r\n");
		RegQueryValueEx(RegKey, "COMPort", NULL, NULL, NULL, &byteLen);
		*ppCOMPort = (char*)calloc(byteLen, 1);
		DEBUG_LOG("Getting COMPort Data\r\n");
		RegQueryValueEx(RegKey, "COMPort", NULL, NULL, (LPBYTE)*ppCOMPort, &byteLen);
		byteLen = 0;
		DEBUG_LOG("Getting Address Length\r\n");
		RegQueryValueEx(RegKey, "Address", NULL, NULL, NULL, &byteLen);
		addLength = byteLen;
		authSize = addLength;
		*authAddress = (unsigned char*)calloc(addLength, 1);
		DEBUG_LOG("Getting Address Data\r\n");
		RegQueryValueEx(RegKey, "Address", NULL, NULL, (LPBYTE)*authAddress, &byteLen);
		byteLen = 1;
		DEBUG_LOG("Getting LoginRSSI Data\r\n");
		RegQueryValueEx(RegKey, "LoginRSSI", NULL, NULL, (LPBYTE)&minRSSI, &byteLen);

		DEBUG_LOG("Getting Filter Length\r\n");
		RegQueryValueEx(RegKey, "FilterStream", NULL, NULL, NULL, &byteLen);
		filterSize = byteLen;
		*filterData = (unsigned char*)calloc(filterSize, 1);
		DEBUG_LOG("Getting Filter Data\r\n");
		RegQueryValueEx(RegKey, "FilterStream", NULL, NULL, (LPBYTE)*filterData, &byteLen);

		RegCloseKey(RegKey);

		DEBUG_LOG("BLEData:\r\n\tCOMPort : %s\r\n\tAddress [%d]: ", *ppCOMPort, addLength);
		for(int i  = 0 ; i < addLength ; ++i)
		{
			DEBUG_LOG("%02lX%s", (*authAddress)[i], (i == (addLength-1) ? "" : ":"));
		}
		DEBUG_LOG("\r\n\tMin RSSI : %d\r\n", minRSSI);
	}
	else
	{
		DEBUG_LOG("BLE Key could not be opened - using defaults\r\n");
		const char* defaultPort = "COM3";
		DEBUG_LOG("Alloc\r\n");
		*ppCOMPort = (char*)calloc(strlen(defaultPort) + 1, sizeof(char));
		DEBUG_LOG("Memcpy\r\n");
		memcpy(*ppCOMPort, defaultPort, sizeof(char) * (strlen(defaultPort) + 1));
		DEBUG_LOG("DefAdd\r\n");
		*authAddress = (unsigned char*)calloc(6, 1);
		unsigned char defaultAuthAddress[6] = {0x00, 0x17, 0xEA, 0x93, 0xCB, 0x42};
		authSize = 6;
		memcpy(*authAddress, &(defaultAuthAddress[0]), 6);

		DEBUG_LOG("DefFilter\r\n");
		*filterData = NULL;
		filterSize = 0;

		DEBUG_LOG("RSSI\r\n");
		minRSSI = -60;
	}
}

DWORD WINAPI SetupBLE(LPVOID lpParam);
bool requireResetOfCreds = false;


DWORD WINAPI WaitForNewThread(LPVOID lpParam)
{
	Sleep(1000);
	return SetupBLE(lpParam);
}



DWORD WINAPI SetupBLE(LPVOID lpParam)
{
	//no longer required hopefully
	return 0;
#ifndef REQUIRE_BLE_API_V2_1

	if(!allowRetryofBLE)
	{
		return 0;
	}
	Provider* pprov = (Provider*)lpParam;
	DEBUG_LOG("BLE Init\r\n");
	unsigned char* authAddress = NULL;
	unsigned char* filterData = NULL;
	int filterLength = 0;
	char* COMPort = NULL;
	char minRSSI;
	int authAddressSize;
	GetBLEInfo(&COMPort, &authAddress, authAddressSize, minRSSI, &filterData, filterLength);
	minLoginRSSI = minRSSI;
	if(BLE_InitializeEx(COMPort, "\\\\.\\pipe\\Mi-TokenBLE", false) == 0)
	{

		DEBUG_MESSAGE("Attempting to close COM port\r\n");
		int ret = BLE_ReleaseCOMPort();
		if(ret == BLE_API_NP_SUCCESS)
		{
			//COM was just closed - or was never connected
			DEBUG_MESSAGE("COM Port closed - Using NP For comms\r\n");
			Sleep(1000); //wait a second to allow the server to setup the COM port again
		}
		else if(ret == BLE_API_NP_ERR_NOT_CONNECTED)
		{
			DEBUG_MESSAGE("Cannot close COM Port : NP_ERR_NOT_CONNECTED\r\n");
		}
		else
		{
			DEBUG_LOG("Tried to close COM, Got unknown error code %d\r\n", ret);
		}

		//Force restart the scanner.
		DEBUG_LOG("Restarting the scanner\r\n");
		BLE_RestartScanner();

		if(filterData)
		{
			pprov->BLEFilterObject = BLE_ConvertByteStreamToFilteredSearch(filterData, filterLength);
		}
		else
		{
			pprov->BLEFilterObject = BLE_StartFilteredSearchEx(0, NULL, (authAddressSize / 6), authAddress);
		}

		int ctime = time(NULL);
		BLE_SetPollID(ctime);
		

		DEBUG_LOG("Create Thread\r\n");
		
		//BLE_Poll();

		Sleep(1000);
		v2Poll(pprov->BLEFilterObject, 1);
		if(globalHasData)
		{
			DEBUG_LOG("PreThread - HadData\r\n");
			pprov->hasData = true;
			if(requireResetOfCreds)
			{
				pprov->OnConnectStatusChanged();
			}
		}
		else
		{
			pprov->thread = CreateThread(NULL, 0, ThreadFunction, pprov, NULL, NULL);
		}
	}
	else
	{
		DEBUG_LOG("BLE could not connect to COMPort %s. Attempting again in 1 second\r\n", COMPort);
		CreateThread(NULL, 0, WaitForNewThread, pprov, NULL, NULL);
		requireResetOfCreds = true;
	}
	free(COMPort);
#endif
	return 0;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT Provider::Advise(
    ICredentialProviderEvents* pcpe,
    UINT_PTR upAdviseContext
    )
{

	allowRetryofBLE = true;
	if(_pcpe)
	{
		_pcpe->Release();
	}
	_pcpe = pcpe;
	_pcpe->AddRef();


	_upAdviseContext = upAdviseContext;

	DEBUG_MESSAGE("Provider: Advise");

	hasData = false;
	globalHasData = false;
	requireResetOfCreds = true;

	DEBUG_LOG("HasData value is %s\r\n", hasData ? "TRUE" : "FALSE");

	if(globalHasData != hasData)
	{
		DEBUG_LOG("Global Has Data != Has Data\r\n");
	}

	DEBUG_LOG("GHD : %s, HD : %s\r\n", (globalHasData ? "T" : "F"), hasData ? "T" : "F");

	if(!hasData) 
	{
		SetupBLE(this);
	}
	
    UNREFERENCED_PARAMETER(pcpe);
    UNREFERENCED_PARAMETER(upAdviseContext);

	return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT Provider::UnAdvise()
{
	allowRetryofBLE = false;
	if(_pcpe)
	{
		_pcpe->Release();
		_pcpe = NULL;
	}
	DEBUG_MESSAGE("Killing Thread (TL)\r\n");
	if(thread != INVALID_HANDLE_VALUE)
	{
		TerminateThread(thread, -1);
		thread = INVALID_HANDLE_VALUE;
	}

#ifndef REQUIRE_BLE_API_V2_1
	BLE_Finalize();
#endif

	thread = INVALID_HANDLE_VALUE;
	DEBUG_MESSAGE("Provider: UnAdvise");
    return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT Provider::GetFieldDescriptorCount(
    DWORD* pdwCount
    )
{
	DEBUG_MESSAGE("Provider: GetFieldDescriptorCount");
    *pdwCount = SFI_NUM_FIELDS;

    return S_OK;
}

// Gets the field descriptor for a particular field
HRESULT Provider::GetFieldDescriptorAt(
    DWORD dwIndex, 
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
	DEBUG_MESSAGE("Provider: GetFieldDescriptorAt");
    HRESULT hr;

    // Verify dwIndex is a valid field.
    if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
    {
        hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
    }
    else
    { 
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is the last cred prov
// get to specify the default tile.
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
HRESULT Provider::GetCredentialCount(
    DWORD* pdwCount,
    DWORD* pdwDefault,
    BOOL* pbAutoLogonWithDefault
    )
{
	DEBUG_MESSAGE("Provider: GetCredentialCount");
    HRESULT hr = E_FAIL;
    if (_bRecreateEnumeratedCredentials)
    {
		DEBUG_MESSAGE("Recreating Enumerated Creds");
        _ReleaseEnumeratedCredentials();
        hr = _CreateEnumeratedCredentials();
        _bRecreateEnumeratedCredentials = false;
    }

    *pdwCount = 0;
    *pdwDefault = 0; //(_bDefaultToFirstCredential && _rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
	DEBUG_LOG("pbAutoLogonWithDefault = %s\r\n", hasData ? "T" : "F");
    *pbAutoLogonWithDefault = hasData;

    if (SUCCEEDED(hr))
    {
        // TODO: it would probably be nicer to keep a count of the number of creds
		DWORD dwNumCreds = _rgpCredentials[0] ? 1 : 0; //we have a max of 1 credential, whihc is in _rgbCredentials[0]
		/*
        for (int i = 0; i < MAX_CREDENTIALS; i++)
        {
            if (_rgpCredentials[i] != NULL)
            {
                dwNumCreds++;
            }
        }
		*/

        switch(_cpus)
        {
        case CPUS_LOGON:
            if (_bAutoSubmitSetSerializationCred)
            {
                *pdwCount = 1;
                *pbAutoLogonWithDefault = TRUE;
				*pdwDefault = (_rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
            }
            else
            {
                
                *pdwCount = dwNumCreds;
				*pdwDefault = (_rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
                // since we have more than one tile and don't keep track of who logged on last, we don't really have a default in this case
            }
            hr = S_OK;
            break;

        case CPUS_UNLOCK_WORKSTATION:
            // in the unlock case, you likely would want to only enumerate tiles for the logged on user (that could be used to unlock)
            // but that's a bit complicated for a sample, so we'll just use our normal tiles
            // that we already set up in the logon case.  The default out params set up at the top work for this case.

			if(_rgpCredentials[0])
			{
				_rgpCredentials[0]->setCredType(KerbWorkstationUnlockLogon);
				*pdwDefault = 0;
			}
            *pdwCount = dwNumCreds;
            hr = S_OK;
            break;

        case CPUS_CREDUI:
            {
               *pdwCount = dwNumCreds;
                hr = S_OK;
            }
            break;

		default:
            hr = E_INVALIDARG;
            break;
        }
    }
    
    return S_OK; // hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT Provider::GetCredentialAt(
    DWORD dwIndex, 
    ICredentialProviderCredential** ppcpc
    )
{
	DEBUG_MESSAGE("Provider: GetCredentialAt");
    HRESULT hr;

    // Validate parameters.
    if((dwIndex < ARRAYSIZE(_rgpCredentials)) && _rgpCredentials[dwIndex] != NULL && ppcpc)
    {
        hr = _rgpCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    }
    else
    {
        hr = E_INVALIDARG;
    }
        
    return hr;
}

wchar_t* getDefaultUser()
{
	DEBUG_VERBOSE("Getting Default Username\n");
	wchar_t* output = NULL;
	HKEY regKey;
	LONG keyResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ, &regKey);

	if(keyResult == ERROR_SUCCESS)
	{
		DWORD result;
		DWORD userlen = 0;
		DWORD valType;
		result = RegQueryValueEx(regKey, "LastUserID", NULL, &valType, NULL, &userlen);

		DEBUG_VERBOSE("Attempted to get reg value length, result = %d bytes with retcode %d\n", userlen, result);
		if(result == ERROR_FILE_NOT_FOUND)
		{
			DEBUG_VERBOSE("Warning : RegValue does not exist\n");
		}
		else
		{
			char* cstr = (char*)calloc(userlen + 1, sizeof(char));
			result = RegQueryValueEx(regKey, "LastUserID", NULL, &valType, (LPBYTE)cstr, &userlen);
			
			DEBUG_VERBOSE("Attempted to get reg value, result = %08lX\n", result);
			
			userlen = strlen(cstr);
			output = (wchar_t*)calloc(userlen + 1, sizeof(wchar_t));
			mbstowcs(output, cstr, userlen);

		}
		RegCloseKey(regKey);
	}
	else
	{
		DEBUG_VERBOSE("Failed to update username : Could not open regkey\n");
	}

	return output;
}

// Creates a Credential with the SFI_USERNAME field's value set to pwzUsername.
HRESULT Provider::_EnumerateOneCredential(
    DWORD dwCredentialIndex,
    PCWSTR pwzUsername
    )
{
	DEBUG_MESSAGE("Provider: EnumerateOneCredential");
    HRESULT hr;

    // Allocate memory for the new credential.
    Credential* ppc = new Credential();
    
    if (ppc)
    {
        // Set the Field State Pair and Field Descriptors for ppc's fields
        // to the defaults (s_rgCredProvFieldDescriptors, and s_rgFieldStatePairs) and the value of SFI_USERNAME
        // to pwzUsername.
		HKEY subKey;
		WCHAR* _defaultDomain = NULL;
		if(!isRDP)
		{
			_defaultDomain = getDefaultUser();
			if(_defaultDomain == NULL)
			{
				LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI", 0, KEY_READ, &subKey);	
				if(result == ERROR_SUCCESS)
				{
					DWORD len = 0;
					result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, NULL, &len);
					if(result == ERROR_SUCCESS)
					{
						char* cbuffer = (char*)calloc(len, 1);
						DEBUG_LOG("LastLoggedOnSAMUser Buffer len = %d\n", len);
						result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, (LPBYTE)cbuffer, &len);
						if(result == ERROR_SUCCESS)
						{
							_defaultDomain = (wchar_t*)calloc(len, sizeof(wchar_t));

							DEBUG_LOG("LastLoggedOnSAMUser CBuffer [%s]\n", cbuffer);
							size_t charsConverted = mbstowcs(_defaultDomain, cbuffer, len);

							DEBUG_LOGW(L"LastLoggedOnSAMUser : %s Found [%d vs %d]\n[Byte Dump] : ", _defaultDomain, charsConverted, len);
							DEBUG_BYTES((unsigned char*)_defaultDomain, len * 2);
							DEBUG_LOG("\n");
						}
						free(cbuffer);
					}
				}
			}
		}
		
		hr = ppc->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, tokenControl->hasDevice(), _defaultDomain, NULL, NULL, tokenControl);
        
        if (SUCCEEDED(hr))
        {
            _rgpCredentials[dwCredentialIndex] = ppc;
        }
        else
        {
            // Release the pointer to account for the local reference.
            ppc->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

//
// Depending on whether SetSerialization has been called and what CPUS we're in
// creates the right set of credentials
//
HRESULT Provider::_CreateEnumeratedCredentials()
{
	DEBUG_MESSAGE("Provider: CreateEnumeratedCredentials");
    HRESULT hr = E_INVALIDARG;
    switch(_cpus)
    {
    case CPUS_LOGON:
        if (_pkiulSetSerialization)
        {
			DEBUG_MESSAGE("Provider: CEC : (_pkiulSS)");
            hr = _EnumerateSetSerialization();
        }
        else
        {
            hr = _EnumerateCredentials();
        }
        break;

    case CPUS_CHANGE_PASSWORD:
        break;

    case CPUS_UNLOCK_WORKSTATION:
        // a more advanced implementation would only enumerate tiles that could gather creds for the logged on user
        // since those are the only creds that will work to unlock the session
        // but we're going with this for simplicity
        hr = _EnumerateCredentials();  
        break;

    case CPUS_CREDUI:
        _bDefaultToFirstCredential = true;

        if (_pkiulSetSerialization)
        {
            hr = _EnumerateSetSerialization();
        }
        /*if (FALSE) //_dwCredUIFlags & CREDUIWIN_ADMINS_ONLY)
        {
            // this sample doesn't handle this particular case
            // You would want to handle this in order to participate
            // User Account Control elevations for a non-admin users (where all the
            // admins tiles are enumerated)
        }
		else */if (!(_dwCredUIFlags & CREDUIWIN_IN_CRED_ONLY))
        {
            // if we're here, then we're supposed to enumerate whatever we should enumerate for the normal case.  
            // In our case, that's our 2 tiles.  We may already have one tile, though
            if (_pkiulSetSerialization && SUCCEEDED(hr))
            {
                hr = _EnumerateCredentials(true);
            }
            else
            {
                hr = _EnumerateCredentials(false);
            }
        }
        break;

    default:
        break;
    }
    return hr;
}


// Sets up the normal 2 tiles for this provider (Administrator and Guest)
HRESULT Provider::_EnumerateCredentials(bool bAlreadyHaveSetSerializationCred)
{
	DEBUG_MESSAGE("Provider: EnumerateCredentials");
    DWORD dwStart = bAlreadyHaveSetSerializationCred ? 1 : 0;
    HRESULT hr = _EnumerateOneCredential(dwStart++, L"");
    return hr;
}

// This enumerates a tile for the info in _pkiulSetSerialization.  See the SetSerialization function comment for
// more information.
HRESULT Provider::_EnumerateSetSerialization()
{
	DEBUG_MESSAGE("Provider: EnumerateSetSerialization");
    KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;

    _bAutoSubmitSetSerializationCred = false;
    _bDefaultToFirstCredential = false;

    // Since this provider only enumerates local users (not domain users) we are ignoring the domain passed in.
    // However, please note that if you receive a serialized cred of just a domain name, that domain name is meant 
    // to be the default domain for the tiles (or for the empty tile if you have one).  Also, depending on your scenario,
    // the presence of a domain other than what you're expecting might be a clue that you shouldn't handle
    // the SetSerialization.  For example, in this sample, we could choose to not accept a serialization for a cred
    // that had something other than the local machine name as the domain.

    // Use a "long" (MAX_PATH is arbitrary) buffer because it's hard to predict what will be
    // in the incoming values.  A DNS-format domain name, for instance, can be longer than DNLEN.
    WCHAR wszUsername[MAX_PATH] = {0};
    WCHAR wszPassword[MAX_PATH] = {0};
	WCHAR wszDomain[MAX_PATH] = {0};

    // since this sample assumes local users, we'll ignore domain.  If you wanted to handle the domain
    // case, you'd have to update Credential::Initialize to take a domain.
    HRESULT hr = StringCbCopyNW(wszUsername, sizeof(wszUsername), pkil->UserName.Buffer, pkil->UserName.Length);

    if (SUCCEEDED(hr))
    {
        hr = StringCbCopyNW(wszPassword, sizeof(wszPassword), pkil->Password.Buffer, pkil->Password.Length);

        if (SUCCEEDED(hr))
        {
			hr = StringCbCopyNW(wszDomain, sizeof(wszDomain), pkil->LogonDomainName.Buffer, pkil->LogonDomainName.Length);
			if(SUCCEEDED(hr))
			{
				DEBUG_MESSAGE("Domain : ");
				DEBUG_WMESSAGE(wszDomain);
				DEBUG_MESSAGE("Username : ");
				DEBUG_WMESSAGE(wszUsername);
				DEBUG_MESSAGE("Password : ");
				DEBUG_WMESSAGE(wszPassword);

				Credential* pCred = new Credential();

				if (pCred)
				{
					HKEY subKey;
					WCHAR* domain;
					WCHAR* username;
					int flag_defaultDomain = 0;
					if(!isRDP)
					{
						LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI", 0, KEY_READ, &subKey);	
						if(result == ERROR_SUCCESS)
						{
							DWORD len = 0;
							result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, NULL, &len);
							if(result == ERROR_SUCCESS)
							{
								char* cbuffer = (char*)calloc(len, 1);
								DEBUG_LOG("LastLoggedOnSAMUser Buffer len = %d\n", len);
								result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, (LPBYTE)cbuffer, &len);
								if(result == ERROR_SUCCESS)
								{
									WCHAR* _defaultDomain;
									_defaultDomain = (wchar_t*)calloc(len, sizeof(wchar_t));

									DEBUG_LOG("LastLoggedOnSAMUser CBuffer [%s]\n", cbuffer);
									size_t charsConverted = mbstowcs(_defaultDomain, cbuffer, len);
									domain = wcstok(_defaultDomain, L"\\");
									username = wcstok(NULL, L"\\");

									DEBUG_LOGW(L"LastLoggedOnSAMUser : %s Found [%d vs %d]\n[Byte Dump] : ", _defaultDomain, charsConverted, len);
									flag_defaultDomain = 1;
									DEBUG_BYTES((unsigned char*)_defaultDomain, len * 2);
									DEBUG_LOG("\n");
								}
								free(cbuffer);
							}
						}
						DEBUG_LOGW(L"Here I am:: flag_defaultDomain: %d, Domain: \'%s\' and username: \'%s\'\n", flag_defaultDomain, domain, username);
					}

					if(flag_defaultDomain == 1)
						hr = pCred->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, username,	domain, wszPassword);
					else
						hr = pCred->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, wszUsername, wszDomain, wszPassword);

					if (SUCCEEDED(hr))
					{
						// for the purposes of this sample, when we enumerate the SetSerialization cred, we only enumerate
						// that cred and no others, so we can assume it just goes in slot 0.
						_rgpCredentials[0] = pCred;

						//if we were able to create a cred, default to it
						_bDefaultToFirstCredential = true;  
					}
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}

				// If we were passed all the info we need (in this case username & password), we're going to automatically submit this credential.
				// (if we're in CPUS_LOGON that is.  In credUI we want the user to at least click the tile to choose to use those creds)
				if (SUCCEEDED(hr) && (0 < wcslen(wszPassword)))
				{
					//_bAutoSubmitSetSerializationCred = true;
				}
			}
        }
    }

    return hr;
}

// Boilerplate code to create our provider.
HRESULT Provider_CreateInstance(REFIID riid, void** ppv)
{
	DEBUG_MESSAGE("Provider: CreateInstance");
    HRESULT hr;

    Provider* pProvider = new Provider();

    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}


void Provider::OnConnectStatusChanged()
{
	DEBUG_MESSAGE("Provider: OnConnectStatusChanged");
	if(_pcpe)
	{
		DEBUG_MESSAGE("Provider: CredentialsChanged");
		_bRecreateEnumeratedCredentials = true;
		_pcpe->CredentialsChanged(_upAdviseContext);
	}
}

void Provider::TokenFoundCallback(unsigned char * ig1, int ig2, unsigned char* ig3, int ig4, bool ig5, void* provider)
{
	Provider* prov = (Provider*)provider;
	prov->OnConnectStatusChanged();
}