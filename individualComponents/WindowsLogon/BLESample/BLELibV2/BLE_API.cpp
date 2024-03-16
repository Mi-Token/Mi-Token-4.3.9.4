#include "BLE_API.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "BLE_IO.h"
#include "BLE_LowLevel.h"

#include <stdio.h>
#include <stdlib.h>

#include "BLE_ASYNC.h"
#include "BLE_Structs.h"
#include "BLE_Callbacks.h"
#include "BLE_API_Ext.h"

#include "DebugLogging.h"
#include "CurrentState.h"

#include "MiTokenBLE.h"
#include "MiTokenBLEConnection.h"
#ifdef WIN32
#define snprintf _snprintf
#endif

#define HIDE_CLASS_ERROR

volatile HANDLE serial_handle;
bool ConnectedToCOM = false;

HANDLE ClientNPConnectThread = INVALID_HANDLE_VALUE;

#ifdef MiTokenBLE_ClasslessMode
MiTokenBLE::MiTokenBLE(const char* COMPort)
{
	int slen = strlen(COMPort);
	_COMPort = new char[slen + 1];
	memcpy(_COMPort, COMPort, slen + 1);

	BLE_Initialize(COMPort);
}
MiTokenBLE::MiTokenBLE(const char* COMPort, const char* NamedPipePort, bool isServer)
{
	int slen = strlen(COMPort);
	_COMPort = new char[slen + 1];
	memcpy(_COMPort, COMPort, slen + 1);

	BLE_InitializeEx(COMPort, NamedPipePort, isServer);
}
MiTokenBLE::~MiTokenBLE()
{
	BLE_Finalize();
}

int MiTokenBLE::ConnectedToCOM()
{
	return BLE_ConnectedToCOM();
}
int MiTokenBLE::ReInitCOM()
{
	return BLE_ReinitCOM(_COMPort);
}

int MiTokenBLE::COMOwnerKnown()
{
	return BLE_COMOwnerKnown();
}
int MiTokenBLE::SetCOMWaitHandle(void* waitHandle)
{
	return BLE_SetCOMWaitHandle(waitHandle);
}
int MiTokenBLE::ReleaseCOMPort()
{
	return BLE_ReleaseCOMPort();
}
void MiTokenBLE::SetPollID(int newPollID)
{
	return BLE_SetPollID(newPollID);
}
int MiTokenBLE::GetDeviceFoundCount()
{
	return BLE_GetDeviceFoundCount();
}
int MiTokenBLE::GetDeviceInfo(int deviceID, DeviceInfo* pDeviceInfo)
{
	return BLE_GetDeviceInfo(deviceID, pDeviceInfo);
}
int MiTokenBLE::GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength)
{
	return BLE_GetDeviceMetaInfo(deviceID, flag, buffer, bufferLength);
}
int MiTokenBLE::RestartScanner()
{
	return BLE_RestartScanner();
}

FilteredSearchObject MiTokenBLE::StartFilteredSearch(int filterCount, DeviceData* filters)
{
	return BLE_StartFilteredSearch(filterCount, filters);
}
FilteredSearchObject MiTokenBLE::StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray)
{
	return BLE_StartFilteredSearchEx(filterCount, filters, addressCount, addressArray);
}
int MiTokenBLE::ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& byteStreamLength)
{
	return BLE_ConvertFilteredSearchToByteStream(filter, byteStream, byteStreamLength);
}
FilteredSearchObject MiTokenBLE::ConvertByteStreamToFilteredSearch(uint8* byteStream, int byteStreamLength)
{
	return BLE_CovnertByteStreamToFilteredSearch(byteStream, byteStreamLength);
}

int MiTokenBLE::ContinueFilteredSearch(FilteredSearchObject filter, DeviceInfo* pDeviceInfo)
{
	return BLE_ContinueFilteredSearch(filter, pDeviceInfo);
}
void MiTokenBLE::RestartFilteredSearch(FilteredSearchObject filter)
{
	return BLE_RestartFilteredSearch(filter);
}
void MiTokenBLE::FinishedFilteredSearch(FilteredSearchObject filter)
{
	return BLE_FinishFilteredSearch(filter);
}


void MiTokenBLE::DEBUG_PrintDeiveMetaData(int deviceID)
{
	return BLE_DEBUG_PrintDeviceMetaData(deviceID);
}
void MiTokenBLE::DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall * dataOut) (int amount))
{
	return BLE_DEBUG_SetIODebugOutput(dataIn, dataOut);
}
MiTokenBLEConnection * MiTokenBLE::StartProfile(mac_address address, REQUEST_ID& requestID)
{
	MiTokenBLEConnection* ret = new MiTokenBLEConnection(this, address);
	requestID = ret->getRequestID();
	return ret;
}


MiTokenBLEConnection::MiTokenBLEConnection(MiTokenBLE* rootConnection, mac_address address)
{
	_rootConnection = rootConnection;
	memcpy(&_address, &address, sizeof(mac_address));

	BLE_CONN_StartProfile(address, _requestID);
}
REQUEST_ID MiTokenBLEConnection::getRequestID()
{
	return _requestID;
}

