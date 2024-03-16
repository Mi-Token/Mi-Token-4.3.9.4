#include "BLEDLLV2.h"

#include "BLE_API.h"
#include "DLLWrapper.h" //used to make certain DLL calls easier

#include "DebugLogging.h"


//All COM stuff for locking workstations is able to be disabled with USE_COM
//#define USE_COM

#ifdef USE_COM
#define _WIN32_DCOM
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
#include <wincred.h>
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "credui.lib")

using namespace std;
#endif

//#define DUMMY_DLL



#ifdef DUMMY_DLL
//If we have a DUMMY DLL, Set the MAG version of DWORD_MAX
#undef DLL_VERSION_MAG
#define DLL_VERSION_MAG (0x7FFF)
#endif

#define DLL_Debug_Path "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Logon\\Debug_DLL.log"

#ifndef REQUIRE_BLE_API_V2_1

BLEDLLV2_API int WINAPI API_Initialize(const char* COMPort)
{
	filepath = DLL_Debug_Path;
	return BLE_Initialize(COMPort);
}

BLEDLLV2_API int WINAPI API_InitializeEx(const char* COMPort, const char* NamedPipePort, bool isServer)
{
	filepath = DLL_Debug_Path;

#ifdef DUMMY_DLL
	return 0;
#endif
	return BLE_InitializeEx(COMPort, NamedPipePort, isServer);
}

BLEDLLV2_API int WINAPI API_ConnectedToCOM()
{
#ifdef DUMMY_DLL
	return 1;
#endif
	return BLE_ConnectedToCOM();
}

BLEDLLV2_API int WINAPI API_ReInitCOM(const char* COMPort)
{
#ifdef DUMMY_DLL
	return 0;
#endif
	return BLE_ReinitCOM(COMPort);
}

BLEDLLV2_API int WINAPI API_Finalize()
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_Finalize();
}

BLEDLLV2_API int WINAPI API_COMOwnerKnown()
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_COMOwnerKnown();
}

BLEDLLV2_API int WINAPI API_WaitForCOMToBeFree()
{
#ifdef DUMMY_DLL
	return 1;
#endif

	HANDLE eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	BLE_SetCOMWaitHandle(eventHandle);
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
	return BLE_CONN_SUCCESS;
}

BLEDLLV2_API int WINAPI API_ReleaseCOMPort()
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_ReleaseCOMPort();
}

BLEDLLV2_API void WINAPI API_SetPollID(int newPollID)
{
#ifdef DUMMY_DLL
	return;
#endif

	return BLE_SetPollID(newPollID);
}

BLEDLLV2_API int WINAPI API_GetDeviceFoundCount()
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_GetDeviceFoundCount();
}

BLEDLLV2_API int WINAPI API_GetDeviceInfo(int deviceID, unsigned char* deviceInfo)
{
#ifdef DUMMY_DLL
	return 0;
#endif

	DeviceInfo devInfo;
	int retCode = 0;
	int i;

	retCode = BLE_GetDeviceInfo(deviceID, &devInfo);
	if(retCode != 0)
	{
		return retCode;
	}

	/*
		deviceInfo format
			6 byte Address (Opposite endianness to the way it is stored in DeviceInfo struct)
			1 byte RSSI
			4 byte 'last seen ID'
	*/

	return ConvertDeviceIntoToUChar(devInfo, deviceInfo);
}

BLEDLLV2_API int WINAPI API_GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength)
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_GetDeviceMetaInfo(deviceID, flag, buffer, bufferLength);
}

BLEDLLV2_API int WINAPI API_RestartScanner()
{
#ifdef DUMMY_DLL
	return 0;
#endif

	return BLE_RestartScanner();
}

BLEDLLV2_API FilteredSearchObject WINAPI API_StartFilteredSearch(FinalizedFilterObject* filterToUse)
{
	return BLE_StartFilteredSearchEx(filterToUse->count, filterToUse->filters, filterToUse->addressCount, filterToUse->addresses);
}

BLEDLLV2_API int WINAPI API_ConvertFilteredSearchToByteStream(FilteredSearchObject searcher, uint8* byteStream, int* length)
{
	return BLE_ConvertFilteredSearchToByteStream(searcher, byteStream, *length);
}

BLEDLLV2_API FilteredSearchObject WINAPI API_ConvertByteStreamToFilteredSearch(uint8* byteStream, int length)
{
	return BLE_CovnertByteStreamToFilteredSearch(byteStream, length);
}

