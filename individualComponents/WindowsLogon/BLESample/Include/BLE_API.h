#pragma once

#ifndef _BLE_API_H_
#define _BLE_API_H_


//Force V2.1 code to be used - V2.0 code is old and shouldn't be used anymore. - Mainly for testing that V2.1 code is the only code being used by all the programs we have running.
#define REQUIRE_BLE_API_V2_1

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

//#define MiTokenBLE_ClasslessMode

#include "BLE_API_Types.h"


/*
	Defines for the version of the BLE API objects to use
*/
#define USE_MITOKEN_BLE_API_V2_1 0x00020100
#define USE_MITOKEN_BLE_API_V2_2 0x00020200


#define CONN_TIMEOUT_JUST_CHECK 0x00000000
#define CONN_TIMEOUT_INFINITE 0xFFFFFFFF


class IMiTokenBLEConnection
{
public:
	virtual BLE_CONN_RET Initialize(mac_address address) = 0;
	//virtual REQUEST_ID getRequestID() = 0;
	virtual bool timedOut() const = 0;
	virtual BLE_CONN_RET TimeoutRequest() = 0;
	virtual void GetAddress(mac_address& address) const = 0;
	virtual BLE_CONN_RET IsConnected() const = 0;
	virtual BLE_CONN_RET ScanServicesInRange(SERVICE_HANDLE handle, uint16 maxRange) = 0;
	virtual BLE_CONN_RET SetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data) = 0;
	virtual BLE_CONN_RET StartGetAttribute(ATTRIBUTE_HANDLE handle) = 0;
	virtual BLE_CONN_RET GetAttribute(ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer) const = 0;
	virtual BLE_CONN_RET Disconnect() = 0;
	virtual BLE_CONN_RET PollRSSI() = 0;
	virtual BLE_CONN_RET GetRSSI(int& rssi) const = 0;
	virtual BLE_CONN_RET ButtonPressedCallback(void(__stdcall *buttonPressed) (IMiTokenBLEConnection* requestID, int buttonID)) = 0;
	virtual BLE_CONN_RET ReadLong(ATTRIBUTE_HANDLE handle) = 0;
	virtual MessageBufferObject CreateNewMessageBuffer() = 0;
	virtual BLE_CONN_RET WriteMessageBuffer(MessageBufferObject messageBuffer) = 0;
	virtual BLE_CONN_RET AddMessageToMessageBuffer(MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data) = 0;
	virtual BLE_CONN_RET GetCharacteristicHandle(ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle) const = 0;
	virtual int GetRequestID() const = 0;

	virtual uint8 GetLastError() const = 0;
};

class IMiTokenBLEConnectionV2_2 : public virtual IMiTokenBLEConnection
{
public:
	virtual BLE_CONN_RET SyncWaitForConnection(int timeout) const = 0;
	virtual BLE_CONN_RET SyncWaitForDisconnection(int timeout) const = 0;
	virtual BLE_CONN_RET SyncScanServiesInRange(SERVICE_HANDLE handle, uint16 maxRange, int timeout) = 0;
	virtual BLE_CONN_RET SyncRequestAttribute(ATTRIBUTE_HANDLE handle, int timeout) = 0;
	virtual BLE_CONN_RET SyncRequestLongAttribute(ATTRIBUTE_HANDLE handle, int timeout) = 0;
	virtual BLE_CONN_RET SyncSetAttribute(ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data, int timeout) = 0;
	virtual BLE_CONN_RET SyncReadAttribute(const ATTRIBUTE_HANDLE handle, bool longRead, uint8& length, uint8*& data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout) = 0;
	virtual BLE_CONN_RET SyncWaitForState(const ATTRIBUTE_HANDLE handle, bool longRead, const uint8* stateList, const uint8 stateCount, const uint8 stateLength, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout) = 0;

	virtual BLE_CONN_RET GetCharacteristicHandleEx(const SERVICE_HANDLE service, const ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, bool serviceIsGUID = false, bool attributeIsGUID = false, int skipCount = 0) = 0;
	virtual BLE_CONN_RET SyncGetCharacteristicHandleEx(SERVICE_HANDLE service, ATTRIBUTE_HANDLE attribute, CHARACTERISTIC_HANDLE& handle, int timeout, bool serviceIsGUID, bool attributeIsGUID = false, int skipCount = 0) = 0;

