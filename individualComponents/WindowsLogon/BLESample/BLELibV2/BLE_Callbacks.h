#pragma once

#ifndef _BLE_CALLBACKS_H_
#define _BLE_CALLBACKS_H_

#include "apitypes.h"
#include "BLE_Structs.h"
#include "MiTokenBLEConnection.h"

void BLE_CALLBACK_ServiceFound(ConnectionData* myData, uint8 length, const uint8* data, uint16 start, uint16 end);
void BLE_CALLBACK_ServiceSearchFinished(ConnectionData* myData);
void BLE_CALLBACK_ServiceInformation(ConnectionData* myData, uint16 characteristicHandle, uint8 dataLen, const uint8* data);
void BLE_CALLBACK_ServiceInformationCompleted(ConnectionData* myData);
void BLE_CALLBACK_PollAttribute(ConnectionData* myData, uint8 type, uint16 attributeHandle, uint8 dataLen, const uint8* data);
void BLE_CALLBACK_AttributeWritten(ConnectionData* myData);
void BLE_CALLBACK_GotRSSI(ConnectionData* myData, int8 rssi);
void BLE_CALLBACK_PrepareWriteCompleted(ConnectionData* myData);
void BLE_CALLBACK_ExtendedWriteCompleted(ConnectionData* myData);
void BLE_CALLBACK_ConnectionClosed(ConnectionData* myData);
void BLE_CALLBACK_LongReadCompleted(ConnectionData* myData, uint16 attributeHandle);

#endif