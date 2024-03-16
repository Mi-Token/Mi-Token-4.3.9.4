#include "MiTokenBLEConnection.h"
#include "MiTokenBLE.h"
#include "BLE_Callbacks.h"

#include <stdio.h>

#define LOW_UINT16( uuid ) ((uint8)(uuid & 0xFF))
#define HIGH_UINT16( uuid) ((uint8)(uuid >> 8))

//T1 Base 128-bit UUID : F000XXXX-0451-4000-B000-000000000000
#define T1_BASE_UUID_128(uuid) 0x00, 0x00, 0x00, 0x00, \
							   0x00, 0x00, 0x00, 0xB0, \
							   0x00, 0x40, 0x51, 0x04, \
							   LOW_UINT16(uuid), HIGH_UINT16(uuid), 0x00, 0xF0

bool UUID_IS_T1_FORMAT(const uint8* uuid)
{
	uint8 buffer1[0x10] = {0};
	uint8 buffer2[0x10] = { T1_BASE_UUID_128(0x0000) };
	memcpy(buffer1, uuid, 0x10);
	buffer1[12] = buffer1[13] = 0x00;
	return (memcmp(buffer1, buffer2, 0x10) == 0);
}

#define UUID_FROM_T1_FORMAT(T1_FORMAT) \
	(uint16)((T1_FORMAT[13] << 8) | (T1_FORMAT[12]));

#define T1_BASE_UUID_128_FIX(var, uuid) var[12] = LOW_UINT16(uuid); \
									    var[13] = HIGH_UINT16(uuid);


//Check if we are currently connected - return if we are not
#define CHECK_IS_CONNECTED \
	if (_isConnected == false) { return BLE_CONN_ERR_NOT_CONNECTED; }

#define CHECK_SERVICE_SCAN \
	if (serviceScanCompleted() == false) { return BLE_CONN_ERR_SERVICES_NOT_SCANNED; }

//Check if we are currently scanning for attributes - return if we are
#define CHECK_ATTRIBUTE_SCAN \
	if (attributeScanCompleted() == false) { return BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS; }

//Check if we are currently writing to the device - return if we are
#define CHECK_WRITES \
	if (doingLongWrite | doingWrite) { return BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS; }

#define NO_CALL_CHECK(checkConn, checkService, checkAttScan, checkWrites, checkReads, checkServiceUUID, checkAttributeUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount) \
	if(timeout == CONN_TIMEOUT_JUST_CHECK) \
	{ \
		return _checkState(checkConn, checkService, checkAttScan, checkWrites, checkReads, checkServiceUUID, checkAttributeUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount); \
	}

#define NO_CALL_CHECK_CHARHANDLE(checkConn, checkService, checkAttScan, checkWrites, checkReads, charHandle) \
	if(timeout == CONN_TIMEOUT_JUST_CHECK) \
	{ \
		return _checkStateCharHandle(checkConn, checkService, checkAttScan, checkWrites, checkReads, charHandle); \
	}


#define SYNC_NORMAL_CASES \
BLE_CONN_ERR_NOT_CONNECTED: \
	case BLE_CONN_ERR_SERVICES_NOT_SCANNED: \
	case BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS: \
	case BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS: \
	case BLE_CONN_ERR_ATTRIBUTE_BEING_READ

#define SYNC_FUNCTION_PROTOTYPE_FULL(lowerFunctionToCall, lowerFunctionParams, checkConn, checkService, checkAttScan, checkWrites, checkReads, checkReadUUID) \
	SYNC_FUNCTION_PROTOTYPE(lowerFunctionToCall, lowerFunctionParams, (checkConn, checkService, checkAttScan, checkWrites, checkReads, 0x0000, checkReadUUID, false, false, 0)); 

#define SYNC_FUNCTION_PROTOTYPE_FULL_EX(lowerFunctionToCall, lowerFunctionParams, checkConn, checkService, checkAttScan, checkWrites, checkReads, checkReadServiceUUID, checkReadAttributeUUID, ServiceUUIDIsGUID, AttributeUUIDIsGUID, skipCount) \
	SYNC_FUNCTION_PROTOTYPE(lowerFunctionToCall, lowerFunctionParams, (checkConn, checkService, checkAttScan, checkWrites, checkReads, checkReadServiceUUID, checkReadAttributeUUID, ServiceUUIDIsGUID, AttributeUUIDIsGUID, skipCount)); 

#define SYNC_FUNCTION_PROTOTYPE_FULL_CHRHANDLE(lowerFunctionToCall, lowerFunctionParams, checkConn, checkService, checkAttScan, checkWrites, checkReads, checkCharHandle) \
	SYNC_FUNCTION_PROTOTYPE_CHRHANDLE(lowerFunctionToCall, lowerFunctionParams, (checkConn, checkService, checkAttScan, checkWrites, checkReads, checkCharHandle)); 

#define SYNC_FUNCTION_PROTOTYPE(lowerFunctionToCall, lowerFunctionParams, checkStateParams) \
	NO_CALL_CHECK checkStateParams ;\
	_resetSyncCheck(); \
	bool allowRetry = true; \
	BLE_CONN_RET tempRet; \
	while(allowRetry) \
	{ \
		tempRet = lowerFunctionToCall lowerFunctionParams ; \
		switch(tempRet) \
		{ \
		case BLE_CONN_SUCCESS: \
			allowRetry = false; \
			break; \
		case SYNC_NORMAL_CASES: \
			if(!_syncWait(timeout)) \
			{ \
				allowRetry = false; \
				tempRet = BLE_CONN_ERR_SYNC_TIMEOUT; \
			} \
			break; \
		default: \
			allowRetry = false; \
			break; \
		} \
	} \
	if(tempRet != BLE_CONN_SUCCESS) \
	{ \
		return tempRet; \
	} \
	allowRetry = true; \
	while(allowRetry) \
	{ \
		tempRet = _checkState checkStateParams ; \
		switch(tempRet) \
		{ \
		case BLE_CONN_SUCCESS: \
			allowRetry = false; \
			break; \
		case SYNC_NORMAL_CASES: \
			if(!_syncWait(timeout)) \
			{ \
				allowRetry = false; \
				tempRet = BLE_CONN_ERR_SYNC_TIMEOUT; \
			} \
			break; \
		default: \
			allowRetry = false; \
			break; \
		} \
	} \
	return tempRet;