	virtual BLE_CONN_RET ScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange) = 0;
	virtual BLE_CONN_RET SyncScanServicesInRangeOfGUIDService(SERVICE_GUID_HANDLE handle, uint16 maxRange, int timeout) = 0;


	virtual BLE_CONN_RET SyncCustomWait(bool checkConnection, bool checkServiceScan, bool checkAttributeScan, bool checkWriting, bool checkReading, uint16 checkServiceUUID, uint16 checkAttributeUUID, bool serviceUUIDIsGUID, bool attributeUUIDIsGUID, int skipCount, int timeout) = 0;

	virtual BLE_CONN_RET SetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data) = 0;
	virtual BLE_CONN_RET SyncSetAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8 length, const uint8* data, int timeout) = 0;
	virtual BLE_CONN_RET RequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead) = 0;
	virtual BLE_CONN_RET SyncRequestAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, bool longRead, int timeout) = 0;
	virtual BLE_CONN_RET ReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value)) = 0;
	virtual BLE_CONN_RET SyncReadAttributeFromCharHandle(CHARACTERISTIC_HANDLE handle, uint8& length, uint8*&data, uint8* (__stdcall *allocator) (uint8 requiredLength), void (__stdcall *deallocator) (uint8* value), int timeout) = 0;
	
	virtual BLE_CONN_RET UpdateConnectionParameters(uint16 minimum, uint16 maximum, uint16 timeout, uint16 latency) = 0;

	virtual BLE_CONN_RET SetWriteBlockBufferCount(int newMaxBlocksToBuffer) = 0;

	virtual BLE_CONN_RET SetExtendedWriteOffsetMode(bool offsetModeEnabled) = 0;
	virtual BLE_CONN_RET SetExtendedWriteBlockSize(int blockSize) = 0; 


	virtual BLE_CONN_RET EnableJeromeAccess() = 0;
	virtual BLE_CONN_RET Jerome_SendNextBlock(bool canBeLastBlock) = 0;

};

//The Mi-Token BLE API V2.1 Class
//This class is mostly finalized and running as expected.
class IMiTokenBLE
{
public:
	virtual ~IMiTokenBLE() = 0;

	virtual BLE_API_RET Initialize(const char* COMPort) = 0;
	virtual BLE_API_RET Initialize(const char* COMPort, const char * NamedPipePort, bool isServer) = 0;
	virtual BLE_API_RET SafeDelete() = 0;

	virtual bool ConnectedToCOM() const = 0;
	virtual BLE_API_RET ReInitCOM() = 0;

	virtual bool COMOwnerKnown() const = 0;
	virtual BLE_API_RET SetCOMWaitHandle(void* waitHandle) = 0;
	virtual BLE_API_NP_RETURNS ReleaseCOMPort() = 0;
	virtual void SetPollID(int newPollID) = 0;
	virtual int GetPollID() const = 0;
	virtual int GetDeviceFoundCount() const = 0;
	virtual BLE_API_RET GetDeviceInfo(int deviceID, struct DeviceInfo* pDeviceInfo) const = 0;
	virtual BLE_API_RET GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength) const = 0;
	virtual BLE_API_RET RestartScanner() = 0;

	virtual FilteredSearchObject StartFilteredSearch(int filterCount, DeviceData* filters) = 0;
	virtual FilteredSearchObject StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray) = 0;
	virtual BLE_API_RET ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& byteStreamLength) = 0;
	virtual FilteredSearchObject ConvertByteStreamToFilteredSearch(uint8* byteStream, int byteStreamLength) = 0;

	virtual BLE_API_RET ContinueFilteredSearch(FilteredSearchObject filter, DeviceInfo* pDeviceInfo) = 0;
	virtual void RestartFilteredSearch(FilteredSearchObject filter) = 0;
	virtual void FinishedFilteredSearch(FilteredSearchObject filter) = 0;


	virtual void DEBUG_PrintDeiveMetaData(int deviceID) const = 0;
	virtual void DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall * dataOut) (int amount)) = 0;
	virtual IMiTokenBLEConnection * StartProfile(mac_address address, REQUEST_ID& requestID) = 0;


	virtual BLE_API_NP_RETURNS Pipe_HasExclusiveAccess() const = 0;
	virtual BLE_API_NP_RETURNS Pipe_ReleaseExclusiveAccess() = 0;
	virtual BLE_API_NP_RETURNS Pipe_RequestExclusiveAccess() = 0;
	virtual BLE_API_NP_RETURNS Pipe_SendMessage(unsigned char* data, int length, int connectionID) = 0;
	virtual BLE_API_NP_RETURNS Pipe_SetNamedPipeMessageCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID)) = 0;

	virtual BLE_API_NP_RETURNS Pipe_DisableBLEForwarding() = 0;

	virtual IMiTokenBLEConnection* GetConnectionWithConnectionID(int connectionID) = 0;
	virtual IMiTokenBLEConnection* GetConnectionWithMACAddress(mac_address address) = 0;

	
};