BLE_CONN_RET MiTokenBLEConnection::IsConnected()
{
	return BLE_CONN_IsConnected(_requestID);
}
BLE_CONN_RET MiTokenBLEConnection::ScanServicesInRange(SERVICE_HANDLE handle, uint16 maxRange)
{
	return BLE_CONN_ScanServicesInRange(_requestID, handle, maxRange);
}
BLE_CONN_RET MiTokenBLEConnection::SetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data)
{
	return BLE_CONN_SetAttribute(_requestID, handle, length, data);
}
BLE_CONN_RET MiTokenBLEConnection::StartGetAttribute(ATTRIBUTE_HANDLE handle)
{
	return BLE_CONN_StartGetAttribute(_requestID, handle);
}
BLE_CONN_RET MiTokenBLEConnection::GetAttribute(ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer)
{
	return BLE_CONN_GetAttribute(_requestID, handle, maxLength, realLength, buffer);
}
BLE_CONN_RET MiTokenBLEConnection::Disconnect()
{
	return BLE_CONN_Disconnect(_requestID);
}
BLE_CONN_RET MiTokenBLEConnection::PollRSSI()
{
	return BLE_CONN_PollRSSI(_requestID);
}
BLE_CONN_RET MiTokenBLEConnection::GetRSSI(int& rssi)
{
	return BLE_CONN_GetRSSI(_requestID, rssi);
}
BLE_CONN_RET MiTokenBLEConnection::ButtonPressedCallback(void (__stdcall *buttonPressed) (int requestID, int buttonID))
{
	return BLE_CONN_ButtonPressedCallback(_requestID, buttonPressed);
}
BLE_CONN_RET MiTokenBLEConnection::ReadLong(ATTRIBUTE_HANDLE handle)
{
	return BLE_CONN_ReadLong(_requestID, handle);
}
MessageBufferObject MiTokenBLEConnection::CreateNewMessageBuffer()
{
	return BLE_CONN_CreateNewMessageBuffer();
}
BLE_CONN_RET MiTokenBLEConnection::WriteMessageBuffer(MessageBufferObject messageBuffer)
{
	return BLE_CONN_WriteMessageBuffer(_requestID, messageBuffer);
}
BLE_CONN_RET MiTokenBLEConnection::AddMessageToMessageBuffer(MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data)
{
	return BLE_CONN_AddMessageToMessageBuffer(messageBuffer, handle, length, data);
}
BLE_CONN_RET MiTokenBLEConnection::GetCharacteristicHandle(ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle)
{
	return BLE_CONN_GetCharacteristicHandle(_requestID, handle, characteristicHandle);
}


#else

MiTokenBLE* _defaultObject = nullptr;
MiTokenBLEConnection* _defaultConnection = nullptr;

#ifndef HIDE_CLASS_ERROR
#error Class Mode of the API currently doesn't work!
#endif

/*
void DeviceConnected(ConnectionData* myData, uint8 connectionHandle, MiTokenBLE* parent)
{
	if(parent != nullptr)
	{
		parent->DeviceConnected(myData, connectionHandle);
	}
	else
	{
		mac_address nulladdr;
		RequestData::finishRequest(myData->connectionID);

		myData->callbacks.serviceFound = &BLE_CALLBACK_ServiceFound;
		myData->callbacks.serviceSearchFinished = &BLE_CALLBACK_ServiceSearchFinished;
		myData->callbacks.serviceInformationFound = &BLE_CALLBACK_ServiceInformation;
		myData->callbacks.handleValue = &BLE_CALLBACK_PollAttribute;
		myData->callbacks.writeAttributeFinished = &BLE_CALLBACK_AttributeWritten;
		myData->callbacks.RSSIValue = &BLE_CALLBACK_GotRSSI;
		myData->callbacks.longReadCompleted = &BLE_CALLBACK_LongReadCompleted;

		memset(nulladdr.addr, 0, sizeof(nulladdr));
		BLE_LL_ConnectToDevice(nulladdr, NULL);

		BLE_LL_DiscoverServicesInitiate(connectionHandle, 0x0001, 0xFFFF);
	}
}
*/

/*
BLE_CONN_RET BLE_CONN_StartProfile(mac_address address, int& requestID)
{

	_defaultObject->StartProfile(address, requestID);

	return BLE_CONN_RET::BLE_CONN_SUCCESS;
}
*/

IMiTokenBLE* CreateNewMiTokenBLEInterface(int BLEAPIVersion)
{
	switch(BLEAPIVersion)
	{
	case USE_MITOKEN_BLE_API_V2_1:
		return new MiTokenBLE();
	case USE_MITOKEN_BLE_API_V2_2:
		return dynamic_cast<IMiTokenBLE*>(new MiTokenBLEV2_2());
	}


	return nullptr;
}



bool getConnDataFromRequestID(int requestID, BLE_CONN_RET& retCode, MiTokenBLEConnection*& connection)
{
	if (_defaultObject->getLastConnection()->GetRequestID() != requestID)
	{
		return BLE_CONN_BAD_REQUEST;
	}

	connection = dynamic_cast<MiTokenBLEConnection*>(_defaultObject->getLastConnection());

	if (connection->timedOut())
	{
		return BLE_CONN_ERR_CONNECTION_TIMED_OUT;
	}

	if (connection->IsConnected() == false)
	{
		return BLE_CONN_ERR_NOT_CONNECTED;
	}
	return true;
}

BLE_CONN_RET BLE_CONN_IsConnected(int requestID)
{
	
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	

	BLE_CONN_RET retCode;
	if(!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->IsConnected();
}

BLE_CONN_RET BLE_CONN_GetCharacteristicHandle(int requestID, uint16 handle, uint16& characteristicHandle)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->GetCharacteristicHandle(handle, characteristicHandle);
}

BLE_CONN_RET BLE_CONN_StartGetAttribute(int requestID, uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->StartGetAttribute(handle);
}

BLE_CONN_RET BLE_CONN_GetAttribute(int requestID, uint16 handle, uint8 maxLength, uint8& realLength, uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->GetAttribute(handle, maxLength, realLength, data);
}

BLE_CONN_RET BLE_CONN_SetAttribute(int requestID, uint16 handle, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->SetAttribute(handle, length, data);
}


