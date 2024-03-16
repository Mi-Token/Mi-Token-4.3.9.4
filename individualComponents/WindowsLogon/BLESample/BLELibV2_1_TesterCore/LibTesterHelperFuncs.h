#pragma once

#include "BLE_API.h"

const char* RequestErrorCodeToString(int requestErrorCode);
bool NormalStageOutput(int response, int& oldResponse, const char* successMessage, const char* failMessage, int& stage, int nextStage);
int ScanServicesNearUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID);
int ScanServicesNearKeyChangeUUID(int requestID, int& stage, int& lastResponse, int nextStage);
int ScanServicesNearSecretServiceUUID(int requestID, int& stage, int& lastResponse, int nextStage);
int WriteDataToUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID, uint8 dataLength, uint8* dataPtr);
int SetSecretAttribute(int requestID, int& stage, int& lastResponse, int nextStage, MessageBufferObject& messageBuffer);
int SetNotifyAttribute(int requestID, int& stage, int& lastResponse, int nextStage);
int GetSecretCharacterHandle(int requestID, int& stage, int& lastResponse, int nextStage, uint16& characteristicHandle);
int LongReadSecret(int requestID, int& stage, int& lastResponse, int nextStage);
int StartReadAttribute(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID);
int ReadAttributeBuffer(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID, uint8& readLength, uint8*& readBuffer);


enum DeviceVersion
{
	DevVersion_Unknown,
	DevVersion1_0,
	DevVersion1_1,
};

//PseudoSync functions as Synchronous wrappers around the Asynchronous function calls
//In the future PseudoSync functions will use the native functions inside IMiTokenBLEConnection to be synchronous (when those functions exist)
int pseudoSync_ScanServicesNearUUID(IMiTokenBLEConnection* conn, uint16 UUID);
int pseudoSync_ReadBuffer(IMiTokenBLEConnection* conn, uint16 UUID, uint8& readLen, uint8*& readPtr);
int pseudoSync_WaitForStateBuffer(IMiTokenBLEConnection* conn, uint16 UUID, uint8 successID, uint8 waitID);
int pseudoSync_GetDeviceVersion(IMiTokenBLEConnection* conn, DeviceVersion& version);
int pseudoSync_WriteToUUID(IMiTokenBLEConnection* conn, uint16 UUID, int length, const char* data);
