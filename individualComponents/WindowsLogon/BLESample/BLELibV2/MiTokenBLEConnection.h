#pragma once

#ifndef _MiTokenBLEConnection_H_
#define _MiTokenBLEConnection_H_

#include "BLE_API.h"
#include "BLE_Structs.h"


class MiTokenBLEConnection : public BaseLinkedList, public virtual IMiTokenBLEConnection
{
	friend class MiTokenBLE;

public:
	MiTokenBLEConnection(MiTokenBLE* parent);
	virtual BLE_CONN_RET Initialize(mac_address address);
	//virtual REQUEST_ID getRequestID();
	virtual bool timedOut() const;
	virtual BLE_CONN_RET TimeoutRequest();

	virtual void GetAddress(mac_address& address) const;
	virtual BLE_CONN_RET IsConnected() const;
	virtual BLE_CONN_RET ScanServicesInRange(SERVICE_HANDLE handle, uint16 maxRange);
	virtual BLE_CONN_RET SetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data);
	virtual BLE_CONN_RET StartGetAttribute(ATTRIBUTE_HANDLE handle);
	virtual BLE_CONN_RET GetAttribute(ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer) const;
	virtual BLE_CONN_RET Disconnect();
	virtual BLE_CONN_RET PollRSSI();
	virtual BLE_CONN_RET GetRSSI(int& rssi) const;
	virtual BLE_CONN_RET ButtonPressedCallback(void(__stdcall *buttonPressed) (IMiTokenBLEConnection* requestID, int buttonID));
	virtual BLE_CONN_RET ReadLong(ATTRIBUTE_HANDLE handle);
	virtual MessageBufferObject CreateNewMessageBuffer();
	virtual BLE_CONN_RET WriteMessageBuffer(MessageBufferObject messageBuffer);
	virtual BLE_CONN_RET AddMessageToMessageBuffer(MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data);
	virtual BLE_CONN_RET GetCharacteristicHandle(ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle) const;

	virtual int GetRequestID() const;
	
	uint8 GetLastError() const;
	
	//Carry over from ConnectionData

	void gotConnection(uint8 connectionHandle);

	bool doingLongWrite, doingWrite;
	ConnectionServices* getService(uint16 uuid, bool createNewService = true);
	ConnectionServices* getServiceLongUUID(const uint8* uuid, uint8 uuidLen, bool createNewService = false);
	ConnectionAttributes* getAttribute(uint16 handle, bool createNewAttribute = true, uint16 characteristicHandle = 0, bool GUIDHandle = false);
	ConnectionAttributes* constGetAttribute(uint16 handle) const;

	ConnectionAttributes* getAttributeFromCharacteristicsHandle(uint16 characteristicsHandle);
	void setAttributeValue(uint16 uuid, uint8 length, const uint8* data, bool createNewAttribute = false);
	bool getAttributeValue(uint16 uuid, uint8& value);
	bool getAttributeValue(uint16 uuid, uint16& value);

	int serviceCount();
	int attributeCount();

	bool attributeScanCompleted() const;
	bool serviceScanCompleted() const;

	bool beginAttributeScan(uint16 serviceUID);
	void finishServiceScan();

	void finishAttributeScan();

	int getAverageRSSI() const;
	void addPoll(int pollID, int rssi);

	void(__stdcall *buttonPushed) (IMiTokenBLEConnection* sender, int buttonID);
	
	void SetNextExtendedWrite(uint16 attributeID, uint16 dataLength, const void* data);
	bool GetNextExtendedWrite(uint16& attributeID, uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft);
	void AppendToExtendedWrite(MessageBuffer* messageBuffer);


	static MiTokenBLEConnection* findNodeWithID(MiTokenBLEConnection** pRootNode, int connectionID, bool createNewNode = true);
	static void removeNodeWithID(MiTokenBLEConnection** pRootNode, int connectionID);
	static void CleanupData(MiTokenBLEConnection** pRootNode);


	

	//Callbacks
	void CALLBACK_ServiceSearchFinished();
	void CALLBACK_ServiceInformationFinished();
	void CALLBACK_WriteAttributeFinished();
	virtual void CALLBACK_LongWriteSectionCompleted();
	void CALLBACK_LongReadCompleted(uint16 handle);
	void CALLBACK_ExtendedWriteCompleted();
	void CALLBACK_ConnectionClosed();

	void CALLBACK_ServiceFound(uint8 serviceHandleLength, const uint8* serviceHandleData, uint16 start, uint16 end);
	void CALLBACK_ServiceInformationFound(uint16 characteristicHandle, uint8 dataLen, const uint8* data);
	void CALLBACK_HandleValue(uint8 type, uint16 attributeHandle, uint8 dataLength, const uint8* data);
	void CALLBACK_RSSIValue(int8 rssi);
	void CALLBACK_PrepareWriteResponse(uint16 result);

	virtual void CALLBACK_ProcedureCompleted(uint16 chrhandle, uint8 result);
	void CALLBACK_Connected(uint8 connectionID);