BLE_CONN_RET BLE_CONN_ScanServicesInRange(int requestID, uint16 handle, uint16 maxRange)
{
	DEBUG_ENTER_FUNCTION;
	
	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->ScanServicesInRange(handle, maxRange);
}

BLE_CONN_RET BLE_CONN_PollRSSI(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->PollRSSI();

}

BLE_CONN_RET BLE_CONN_GetRSSI(int requestID, int& rssi)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->GetRSSI(rssi);
}

BLE_CONN_RET BLE_CONN_ButtonPressedCallback(int requestID, void (__stdcall *buttonPressed) (IMiTokenBLEConnection* sender, int buttonID))
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->ButtonPressedCallback(buttonPressed);
}

BLE_CONN_RET BLE_CONN_Disconnect(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}
	
	return connData->Disconnect();
}

int BLE_RestartScanner()
{
	DEBUG_ENTER_FUNCTION;

	int ret = BLE_LL_RestartScanner();

	DEBUG_RETURN ret;
}

MessageBufferObject BLE_CONN_CreateNewMessageBuffer()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN new MessageBuffer();
}

BLE_CONN_RET BLE_CONN_AddMessageToMessageBuffer(MessageBufferObject messageBuffer, uint16 handle, uint16 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	buffer->addMessage(handle, length, data);

	DEBUG_RETURN BLE_CONN_RET::BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_WriteMessageBuffer(int requestID, MessageBufferObject messageBuffer)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->WriteMessageBuffer(messageBuffer);
}

BLE_CONN_RET BLE_CONN_ReadLong(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle)
{
	DEBUG_ENTER_FUNCTION;

	MiTokenBLEConnection* connData;
	BLE_CONN_RET retCode;

	if (!getConnDataFromRequestID(requestID, retCode, connData))
	{
		DEBUG_RETURN retCode;
	}

	return connData->ReadLong(handle);
}


#ifdef BLEAPI_USEEX

int BLE_EXT_MakeStoreBuffer(int dataLength, const uint8* data, int& storeLength, uint8*& storeData)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeStoreBlock(dataLength, data, storeLength, storeData);
}

int BLE_EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_DecryptReadBlock(length, data, outlength, outdata);
}

int BLE_EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeReadBlock(length, data,outlength, outdata);
}

#else
int BLE_EXT_MakeStoreBuffer(int dataLength, const uint8* data, int& storeLength, uint8*& storeData)
{
	return -256;
}
int BLE_EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	return -256;
}
int BLE_EXT_MakeReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	return -256;
}
#endif
BLE_API_NP_RETURNS BLE_NP_HasExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	return _defaultObject->GetIO()->ExclusiveAccessStatus();
	//DEBUG_RETURN IO_Interface->ExclusiveAccessStatus();
}

BLE_API_NP_RETURNS BLE_NP_ReleaseExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	return _defaultObject->GetIO()->ReleaseExclusiveAccess();
	//DEBUG_RETURN IO_Interface->ReleaseExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_RequestExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	return _defaultObject->GetIO()->RequestExclusiveAccess();
	//DEBUG_RETURN IO_Interface->RequestExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_SendMessage(unsigned char* data, int length, int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	return _defaultObject->GetIO()->SendMessageOverNP(data, length, connectionID);
	//DEBUG_RETURN IO_Interface->SendMessageOverNP(data, length, connectionID);
}

BLE_API_NP_RETURNS BLE_NP_SetMessageCallback(void (*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID))
{
	DEBUG_ENTER_FUNCTION;

	return _defaultObject->GetIO()->SetCallback(callback);
	//DEBUG_RETURN IO_Interface->SetCallback(callback);
}


void BLE_SetTraceFile(const char* fileTracePath)
{
	
	int len = strlen(fileTracePath);
	char* tfp = new char[len + 1];
	memset(tfp, 0, len + 1);
	memcpy(tfp, fileTracePath, len);
	filepath = tfp;
}

int BLE_Initialize(const char* COMPort)
{
	_defaultObject = new MiTokenBLE();
	return _defaultObject->Initialize(COMPort);
}

int BLE_InitializeEx(const char* COMPort, const char* NamedPipe, bool isServer)
{
	_defaultObject = new MiTokenBLE();
	defaultLowLevel = _defaultObject->getLowLevel();
	return _defaultObject->Initialize(COMPort, NamedPipe, isServer);
}

int BLE_ConnectedToCOM()
{
	return _defaultObject->ConnectedToCOM();
}

int BLE_ReinitCOM(const char* COMPort)
{
	return _defaultObject->ReInitCOM();
}

int BLE_Finalize()
{
	delete _defaultObject;
	_defaultObject = nullptr;

	return 0;
}

int BLE_COMOwnerKnown()
{
	return _defaultObject->COMOwnerKnown();
}

int BLE_SetCOMWaitHandle(void* waitHandle)
{
	return _defaultObject->SetCOMWaitHandle(waitHandle);
}

int BLE_ReleaseCOMPort()
{
	return _defaultObject->ReleaseCOMPort();
}


void BLE_SetPollID(int newPollID)
{
	if(_defaultObject != nullptr)
	{
		return _defaultObject->SetPollID(newPollID);
	}

	return;
}

int BLE_GetDeviceFoundCount() 
{
	return _defaultObject->GetDeviceFoundCount();
}

struct SingleLinkedList* lastDevInfoNode = NULL;
int lastDevInfoID = 0;

/*
int CopyFromPollDataToDeviceInfo(PollData* data, struct DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;

	memcpy(pDeviceInfo->address, data->address, 6);
	pDeviceInfo->lastSeen = data->lastSeen;
	pDeviceInfo->RSSI = data->CalculateRSSI();

	DEBUG_RETURN 0;
}
*/