#define SYNC_FUNCTION_PROTOTYPE_CHRHANDLE(lowerFunctionToCall, lowerFunctionParams, checkStateParams) \
	NO_CALL_CHECK_CHARHANDLE checkStateParams ;\
	_resetSyncCheck(); \
	bool allowRetry = true; \
	BLE_CONN_RET tempRet; \
	while(allowRetry) \
	{ \
		tempRet = lowerFunctionToCall lowerFunctionParams ; \
		switch(tempRet) \
		{ \
		case BLE_CONN_SUCCESS: \
			allowRetry = false; \
			break; \
		case SYNC_NORMAL_CASES: \
			if(!_syncWait(timeout)) \
			{ \
				allowRetry = false; \
				tempRet = BLE_CONN_ERR_SYNC_TIMEOUT; \
			} \
			break; \
		default: \
			allowRetry = false; \
			break; \
		} \
	} \
	if(tempRet != BLE_CONN_SUCCESS) \
	{ \
		return tempRet; \
	} \
	allowRetry = true; \
	while(allowRetry) \
	{ \
		tempRet = _checkStateCharHandle checkStateParams ; \
		switch(tempRet) \
		{ \
		case BLE_CONN_SUCCESS: \
			allowRetry = false; \
			break; \
		case SYNC_NORMAL_CASES: \
			if(!_syncWait(timeout)) \
			{ \
				allowRetry = false; \
				tempRet = BLE_CONN_ERR_SYNC_TIMEOUT; \
			} \
			break; \
		default: \
			allowRetry = false; \
			break; \
		} \
	} \
	return tempRet;


MiTokenBLEConnection::MiTokenBLEConnection(MiTokenBLE* parent)
{
	doingWrite = doingLongWrite = false;

	this->_extendedWriteCurrentOffset = false;
	this->_extendedWriteBlockSize = 18;

	_parent = nullptr;
	memset(&_address, 0, sizeof(_address));
	_connectionID = 0;
	_timedOut = false;
	_isConnected = false;
	_requestID = 0;

	_extendedWriteBuffer = new MessageBuffer();
	_extendedWriteData = nullptr;
	_extendedWriteDataLength = 0;
	_extendedWriteAttributeID = 0;
	_extendedWriteCurrentOffset = 0;

	_currentRSSIPollID = 0;
	_totalRSSI = 0;
	_currentRSSICount = 0;

	_attributeScanCompleted = true;
	_serviceScanCompleted = false;

	_rootAttributeNode = nullptr;
	_rootServiceNode = nullptr;

	_attributesWrittenToInLongWrite = nullptr;
	_attributesWrittenToInLongWriteCount = 0;

	nextNode = nullptr;

	buttonPushed = nullptr;

	_parent = parent;
	this->_requestID = _parent->GetNextRequestID();
}



/*
void DeviceConnected(ConnectionData* myData, uint8 connectionHandle)
{
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
*/