BLEDLLV2_API int WINAPI API_ContinueFilteredSearch(FilteredSearchObject searcher, unsigned char* deviceInfo)
{
	DeviceInfo devInfo;
	int retCode = BLE_ContinueFilteredSearch(searcher, &devInfo);
	
	if(retCode != 0)
	{
		return retCode;
	}

	return ConvertDeviceIntoToUChar(devInfo, deviceInfo);
}

BLEDLLV2_API void WINAPI API_RestartFilteredSearch(FilteredSearchObject searcher)
{
	BLE_RestartFilteredSearch(searcher);
}

BLEDLLV2_API void WINAPI API_FinishedFilteredSearch(FilteredSearchObject searcher)
{
	BLE_FinishFilteredSearch(searcher);
}

BLEDLLV2_API int WINAPI CONN_StartProfile(char* address, int* requestID)
{
	mac_address bdaddress;
	
	for(int i = 0 ; i < 6 ; ++i)
	{
		bdaddress.addr[i] = address[i];
	}

	return BLE_CONN_StartProfile(bdaddress, *requestID);
}

BLEDLLV2_API int WINAPI CONN_IsConnected(int requestID)
{
	return BLE_CONN_IsConnected(requestID);
}

BLEDLLV2_API int WINAPI CONN_ScanServicesInRange(int requestID, uint16 handle, uint16 maxRange)
{
	return BLE_CONN_ScanServicesInRange(requestID, handle, maxRange);
}

BLEDLLV2_API int WINAPI CONN_SetAttribute(int requestID, uint16 handle, uint8 length, const uint8* data)
{
	return BLE_CONN_SetAttribute(requestID, handle, length, data);
}

BLEDLLV2_API int WINAPI CONN_StartGetAttribute(int requestID, uint16 handle)
{
	return BLE_CONN_StartGetAttribute(requestID, handle);
}

BLEDLLV2_API int WINAPI CONN_GetAttribute(int requestID, uint16 handle, uint8 maxLength, uint8* length, uint8* outData)
{
	return BLE_CONN_GetAttribute(requestID, handle, maxLength, *length, outData);
}

BLEDLLV2_API int WINAPI CONN_Disconnect(int requestID)
{
	return BLE_CONN_Disconnect(requestID);
}

BLEDLLV2_API int WINAPI CONN_PollRSSI(int requestID)
{
	return BLE_CONN_PollRSSI(requestID);
}

BLEDLLV2_API int WINAPI CONN_GetRSSI(int requestID, int* rssi)
{
	return BLE_CONN_GetRSSI(requestID, *rssi);
}

BLEDLLV2_API int WINAPI CONN_ButtonPressedCallback(int requestID, void (__stdcall *buttonPressed) (IMiTokenBLEConnection* sender, int buttonID))
{
	return BLE_CONN_ButtonPressedCallback(requestID, buttonPressed);
}

BLEDLLV2_API int WINAPI CONN_ReadLong(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle)
{
	return BLE_CONN_ReadLong(requestID, handle);
}

BLEDLLV2_API MessageBufferObject WINAPI CONN_CreateNewMessageBuffer()
{
	return BLE_CONN_CreateNewMessageBuffer();
}

BLEDLLV2_API int WINAPI CONN_WriteMessageBuffer(int requestID, MessageBufferObject messageBuffer)
{
	return BLE_CONN_WriteMessageBuffer(requestID, messageBuffer);
}

BLEDLLV2_API int WINAPI CONN_AddMessageToMessageBuffer(MessageBufferObject messageBuffer, uint16 handle, uint16 length, const uint8* data)
{
	return BLE_CONN_AddMessageToMessageBuffer(messageBuffer, handle, length, data);
}

BLEDLLV2_API int WINAPI CONN_GetCharacteristicsHandle(int requestID, uint16 handle, uint16* characteristicsHandle)
{
	return BLE_CONN_GetCharacteristicHandle(requestID, handle, *characteristicsHandle);
}

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_RequestExclusiveAccess()
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_RequestExclusiveAccess();
}

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_HasExclusiveAccess()
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_HasExclusiveAccess();
}

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_ReleaseExclusiveAccess()
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_ReleaseExclusiveAccess();
}

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_SendMessage(uint8* data, int length, int connectionID)
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_SendMessage(data, length, connectionID);
}

/*
BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_SendMessage(uint8* data, int length, int connectionID)
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_SendMessage(data, length, connectionID);
}*/

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI NP_SetMessageCallback(void (*NP_MessageCallback) (IMiTokenBLE* sender, uint8* data, int length, int connectionID))
{
#ifdef DUMMY_DLL
	return BLE_API_NP_SUCCESS;
#endif

	return BLE_NP_SetMessageCallback(NP_MessageCallback);
}

