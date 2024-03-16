#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )

//Uncomment to include code that will allow for debugging of the UpdateRemoteCredential function
//#define DEBUG_UpdateRemoteCredential

#include <credentialprovider.h>
#include "Provider.h"
#include "Credential.h"
#include "guid.h"
#include <wincred.h>
#include <time.h>

#include "CredentialFilter.h"

bool isRDP;
long filterRDP;
long filterLocal;
long filterConnection;
bool rdpPassthroughEnabled;
bool rdpInfo;

static const GUID mGUID = 
{ 0xFFFD3EB9, 0x2E00, 0x44BC, { 0x8E, 0xC0, 0x22, 0x1A, 0x98, 0x4E, 0x42, 0xB6 } };


//Filters no CPs
#define FILTER_NONE 0
//Filters windows default CP
#define FILTER_DEFAULT 1
//Filters the Mi-Token CP
#define FILTER_MITOKEN 2
//Filters all BUT the Mi-Token CP
#define FILTER_ALL 3


bool ReadRegistryInt(const HKEY key, const TCHAR* keyName, int& value)
{
	DWORD len = sizeof(value);
	return (RegQueryValueEx(key, keyName, NULL, NULL, (unsigned char*)&value, &len) == ERROR_SUCCESS);
}

bool ReadRegistryIntAsBool(const HKEY key, const TCHAR* keyName, int trueValue)
{
	int value;
	return ((ReadRegistryInt(key, keyName, value)) && (value == trueValue));
}

CredentialFilter::CredentialFilter()
{
	HKEY subKey;

	DllAddRef();

	filterRDP = FILTER_DEFAULT; //turn off windows logon on remote
	filterLocal = FILTER_DEFAULT; //filter nothing on local connections
	rdpPassthroughEnabled = true;
	rdpInfo = false;

	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon", 0, KEY_READ, &subKey);
	if(result == ERROR_SUCCESS)
	{
		int value;
		if(ReadRegistryInt(subKey, "CPDebugMode", value))
		{
			debugLog = value;
		}

		if(ReadRegistryInt(subKey, "filterRDP", value))
		{
			filterRDP = value;
		}
		
		if(ReadRegistryInt(subKey, "filterLocal", value))
		{
			filterLocal = value;
		}
		if((ReadRegistryInt(subKey, "disableRDPPassthrough", value)) && (value > 0))
		{
			rdpPassthroughEnabled = false;
		}

		RegCloseKey(subKey);
	}

	DEBUG_LOG("Settings Info [V1.1.6] :\n\tDebug Mode : %d\n\tFilter RDP : %d\n\tFilter Local : %d\n\tRDP Passthrough : %s\n", debugLog, filterRDP, filterLocal, (rdpPassthroughEnabled ? "true" : "false"));
}

CredentialFilter::~CredentialFilter()
{
	DllRelease();
}

//Filters out non-wanted Credentials
//There are 2 current versions, either
//1- No default credential provier
//2- Only Mi-Token credential provider
IFACEMETHODIMP CredentialFilter::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders)
{
	//MessageBox(NULL, "Filter : Filtering", "TRACE", MB_OK);
	UNREFERENCED_PARAMETER(cpus);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(rgclsidProviders);
	UNREFERENCED_PARAMETER(cProviders);
	UNREFERENCED_PARAMETER(rgbAllow);

	DEBUG_MESSAGE("CredentialFilter::Filter");
	switch(cpus)
	{
	//only edit it if its a logon or unlock
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:
	case CPUS_CREDUI:
		filterConnection = rdpInfo ? filterRDP : filterLocal;
		//if(!isRDP)
		//	return S_OK; //don't filter anything
		switch(filterConnection)
		{
		case FILTER_NONE:
			DEBUG_MESSAGE("Filter Mode : ALLOW ALL");
			break;
		case FILTER_DEFAULT:
			DEBUG_MESSAGE("Filter Mode : DEFAULT");
			break;
		case FILTER_MITOKEN:
			DEBUG_MESSAGE("Filter Mode : DENY MI-TOKEN");
			break;
		case FILTER_ALL:
			DEBUG_MESSAGE("Filter Mode : ALLOW ONLY MI-TOKEN");
			break;
		}

		for(unsigned int i = 0 ; i < cProviders ; ++i)
		{
			//Turn off the default Password Credential Provider
			switch(filterConnection)
			{
			case FILTER_NONE:
				//don't filter anything
				break;
			case FILTER_DEFAULT:
				//filter out windows default CP
				if(IsEqualGUID(rgclsidProviders[i], CLSID_PasswordCredentialProvider))
					rgbAllow[i] = false;
				break;
			case FILTER_MITOKEN:
				//Filter only the mi-token one
				if(rgclsidProviders[i] == mGUID)
					rgbAllow[i] = false;
				break;
			case FILTER_ALL:
				//Filter all
				rgbAllow[i] = false;
				//Allow Mi-Token
				if(rgclsidProviders[i] == mGUID)
					rgbAllow[i] = true;
				break;
			}
		}

		break;
	}
	return S_OK;
}