int BLE_GetDeviceInfo(int deviceID, struct DeviceInfo* pDeviceInfo) 
{
	return _defaultObject->GetDeviceInfo(deviceID, pDeviceInfo);
}

int BLE_GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength)
{
	return _defaultObject->GetDeviceMetaInfo(deviceID, flag, buffer, bufferLength);
}

FilteredSearchObject BLE_StartFilteredSearch(int filterCount, DeviceData* filters)
{
	return _defaultObject->StartFilteredSearch(filterCount, filters);
}

FilteredSearchObject BLE_StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray)
{
	return _defaultObject->StartFilteredSearchEx(filterCount, filters, addressCount, addressArray);
}

int BLE_ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& length)
{
	return _defaultObject->ConvertFilteredSearchToByteStream(filter, byteStream, length);
}

FilteredSearchObject BLE_CovnertByteStreamToFilteredSearch(uint8* byteStream, int length)
{
	return _defaultObject->ConvertByteStreamToFilteredSearch(byteStream, length);
}

int BLE_ContinueFilteredSearch(FilteredSearchObject filter, struct DeviceInfo* pDeviceInfo)
{
	return _defaultObject->ContinueFilteredSearch(filter, pDeviceInfo);
}

void BLE_RestartFilteredSearch(FilteredSearchObject filter)
{
	return _defaultObject->RestartFilteredSearch(filter);
}

void BLE_FinishFilteredSearch(FilteredSearchObject filter)
{
	return _defaultObject->FinishedFilteredSearch(filter);
}


void BLE_DEBUG_PrintDeviceMetaData(int deviceID)
{
	return _defaultObject->DEBUG_PrintDeiveMetaData(deviceID);
}


void BLE_DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall *dataOut) (int amount))
{
	return _defaultObject->DEBUG_SetIODebugOutput(dataIn, dataOut);
}

void DeviceConnected(ConnectionData* myData, uint8 connectionHandle)
{
	DEBUG_ENTER_FUNCTION;

	mac_address nuladd;
	
	RequestData::finishRequest(myData->connectionID);
	//finishRequest(connectionHandle);
	
	myData->callbacks.serviceFound = &BLE_CALLBACK_ServiceFound;
	myData->callbacks.serviceSearchFinished = &BLE_CALLBACK_ServiceSearchFinished;
	myData->callbacks.serviceInformationFound = &BLE_CALLBACK_ServiceInformation;
	myData->callbacks.handleValue = &BLE_CALLBACK_PollAttribute;
	myData->callbacks.writeAttributeFinished = &BLE_CALLBACK_AttributeWritten;
	myData->callbacks.RSSIValue = &BLE_CALLBACK_GotRSSI;
	myData->callbacks.longReadCompleted = &BLE_CALLBACK_LongReadCompleted;

	memset(nuladd.addr, 0, sizeof(nuladd));
	BLE_LL_ConnectToDevice(nuladd, NULL);

	BLE_LL_DiscoverServicesInitiate(connectionHandle, 0x0001, 0xFFFF);

}

BLE_CONN_RET BLE_CONN_StartProfile(mac_address address, int& requestID)
{
	/*
	if(_defaultConnection != nullptr)
	{
		return BLE_CONN_RET::BLE_CONN_BAD_REQUEST;
	}
	*/
	_defaultConnection = dynamic_cast<MiTokenBLEConnection*>(_defaultObject->StartProfile(address, requestID));

	return BLE_CONN_RET::BLE_CONN_SUCCESS;
}

IMiTokenBLEConnection* BLE_GetConnectionV2Wrapper()
{
	if(_defaultObject != nullptr)
	{
		return _defaultObject->getLastConnection();
	}

	return nullptr;
}

bool getConnDataFromRequestID(int requestID, BLE_CONN_RET& retCode, RequestData*& request, ConnectionData*& connection)
{
	DEBUG_ENTER_FUNCTION;

	request = RequestData::findNodeWithID(requestID);
	if(request == NULL)
	{
		retCode = BLE_CONN_BAD_REQUEST;
		DEBUG_RETURN false;
	}
	if(request->timedOut)
	{
		retCode = BLE_CONN_ERR_CONNECTION_TIMED_OUT;
		DEBUG_RETURN false;
	}

	if(!request->connected)
	{
		retCode = BLE_CONN_ERR_NOT_CONNECTED;
		DEBUG_RETURN false;
	}
	
	connection = ConnectionData::findNodeWithID(request->linkedConnectionID);

	if(connection == NULL)
	{
		retCode = BLE_CONN_ERR_NO_SUCH_CONNECTION;
		DEBUG_RETURN false;
	}
	DEBUG_RETURN true;

}

/*
BLE_CONN_RET BLE_CONN_IsConnected(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	DEBUG_RETURN BLE_CONN_SUCCESS;
}
*/