class IBLE_COMMAND_CHAIN_LINK;

//Class contains new features only avaliable in MiToken API V2.2
//This class is currently WIP, do not use it unless you know what you are doing.
class IMiTokenBLEV2_2 : public virtual IMiTokenBLE
{
public:
	virtual BLE_API_RET Initialize(const char* COMPort, const char * NamedPipePort, bool isServer, void (*pipeConnectedCallback) (IMiTokenBLE* instance)) = 0;
	virtual BLE_API_RET AppendCommandChain(IBLE_COMMAND_CHAIN_LINK* newLink, bool checkAndRemoveDuplicatePointers = false) = 0;
    virtual void EnableNonUniqueAttributeUUIDMode() = 0;
	virtual void ForceUnsafeDisconnection(uint8 connectionHandle) = 0;
};


class IBLE_COMMAND_CHAIN_LINK
{
public:
	IBLE_COMMAND_CHAIN_LINK();
	virtual ~IBLE_COMMAND_CHAIN_LINK();

	virtual bool isLinkModular() const = 0;
	const IBLE_COMMAND_CHAIN_LINK* getNextLink() const;
	void appendLink(IBLE_COMMAND_CHAIN_LINK* link);
	void cutNextLink();

protected:
	IBLE_COMMAND_CHAIN_LINK* nextLink;
};

//Create a new Mi-Token BLE Interface. You generally will want to use the default APIVersion.
IMiTokenBLE* CreateNewMiTokenBLEInterface(int APIVersion = USE_MITOKEN_BLE_API_V2_1);


#ifndef REQUIRE_BLE_API_V2_1

void BLE_SetTraceFile(const char* filePath);


//Gets the connection that any V2.0 lib function calls would use. - Use it only to get V2.1 functionality from V2.0 code
//Otherwise you should already have this connection when calling IMiTokenBLE->StartProfile(...)
IMiTokenBLEConnection* BLE_GetConnectionV2Wrapper();

//Initialize the BLE Scanner
int BLE_Initialize(const char* COMPort);

//Initializes the BLE Scanner with support for NamedPipe IO
//COMPort : The COMPort to connect to - this will be used if (isServer) is true, or the client is unable to connect to the server
//NamedPipePort : The NamedPipePort to open / connect to
//isServer : Is this program running as the Server of the NamedPipe or the Client.
int BLE_InitializeEx(const char* COMPort, const char* NamedPipePort, bool isServer);

//Returns 0 if you are not connected to the COMPort, and 1 if you are
int BLE_ConnectedToCOM();

//Only used if you are the server
//Will attempt to initialize the COM Port again in case it didn't work already due to a client having the COM port
int BLE_ReinitCOM(const char* COMPort);

//Finalize the BLE Scanner
int BLE_Finalize();

//For servers : returns true (1) when we know which client owns the COM Port, and false (0) if we don't know who owns it (or no clients own it)
//For clients : always returns false (0)
int BLE_COMOwnerKnown();

//For servers : once the client who owns the COM Port has released it, an event will be raised on waitHandle
//				returns true (1) if the COM Owner is known, sets the event and returns false (0) if we don't know who owns it
//For clients : sets the event and returns false (0)
int BLE_SetCOMWaitHandle(void* waitHandle);


//For servers : does nothing, always returns SUCCESS (1)
//For clients : If server is connected : will return success (1) release the COM Port, and send a message to the server informing it that the COM Port has been released
//				If server is disconnected : will return false (0) and not release the COM Port
int BLE_ReleaseCOMPort();


//Non-Connected POLL functions

//Set's the PollID. Any polls taken while the poll ID is unchanged are added up and averaged in DeviceInfo.RSSI and calls to BLE_CONN_GetRSSI
void BLE_SetPollID(int newPollID);

//Gets how many devices have been found. Note : This does no filtering on device Metadata. To do that use BLE_StartFilteredSearch/BLE_ContinueFilteredSearch/BLE_FinishFilteredSearch instead
int BLE_GetDeviceFoundCount();

//Gets the DeviceInfo for a certain device. deviceID range is 0 <= deviceID < BLE_GetDeviceFoundCount(). Values outside that range will return junk in pDeviceInfo and a return code of -1.
int BLE_GetDeviceInfo(int deviceID, struct DeviceInfo* pDeviceInfo);



