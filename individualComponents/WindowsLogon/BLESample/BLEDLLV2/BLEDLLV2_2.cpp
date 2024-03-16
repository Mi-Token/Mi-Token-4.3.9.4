#include "BLEDLLV2_2.h"
#include "BLE_Command_Chain_Link.h"

BLEDLLV2_API IMiTokenBLE* WINAPI MiTokenBLE_CreateInterfaceOfVersion(int versionID)
{
	return CreateNewMiTokenBLEInterface(versionID);
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkGetCopy(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChian)
{
	return BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED;
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkSetFromCopy(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChain)
{
	return BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED;
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_BaseCommandLinkReset(IMiTokenBLEV2_2* instance)
{
	return BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED;
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_AppendCommandChainLink(IMiTokenBLEV2_2* instance, IBLE_COMMAND_CHAIN_LINK* pCommandChain, bool checkAndRemoveDuplicatePointers)
{
	return instance->AppendCommandChain(pCommandChain, checkAndRemoveDuplicatePointers);
}
BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_GetBaseCommandChain(IMiTokenBLEV2_2* instance, const IBLE_COMMAND_CHAIN_LINK* *  pCommandChain)
{
	return BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED;
}

BLEDLLV2_API void WINAPI MiTokenBLEV2_2_ForceUnsafeDisconnection(IMiTokenBLEV2_2* instance, uint8 connectionHandle)
{
	return instance->ForceUnsafeDisconnection(connectionHandle);
}


BLEDLLV2_API IBLE_COMMAND_CHAIN_LINK* WINAPI MiTokenBLEV2_2_CreateNewCommandChainLinkObject(int& objectSize)
{
	IBLE_COMMAND_CHAIN_LINK* obj = new BLE_COMMAND_CHAIN_LINK_FULL();
	objectSize = sizeof(BLE_COMMAND_CHAIN_LINK_FULL);

	return obj;
}

BLEDLLV2_API int WINAPI MiTokenBLEV2_2_Inner_GetChainPointerOffset(IBLE_COMMAND_CHAIN_LINK* obj)
{
	BLE_COMMAND_CHAIN_LINK_FULL* pfull = (BLE_COMMAND_CHAIN_LINK_FULL*)obj;
	void* vpl = &pfull->evt.system.boot;


	int iv = (int)vpl - (int)pfull;

	return iv;
}

BLEDLLV2_API void WINAPI MiTokenBLEV2_2_DeleteCommandChainLinkObject(IBLE_COMMAND_CHAIN_LINK* object)
{
	delete object;
}

BLEDLLV2_API BLE_API_RET WINAPI MiTokenBLEV2_2_EnableNonUniqueAttributeUUIDMode(IMiTokenBLEV2_2* instance)
{
	instance->EnableNonUniqueAttributeUUIDMode();
	return BLE_API_SUCCESS;
}

BLEDLLV2_API IMiTokenBLEV2_2* WINAPI MiTokenBLEV2_2_GetV2_2_Interface(IMiTokenBLE* V2_1Interface)
{
	return dynamic_cast<IMiTokenBLEV2_2*>(V2_1Interface);
}

BLEDLLV2_API IMiTokenBLEConnectionV2_2* WINAPI MiTokenBLEV2_2_CONN_GetV2_2_Interface(IMiTokenBLEConnection* V2_1Interface)
{
	return dynamic_cast<IMiTokenBLEConnectionV2_2*>(V2_1Interface);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForConnection(IMiTokenBLEConnectionV2_2* instance, int timeout)
{
	return instance->SyncWaitForConnection(timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForDisconnection(IMiTokenBLEConnectionV2_2* instance, int timeout)
{
	return instance->SyncWaitForDisconnection(timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncScanServicesInRange(IMiTokenBLEConnectionV2_2* instance, SERVICE_HANDLE handle, uint16 maxRange, int timeout)
{
	return instance->SyncScanServiesInRange(handle, maxRange, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, int timeout)
{
	return instance->SyncRequestAttribute(handle, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestLongAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, int timeout)
{
	return instance->SyncRequestLongAttribute(handle, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncSetAttribute(IMiTokenBLEConnectionV2_2* instance, ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data, int timeout)
{
	return instance->SyncSetAttribute(handle, length, data, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncReadAttribute(IMiTokenBLEConnectionV2_2* instance, const ATTRIBUTE_HANDLE handle, bool longRead, uint8* length, uint8** data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* pointer), int timeout)
{
	return instance->SyncReadAttribute(handle, longRead, *length, *data, allocator, deallocator, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncWaitForState(IMiTokenBLEConnectionV2_2* instance, const ATTRIBUTE_HANDLE handle, bool longRead, const uint8* stateList, const uint8 stateCount, const uint8 stateLength, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* pointer), int timeout)
{
	return instance->SyncWaitForState(handle, longRead, stateList, stateCount, stateLength, allocator, deallocator, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_GetCharacteristicHandleEx(IMiTokenBLEConnectionV2_2* instance, const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID, bool attributeIsGUID, int skipCount)
{
	return instance->GetCharacteristicHandleEx(service, attribute, handle, serviceIsGUID,attributeIsGUID, skipCount);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncGetCharacteristicHandleEx(IMiTokenBLEConnectionV2_2* instance, const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID, bool attributeIsGUID, int skipCount, int timeout)
{
	return instance->SyncGetCharacteristicHandleEx(service, attribute, handle, timeout, serviceIsGUID, attributeIsGUID, skipCount);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_ScanServicesInRangeOfGUIDService(IMiTokenBLEConnectionV2_2* instance, SERVICE_GUID_HANDLE handle, uint16 maxRange)
{
	return instance->ScanServicesInRangeOfGUIDService(handle, maxRange);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncScanServicesInRangeOfGUIDService(IMiTokenBLEConnectionV2_2* instance, SERVICE_GUID_HANDLE handle, uint16 maxRange, int timeout)
{
	return instance->SyncScanServicesInRangeOfGUIDService(handle, maxRange, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncCustomWait(IMiTokenBLEConnectionV2_2* instance, bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, uint16 checkServiceUUID, uint16 checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout)
{
	return instance->SyncCustomWait(checkConnection, checkServiceScan, checkAttributeScan, checkWriting, checkReading, checkServiceUUID, checkAttributeUUID, serviceUUIDIsGUID, attributeUUIDIsGUID, skipCount, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data)
{
	return instance->SetAttributeFromCharHandle(handle, length, data);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncSetAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data, int timeout)
{
	return instance->SyncSetAttributeFromCharHandle(handle, length, data, timeout);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_RequestAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, bool longRead)
{
	return instance->RequestAttributeFromCharHandle(handle, longRead);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncRequestAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, bool longRead, int timeout)
{
	return instance->SyncRequestAttributeFromCharHandle(handle, longRead, timeout);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_ReadAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value))
{
	return instance->ReadAttributeFromCharHandle(handle, length, data, allocator, deallocator);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SyncReadAttributeFromCharHandle(IMiTokenBLEConnectionV2_2* instance, CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout)
{
	return instance->SyncReadAttributeFromCharHandle(handle, length, data, allocator, deallocator, timeout);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_UpdateConnectionParameters(IMiTokenBLEConnectionV2_2* instance, uint16 minimum, uint16 maximum, uint16 timeout, uint16 latency)
{
	return instance->UpdateConnectionParameters(minimum, maximum, timeout, latency);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetWriteBlockBufferCount(IMiTokenBLEConnectionV2_2* instance, int newMaxBlocksToBuffer)
{
	return instance->SetWriteBlockBufferCount(newMaxBlocksToBuffer);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetExtendedWriteOffsetMode(IMiTokenBLEConnectionV2_2* instance, bool offsetModeEnabled)
{
	return instance->SetExtendedWriteOffsetMode(offsetModeEnabled);
}
BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_SetExtendedWriteBlockSize(IMiTokenBLEConnectionV2_2* instance, int newBlockSize)
{
	return instance->SetExtendedWriteBlockSize(newBlockSize);
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_EnableJeromeAccess(IMiTokenBLEConnectionV2_2* instance)
{
	return instance->EnableJeromeAccess();
}

BLEDLLV2_API BLE_CONN_RET WINAPI MiTokenBLEV2_2_CONN_Jerome_SendNextBlock(IMiTokenBLEConnectionV2_2* instance, bool canBeLastBlock)
{
	return instance->Jerome_SendNextBlock(canBeLastBlock);
}