protected:

	void longWriteWroteToAttribute(uint16 attributeID);
	
	MiTokenBLE* _parent;

	mac_address _address;
	uint32 _connectionID;
	bool _timedOut;
	bool _isConnected;
	int _requestID;

	MessageBuffer* _extendedWriteBuffer;
	void* _extendedWriteData;
	uint16 _extendedWriteDataLength;
	uint16 _extendedWriteAttributeID;
	uint16 _extendedWriteCurrentOffset;

	int _currentRSSIPollID;
	int _totalRSSI;
	int _currentRSSICount;

	bool _attributeScanCompleted;
	bool _serviceScanCompleted;

	uint16* _attributesWrittenToInLongWrite;
	int _attributesWrittenToInLongWriteCount;

	ConnectionAttributes* _rootAttributeNode;
	ConnectionServices* _rootServiceNode;

	virtual BaseLinkedList* createNewNode(void* initializeData);

	uint8 _lastError;
	uint16 _currentServiceUUID;
	bool _currentServiceIsGUID;

	bool _extendedWriteResetOffset;
	int _extendedWriteBlockSize;
	
};


#pragma warning(push)
#pragma warning(disable : 4250) //disable "class X inherits classY:function via dominance messages

class MiTokenBLEConnectionV2_2 : public virtual IMiTokenBLEConnectionV2_2, public MiTokenBLEConnection
{
public:
	MiTokenBLEConnectionV2_2(MiTokenBLE* parent);
	~MiTokenBLEConnectionV2_2();

	BLE_CONN_RET SyncWaitForConnection(int timeout) const;
	BLE_CONN_RET SyncWaitForDisconnection(int timeout) const;
	BLE_CONN_RET SyncScanServiesInRange(SERVICE_HANDLE handle, uint16 maxRange, int timeout);
	BLE_CONN_RET SyncRequestAttribute(ATTRIBUTE_HANDLE handle, int timeout);
	BLE_CONN_RET SyncSetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data, int timeout);
	BLE_CONN_RET SyncReadAttribute(const ATTRIBUTE_HANDLE handle, bool longRead, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout);
	BLE_CONN_RET SyncWaitForState(const ATTRIBUTE_HANDLE handle, bool longRead, const uint8* stateList, const uint8 stateCount, const uint8 stateLength, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout);
	BLE_CONN_RET MiTokenBLEConnectionV2_2::SyncRequestLongAttribute(ATTRIBUTE_HANDLE handle, int timeout);

	BLE_CONN_RET GetCharacteristicHandleEx(const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount = 0);
	BLE_CONN_RET SyncGetCharacteristicHandleEx(const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, int timeout, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount = 0);


	BLE_CONN_RET ScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange);
	BLE_CONN_RET SyncScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange, int timeout);

	BLE_CONN_RET SyncCustomWait(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, uint16 checkServiceUUID, uint16 checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout);

	BLE_CONN_RET SetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data);
	BLE_CONN_RET SyncSetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data, int timeout);
	BLE_CONN_RET RequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead);
	BLE_CONN_RET SyncRequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead, int timeout);
	BLE_CONN_RET ReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value));
	BLE_CONN_RET SyncReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout);
	
	BLE_CONN_RET UpdateConnectionParameters(uint16 minimum, uint16 maximum, uint16 timeout, uint16 latency);

	void TriggerSyncCheck() const;
	
	void CALLBACK_LongWriteSectionCompleted();
	void CALLBACK_ProcedureCompleted(uint16 chrhandle, uint8 result);
	BLE_CONN_RET SetWriteBlockBufferCount(int newMaxBlocksToBuffer);

	BLE_CONN_RET SetExtendedWriteOffsetMode(bool offsetModeEnabled);
	BLE_CONN_RET SetExtendedWriteBlockSize(int newBlockSize);


	BLE_CONN_RET EnableJeromeAccess();
	BLE_CONN_RET Jerome_SendNextBlock(bool canBeLastBlock);

protected:

	bool _hasJeromeAccess;

	volatile int _blocksToBuffer;
	volatile int _blocksBeingWritten;
	HANDLE _blockBufferMutex;
	
	bool _nextExtendedWriteWouldBeLast();
	
	BLE_CONN_RET returnSuccess() const;
	void _resetSyncCheck() const;
	bool _syncWait(int timeout) const;
	HANDLE _syncHandle;
	virtual BaseLinkedList* createNewNode(void* initializeData);

	BLE_CONN_RET _checkState(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWrites, bool checkAttributeRead, uint16 checkServiceReadUUID, uint16 checkAttributeReadUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount);
	BLE_CONN_RET _checkStateCharHandle(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWrites, bool checkAttributeRead, uint16 checkCharHandle);
	ConnectionAttributes* getAttributeEx(uint16 serviceUUID, uint16 attributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount);
	

};

#pragma warning(pop)

#endif