/*
BLE_CONN_RET BLE_CONN_GetCharacteristicHandle(int requestID, uint16 handle, uint16& characteristicHandle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	characteristicHandle = attribute->characteristicsHandle;

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


BLE_CONN_RET BLE_CONN_StartGetAttribute(int requestID, uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;

	BLE_LL_ReadAttribute(connData->connectionID, attribute->characteristicsHandle);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


BLE_CONN_RET BLE_CONN_GetAttribute(int requestID, uint16 handle, uint8 maxLength, uint8& realLength, uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	realLength = attribute->dataLength;
	if(data == NULL)
	{
		DEBUG_RETURN BLE_CONN_SUCCESS;
	}

	if(maxLength < realLength)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA;
	}

	memcpy(data, attribute->data, realLength);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


BLE_CONN_RET BLE_CONN_SetAttribute(int requestID, uint16 handle, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	if(length > 20)
	{
		//extended write
		connData->doingLongWrite = true;
		connData->SetNextExtendedWrite(attribute->characteristicsHandle, length, data);
		connData->callbacks.longWriteSectionCompleted = BLE_CALLBACK_PrepareWriteCompleted;

		//The callback controls the writing of data so call it now
		connData->callbacks.longWriteSectionCompleted(connData);

	}
	else
	{
		connData->doingWrite = true;
		connData->callbacks.writeAttributeFinished = BLE_CALLBACK_AttributeWritten;

		BLE_LL_AttributeWrite(connData->connectionID, attribute->characteristicsHandle, length, data);
	}

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


BLE_CONN_RET BLE_CONN_ScanServicesInRange(int requestID, uint16 handle, uint16 maxRange)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionServices* service;
	
	BLE_CONN_RET retCode;
	
	uint16 start, end, length;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}
	
	if(connData->serviceScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_SERVICES_NOT_SCANNED;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	service = connData->getService(handle, false);

	if(service == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_SERVICE;
	}

	start = service->start;
	end = service->end;
	length = end - start;
	if(length > maxRange)
	{
		end = start + maxRange;
	}
	
	connData->callbacks.serviceInformationFinished = BLE_CALLBACK_ServiceInformationCompleted;

	connData->beginAttributeScan();
	BLE_LL_FindInformationOnService(connData->connectionID, start, end);

	
	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_PollRSSI(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	BLE_LL_PollRSSI(connData->connectionID);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_GetRSSI(int requestID, int& rssi)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	
	BLE_CONN_RET retCode;
	
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	rssi = connData->getAverageRSSI();

	DEBUG_RETURN BLE_CONN_SUCCESS;
	
}
*/

BLE_CONN_RET BLE_CONN_ButtonPressedCallback(int requestID, void (__stdcall *buttonPressed) (int requestID, int buttonID))
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	connData->buttonPushed = buttonPressed;

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

/*
BLE_CONN_RET BLE_CONN_Disconnect(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	
	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		if(retCode == BLE_CONN_ERR_CONNECTION_TIMED_OUT)
		{
			//The Request exists, we just don't have a connection because we timed out first
			RequestData::closeRequest(requestID);
			DEBUG_RETURN BLE_CONN_SUCCESS;
		}
		else
		{
			DEBUG_RETURN retCode;
		}
	}

	RequestData::closeRequest(requestID);

	BLE_LL_Disconnect(connData->connectionID);
	
	DEBUG_RETURN BLE_CONN_SUCCESS;
}


int BLE_RestartScanner()
{
	DEBUG_ENTER_FUNCTION;

	int ret = BLE_LL_RestartScanner();

	DEBUG_RETURN ret;
}

MessageBufferObject BLE_CONN_CreateNewMessageBuffer()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN new MessageBuffer();
}


BLE_CONN_RET BLE_CONN_AddMessageToMessageBuffer(MessageBufferObject messageBuffer, uint16 handle, uint16 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	buffer->addMessage(handle, length, data);

	DEBUG_RETURN BLE_CONN_RET::BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_WriteMessageBuffer(int requestID, MessageBufferObject messageBuffer)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	connData->doingLongWrite = true;
	connData->AppendToExtendedWrite(buffer);
	delete buffer;
	connData->callbacks.longWriteSectionCompleted = BLE_CALLBACK_PrepareWriteCompleted;
	connData->callbacks.longWriteSectionCompleted(connData);

	DEBUG_RETURN BLE_CONN_SUCCESS;
	
}


BLE_CONN_RET BLE_CONN_ReadLong(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;

	BLE_LL_LongReadAttribute(connData->connectionID, attribute->characteristicsHandle);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}
*/

#ifdef BLEAPI_USEEX

int BLE_EXT_MakeStoreBuffer(int dataLength, const uint8* data, int& storeLength, uint8*& storeData)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeStoreBlock(dataLength, data, storeLength, storeData);
}

int BLE_EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_DecryptReadBlock(length, data, outlength, outdata);
}

int BLE_EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeReadBlock(length, data,outlength, outdata);
}

#endif

/*
BLE_API_NP_RETURNS BLE_NP_HasExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->ExclusiveAccessStatus();
}

BLE_API_NP_RETURNS BLE_NP_ReleaseExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->ReleaseExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_RequestExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->RequestExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_SendMessage(unsigned char* data, int length, int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->SendMessageOverNP(data, length, connectionID);
}

BLE_API_NP_RETURNS BLE_NP_SetMessageCallback(void (*callback) (unsigned char* data, int length, int connectionID))
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->SetCallback(callback);
}

*/

#endif



#ifdef MiTokenBLE_ClasslessMode

void BLE_SetTraceFile(const char* fileTracePath)
{
	int len = strlen(fileTracePath);
	char* tfp = new char[len + 1];
	memset(tfp, 0, len + 1);
	memcpy(tfp, fileTracePath, len);
	filepath = tfp;
}

int BLE_Initialize(const char* COMPort)
{
	DEBUG_ENTER_FUNCTION;
	/*
	char str[80];
	int retCode;

	snprintf(str, sizeof(str) -1, "\\\\.\\%s", COMPort);
	retCode = IO_Interface->OpenSerialHandle(str);

	if(retCode != 0)
	{
		DEBUG_RETURN retCode;
	}

	BLE_LL_InitBLEDevice();


	BLE_ASYNC_ReadPollStart();
	
	DEBUG_RETURN 0;

	*/

	DEBUG_RETURN -256;
}