#ifdef BLEAPI_USEEX
BLEDLLV2_API int WINAPI DLLEXT_MakeStoreBuffer(int dataLength, const uint8* data, int* storeLength, uint8* storeData)
{
	return BLE_EXT_MakeStoreBuffer(dataLength, data, *storeLength, storeData);
}

BLEDLLV2_API int WINAPI DLLEXT_MakeReadBuffer(int dataLength, const uint8* data, int* storeLength, uint8* storeData)
{
	return BLE_EXT_MakeReadBlock(dataLength, data, *storeLength, storeData);
}

BLEDLLV2_API int WINAPI DLLEXT_DecryptReadBlock(int length, const uint8* data, int* outlength, uint8* outData)
{
	return BLE_EXT_DecryptReadBlock(length, data, *outlength, outData);
}
#else
BLEDLLV2_API int WINAPI DLLEXT_MakeStoreBuffer(int dataLength, const uint8* data, int* storeLength, uint8* storeData)
{
	return -256;
}

BLEDLLV2_API int WINAPI DLLEXT_MakeReadBuffer(int dataLength, const uint8* data, int* storeLength, uint8* storeData)
{
	return -256;
}

BLEDLLV2_API int WINAPI DLLEXT_DecryptReadBlock(int length, const uint8* data, int* outlength, uint8* outData)
{
	return -256;
}
#endif

BLEDLLV2_API int WINAPI DEBUG_SetDebugStreams(void (__stdcall* dataInStream) (int amount), void (__stdcall* dataOutStream) (int amount))
{
	BLE_DEBUG_SetIODebugOutput(dataInStream, dataOutStream);
	return 0;
}



//DLL_ functions are those that are only used in the DLL (they have no API equivilent)
//Generally used for stuff where the API knew the format of the data to a level C# doesn't allow
BLEDLLV2_API void* WINAPI DLL_CreateNewFilter()
{
	FilterObject* object = new FilterObject();
	return object;
}

BLEDLLV2_API void WINAPI DLL_AddFilter(FilterObject* filter, int flag, int len, unsigned char* data)
{
	while(filter->nextNode != NULL)
	{
		filter = filter->nextNode;
	}

	filter->flag = flag;
	filter->len = len;
	filter->data = new unsigned char[len];
	memcpy(filter->data, data, len);
	filter->nextNode = new FilterObject();

	return;
}

BLEDLLV2_API void WINAPI DLL_AddAddressFilter(FilterObject* filter, int addressCount, unsigned char* addresses)
{
	if(filter->filterAddressCount > 0)
	{
		delete[] filter->filterAddresses;
	}

	filter->filterAddresses = new unsigned char[addressCount * 6];
	memcpy(filter->filterAddresses, addresses, (6 * addressCount));

	return;
}

BLEDLLV2_API FinalizedFilterObject* WINAPI DLL_FinalizeFilter(FilterObject* filter)
{
	int count = 0;
	FilterObject* temp = filter;

	while(temp->nextNode != NULL)
	{
		count++;
		temp = temp->nextNode;
	}

	FinalizedFilterObject* ret = new FinalizedFilterObject();
	ret->count = count;
	ret->filters = new DeviceData[count];
	ret->addressCount = filter->filterAddressCount;
	ret->addresses = new unsigned char[filter->filterAddressCount * 6];
	memcpy(ret->addresses, filter->filterAddresses, (filter->filterAddressCount * 6));

	temp = filter;
	for(int i = 0 ; i < count ; ++i)
	{
		ret->filters[i].length = temp->len;
		ret->filters[i].flag = temp->flag;
		ret->filters[i].data = temp->data;
		filter->data = NULL; //set data to null so the destructor doesn't try to destroy it (we've already moved it over to ret->filters[i].data and we will destroy this object before returning)
		temp = temp->nextNode;
	}

	//we no longer need the FilterObject so delete it - note : the destructor will recursively destroy each node
	delete filter;

	//we can return our FinalizedFilterObject
	return ret;
}

#endif

BLEDLLV2_API int WINAPI DLL_GetVersion()
{
	return (DLL_VERSION_MAG << 16) | (DLL_VERSION_MIN << 8) | (DLL_VERSION_REV);
}

//We can now enfore a certain version of the DLL Wrapper to use this code
BLEDLLV2_API int WINAPI DLL_CheckWrapperVersion(int versionID)
{
	if(versionID < ((MIN_WRAPPER_VERSION_MAG << 16) | (MIN_WRAPPER_VERSION_MIN << 8) | (MIN_WRAPPER_VERSION_REV)))
	{
		return -1;
	}
	return 0;
}