int BLE_GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength);


//Restarts the Device Scanner. On Connections to a device this scanner will be terminated and unable to restart until all open connections are closed
int BLE_RestartScanner();

//Better searching of devices in cases where there are a lot of devices found and only ones with certain meta-data should be used
//FilterCount is the size of the filters array.
//filters is an array of DeviceData structs that stores what filters we expect to match
//Note : This will filter out every object except those that match ALL supplied filters.
FilteredSearchObject BLE_StartFilteredSearch(int filterCount, DeviceData* filters);

//Extension of the BLE_StartFilteredSearch function. In this case you can also filter so that only certain MAC addresses are shown
//addressCount : how many MAC addresses are stored in addressArray. addressArray must be 6*addressCount bytes large
//addressArray : an array of MAC addresses that are allowed by the filter. For a device to be returned it must match one of the addresses in this array
FilteredSearchObject BLE_StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray);

//Converts a FilteredSearchObject into a stream of bytes
//filter : The Filter to convert
//byteStream : A pointer to the bytestream where the output should be stored. Can be NULL (in which case it will store the required length in byteStreamLength)
//byteStreamLength : Input is the length of byteStream, Output will have the length (either required or stored) in byteStream
//Return Code:
/*	
		SUCCESS : byteStream contains data, byteStreamLength contains length of byteStream stored
		MORE_DATA : byteStream hasn't been touched, byteStreamLength contains length required in byteStream
						Returned if byteStream is NULL or byteStreamLength was < required space.
*/
int BLE_ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& byteStreamLength);

//Converts a bytestream to a FilteredSearchObject
//byteStream : data returned from a BLE_ConvertFilteredSearchToByteStream function call
//byteStreamLength : length of the byteStream
//Returns the initialized FilteredSearchObject
#define BLE_CovnertByteStreamToFilteredSearch BLE_ConvertByteStreamToFilteredSearch
FilteredSearchObject BLE_ConvertByteStreamToFilteredSearch(uint8* byteStream, int byteStreamLength);


//Find the next BLE Device that matches all the filters. 
//FilteredSearchObject is what was returned by BLE_StartFilteredSearch
//pDeviceInfo will be filled with the device info of the next device that matches all the filters
//The return code will be 0 in the case of success, and non-zero in all other cases
int BLE_ContinueFilteredSearch(FilteredSearchObject filter, struct DeviceInfo* pDeviceInfo);

//Will restart the filtered search from the start (as if a new BLE_StartSearchFilter call had been made)
void BLE_RestartFilteredSearch(FilteredSearchObject filter);

//This will clear up all the underlying data of the FilteredSearchObject. After calling this you should set the variable passed in to NULL.
//Do NOT use free or delete on the filter object, it will have already been cleaned up by the time this function returns.
void BLE_FinishFilteredSearch(FilteredSearchObject filter);

//DEBUG functions that are only built with the _DEBUG Flag - release builds will do nothing
void BLE_DEBUG_PrintDeviceMetaData(int deviceID);
void BLE_DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall *dataOut) (int amount));


//Connected Functions
//Will attempt to connect to the device with the MAC Address specified in address
//If no succesful conneciton is made within 10 seconds, the attempt will fail with a TIMEOUT reason
//This function will return either (BLE_CONN_ERR_BAD_REQUEST) in cases where it could not start a connection request, or a value >= 1 in cases where it could start a valid request.
//For cases where the return code is >= 1, this value is the (requestID) that should be used with all future 
BLE_CONN_RET BLE_CONN_StartProfile(mac_address address, REQUEST_ID& requestID);

//Will either return that the device is connected (BLE_CONN_SUCCESS) or the reason it isn't connected (eg, BLE_CONN_ERR_CONNECTION_TIMED_OUT)
BLE_CONN_RET BLE_CONN_IsConnected(REQUEST_ID requestID);

//Will scan for all servics that exist from (handle) onwards. It will scan at most (maxRange) services. 
BLE_CONN_RET BLE_CONN_ScanServicesInRange(REQUEST_ID requestID, SERVICE_HANDLE handle, uint16 maxRange);

//Write (data) of length (length) to the attribute with the attribute handle of (handle).
BLE_CONN_RET BLE_CONN_SetAttribute(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle, uint8 length, const uint8* data);

//Request a read of the attribute with the attribute handle of (handle). This read may take some time depending on the length of the data it needs to read
BLE_CONN_RET BLE_CONN_StartGetAttribute(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle);