int BLE_InitializeEx(const char* COMPort, const char* NamedPipe, bool isServer)
{
	DEBUG_ENTER_FUNCTION;
	char str[80];
	int retCode = 0;
	bool COMOpened = false;

	if(COMPort != NULL)
	{
		snprintf(str, sizeof(str) -1, "\\\\.\\%s", COMPort);
	}

	if(isServer && (COMPort != NULL))
	{
		retCode = IO_Interface->OpenSerialHandle(str);
		if(retCode == 0)
		{
			COMOpened = true;
			ConnectedToCOM = true;
		}
	}

	//if retCode != 0 we failed to open the COM port, this could be because we have a client piece already using it due to being launched before us. Thus we will ignore the fact the COM Port wasn't opened

	//Always initialize the named pipe, the client may want to send us messages
	IO_Interface->InitializeNamedPipe(NamedPipe, isServer);

	if(isServer && COMOpened)
	{
		//We can only initialize the BLEDevice if the COM Port was opened
		IO_Interface->InitBLEDevice();
	}
	else if(!isServer)
	{
		Sleep(1000);
		if(!IO_Interface->ClientConnectedToNP())
		{
			if(COMPort != NULL)
			{
				//we need to initialize and connect to the COM Port - the server is not online yet so we will control it.
				//Once the server comes online, we will inform it that we are connected to the COM Port and it should wait for us to finish
				retCode = IO_Interface->OpenSerialHandle(str);

				if(retCode != 0)
				{
					DEBUG_RETURN retCode;
				}

				ConnectedToCOM = true;
				IO_Interface->InitBLEDevice();
			}
		}
	}

	if(!isServer || COMOpened)
	{
		IO_Interface->StartASyncRead();
	}

	DEBUG_RETURN 0;
}

int BLE_ConnectedToCOM()
{
	DEBUG_ENTER_FUNCTION;
	if(ConnectedToCOM)
	{
		DEBUG_RETURN 1;
	}
	else
	{
		DEBUG_RETURN 0;
	}
}

int BLE_ReinitCOM(const char* COMPort)
{
	DEBUG_ENTER_FUNCTION;
	char str[80];
	snprintf(str, sizeof(str) -1, "\\\\.\\%s", COMPort);

	int retCode = 0;
	if(!ConnectedToCOM)
	{
		retCode = IO_Interface->OpenSerialHandle(str);
		if(retCode == 0)
		{
			ConnectedToCOM = true;
			BLE_LL_InitBLEDevice();
			IO_Interface->StartASyncRead();
		}
	}


	DEBUG_RETURN retCode;
}

int BLE_Finalize()
{
	DEBUG_ENTER_FUNCTION;

	//abort everything
	CurrentState::abort();

	ConnectedToCOM = false;

	IO_Interface->ReleaseCOMPort();

	IO_Interface->StopASyncRead();

	IO_Interface->CloseSerialHandle();

	IO_Interface->CloseNamedPipe();

	PollData::finalize();
	
	
	DEBUG_RETURN 0;
}

int BLE_COMOwnerKnown()
{
	DEBUG_ENTER_FUNCTION;
	int ret = IO_Interface->COMOwnerKnown();

	DEBUG_RETURN ret;
}

int BLE_SetCOMWaitHandle(void* waitHandle)
{
	DEBUG_ENTER_FUNCTION;
	int ret = IO_Interface->SetCOMWaitHandle(waitHandle);

	DEBUG_RETURN ret;
}

int BLE_ReleaseCOMPort()
{
	DEBUG_ENTER_FUNCTION;
	int ret = IO_Interface->ReleaseCOMPort();

	DEBUG_RETURN ret;
}


void BLE_SetPollID(int newPollID)
{
	DEBUG_ENTER_FUNCTION;
	CurrentPollID = newPollID;
	currentPollID = newPollID;
	DEBUG_END_FUNCTION;
}

int BLE_GetDeviceFoundCount() 
{
	DEBUG_ENTER_FUNCTION;
	int ret = PollData::GetDeviceCount();


	DEBUG_RETURN ret;
}

struct SingleLinkedList* lastDevInfoNode = NULL;
int lastDevInfoID = 0;

int CopyFromPollDataToDeviceInfo(PollData* data, struct DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;

	memcpy(pDeviceInfo->address, data->address, 6);
	pDeviceInfo->lastSeen = data->lastSeen;
	pDeviceInfo->RSSI = data->CalculateRSSI();

	DEBUG_RETURN 0;
}

int BLE_GetDeviceInfo(int deviceID, struct DeviceInfo* pDeviceInfo) 
{
	DEBUG_ENTER_FUNCTION;
	PollData* data = PollData::GetDeviceFromID(deviceID);

	if(data == NULL)
	{
		DEBUG_RETURN -1;
	}


	int ret = CopyFromPollDataToDeviceInfo(data, pDeviceInfo);

	DEBUG_RETURN ret;
}

int BLE_GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength)
{
	DEBUG_ENTER_FUNCTION;
	if(bufferLength == NULL)
	{
		DEBUG_RETURN BLE_API_ERR_NULL_POINTER;
	}

	PollData* data = PollData::GetDeviceFromID(deviceID);
	
	if(data == NULL)
	{
		DEBUG_RETURN BLE_API_ERR_NO_SUCH_DEVICE;
	}

	int inputLength = *bufferLength;

	int indexCache = 0;
	if(!data->getMetaDataLength(flag, bufferLength, indexCache))
	{
		//we couldn't get the meta data for some reason
		DEBUG_RETURN BLE_API_ERR_NO_SUCH_META_FLAG;
	}

	if((inputLength < *bufferLength) || (buffer == NULL))
	{
		DEBUG_RETURN BLE_API_MORE_DATA;
	}

	if(!data->getMetaData(flag, buffer, *bufferLength, indexCache))
	{
		DEBUG_RETURN BLE_API_ERR_FAILED_GETTING_META_DATA;
	}

	DEBUG_RETURN BLE_API_SUCCESS;	
}

