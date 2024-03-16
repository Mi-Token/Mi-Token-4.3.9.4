#pragma once

#include "BLEDLLV2.h"


BLEDLLV2_API IMiTokenBLE* WINAPI MiTokenBLE_CreateInterfaceOfVersion(int versionID);
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkGetCopy(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChian);
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkSetFromCopy(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChain);
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkReset(IMiTokenBLEV2_2* instance);
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_AppendCommandChainLink(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChain, bool checkAndRemoveDuplicatePointers);
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_GetBaseCommandChain(IMiTokenBLEV2_2* instance, const IBLE_COMMAND_CHAIN_LINK* *  pCommandChain);
BLEDLLV2_API void WINAPI MiTokenBLEV2_2_ForceUnsafeDisconnection(IMiTokenBLEV2_2* instance, uint8 connectionHandle);

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_EnableNonUniqueAttributeUUIDMode(IMiTokenBLEV2_2* instance);
BLEDLLV2_API IBLE_COMMAND_CHAIN_LINK* WINAPI MiTokenBLEV2_2_CreateNewCommandChainLinkObject(int& objectSize);
BLEDLLV2_API void WINAPI MiTokenBLEV2_2_DeleteCommandChainLinkObject(IBLE_COMMAND_CHAIN_LINK* object);

BLEDLLV2_API IMiTokenBLEV2_2* WINAPI MiTokenBLEV2_2_GetV2_2_Interface(IMiTokenBLE* V2_1Interface);
BLEDLLV2_API IMiTokenBLEConnectionV2_2* WINAPI MiTokenBLEV2_2_CONN_GetV2_2_Interface(IMiTokenBLEConnection* V2_1Interface);

BLEDLLV2_API int WINAPI MiTokenBLEV2_2_Inner_GetChainPointerOffset(IBLE_COMMAND_CHAIN_LINK* object);


BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForConnection(IMiTokenBLEConnectionV2_2* instance, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForDisconnection(IMiTokenBLEConnectionV2_2* instance, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncScanServicesInRange(IMiTokenBLEConnectionV2_2* instance, SERVICE_HANDLE handle, uint16 maxRange, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestLongAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncSetAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncReadAttribute(IMiTokenBLEConnectionV2_2* instance, const ATTRIBUTE_HANDLE handle, bool longRead, uint8* length, uint8** data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* pointer), int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForState(IMiTokenBLEConnectionV2_2* instance, const ATTRIBUTE_HANDLE handle, bool longRead, const uint8* stateList, const uint8 stateCount, const uint8 stateLength, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* pointer), int timeout);

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_GetCharacteristicHandleEx(IMiTokenBLEConnectionV2_2* instance, const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID, bool attributeIsGUID, int skipCount);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncGetCharacteristicHandleEx(IMiTokenBLEConnectionV2_2* instance, const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID, bool attributeIsGUID, int skipCount, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_ScanServicesInRangeOfGUIDService(IMiTokenBLEConnectionV2_2* instance, SERVICE_GUID_HANDLE handle, uint16 maxRange);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncScanServicesInRangeOfGUIDService(IMiTokenBLEConnectionV2_2* instance, SERVICE_GUID_HANDLE handle, uint16 maxRange, int timeout);

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncCustomWait(IMiTokenBLEConnectionV2_2* instance, bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, uint16 checkServiceUUID, uint16 checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncSetAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_RequestAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, bool longRead);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, bool longRead, int timeout);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_ReadAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value));
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncReadAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout);
	
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_UpdateConnectionParameters(IMiTokenBLEConnectionV2_2* instance, uint16 minimum, uint16 maximum, uint16 timeout, uint16 latency);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetWriteBlockBufferCount(IMiTokenBLEConnectionV2_2* instance, int newMaxBlocksToBuffer);

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetExtendedWriteOffsetMode(IMiTokenBLEConnectionV2_2* instance, bool offsetModeEnabled);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetExtendedWriteBlockSize(IMiTokenBLEConnectionV2_2* instance, int newMaxBlocksToBuffer);

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_EnableJeromeAccess(IMiTokenBLEConnectionV2_2* instance);
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_Jerome_SendNextBlock(IMiTokenBLEConnectionV2_2* instance, bool canBeLastBlock);

