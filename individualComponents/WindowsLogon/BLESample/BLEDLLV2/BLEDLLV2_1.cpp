#include "BLEDLLV2_1.h"

BLEDLLV2_API IMiTokenBLE* WINAPI MiTokenBLE_CreateInterface()
{
	DEBUG_ENTER_FUNCTION;
	return CreateNewMiTokenBLEInterface();
}

BLEDLLV2_API void WINAPI MiTokenBLE_DeleteInterface(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	instance->SafeDelete();
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_Initialize(IMiTokenBLE* instance, const char* COMPort, const char* NamedPipePort, bool isServer)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Initialize(COMPort, NamedPipePort, isServer);
}

BLEDLLV2_API bool WINAPI MiTokenBLE_ConnectedToCOM(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ConnectedToCOM();
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_ReInitCOM(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ReInitCOM();
}

BLEDLLV2_API bool WINAPI MiTokenBLE_COMOwnerKnown(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->COMOwnerKnown();
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_SetCOMWaitHandle(IMiTokenBLE* instance, void* waitHandle)
{
	DEBUG_ENTER_FUNCTION;
	return instance->SetCOMWaitHandle(waitHandle);
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_WaitForCOMToBeFree(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	HANDLE eventHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	MiTokenBLE_SetCOMWaitHandle(instance, eventHandle);
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
	return BLE_API_SUCCESS;
}

BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_ReleaseCOMPort(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ReleaseCOMPort();
}
BLEDLLV2_API void WINAPI MiTokenBLE_SetPollID(IMiTokenBLE* instance, int newPollID)
{
	DEBUG_ENTER_FUNCTION;
	return instance->SetPollID(newPollID);
}
BLEDLLV2_API int WINAPI MiTokenBLE_GetPollID(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetPollID();
}
BLEDLLV2_API int WINAPI MiTokenBLE_GetDeviceFoundCount(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetDeviceFoundCount();
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_GetDeviceInfo(IMiTokenBLE* instance, int deviceID, struct DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetDeviceInfo(deviceID, pDeviceInfo);
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_GetDeviceMetaInfo(IMiTokenBLE* instance, int deviceID, int flag, uint8* buffer, int* bufferLength)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetDeviceMetaInfo(deviceID, flag, buffer, bufferLength);
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_RestartScanner(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->RestartScanner();
}

BLEDLLV2_API FilteredSearchObject WINAPI MiTokenBLE_StartFilteredSearch(IMiTokenBLE* instance, int filterCount, DeviceData* filters)
{
	DEBUG_ENTER_FUNCTION;
	return instance->StartFilteredSearch(filterCount, filters);
}
BLEDLLV2_API FilteredSearchObject WINAPI MiTokenBLE_StartFilteredSearchEx(IMiTokenBLE* instance, int filterCount, DeviceData* filters, int addressCount, uint8* addressArray)
{
	DEBUG_ENTER_FUNCTION;
	return instance->StartFilteredSearchEx(filterCount, filters, addressCount, addressArray);
}

BLEDLLV2_API FilteredSearchObject WINAPI MiTokenBLE_StartFilteredSearchFromDLLExt(IMiTokenBLE* instance, FinalizedFilterObject* filterToUse)
{
	DEBUG_ENTER_FUNCTION;
	return instance->StartFilteredSearchEx(filterToUse->count, filterToUse->filters, filterToUse->addressCount, filterToUse->addresses);
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_ConvertFilteredSearchToByteStream(IMiTokenBLE* instance, FilteredSearchObject filter, uint8* byteStream, int& byteStreamLength)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ConvertFilteredSearchToByteStream(filter, byteStream, byteStreamLength);
}
BLEDLLV2_API FilteredSearchObject WINAPI MiTokenBLE_ConvertByteStreamToFilteredSearch(IMiTokenBLE* instance, uint8* byteStream, int byteStreamLength)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ConvertByteStreamToFilteredSearch(byteStream, byteStreamLength);
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLE_ContinueFilteredSearch(IMiTokenBLE* instance, FilteredSearchObject filter, DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ContinueFilteredSearch(filter, pDeviceInfo);
}
BLEDLLV2_API void WINAPI MiTokenBLE_RestartFilteredSearch(IMiTokenBLE* instance, FilteredSearchObject filter)
{
	DEBUG_ENTER_FUNCTION;
	return instance->RestartFilteredSearch(filter);
}
BLEDLLV2_API void WINAPI MiTokenBLE_FinishedFilteredSearch(IMiTokenBLE* instance, FilteredSearchObject filter)
{
	DEBUG_ENTER_FUNCTION;
	return instance->FinishedFilteredSearch(filter);
}

BLEDLLV2_API IMiTokenBLEConnection * WINAPI MiTokenBLE_StartProfile(IMiTokenBLE* instance, mac_address* address, REQUEST_ID* requestID)
{
	DEBUG_ENTER_FUNCTION;
	return instance->StartProfile(*address, *requestID);
}

BLEDLLV2_API FilterObject* WINAPI MiTokenBLE_DLLExt_CreateNewFilter()
{
	DEBUG_ENTER_FUNCTION;
	return new FilterObject();
}
BLEDLLV2_API FilterObject* WINAPI MiTokenBLE_DLLExt_AddFilter(FilterObject* filter, int flag, int len, unsigned char* data)
{
	DEBUG_ENTER_FUNCTION;
	while (filter->nextNode != nullptr)
	{
		filter = filter->nextNode;
	}

	filter->flag = flag;
	filter->len = len;
	filter->data = new unsigned char[len];
	memcpy(filter->data, data, len);
	filter->nextNode = new FilterObject();

	return filter;
}
BLEDLLV2_API void WINAPI MiTokenBLE_DLLExt_AddAddressFilter(FilterObject* filter, int addressCount, unsigned char* addresses)
{
	DEBUG_ENTER_FUNCTION;
	if (filter->filterAddressCount > 0)
	{
		delete[] filter->filterAddresses;
	}

	filter->filterAddresses = new unsigned char[addressCount * 6];
	memcpy(filter->filterAddresses, addresses, (6 * addressCount));
	filter->filterAddressCount++;
	return;
}
BLEDLLV2_API FinalizedFilterObject* WINAPI MiTokenBLE_DLLExt_FinalizeFilter(FilterObject* filter)
{
	DEBUG_ENTER_FUNCTION;
	int count = 0;
	FilterObject* temp = filter;
	while (temp->nextNode != nullptr)
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
	for (int i = 0; i < count; ++i)
	{
		ret->filters[i].length = temp->len;
		ret->filters[i].flag = temp->flag;
		ret->filters[i].data = temp->data;
		filter->data = nullptr; //set data to nullptr so that the destructor doesn't try to destroy it (we've alreadt moved it over to ret->filters[i].data and will destory filter before returning)
		temp = temp->nextNode;
	}
	
	//we no longer need the FilterObject so delete it - note: the destructor will recursively destroy each node
	delete filter;

	//now reutrn our FinalizedFilterObject
	return ret;
}


BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_Pipe_HasExclusiveAccess(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Pipe_HasExclusiveAccess();
}
BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_Pipe_ReleaseExclusiveAccess(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Pipe_ReleaseExclusiveAccess();
}
BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_Pipe_RequestExclusiveAccess(IMiTokenBLE* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Pipe_RequestExclusiveAccess();
}
BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_Pipe_SendMessage(IMiTokenBLE* instance, unsigned char* data, int length, int connectionID)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Pipe_SendMessage(data, length, connectionID);
}
BLEDLLV2_API BLE_API_NP_RETURNS WINAPI MiTokenBLE_Pipe_SetNamedPipeMessageCallback(IMiTokenBLE* instance, void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID))
{
	DEBUG_ENTER_FUNCTION;
	return instance->Pipe_SetNamedPipeMessageCallback(callback);
}


BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_Initialize(IMiTokenBLEConnection* instance, mac_address address)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Initialize(address);
}
BLEDLLV2_API bool WINAPI MiTokenBLE_CONN_timedOut(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->timedOut();
}
BLEDLLV2_API void WINAPI MiTokenBLE_CONN_GetAddress(IMiTokenBLEConnection* instance, mac_address& address)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetAddress(address);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_IsConnected(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->IsConnected();
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_ScanServicesInRange(IMiTokenBLEConnection* instance, SERVICE_HANDLE handle, uint16 maxRange)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ScanServicesInRange(handle, maxRange);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_SetAttribute(IMiTokenBLEConnection* instance, ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;
	return instance->SetAttribute(handle, length, data);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_StartGetAttribute(IMiTokenBLEConnection* instance, ATTRIBUTE_HANDLE handle)
{
	DEBUG_ENTER_FUNCTION;
	return instance->StartGetAttribute(handle);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_GetAttribute(IMiTokenBLEConnection* instance, ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetAttribute(handle, maxLength, realLength, buffer);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_Disconnect(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->Disconnect();
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_PollRSSI(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->PollRSSI();
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_GetRSSI(IMiTokenBLEConnection* instance, int& rssi)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetRSSI(rssi);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_ButtonPressedCallback(IMiTokenBLEConnection* instance, void(__stdcall *buttonPressed) (IMiTokenBLEConnection* requestID, int buttonID))
{
	DEBUG_ENTER_FUNCTION;
	return instance->ButtonPressedCallback(buttonPressed);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_ReadLong(IMiTokenBLEConnection* instance, ATTRIBUTE_HANDLE handle)
{
	DEBUG_ENTER_FUNCTION;
	return instance->ReadLong(handle);
}
BLEDLLV2_API MessageBufferObject WINAPI MiTokenBLE_CONN_CreateNewMessageBuffer(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->CreateNewMessageBuffer();
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_WriteMessageBuffer(IMiTokenBLEConnection* instance, MessageBufferObject messageBuffer)
{
	DEBUG_ENTER_FUNCTION;
	return instance->WriteMessageBuffer(messageBuffer);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_AddMessageToMessageBuffer(IMiTokenBLEConnection* instance, MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;
	return instance->AddMessageToMessageBuffer(messageBuffer, handle, length, data);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_GetCharacteristicHandle(IMiTokenBLEConnection* instance, ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetCharacteristicHandle(handle, characteristicHandle);
}
BLEDLLV2_API int WINAPI MiTokenBLE_CONN_GetRequestID(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetRequestID();
}
BLEDLLV2_API uint8 WINAPI MiTokenBLE_CONN_GetLastError(IMiTokenBLEConnection* instance)
{
	DEBUG_ENTER_FUNCTION;
	return instance->GetLastError();
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLE_CONN_TimeoutRequest(IMiTokenBLEConnection* instance)
{
	return instance->TimeoutRequest();
}