FilteredSearchObject BLE_StartFilteredSearch(int filterCount, DeviceData* filters)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearchObject ret = new FilteredSearch(filterCount, filters);
	DEBUG_RETURN ret;
}

FilteredSearchObject BLE_StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearchObject ret = new FilteredSearch(filterCount, filters, addressCount, addressArray);

	DEBUG_RETURN ret;
}

int BLE_ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& length)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearch* pFilter = static_cast<FilteredSearch*>(filter);
	int ret = pFilter->convertToByteStream(byteStream, length);

	DEBUG_RETURN ret;
}

FilteredSearchObject BLE_CovnertByteStreamToFilteredSearch(uint8* byteStream, int length)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearchObject ret = new FilteredSearch(byteStream, length);

	DEBUG_RETURN ret;
}

int BLE_ContinueFilteredSearch(FilteredSearchObject filter, struct DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearch* pFilter = (FilteredSearch*)filter;
	PollData* node = pFilter->findNextNode();
	if(node == NULL)
	{
		DEBUG_RETURN -1;
	}

	int ret = CopyFromPollDataToDeviceInfo(node, pDeviceInfo);

	DEBUG_RETURN ret;
}

void BLE_RestartFilteredSearch(FilteredSearchObject filter)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearch* pFilter = (FilteredSearch*)filter;
	pFilter->restart();

	;
}

void BLE_FinishFilteredSearch(FilteredSearchObject filter)
{
	DEBUG_ENTER_FUNCTION;

	FilteredSearch* pFilter = (FilteredSearch*)filter;

	delete pFilter;
	
	;
}


void BLE_DEBUG_PrintDeviceMetaData(int deviceID)
{
#ifndef _DEBUG
	//do nothing
	DEBUG_END_FUNCTION;
#else //_DEBUG is defined
	PollData* data = PollData::GetDeviceFromID(deviceID);

	if(data == NULL)
	{
		printf("Device with ID not found\r\n");
	}

	data->printMetaData();
#endif
}


void BLE_DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall *dataOut) (int amount))
{
#ifndef _DEBUG
	//do nothing
	DEBUG_END_FUNCTION;
#else
	debugDataIn = dataIn;
	debugDataOut = dataOut;
#endif
}

void DeviceConnected(ConnectionData* myData, uint8 connectionHandle)
{
	DEBUG_ENTER_FUNCTION;

	mac_address nuladd;
	
	RequestData::finishRequest(myData->connectionID);
	//finishRequest(connectionHandle);
	
	myData->callbacks.serviceFound = &BLE_CALLBACK_ServiceFound;
	myData->callbacks.serviceSearchFinished = &BLE_CALLBACK_ServiceSearchFinished;
	myData->callbacks.serviceInformationFound = &BLE_CALLBACK_ServiceInformation;
	myData->callbacks.handleValue = &BLE_CALLBACK_PollAttribute;
	myData->callbacks.writeAttributeFinished = &BLE_CALLBACK_AttributeWritten;
	myData->callbacks.RSSIValue = &BLE_CALLBACK_GotRSSI;
	myData->callbacks.longReadCompleted = &BLE_CALLBACK_LongReadCompleted;

	memset(nuladd.addr, 0, sizeof(nuladd));
	BLE_LL_ConnectToDevice(nuladd, NULL);

	BLE_LL_DiscoverServicesInitiate(connectionHandle, 0x0001, 0xFFFF);

	;
}

BLE_CONN_RET BLE_CONN_StartProfile(mac_address address, int& requestID)
{
	DEBUG_ENTER_FUNCTION;
	requestID = -1;

	RequestData* data;
	//struct ProfileRequestExtraData* data;

	data = RequestData::startRequest();

	if(data == NULL)
	{
		DEBUG_RETURN BLE_CONN_BAD_REQUEST;
	}

	BLE_LL_ConnectToDevice(address, &DeviceConnected);

	requestID = data->ID;

	DEBUG_RETURN BLE_CONN_RET::BLE_CONN_SUCCESS;
}

bool getConnDataFromRequestID(int requestID, BLE_CONN_RET& retCode, RequestData*& request, ConnectionData*& connection)
{
	DEBUG_ENTER_FUNCTION;

	request = RequestData::findNodeWithID(requestID);
	if(request == NULL)
	{
		retCode = BLE_CONN_BAD_REQUEST;
		DEBUG_RETURN false;
	}
	if(request->timedOut)
	{
		retCode = BLE_CONN_ERR_CONNECTION_TIMED_OUT;
		DEBUG_RETURN false;
	}

	if(!request->connected)
	{
		retCode = BLE_CONN_ERR_NOT_CONNECTED;
		DEBUG_RETURN false;
	}
	
	connection = ConnectionData::findNodeWithID(request->linkedConnectionID);

	if(connection == NULL)
	{
		retCode = BLE_CONN_ERR_NO_SUCH_CONNECTION;
		DEBUG_RETURN false;
	}
	DEBUG_RETURN true;

}