//Request the results of a previous BLE_CONN_StartGetAttribute call. To completely get the data it will require 2 calls to this function:
//The first call should have (maxLength) set to 0 and (buffer) set to NULL. This call will set (realLength) to the length of the buffer required and return BLE_CONN_SUCCESS
//The second call should have (maxLength) set to at least (realLength) and (buffer) pointing to an uint8 block of at least (realLength) bytes. This call will set (realLength) to the length of the buffer required and copy the data to (buffer)
BLE_CONN_RET BLE_CONN_GetAttribute(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle, uint8 maxLength, uint8& realLength, uint8* buffer);

//Request that we disconnect from a certain BLE Token device.
BLE_CONN_RET BLE_CONN_Disconnect(REQUEST_ID requestID);

//Request the current RSSI of a connected token
BLE_CONN_RET BLE_CONN_PollRSSI(REQUEST_ID requestID);

//Check the current RSSI of a connected token.
BLE_CONN_RET BLE_CONN_GetRSSI(REQUEST_ID requestID, int& rssi);

//Set the callback function for whenever a button is pressed on the device.
//For the callback function (requestID) is the requestID that the device is bound to, and (buttonID) is the ID of the button that was pressed (bit masked)
BLE_CONN_RET BLE_CONN_ButtonPressedCallback(REQUEST_ID requestID, void (__stdcall *buttonPressed) (IMiTokenBLEConnection* sender, int buttonID));

//Request a long read (>18 bytes) from the BLE device.
BLE_CONN_RET BLE_CONN_ReadLong(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle);

//MessageBufferObjects are used for when you want to send several messages to the device with one command
//Creates a new MessageBuffer object.
MessageBufferObject BLE_CONN_CreateNewMessageBuffer();

//Write the MessageBufferObject to the device bound to (requestID)
BLE_CONN_RET BLE_CONN_WriteMessageBuffer(REQUEST_ID requestID, MessageBufferObject messageBuffer);

//Add a message to the MessageBufferObject. This message will be sent when you call BLE_CONN_WriteMessageBuffer.
//In this case (handle) is the charactersticHandle returned from BLE_CONN_GetCharacteristicHandle
BLE_CONN_RET BLE_CONN_AddMessageToMessageBuffer(MessageBufferObject messageBuffer, ATTRIBUTE_HANDLE handle, uint16 length, const uint8* data);

//As MessageBufferObjects use characteristicHandles instead of attribute handles, you will need to call BLE_CONN_GetCharacteristicHandle to convert an attribute handle UUID to a characteristic handle UUID.
//Characteristic Handles do not change during a connection and can thus be safely cached.
BLE_CONN_RET BLE_CONN_GetCharacteristicHandle(REQUEST_ID requestID, ATTRIBUTE_HANDLE handle, CHARACTERISTIC_HANDLE& characteristicHandle);


//Functions that work with the NamedPipe part of the API
//All these functions are written as "BLE_NP_[name]"

//Requests exclusive access from the named pipe. When using named pipes this must be done before any CONN functions are called. 
BLE_API_NP_RETURNS BLE_NP_RequestExclusiveAccess();

//Check if we have exclusive access to the pipe.
BLE_API_NP_RETURNS BLE_NP_HasExclusiveAccess();

//Release exclusive access over the pipe. Do this with the BLE_CONN_Disconnect call
BLE_API_NP_RETURNS BLE_NP_ReleaseExclusiveAccess();

//send a message via the NamedPipe.
BLE_API_NP_RETURNS BLE_NP_SendMessage(uint8* data, int length, int connectionID);

//set the callback function for when we recieve a message over the NamedPipe
BLE_API_NP_RETURNS BLE_NP_SetMessageCallback(void (*NP_MessageCallback) (IMiTokenBLE* sender, uint8* data, int length, int connectionID));


//Only define this if you are using the BLE Tokens storage functions. These functions will require the OpenSSL library to build correctly.
#define ALWAYS_INCLUDE_EX

#ifdef ALWAYS_INCLUDE_EX || BLE_API_USEEX

//Extension functions - note: these functions may have other requirements for you to be able to use them (eg openssl)

//Creates a message for the BLE Device to store (data) on the device
int BLE_EXT_MakeStoreBuffer(int datalength, const uint8* data, int& storeLength, uint8*& storeData);
//Create a message to request encrypted data from the device
int BLE_EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata);
//Decrypt an encrypted message from the device.
int BLE_EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata);

#endif

#endif //REQUIRE_BLE_API_V2_1

/*
#ifdef __cplusplus
}
#endif
*/

#endif