BLE_CONN_RET MiTokenBLEConnection::Initialize(mac_address address)
{
	memcpy(&_address, &address, sizeof(mac_address));
	_parent->getLowLevel()->ConnectToDevice(address);
	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::TimeoutRequest()
{
	if(this->IsConnected() == BLE_CONN_ERR_NOT_CONNECTED)
	{
		_parent->getLowLevel()->EndProcedure();
		_timedOut = true;
		_parent->getLowLevel()->GapDiscover(1);
		return BLE_CONN_ERR_CONNECTION_TIMED_OUT;
	}
	return BLE_CONN_SUCCESS;
}

//REQUEST_ID MiTokenBLEConnection::getRequestID(){}
bool MiTokenBLEConnection::timedOut() const
{
	return _timedOut;
}

BLE_CONN_RET MiTokenBLEConnection::IsConnected() const
{
	if (_isConnected)
	{
		return BLE_CONN_SUCCESS;
	}

	return BLE_CONN_ERR_NOT_CONNECTED;
}
BLE_CONN_RET MiTokenBLEConnection::ScanServicesInRange(SERVICE_HANDLE handle, uint16 maxRange)
{
	CHECK_IS_CONNECTED;
	CHECK_SERVICE_SCAN;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionServices* service = getService(handle, false);

	if (service == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_SERVICE;
	}

	uint16 start, end, length;
	start = service->start;
	end = service->end;
	length = end - start;

	if (length > maxRange)
	{
		length = maxRange;
		end = start + length;
	}

	
	beginAttributeScan(handle);
	_parent->getLowLevel()->FindInformationOnService(_connectionID, start, end);

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::SetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = getAttribute(handle, false);

	if (attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	attribute->writing = true;
	if (length > 20)
	{
		//do extended write
		doingLongWrite = true;
		SetNextExtendedWrite(attribute->characteristicsHandle, length, data);
		CALLBACK_LongWriteSectionCompleted();
	}
	else
	{
		doingWrite = true;
		_parent->getLowLevel()->AttributeWrite(_connectionID, attribute->characteristicsHandle, length, data);
	}

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::StartGetAttribute(ATTRIBUTE_HANDLE handle)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = getAttribute(handle, false);

	if(attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	if (attribute->reading)
	{
		return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;
	_parent->getLowLevel()->ReadAttribute(_connectionID, attribute->characteristicsHandle);

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::GetAttribute(ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer) const
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = constGetAttribute(handle);

	if(attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	if (attribute->reading)
	{
		return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	realLength = attribute->dataLength;

	if (buffer == nullptr)
	{
		return BLE_CONN_SUCCESS;
	}

	if (maxLength < realLength)
	{
		return BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA;
	}

	memcpy(buffer, attribute->data, realLength);

	return BLE_CONN_SUCCESS;
}
BLE_CONN_RET MiTokenBLEConnection::Disconnect()
{
	CHECK_IS_CONNECTED;

	_isConnected = false;
	doingWrite = false;
	doingLongWrite = false;	

	_parent->getLowLevel()->Disconnect(_connectionID);

	ConnectionAttributes* pAttr = this->_rootAttributeNode;
	while(pAttr)
	{
		pAttr->reading = false;
		pAttr->longread = false;
		pAttr = (ConnectionAttributes*)pAttr->nextNode;
	}

	_connectionID = -1;
}

BLE_CONN_RET MiTokenBLEConnection::PollRSSI()
{
	CHECK_IS_CONNECTED;
	_parent->getLowLevel()->PollRSSI(_connectionID);
}

BLE_CONN_RET MiTokenBLEConnection::GetRSSI(int& rssi) const
{
	CHECK_IS_CONNECTED;
	rssi = getAverageRSSI();

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::ButtonPressedCallback(void(__stdcall *buttonPressed) (IMiTokenBLEConnection* sender, int buttonID))
{
	CHECK_IS_CONNECTED;

	this->buttonPushed = buttonPressed;
}

BLE_CONN_RET MiTokenBLEConnection::ReadLong(ATTRIBUTE_HANDLE handle)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = getAttribute(handle, false);
	if (attribute->reading)
	{
		return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	attribute->reading = true;
	_parent->getLowLevel()->LongReadAttribute(_connectionID, attribute->characteristicsHandle);

	return BLE_CONN_SUCCESS;
}
MessageBufferObject MiTokenBLEConnection::CreateNewMessageBuffer()
{
	return new MessageBuffer();
}

BLE_CONN_RET MiTokenBLEConnection::WriteMessageBuffer(MessageBufferObject messageBuffer)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	doingLongWrite = true;
	AppendToExtendedWrite((MessageBuffer*)messageBuffer);

	delete ((MessageBuffer*)messageBuffer);
	CALLBACK_LongWriteSectionCompleted();

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::AddMessageToMessageBuffer(MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data)
{
	MessageBuffer* buffer = (MessageBuffer*)messageBuffer;

	buffer->addMessage(handle, length, data);

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnection::GetCharacteristicHandle(ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle) const
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = constGetAttribute(handle);

	if (attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	characteristicHandle = attribute->characteristicsHandle;

	return BLE_CONN_SUCCESS;
}

int MiTokenBLEConnection::GetRequestID() const
{
	return _requestID;
}

void MiTokenBLEConnection::gotConnection(uint8 connectionHandle)
{
	this->_connectionID = connectionHandle;
	this->_isConnected = true;
	this->_timedOut = false;
}

void MiTokenBLEConnection::CALLBACK_ServiceSearchFinished()
{
	finishServiceScan();
}

void MiTokenBLEConnection::CALLBACK_ServiceInformationFinished()
{
	finishAttributeScan();
}

void MiTokenBLEConnection::CALLBACK_WriteAttributeFinished()
{
	doingWrite = false;
}

void MiTokenBLEConnection::CALLBACK_LongWriteSectionCompleted()
{
	uint8 nextWrite[18] = { 0 }; //writes can be up to 18 bytes
	uint8 writeSize = sizeof(nextWrite);
	uint16 attributeID = 0;
	uint16 offset = 0;
	uint16 totalSizeLeft = 0;

	if (GetNextExtendedWrite(attributeID, offset, writeSize, nextWrite, totalSizeLeft))
	{
		longWriteWroteToAttribute(attributeID);
		if(this->_extendedWriteResetOffset)
		{
			offset = 0;
		}

		if(this->_extendedWriteResetOffset)
		{
			_ble::_cmd::_attclient::_write::_command(_parent->getLowLevel(), this->_connectionID, attributeID, writeSize, nextWrite);
		}
		else
		{
			_parent->getLowLevel()->ExtendedAttributeWrite(_connectionID, attributeID, offset, writeSize, nextWrite);
		}
	}
	else
	{
		//we have no more data to write
		doingWrite = true;
		_parent->getLowLevel()->ExecuteWrite(_connectionID, 1);
	}
}
void MiTokenBLEConnection::CALLBACK_LongReadCompleted(uint16 handle)
{
	ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(handle);
	if (attribute == nullptr)
	{
		if (attribute->longread == true)
		{
			attribute->longread = false;
			attribute->reading = false;
		}
	}
}
void MiTokenBLEConnection::CALLBACK_ExtendedWriteCompleted()
{
	doingLongWrite = false;
}
void MiTokenBLEConnection::CALLBACK_ConnectionClosed()
{
	Disconnect();
}

void MiTokenBLEConnection::CALLBACK_ServiceFound(uint8 serviceHandleLength, const uint8* serviceHandleData, uint16 start, uint16 end)
{
	ConnectionServices* service;
	if (serviceHandleLength != 2)
	{
		if(_parent->allowLongServiceUUIDs())
		{
			printf("Found long service : (");
			for(int i = 0 ; i < serviceHandleLength ; ++i)
			{
				printf("%02lX", serviceHandleData[i]);
			}
			printf(") ");
			if(serviceHandleLength == 0x10)
			{
				printf("GUID Handle [%02lX%02lX] ", serviceHandleData[13], serviceHandleData[12]);
			}
			printf("[%04lX:%04lX]\r\n", start, end);
			service = getServiceLongUUID(serviceHandleData, serviceHandleLength, true);
			service->start = start;
			service->end = end;
		}
		//something is wrong if the handle length isn't 2
	}

	else
	{
		printf("Found service %04lX [%04lX:%04lX]\r\n", *(uint16*)serviceHandleData, start, end);
		service = getService(*((uint16*)serviceHandleData), true);
		service->start = start;
		service->end = end;
	}
}

void MiTokenBLEConnection::CALLBACK_ServiceInformationFound(uint16 characteristicHandle, uint8 dataLen, const uint8* data)
{
	ConnectionAttributes* attribute;
	if (dataLen != 2)
	{
		MiTokenBLEV2_2* v2_2_interface = dynamic_cast<MiTokenBLEV2_2*>(_parent);
		if(v2_2_interface->allowLongServiceUUIDs())
		{
			if(dataLen == 0x10)
			{
				if(UUID_IS_T1_FORMAT(data))
				{
					uint16 T1GUID = UUID_FROM_T1_FORMAT(data);
					attribute = getAttribute(T1GUID, true, characteristicHandle, true);
					attribute->characteristicsHandle = characteristicHandle;
					attribute->serviceUUID = _currentServiceUUID;
					attribute->serviceIsGUID = _currentServiceIsGUID;
					attribute->attributeIsGUID = true;
				}
			}
		}
		//something is wrong if the handle length isn't 2	
	}
	else
	{
		printf("Found attribute %04lX @ %04lX\r\n", characteristicHandle, *(uint16*)data);
		attribute = getAttribute(*((uint16*)data), true, characteristicHandle);
		attribute->characteristicsHandle = characteristicHandle;
		attribute->serviceUUID = _currentServiceUUID;
		attribute->serviceIsGUID = _currentServiceIsGUID;
	}
}

void MiTokenBLEConnection::CALLBACK_HandleValue(uint8 type, uint16 attributeHandle, uint8 dataLength, const uint8* data)
{
	ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(attributeHandle);

	if ((attribute != NULL) && (attribute->reading))
	{
		if (type == 4)
		{
			if (!attribute->longread)
			{
				attribute->dataLength = dataLength;
				if (attribute->data)
				{
					free(attribute->data);
				}
				attribute->data = (uint8*)malloc(dataLength);
				memcpy(attribute->data, data, dataLength);
				attribute->longread = true;
			}
			else
			{
				int newLen;
				uint8* newData;
				newLen = attribute->dataLength + dataLength;
				newData = (uint8*)malloc(newLen);
				memcpy(newData, attribute->data, attribute->dataLength);
				memcpy(newData + attribute->dataLength, data, dataLength);
				free(attribute->data);

				attribute->dataLength = newLen;
				attribute->data = newData;
			}
		}
		else
		{
			attribute->dataLength = dataLength;
			if (attribute->data)
			{
				free(attribute->data);
			}
			attribute->data = (uint8*)malloc(dataLength);
			memcpy(attribute->data, data, dataLength);

			attribute->reading = false;
		}
	}
	else
	{
		if (buttonPushed)
		{
			int value = 0;
			for (int i = 0; i < dataLength; ++i)
			{
				value <<= 8;
				value |= data[i];
			}

			buttonPushed(this, value);
		}
	}
}

void MiTokenBLEConnection::CALLBACK_RSSIValue(int8 rssi)
{
	addPoll(_parent->GetPollID(), rssi);

}

void MiTokenBLEConnection::CALLBACK_PrepareWriteResponse(uint16 result)
{
	//no idea what this should do
}

void MiTokenBLEConnection::AppendToExtendedWrite(MessageBuffer* messageBuffer)
{
	this->_extendedWriteBuffer->append(messageBuffer);
}

bool MiTokenBLEConnection::GetNextExtendedWrite(uint16& attributeID,  uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft)
{
	if(_extendedWriteBuffer->hasDataWaiting())
	{
		bool lastWrite = _extendedWriteBuffer->getWaitingData(attributeID, offset, dataLength, data, totalSizeLeft);
	}
	else
	{
		if(_extendedWriteData == nullptr)
		{
			return false;
		}

		bool lastWrite = true;
		attributeID = _extendedWriteAttributeID;
		int realDataLength = (_extendedWriteDataLength - _extendedWriteCurrentOffset);
		if(realDataLength > dataLength)
		{
			lastWrite = false;
			realDataLength = dataLength;
		}

		dataLength = realDataLength;

		offset = _extendedWriteCurrentOffset;
		memcpy(data, &(((uint8*)_extendedWriteData)[_extendedWriteCurrentOffset]), dataLength);
		_extendedWriteCurrentOffset += dataLength;


		if(lastWrite)
		{
			free(_extendedWriteData);
			_extendedWriteData = nullptr;
			_extendedWriteAttributeID = _extendedWriteDataLength = _extendedWriteCurrentOffset = 0;
		}
	}

	return true;
}

void MiTokenBLEConnection::CALLBACK_Connected(uint8 connectionID)
{
	_connectionID = connectionID;
	_isConnected = true;
	_timedOut = false;
	_parent->getLowLevel()->DiscoverServicesInitiate(connectionID, 0x0001, 0xFFFF);

}

void MiTokenBLEConnection::CALLBACK_ProcedureCompleted(uint16 chrhandle, uint8 result)
{
	_lastError = result;
	if(result != 0)
	{
		printf("!!! SOMETHING IS SUPER WRONG GUYS [%d]!!!\r\n", result);
	}
	if(doingLongWrite)
	{
		if(!doingWrite)
		{
			//we are not yet up to the final write
			CALLBACK_LongWriteSectionCompleted();
			return;
		}
		else
		{
			//we have finished all the writes
			for(int i = 0 ; i < _attributesWrittenToInLongWriteCount ; ++i)
			{
				ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(_attributesWrittenToInLongWrite[i]);
				if(attr != nullptr)
				{
					attr->writing = false;
				}
			}
			_attributesWrittenToInLongWriteCount = 0;
			
			delete[] _attributesWrittenToInLongWrite;
			_attributesWrittenToInLongWrite = nullptr;
			doingWrite = false;
			doingLongWrite = false;
			return;
		}
	}
	else if(doingWrite)
	{
		int x = 0;
		ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(chrhandle);
		if(attr != nullptr)
		{
			if(attr->writing)
			{
				attr->writing = false;
				doingWrite = false;
			}
		}
		return;
	}
	else
	{
		//assume it maybe a read
		ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(chrhandle);
		if(attribute != nullptr)
		{
			if((attribute->reading) || (attribute->longread))
			{
				attribute->reading = attribute->longread = false;
			}
		}
	}
	
	if(_attributeScanCompleted == false)
	{
		_currentServiceUUID = 0;
	}

	_serviceScanCompleted = true;
	_attributeScanCompleted = true;

	//TODO
	printf("Procedure Completed, ChrHandle = %d, Result = %d\r\n", chrhandle, result);
	switch(chrhandle)
	{
	case 0:
		_serviceScanCompleted = true;
		break;
	default:
		//printf("!!! PROCEDURE COMPLETED, UNHANDLED CHRHANDLE! CHRHANDLE = %d, RESULT = %d\r\n", chrhandle, result);
		break;
	}
}

void MiTokenBLEConnection::SetNextExtendedWrite(uint16 attributeID, uint16 dataLength, const void* data)
{
	_extendedWriteData = malloc(dataLength);
	memcpy(_extendedWriteData, data, dataLength);
	_extendedWriteAttributeID = attributeID;
	_extendedWriteCurrentOffset = 0;
	_extendedWriteDataLength = dataLength;
}

void MiTokenBLEConnection::addPoll(int pollID, int rssi)
{
	if(_currentRSSIPollID != pollID)
	{
		_currentRSSIPollID = pollID;
		_totalRSSI = 0;
		_currentRSSICount = 0;
	}

	if(rssi <= -103)
	{
		return;
	}

	_totalRSSI += rssi;
	_currentRSSICount++;

	return;
}

int MiTokenBLEConnection::getAverageRSSI() const
{
	if(_currentRSSICount == 0)
	{
		return 0;
	}

	return (_totalRSSI / _currentRSSICount);
}

void MiTokenBLEConnection::finishAttributeScan()
{
	_attributeScanCompleted = true;
}

bool MiTokenBLEConnection::attributeScanCompleted() const
{
	return _attributeScanCompleted;
}

void MiTokenBLEConnection::finishServiceScan()
{
	_serviceScanCompleted = true;
}

bool MiTokenBLEConnection::serviceScanCompleted() const
{
	return _serviceScanCompleted;
}

bool MiTokenBLEConnection::beginAttributeScan(uint16 serviceUID)
{
	_currentServiceUUID = serviceUID;
	_currentServiceIsGUID = false;
	_attributeScanCompleted = false;
	return true;
}

ConnectionAttributes* MiTokenBLEConnection::getAttributeFromCharacteristicsHandle(uint16 handle)
{
	ConnectionAttributes* ret = _rootAttributeNode;

	while(ret != nullptr)
	{
		if(ret->characteristicsHandle == handle)
		{
			return ret;
		}

		ret = (ConnectionAttributes*)ret->nextNode;
	}

	return ret;
}

ConnectionAttributes* MiTokenBLEConnection::constGetAttribute(uint16 uuid) const
{
	//just do a const_cast. createNewAttribute
	return const_cast<MiTokenBLEConnection*>(this)->getAttribute(uuid, false);
}

ConnectionAttributes* MiTokenBLEConnection::getAttribute(uint16 uuid, bool createNewAttribute, uint16 characteristicHandle, bool GUILDHandle)
{
	MiTokenBLEV2_2* iV2_2 = dynamic_cast<MiTokenBLEV2_2*>(_parent);

	bool useNonUniqueUUIDMode = false;

	if((iV2_2 != nullptr) && (iV2_2->useNonUniqueAttributeUUIDMode()))
	{
		useNonUniqueUUIDMode = true;
	}

	if(_rootAttributeNode == nullptr)
	{
		if(createNewAttribute)
		{
			_rootAttributeNode = new ConnectionAttributes(uuid);
			return _rootAttributeNode;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		if(!useNonUniqueUUIDMode)
		{
			int offset = GetOffsetFromVoidPointers(&(_rootAttributeNode->ID), _rootAttributeNode);
			if(createNewAttribute)
			{
				return (ConnectionAttributes*)getNodeBasedOnDataOrCreateNew(_rootAttributeNode, offset, sizeof(uuid), &uuid, &uuid);
			}
			else
			{
				return (ConnectionAttributes*)getNodeBasedOnDataIfExists(_rootAttributeNode, offset, sizeof(uuid), &uuid, nullptr);
			}
		}
		else
		{
			ConnectionAttributes* att = _rootAttributeNode, *last = nullptr;
			while(att != nullptr)
			{
				if(characteristicHandle == 0x0000)
				{
					//we cannot search on a charhandle of 0, so fall back to the UUID
					if(att->ID == uuid)
					{
						return att;
					}
				}
				else
				{
					if(att->characteristicsHandle == characteristicHandle)
					{
						return att;
					}
				}
				last = att;
				att = (ConnectionAttributes*)(att->nextNode);
			}
			if(last->nextNode == nullptr)
			{
				last->nextNode = new ConnectionAttributes(uuid);
				att = (ConnectionAttributes*)(last->nextNode);
				att->characteristicsHandle = characteristicHandle;
				return att;
			}
		}
	}
}

ConnectionAttributes* MiTokenBLEConnectionV2_2::getAttributeEx(uint16 serviceUUID, uint16 attributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount)
{
	ConnectionAttributes* node = _rootAttributeNode;

	while(node != nullptr)
	{
		if(((node->serviceUUID == serviceUUID) || (serviceUUID == 0x0000)) 
			&& (node->ID == attributeUUID) 
			&& (node->serviceIsGUID == serviceUUIDIsGUID) 
			&& (node->attributeIsGUID == attributeUUIDIsGUID))
		{
			if(skipCount)
			{
				skipCount--;
			}
			else
			{
				return node;
			}
		}

		node = (ConnectionAttributes*)(node->getNextNode());
	}

	return nullptr;
}

ConnectionServices* MiTokenBLEConnection::getService(uint16 uuid, bool createNewService)
{
	if(_rootServiceNode == nullptr)
	{
		if(createNewService)
		{
			_rootServiceNode = new ConnectionServices(uuid);
			return _rootServiceNode;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		//int offset = GetOffsetFromVoidPointers(&(_rootServiceNode->ID), _rootServiceNode);
		int offset = GetOffsetFromVoidPointersV2(_rootServiceNode, ID);
		if(createNewService)
		{
			return (ConnectionServices*)getNodeBasedOnDataOrCreateNew(_rootServiceNode, offset, sizeof(uuid), &uuid, &uuid);
		}
		else
		{
			return (ConnectionServices*)getNodeBasedOnDataIfExists(_rootServiceNode, offset, sizeof(uuid), &uuid, nullptr);
		}
	}
}

ConnectionServices* MiTokenBLEConnection::getServiceLongUUID(const uint8* uuid, uint8 uuidLen, bool createNewService)
{
	if(_rootServiceNode == nullptr)
	{
		if(createNewService)
		{
			_rootServiceNode = new ConnectionServices(uuid, uuidLen);
			return _rootServiceNode;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		ConnectionServices* node = nullptr, *lastNode = nullptr;
		node = _rootServiceNode;
		while(node != nullptr)
		{
			if(node->longID != nullptr)
			{
				if((node->longIDLen == uuidLen) &&
					(memcmp(node->longID, uuid, uuidLen) == 0))
				{
					return node;
				}
			}
			lastNode = node;
			node = (ConnectionServices*)node->nextNode;
		}

		if(node == nullptr)
		{
			if(createNewService)
			{
				node = new ConnectionServices(uuid, uuidLen);
				lastNode->nextNode = node;
				return node;
			}
			else
			{
				return nullptr;
			}
		}
	}
}

void MiTokenBLEConnection::GetAddress(mac_address& address) const
{
	memcpy(&address, &(_address), sizeof(mac_address));
}

BaseLinkedList* MiTokenBLEConnection::createNewNode(void* initializeData)
{
	this->nextNode = new MiTokenBLEConnection((MiTokenBLE*)initializeData);

	return this->nextNode;
}

uint8 MiTokenBLEConnection::GetLastError() const
{
	return _lastError;
}

void MiTokenBLEConnection::longWriteWroteToAttribute(uint16 chrHandle)
{
	bool exists = false;
	for(int i = 0 ; ((i < _attributesWrittenToInLongWriteCount) && (!exists)) ; ++i)
	{
		if(_attributesWrittenToInLongWrite[i] == chrHandle)
		{
			exists = true;
		}
	}

	if(!exists)
	{
		int newCount = _attributesWrittenToInLongWriteCount + 1;
		uint16* newArray = new uint16[newCount];
		newArray[0] = chrHandle;
		memcpy(newArray + 1, _attributesWrittenToInLongWrite, sizeof(uint16) * _attributesWrittenToInLongWriteCount);
		delete[] _attributesWrittenToInLongWrite;
		_attributesWrittenToInLongWrite = newArray;
		_attributesWrittenToInLongWriteCount = newCount;

		ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(chrHandle);
		if(attr != nullptr)
		{
			attr->writing = true;
		}
	}
}


BaseLinkedList* MiTokenBLEConnectionV2_2::createNewNode(void* initializeData)
{
	this->nextNode = new MiTokenBLEConnectionV2_2((MiTokenBLE*)initializeData);

	return this->nextNode;
}


MiTokenBLEConnectionV2_2::MiTokenBLEConnectionV2_2(MiTokenBLE* parent) : MiTokenBLEConnection(parent)
{
	_syncHandle = CreateEvent(nullptr, false, false, nullptr);
	_blockBufferMutex = CreateMutex(nullptr, false, nullptr);
	_extendedWriteResetOffset = false;
	_blocksBeingWritten = 0;
	_blocksToBuffer = 1;
	_hasJeromeAccess = false;
}

MiTokenBLEConnectionV2_2::~MiTokenBLEConnectionV2_2()
{
	CloseHandle(_syncHandle);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncWaitForConnection(int timeout) const
{
	_resetSyncCheck();

	BLE_CONN_RET tempRet = IsConnected();

	switch(tempRet)
	{
	case BLE_CONN_ERR_NOT_CONNECTED:
		//wait for the _syncHandle event
		if(!_syncWait(timeout))
		{
			tempRet = BLE_CONN_ERR_SYNC_TIMEOUT;
		}
		else
		{
			tempRet = IsConnected();
		}
		break;
	case BLE_CONN_SUCCESS:
		//already done
		break;
	}

	return tempRet;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncWaitForDisconnection(int timeout) const
{
	_resetSyncCheck();

	BLE_CONN_RET tempRet = IsConnected();
	while((tempRet != BLE_CONN_ERR_SYNC_TIMEOUT) && (tempRet != BLE_CONN_ERR_NOT_CONNECTED))
	{
		switch(tempRet)
		{
		case BLE_CONN_ERR_NOT_CONNECTED:
			break; // already done
		case BLE_CONN_SUCCESS:
			if(!_syncWait(timeout))
			{
				tempRet = BLE_CONN_ERR_SYNC_TIMEOUT;
			}
			else
			{
				tempRet = IsConnected();
			}
		}
	}

	if(tempRet == BLE_CONN_ERR_NOT_CONNECTED)
	{
		tempRet = BLE_CONN_SUCCESS;
	}

	return tempRet;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncScanServiesInRange(SERVICE_HANDLE handle, uint16 maxRange, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(ScanServicesInRange, (handle, maxRange), true, true, true, true, false, -1);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncRequestAttribute(ATTRIBUTE_HANDLE handle, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(StartGetAttribute, (handle), true, true, true, true, true, handle);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncRequestLongAttribute(ATTRIBUTE_HANDLE handle, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(ReadLong, (handle), true, true, true, true, true, handle);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncSetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(SetAttribute, (handle, length, data), true, true, true, true, false, -1);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncReadAttribute(const ATTRIBUTE_HANDLE handle, bool longRead, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout)
{
	BLE_CONN_RET tempRet ;
	if(!longRead)
	{
		tempRet = SyncRequestAttribute(handle, timeout);
	}
	else
	{
		tempRet = SyncRequestLongAttribute(handle, timeout);
	}

	if(tempRet != BLE_CONN_SUCCESS)
	{
		return tempRet;
	}

	auto attribute = getAttribute(handle, false);
	while(attribute->reading)
	{
		if(!_syncWait(timeout))
		{
			return BLE_CONN_ERR_SYNC_TIMEOUT;
		}
	}

	if((attribute->dataLength > length) || (data == nullptr))
	{
		if(data != nullptr)
		{
			deallocator(data);
		}

		data = allocator(attribute->dataLength);
		length = attribute->dataLength;
	}

	memcpy(data, attribute->data, length);

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncWaitForState(const ATTRIBUTE_HANDLE handle, bool longRead, const uint8* stateList, const uint8 stateCount, const uint8 stateLength, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout)
{
	bool needToFind = true;
	uint8 length = 0;
	uint8* data = nullptr;

	BLE_CONN_RET tempRet = BLE_CONN_ERR_INTERNAL_ERROR;
	while(needToFind)
	{
		tempRet = SyncReadAttribute(handle, longRead, length, data, allocator, deallocator, timeout);
		if(tempRet == BLE_CONN_SUCCESS)
		{
			if(length == stateLength)
			{
				bool found = false;
				for(int i = 0 ; i < stateCount; ++i)
				{
					if(memcmp(data, &stateList[i * stateLength], stateLength) == 0)
					{
						needToFind = false;
						tempRet = BLE_CONN_SUCCESS;
					}
				}
			}
		}
		else
		{
			needToFind = false;
		}
	}

	if(data != nullptr)
	{
		deallocator(data);
	}

	return tempRet;
}


void MiTokenBLEConnectionV2_2::TriggerSyncCheck() const
{
	SetEvent(_syncHandle);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::GetCharacteristicHandleEx(SERVICE_HANDLE service, ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID, bool attributeIsGUID, int skipCount)
{
	CHECK_IS_CONNECTED;
	CHECK_SERVICE_SCAN;
	CHECK_ATTRIBUTE_SCAN;

	ConnectionAttributes* node = _rootAttributeNode;

	while(node != nullptr)
	{
		if((node->serviceUUID == service) && (node->ID == attribute) && (node->serviceIsGUID == serviceIsGUID) && (node->attributeIsGUID == attributeIsGUID))
		{
			if(skipCount)
			{
				skipCount--;
			}
			else
			{
				handle = node->characteristicsHandle;
				return BLE_CONN_SUCCESS;
			}
		}
		node = node->getNextNode();
	}

	return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncGetCharacteristicHandleEx(SERVICE_HANDLE service, ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, int timeout, bool serviceIsGUID, bool attributeIsGUID, int skipCount)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(GetCharacteristicHandleEx, (service, attribute, handle, serviceIsGUID, attributeIsGUID, skipCount), true, true, true, false, false, -1);
}


BLE_CONN_RET MiTokenBLEConnectionV2_2::ScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange)
{
	uint8 guidID[0x10] = {T1_BASE_UUID_128(handle)};
	T1_BASE_UUID_128_FIX(guidID, handle);

	CHECK_IS_CONNECTED;
	CHECK_SERVICE_SCAN;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionServices* service = getServiceLongUUID(guidID, 0x10, false);
	if(service == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_SERVICE;
	}

	uint16 start, end, length;
	start = service->start;
	end = service->end;
	length = end - start;
	if(length > maxRange)
	{
		length = maxRange;
		end = start + length;
	}

	beginAttributeScan(handle);
	_currentServiceIsGUID = true; //this service is a GUID
	_parent->getLowLevel()->FindInformationOnService(_connectionID, start, end);

	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL(ScanServicesInRangeOfGUIDService, (handle, maxRange), true, true, true, true, false, -1);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncCustomWait(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, uint16 checkServiceUUID, uint16 checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL_EX(returnSuccess, (), checkConnection, checkServiceScan, checkAttributeScan, checkWriting, checkReading, checkServiceUUID, checkAttributeUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount);
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(handle);

	if(attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	attribute->writing = true;
	if(length > 20)
	{
		doingLongWrite = true;
		SetNextExtendedWrite(handle, length, data);
		CALLBACK_LongWriteSectionCompleted();
	}
	else
	{
		doingWrite = true;
		_parent->getLowLevel()->AttributeWrite(_connectionID, handle, length, data);
	}

	return BLE_CONN_SUCCESS;
}
BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncSetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL_CHRHANDLE(SetAttributeFromCharHandle, (handle, length, data), true, true, true, true, true, handle);
}
BLE_CONN_RET MiTokenBLEConnectionV2_2::RequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead)
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(handle);

	if(attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}

	if(attribute->reading)
	{
		return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}

	if(longRead)
	{
		attribute->reading = true;
		_parent->getLowLevel()->LongReadAttribute(_connectionID, handle);

		return BLE_CONN_SUCCESS;
	}
	else
	{
		attribute->reading = true;
		_parent->getLowLevel()->ReadAttribute(_connectionID, handle);
		return BLE_CONN_SUCCESS;
	}
}
BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncRequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead, int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL_CHRHANDLE(RequestAttributeFromCharHandle, (handle, longRead), true, true, true, true, true, handle);
}
BLE_CONN_RET MiTokenBLEConnectionV2_2::ReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value))
{
	CHECK_IS_CONNECTED;
	CHECK_ATTRIBUTE_SCAN;
	CHECK_WRITES;

	auto attribute = getAttributeFromCharacteristicsHandle(handle);
	if(attribute == nullptr)
	{
		return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
	}
	if(attribute->reading)
	{
		return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
	}
	if((attribute->dataLength > length) || (data == nullptr))
	{
		if(data != nullptr)
		{
			deallocator(data);
		}

		data = allocator(attribute->dataLength);
		length = attribute->dataLength;
	}

	memcpy(data, attribute->data, length);
	return BLE_CONN_SUCCESS;

}
BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout)
{
	SYNC_FUNCTION_PROTOTYPE_FULL_CHRHANDLE(ReadAttributeFromCharHandle, (handle, length, data, allocator, deallocator), true, true, true, true, true, handle);	
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::UpdateConnectionParameters(uint16 minimum, uint16 maximum, uint16 timeout, uint16 latency)
{
	_ble::_cmd::_connection::_update(_parent->getLowLevel(), _connectionID, minimum, maximum, latency, timeout);
	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::returnSuccess() const
{
	return BLE_CONN_SUCCESS;
}

void MiTokenBLEConnectionV2_2::_resetSyncCheck() const
{
	ResetEvent(_syncHandle);
}

bool MiTokenBLEConnectionV2_2::_syncWait(int timeout) const
{
	if(WaitForSingleObject(_syncHandle, timeout) == WAIT_OBJECT_0)
	{
		return true;
	}
	return false;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::_checkState(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWrites, bool checkAttributeRead, uint16 checkServiceReadUUID, uint16 checkAttributeReadUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount)
{
	if(checkConnection)
	{
		CHECK_IS_CONNECTED;
	}
	if(checkServiceScan)
	{
		CHECK_SERVICE_SCAN;
	}
	if(checkAttributeScan)
	{
		CHECK_ATTRIBUTE_SCAN;
	}
	if(checkWrites)
	{
		CHECK_WRITES;
	}
	if(checkAttributeRead)
	{
		ConnectionAttributes* attribute = getAttributeEx(checkServiceReadUUID, checkAttributeReadUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount);
		if(attribute == nullptr)
		{
			return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
		}
		if(attribute->reading)
		{
			return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
		}
	}

	return BLE_CONN_SUCCESS;
	
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::_checkStateCharHandle(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWrites, bool checkAttributeRead, uint16 charHandle)
{
	if(checkConnection)
	{
		CHECK_IS_CONNECTED;
	}
	if(checkServiceScan)
	{
		CHECK_SERVICE_SCAN;
	}
	if(checkAttributeScan)
	{
		CHECK_ATTRIBUTE_SCAN;
	}
	if(checkWrites)
	{
		CHECK_WRITES;
	}
	if(checkAttributeRead)
	{
		ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(charHandle);
		if(attribute == nullptr)
		{
			return BLE_CONN_ERR_NO_SUCH_ATTRIBUTE;
		}
		if(attribute->reading)
		{
			return BLE_CONN_ERR_ATTRIBUTE_BEING_READ;
		}
	}

	return BLE_CONN_SUCCESS;
	
}

//Extra wrapping code around the MiTokenBLEConnection::CALLBACK_LongWriteSectionCompleted to deal with a buffered write.
void MiTokenBLEConnectionV2_2::CALLBACK_LongWriteSectionCompleted()
{
	WaitForSingleObject(_blockBufferMutex, INFINITE);

	
	while(_blocksToBuffer > 0 && (!doingWrite) && (!(_blocksBeingWritten != 0 && _nextExtendedWriteWouldBeLast())))
	{
		printf("Doing a buffered write, _blockToBuffer : %d, _blocksBeingWritten : %d\r\n", _blocksToBuffer, _blocksBeingWritten);
		MiTokenBLEConnection::CALLBACK_LongWriteSectionCompleted();
		_blocksToBuffer--;
		_blocksBeingWritten++;
	}

	ReleaseMutex(_blockBufferMutex);
}

bool MiTokenBLEConnectionV2_2::_nextExtendedWriteWouldBeLast()
{
	if(_extendedWriteBuffer->hasDataWaiting())
	{
		return _extendedWriteBuffer->getWaitingDataSize() < 18;
	}
	if(_extendedWriteData == nullptr)
	{
		return true;
	}
	int realLen = (_extendedWriteDataLength - _extendedWriteCurrentOffset);
	return (realLen < 18);
}

void MiTokenBLEConnectionV2_2::CALLBACK_ProcedureCompleted(uint16 chrhandle, uint8 result)
{
	_lastError = result;
	if(result != 0)
	{
		printf("!!! SOMETHING IS SUPER WRONG GUYS [%d]!!!\r\n", result);
	}
	if(doingLongWrite)
	{
		bool allBlocksWritten = false;
		WaitForSingleObject(_blockBufferMutex, INFINITE);
		
		_blocksBeingWritten--;
		_blocksToBuffer++;
		printf("Block written, _blockToBuffer : %d, _blocksBeingWritten : %d\r\n", _blocksToBuffer, _blocksBeingWritten);
		allBlocksWritten = (_blocksBeingWritten == 0);
		ReleaseMutex(_blockBufferMutex);

		if(!doingWrite)
		{
			//we are not yet up to the final write
			CALLBACK_LongWriteSectionCompleted();
			return;
		}
		else
		{
			if(allBlocksWritten) //check we are not writing any more blocks.
			{
				//we have finished all the writes
				for(int i = 0 ; i < _attributesWrittenToInLongWriteCount ; ++i)
				{
					ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(_attributesWrittenToInLongWrite[i]);
					if(attr != nullptr)
					{
						attr->writing = false;
					}
				}
				_attributesWrittenToInLongWriteCount = 0;
			
				delete[] _attributesWrittenToInLongWrite;
				_attributesWrittenToInLongWrite = nullptr;
				doingWrite = false;
				doingLongWrite = false;
			}
			return;
		}
	}
	else if(doingWrite)
	{
		int x = 0;
		ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(chrhandle);
		if(attr != nullptr)
		{
			if(attr->writing)
			{
				attr->writing = false;
				doingWrite = false;
			}
		}
		return;
	}
	else
	{
		//assume it maybe a read
		ConnectionAttributes* attribute = getAttributeFromCharacteristicsHandle(chrhandle);
		if(attribute != nullptr)
		{
			if((attribute->reading) || (attribute->longread))
			{
				attribute->reading = attribute->longread = false;
			}
		}
	}
	
	if(_attributeScanCompleted == false)
	{
		_currentServiceUUID = 0;
	}

	_serviceScanCompleted = true;
	_attributeScanCompleted = true;

	//TODO
	printf("Procedure Completed, ChrHandle = %d, Result = %d\r\n", chrhandle, result);
	switch(chrhandle)
	{
	case 0:
		_serviceScanCompleted = true;
		break;
	default:
		//printf("!!! PROCEDURE COMPLETED, UNHANDLED CHRHANDLE! CHRHANDLE = %d, RESULT = %d\r\n", chrhandle, result);
		break;
	}
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SetWriteBlockBufferCount(int newMaxBlocksToBuffer)
{
	WaitForSingleObject(_blockBufferMutex, INFINITE);
	if(_blocksBeingWritten > 0)
	{
		ReleaseMutex(_blockBufferMutex);
		return BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS;
	}
	else
	{
		_blocksToBuffer = newMaxBlocksToBuffer;
		ReleaseMutex(_blockBufferMutex);
		return BLE_CONN_SUCCESS;
	}

}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SetExtendedWriteOffsetMode(bool offsetModeEnabled)
{
	this->_extendedWriteResetOffset = !offsetModeEnabled;
	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::SetExtendedWriteBlockSize(int newBlockSize)
{
	if(newBlockSize > 18)
	{
		return BLE_CONN_ERR_INTERNAL_ERROR;
	}

	_extendedWriteBlockSize = newBlockSize;
	return BLE_CONN_SUCCESS;
}

BLE_CONN_RET MiTokenBLEConnectionV2_2::EnableJeromeAccess()
{
	_hasJeromeAccess = true;
	return BLE_CONN_SUCCESS;
}
BLE_CONN_RET MiTokenBLEConnectionV2_2::Jerome_SendNextBlock(bool canBeLastBlock)
{
	if(!_hasJeromeAccess)
	{
		return BLE_CONN_ERR_INTERNAL_ERROR;
	}

	uint8 nextWrite[18] = { 0 }; //writes can be up to 18 bytes
	uint8 writeSize = sizeof(nextWrite);
	uint16 attributeID = 0;
	uint16 offset = 0;
	uint16 totalSizeLeft = 0;

	if (GetNextExtendedWrite(attributeID, offset, writeSize, nextWrite, totalSizeLeft))
	{
		longWriteWroteToAttribute(attributeID);
		offset = 0;
	
		_ble::_cmd::_attclient::_write::_command(_parent->getLowLevel(), this->_connectionID, attributeID, writeSize, nextWrite);
		return BLE_CONN_SUCCESS;
	}
	else
	{
		if(!canBeLastBlock)
		{
			//we have finished all the writes
			for (int i = 0; i < _attributesWrittenToInLongWriteCount; ++i)
			{
				ConnectionAttributes* attr = getAttributeFromCharacteristicsHandle(_attributesWrittenToInLongWrite[i]);
				if (attr != nullptr)
				{
					attr->writing = false;
				}
			}
			_attributesWrittenToInLongWriteCount = 0;

			delete[] _attributesWrittenToInLongWrite;
			_attributesWrittenToInLongWrite = nullptr;
			doingWrite = false;
			doingLongWrite = false;

			return BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA;
		}
		//we have no more data to write
		doingWrite = true;
		_parent->getLowLevel()->ExecuteWrite(_connectionID, 1);
		return BLE_CONN_SUCCESS;
	}
	
	
}