#ifdef DEBUG_UpdateRemoteCredential
int HexToInt(char hex)
{
	if((hex >= '0') && (hex <= '9'))
		return hex - '0';
	if((hex >= 'A') && (hex <= 'F'))
		return hex - 'A' + 10;
	if((hex >= 'a') && (hex <= 'f'))
		return hex - 'a' + 10;
}
char* skipNewline(char* p)
{
	while((*p == '\n') || (*p == '\r'))
		p++;

	return p;
}

int grabHexBytes(char** pos, int bytesToGrab)
{
	int v = 0;
	char* p = *pos;
	for(int i = 0 ; i < bytesToGrab ; ++i)
	{
		v <<= 4;
		v += HexToInt(*p);
		p++;
	}
	*pos = p;
	return v;
}

int grabDecBytes(char** pos)
{
	int v = 0;
	char* p = *pos;
	while((*p >= '0') && (*p <= '9'))
	{
		v *= 10;
		v += (*p);
		p++;
	}

	*pos = p;
	return v;
}
void debugURC(void** data)
{
	FILE* fp;
	fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerDebug.log", "r+");
	if(fp)
	{
		char* temp;
		size_t fileSize;
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		temp = (char*)malloc(fileSize);
		fread(temp, 1, fileSize, fp);
		fclose(fp);
		remove("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerDebug.log");

		//now to parse the file
		CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION sout = {0};
		sout.ulAuthenticationPackage = 0;
		sout.ulAuthenticationPackage = grabHexBytes(&temp, 8);
		temp = skipNewline(temp);

		sout.clsidCredentialProvider.Data1 = grabHexBytes(&temp, 8);
		temp++;
		sout.clsidCredentialProvider.Data2 = grabHexBytes(&temp, 4);
		temp++;
		sout.clsidCredentialProvider.Data3 = grabHexBytes(&temp, 4);
		temp++;
		for(int i = 0 ; i < 8 ; ++i)
		{
			sout.clsidCredentialProvider.Data4[i] = grabHexBytes(&temp, 2);
		}
		temp = skipNewline(temp);

		sout.cbSerialization = grabDecBytes(&temp);
		temp = skipNewline(temp);
		sout.rgbSerialization = (byte *)HeapAlloc(GetProcessHeap(), 0, sout.cbSerialization);
		memcpy(sout.rgbSerialization, temp, sout.cbSerialization);

		*data = malloc(sizeof(sout));
		memcpy(*data, &sout, sizeof(sout));
	}
}
#endif

//RDP Data passthrough
IFACEMETHODIMP CredentialFilter::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut)
{
	UNREFERENCED_PARAMETER(pcpcsIn);
	UNREFERENCED_PARAMETER(pcpcsOut);
	DEBUG_MESSAGE("Filter : Update Remote Credential (V1.0.5)");

#ifdef DEBUG_UpdateRemoteCredential
	debugURC((void**)&pcpcsIn);
#endif
	if(pcpcsIn != NULL)
	{
		rdpInfo = true;
		//sometimes windows XP will send in a 0 byte pcpcsIn blob, so make sure there is actually data in it.
		if((rdpPassthroughEnabled == true) && (pcpcsIn->cbSerialization > 0))
		{
			isRDP = true;
			DEBUG_MESSAGE("Filter : Update Remote Credential pcpcsIn != NULL");
#ifdef ENABLE_LOGGING
			if((debugLog & (LOG_SENSITIVE | LOG_VERBOSE)) == (LOG_SENSITIVE | LOG_VERBOSE)) //require sensitive and verbose logging
			{
				FILE * fp;
				fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerialized.log", "a+");
				if(fp != NULL)
				{
					fprintf(fp, "[Auth Package] {CLSID}\nCB SER [PTR SER]\n{SER DATA}");
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
			DEBUG_MESSAGE("Filter : memcpy into pcpcsOut ");
			memcpy(pcpcsOut, pcpcsIn, sizeof(CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION));

			//MUST use HeapAlloc, WinLogon will do a HeapFree after Provider::SetSerialization
			pcpcsOut->rgbSerialization = (byte*)HeapAlloc(GetProcessHeap(), 0, pcpcsIn->cbSerialization);
			memcpy(pcpcsOut->rgbSerialization, pcpcsIn->rgbSerialization, pcpcsIn->cbSerialization);
			DEBUG_MESSAGE("Filter : Set CLSID");
			pcpcsOut->clsidCredentialProvider = CLSID_Provider;
		}
	}
	return S_OK;	
}


//Creates an instance of the CredentialFilter class
HRESULT CredentialFilter_CreateInstance(REFIID riid, void** ppv)
{
    HRESULT hr;

	//MessageBox(NULL, "FILTER->CREATE","TRACE",MB_OK);
    CredentialFilter* pFilter = new CredentialFilter();

    if (pFilter)
    {
        hr = pFilter->QueryInterface(riid, ppv);
        pFilter->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}
