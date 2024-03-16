#include "BLE_Callbacks.h"
#include "BLE_LowLevel.h"

#include "DebugLogging.h"

#include "BLE_API.h"

#include <stdio.h>


#ifndef REQUIRE_BLE_API_V2_1

void BLE_CALLBACK_ServiceFound(ConnectionData* myData, uint8 length, const uint8* data, uint16 start, uint16 end)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionServices* service;
	if(length != 2)
	{
#ifndef _NOPRINT
		printf("Something is wrong! at " __FILE__ " and %d where Length != 2 but %d\r\n", __LINE__, length);
#endif
	}

	else
	{
		service = myData->getService(*((uint16*)data), true);
		service->start = start;
		service->end = end;
#ifdef _DEBUG 
#ifndef _NOPRINT
		printf("Found service at [%04lX], Location : %04lX : %04lX\r\n", service->ID, start, end);
#endif
#endif

	}
#ifndef _NOPRINT
	printf("Service Found\r\n");
#endif

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_ServiceSearchFinished(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	myData->finishServiceScan();
	myData->callbacks.serviceSearchFinished = NULL;
#ifndef _NOPRINT
	printf("All services found\r\n");
#endif

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_ServiceInformation(ConnectionData* myData, uint16 characteristicHandle, uint8 dataLen, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* attribute;
	if(dataLen != 2)
	{
#ifndef _NOPRINT
		printf("Something is wrong! at " __FILE__ " and %d where Length != 2\r\n", __LINE__);
#endif
	}
	else
	{
		attribute = myData->getAttribute(*((uint16*)data), true);
		attribute->characteristicsHandle = characteristicHandle;
#ifdef _DEBUG
#ifndef _NOPRINT
		printf("Found Attribute at [%04lX], Handle : %04lX\r\n", attribute->ID, attribute->characteristicsHandle);
#endif
#endif
	}
#ifndef _NOPRINT
	printf("Attribute Found\r\n");
#endif

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_ServiceInformationCompleted(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	myData->finishAttributeScan();
	myData->callbacks.serviceInformationFinished = NULL;
#ifndef _NOPRINT
	printf("All Attributes found\r\n");
#endif

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_LongReadCompleted(ConnectionData* myData, uint16 attributeHandle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* attribute = myData->getAttributeFromCharacteristicsHandle(attributeHandle);
	if(attribute != NULL)
	{
		if(attribute->longread == true)
		{
			printf("Long read on Attribute [%04lX] completed. Read %d bytes\r\n", attributeHandle, attribute->dataLength);
			attribute->longread = false;
			attribute->reading = false;
		}
	}

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_PollAttribute(ConnectionData* myData, uint8 type, uint16 attributeHandle, uint8 dataLen, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

#ifndef _NOPRINT
	printf("Poll Attribute [%04lX] [%02lX]\r\n\t[%02lX]\r\n\t", attributeHandle, type, dataLen);
	for(int i = 0 ; i < dataLen ; ++i)
	{
		printf("%02lX ", data[i]);
	}
	printf("\r\n");
#endif
	
	ConnectionAttributes* attribute = myData->getAttributeFromCharacteristicsHandle(attributeHandle);

	if((attribute != NULL) && (attribute->reading))
	{
		if(type == 4)
		{
			if(!attribute->longread)
			{
				attribute->dataLength = dataLen;
				if(attribute->data)
				{
					free(attribute->data);
				}
				attribute->data = (uint8*)malloc(dataLen);
				memcpy(attribute->data, data, dataLen);
				attribute->longread = true;
			}
			else
			{
				int newLen;
				uint8* newData;
				newLen = attribute->dataLength + dataLen;
				newData = (uint8*)malloc(newLen);
				memcpy(newData, attribute->data, attribute->dataLength);
				memcpy(newData + attribute->dataLength, data, dataLen);
				free(attribute->data);

				attribute->dataLength = newLen;
				attribute->data = newData;
			}
		}
		else
		{
			attribute->dataLength = dataLen;
			if(attribute->data)
			{
				free(attribute->data);
			}
			attribute->data = (uint8*)malloc(dataLen);
			memcpy(attribute->data, data, dataLen);

			attribute->reading = false;
		}
	}
	else
	{
		if(myData->buttonPushed)
		{
			int value = 0;
			for(int i = 0 ; i < dataLen ; ++i)
			{
				value <<= 8;
				value |= data[i];
			}

			myData->buttonPushed(myData->linkedRequestID, value);
		}
	}

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_AttributeWritten(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	myData->callbacks.writeAttributeFinished = NULL;
	myData->doingWrite = false;

#ifndef _NOPRINT
	printf("Attribute Written\r\n");
#endif

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_GotRSSI(ConnectionData* myData, int8 rssi)
{
	DEBUG_ENTER_FUNCTION;

	//printf("Got RSSI [%d]\r\n", rssi);
	myData->addPoll(currentPollID, rssi);

	DEBUG_END_FUNCTION;
}


void BLE_CALLBACK_ExtendedWriteCompleted(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	myData->doingLongWrite = false;
#ifndef _NOPRINT
	printf("Long write completed\r\n");
#endif
	myData->callbacks.extendedWriteCompleted = NULL;

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_PrepareWriteCompleted(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	uint8 nextWrite[18] = {0}; //writes can be upto 18 bytes
	uint8 writeSize = sizeof(nextWrite);
	uint16 attributeID = 0;
	uint16 offset = 0;
	uint16 totalSizeLeft;

	if(myData->GetNextExtendedWrite(attributeID, offset, writeSize, nextWrite, totalSizeLeft))
	{
#ifndef _NOPRINT
		printf("Writing %d of %d bytes to ID %04lX with offset %d\r\n\t",writeSize, totalSizeLeft, attributeID, offset);
		for(int i = 0 ; i < writeSize ; ++i)
		{
			printf("%02lX", nextWrite[i]);
		}
		printf("\r\n");
#endif
		BLE_LL_ExtendedAttributeWrite(myData->connectionID, attributeID, offset, writeSize, nextWrite);
	}
	else
	{
		//we have no more data to write
		myData->callbacks.longWriteSectionCompleted = NULL;
		myData->callbacks.extendedWriteCompleted = BLE_CALLBACK_ExtendedWriteCompleted;
		BLE_LL_ExecuteWrite(myData->connectionID, 1);
	}

	DEBUG_END_FUNCTION;
}

void BLE_CALLBACK_ConnectionClosed(ConnectionData* myData)
{
	DEBUG_ENTER_FUNCTION;

	BLE_CONN_Disconnect(myData->linkedRequestID);

	DEBUG_END_FUNCTION;
}

#else


void BLE_CALLBACK_ServiceFound(ConnectionData* myData, uint8 length, const uint8* data, uint16 start, uint16 end)
{
}

void BLE_CALLBACK_ServiceSearchFinished(ConnectionData* myData)
{
}

void BLE_CALLBACK_ServiceInformation(ConnectionData* myData, uint16 characteristicHandle, uint8 dataLen, const uint8* data)
{
}

void BLE_CALLBACK_ServiceInformationCompleted(ConnectionData* myData)
{
}

void BLE_CALLBACK_LongReadCompleted(ConnectionData* myData, uint16 attributeHandle)
{
}

void BLE_CALLBACK_PollAttribute(ConnectionData* myData, uint8 type, uint16 attributeHandle, uint8 dataLen, const uint8* data)
{
}

void BLE_CALLBACK_AttributeWritten(ConnectionData* myData)
{
}

void BLE_CALLBACK_GotRSSI(ConnectionData* myData, int8 rssi)
{
}


void BLE_CALLBACK_ExtendedWriteCompleted(ConnectionData* myData)
{
}

void BLE_CALLBACK_PrepareWriteCompleted(ConnectionData* myData)
{
}

void BLE_CALLBACK_ConnectionClosed(ConnectionData* myData)
{
}


#endif