BLE_CONN_RET BLE_CONN_IsConnected(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_GetCharacteristicHandle(int requestID, uint16 handle, uint16& characteristicHandle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	characteristicHandle = attribute->characteristicsHandle;

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_StartGetAttribute(int requestID, uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;

	BLE_LL_ReadAttribute(connData->connectionID, attribute->characteristicsHandle);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_GetAttribute(int requestID, uint16 handle, uint8 maxLength, uint8& realLength, uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	realLength = attribute->dataLength;
	if(data == NULL)
	{
		DEBUG_RETURN BLE_CONN_SUCCESS;
	}

	if(maxLength < realLength)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA;
	}

	memcpy(data, attribute->data, realLength);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_SetAttribute(int requestID, uint16 handle, uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	if(length > 20)
	{
		//extended write
		connData->doingLongWrite = true;
		connData->SetNextExtendedWrite(attribute->characteristicsHandle, length, data);
		connData->callbacks.longWriteSectionCompleted = BLE_CALLBACK_PrepareWriteCompleted;

		//The callback controls the writing of data so call it now
		connData->callbacks.longWriteSectionCompleted(connData);

	}
	else
	{
		connData->doingWrite = true;
		connData->callbacks.writeAttributeFinished = BLE_CALLBACK_AttributeWritten;

		BLE_LL_AttributeWrite(connData->connectionID, attribute->characteristicsHandle, length, data);
	}

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


BLE_CONN_RET BLE_CONN_ScanServicesInRange(int requestID, uint16 handle, uint16 maxRange)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionServices* service;
	
	BLE_CONN_RET retCode;
	
	uint16 start, end, length;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}
	
	if(connData->serviceScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_SERVICES_NOT_SCANNED;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	service = connData->getService(handle, false);

	if(service == NULL)
	{
		DEBUG_RETURN BLE_CONN_ERR_NO_SUCH_SERVICE;
	}

	start = service->start;
	end = service->end;
	length = end - start;
	if(length > maxRange)
	{
		end = start + maxRange;
	}
	
	connData->callbacks.serviceInformationFinished = BLE_CALLBACK_ServiceInformationCompleted;

	connData->beginAttributeScan();
	BLE_LL_FindInformationOnService(connData->connectionID, start, end);

	
	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_PollRSSI(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	BLE_LL_PollRSSI(connData->connectionID);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_GetRSSI(int requestID, int& rssi)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	
	BLE_CONN_RET retCode;
	
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	rssi = connData->getAverageRSSI();

	DEBUG_RETURN BLE_CONN_SUCCESS;
	
}

BLE_CONN_RET BLE_CONN_ButtonPressedCallback(int requestID, void (__stdcall *buttonPressed) (int requestID, int buttonID))
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	connData->buttonPushed = buttonPressed;

	DEBUG_RETURN BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_Disconnect(int requestID)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	
	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		if(retCode == BLE_CONN_ERR_CONNECTION_TIMED_OUT)
		{
			//The Request exists, we just don't have a connection because we timed out first
			RequestData::closeRequest(requestID);
			DEBUG_RETURN BLE_CONN_SUCCESS;
		}
		else
		{
			DEBUG_RETURN retCode;
		}
	}

	RequestData::closeRequest(requestID);

	BLE_LL_Disconnect(connData->connectionID);
	
	DEBUG_RETURN BLE_CONN_SUCCESS;
}

int BLE_RestartScanner()
{
	DEBUG_ENTER_FUNCTION;

	int ret = BLE_LL_RestartScanner();

	DEBUG_RETURN ret;
}

MessageBufferObject BLE_CONN_CreateNewMessageBuffer()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN new MessageBuffer();
}

BLE_CONN_RET BLE_CONN_AddMessageToMessageBuffer(MessageBufferObject messageBuffer, uint16 handle, uint16 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	buffer->addMessage(handle, length, data);

	DEBUG_RETURN BLE_CONN_RET::BLE_CONN_SUCCESS;
}

BLE_CONN_RET BLE_CONN_WriteMessageBuffer(int requestID, MessageBufferObject messageBuffer)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	BLE_CONN_RET retCode;

	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	connData->doingLongWrite = true;
	connData->AppendToExtendedWrite(buffer);
	delete buffer;
	connData->callbacks.longWriteSectionCompleted = BLE_CALLBACK_PrepareWriteCompleted;
	connData->callbacks.longWriteSectionCompleted(connData);

	DEBUG_RETURN BLE_CONN_SUCCESS;
	
}

BLE_CONN_RET BLE_CONN_ReadLong(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionData* connData;
	RequestData* request;
	ConnectionAttributes* attribute;

	BLE_CONN_RET retCode;
	if(!getConnDataFromRequestID(requestID, retCode, request, connData))
	{
		DEBUG_RETURN retCode;
	}

	if(connData->attributeScanCompleted() == false)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS;
	}

	if((connData->doingLongWrite) || (connData->doingWrite))
	{
		DEBUG_RETURN BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}

	attribute = connData->getAttribute(handle, false);

	if(attribute->reading)
	{
		DEBUG_RETURN BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;

	BLE_LL_LongReadAttribute(connData->connectionID, attribute->characteristicsHandle);

	DEBUG_RETURN BLE_CONN_SUCCESS;
}


#ifdef BLEAPI_USEEX

int BLE_EXT_MakeStoreBuffer(int dataLength, const uint8* data, int& storeLength, uint8*& storeData)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeStoreBlock(dataLength, data, storeLength, storeData);
}

int BLE_EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_DecryptReadBlock(length, data, outlength, outdata);
}

int BLE_EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN EXT_MakeReadBlock(length, data,outlength, outdata);
}

#endif
BLE_API_NP_RETURNS BLE_NP_HasExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->ExclusiveAccessStatus();
}

BLE_API_NP_RETURNS BLE_NP_ReleaseExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->ReleaseExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_RequestExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->RequestExclusiveAccess();
}

BLE_API_NP_RETURNS BLE_NP_SendMessage(unsigned char* data, int length, int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->SendMessageOverNP(data, length, connectionID);
}

BLE_API_NP_RETURNS BLE_NP_SetMessageCallback(void (*callback) (unsigned char* data, int length, int connectionID))
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN IO_Interface->SetCallback(callback);
}

#endif