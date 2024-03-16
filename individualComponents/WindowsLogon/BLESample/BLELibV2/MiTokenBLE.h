#pragma once

#ifndef _MiTokenBLE_H_
#define _MiTokenBLE_H_

#include "BLE_API.h"
#include "BLE_IO.h"
#include "BLE_Structs.h"
#include "BLE_LowLevel.h"
#include "ReadWriteLocker.h"
#include "CurrentState.h"
#include "IConstReader.h"
#include "SafeObject.h"
#include "BLE_Command_Chain_Link.h"

class MiTokenBLEConnection;
class MiTokenBLEConnectionV2_2;

#if USE_MEM_CHECK
#include "MainLib.h"
#pragma (lib, "MemLeakChecker.lib")
#endif




class MiTokenBLE : public virtual IMiTokenBLE, public BaseSafeObject
{
public:
	MiTokenBLE();
	~MiTokenBLE();

	BLE_API_RET Initialize(const char* COMPort);
	BLE_API_RET Initialize(const char* COMPort, const char * NamedPipePort, bool isServer);
	BLE_API_RET SafeDelete();

	bool ConnectedToCOM() const;
	BLE_API_RET ReInitCOM();

	bool COMOwnerKnown() const;
	BLE_API_RET SetCOMWaitHandle(void* waitHandle);
	BLE_API_NP_RETURNS ReleaseCOMPort();
	void SetPollID(int newPollID);
	int GetPollID() const;
	int GetDeviceFoundCount() const;
	BLE_API_RET GetDeviceInfo(int deviceID, struct DeviceInfo* pDeviceInfo) const;
	BLE_API_RET GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength) const;
	BLE_API_RET RestartScanner();

	FilteredSearchObject StartFilteredSearch(int filterCount, DeviceData* filters);
	FilteredSearchObject StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray);
	BLE_API_RET ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& byteStreamLength);
	FilteredSearchObject ConvertByteStreamToFilteredSearch(uint8* byteStream, int byteStreamLength);

	BLE_API_RET ContinueFilteredSearch(FilteredSearchObject filter, DeviceInfo* pDeviceInfo);
	void RestartFilteredSearch(FilteredSearchObject filter);
	void FinishedFilteredSearch(FilteredSearchObject filter);


	void DEBUG_PrintDeiveMetaData(int deviceID) const;
	void DEBUG_SetIODebugOutput(void (__stdcall *dataIn) (int amount), void (__stdcall * dataOut) (int amount));
	IMiTokenBLEConnection * StartProfile(mac_address address, REQUEST_ID& requestID);

	BLE_API_NP_RETURNS Pipe_HasExclusiveAccess() const;
	BLE_API_NP_RETURNS Pipe_ReleaseExclusiveAccess();
	BLE_API_NP_RETURNS Pipe_RequestExclusiveAccess();
	BLE_API_NP_RETURNS Pipe_SendMessage(unsigned char* data, int length, int connectionID);
	BLE_API_NP_RETURNS Pipe_SetNamedPipeMessageCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID));

	BLE_API_NP_RETURNS Pipe_DisableBLEForwarding();



	//Functions that aren't publicly visible
	void DeviceConnected(mac_address address, uint8 connectionHandle);
	IMiTokenBLEConnection* getLastConnection();
	void DisconnectDevice(IMiTokenBLEConnection* connection);

	PollData** GetPRootPoll();
	BLE_LowLevel* getLowLevel();

	int GetNextRequestID();
	IBLE_IO* GetIO() const;

	ReadWriteLocker* GetPollMetaDataLock();
	CurrentState* GetState();
	
	void RegisterNewChild();
	void UnregisterChild();

	SafeObject<MessageQueue> GetQueueBuffer();
	//MessageQueue* GetQueueBuffer();

	IMiTokenBLEConnection* GetConnectionWithConnectionID(int connectionID);
	IMiTokenBLEConnection* GetConnectionWithMACAddress(mac_address address);

	bool connectionUpdateRequired(uint16 interval_min, uint16 interval_max, uint16 latency, uint16 timeout, bool justCheck);
	void forceConnectionUpdateRequired();
	bool allowLongServiceUUIDs();

protected:
	bool _allowLongServiceUUIDs;

	IBLE_IO* _BLE_IO;
	PollData* _rootPoll;
	bool _connectedToCOM;
	char* _COMPort;
	int _curPollID;
	IMiTokenBLEConnection* _curConnection;
	BLE_LowLevel* _LowLevel;

	IMiTokenBLEConnection* _rootConnection;

	int _nextRequestID;
	ReadWriteLocker* _PollMetaDataLock;
	CurrentState* _myState;
	int _childCount;
	HANDLE _registerMutex;
	HANDLE _safeToDelete;

	static DWORD WINAPI deleteThread(LPVOID instance);

	IConstReader* _BLEReader;
	MessageQueue* _IOBuffer;

	struct 
	{
		uint16 interval_min, interval_max, latency, timeout;
		bool set;
	}_connSettings;
	

	
};

#pragma warning(push)
#pragma warning(disable : 4250) //disable "class X inherits classY:function via dominance messages

//New features only avaliable in MiTokenBLE_API V2.2
class MiTokenBLEV2_2 : public virtual IMiTokenBLEV2_2, public MiTokenBLE
{
public:
	MiTokenBLEV2_2();
	~MiTokenBLEV2_2();

	BLE_API_RET Initialize(const char* COMPort, const char * NamedPipePort, bool isServer, void (*pipeConnectedCallback) (IMiTokenBLE* instance));

	const IBLE_COMMAND_CHAIN_LINK* GetBaseCommandChain();
	BLE_API_RET AppendCommandChain(IBLE_COMMAND_CHAIN_LINK* newLink, bool checkAndRemoveDuplicatePointers = false);

    void EnableNonUniqueAttributeUUIDMode();
	bool useNonUniqueAttributeUUIDMode();
	void ForceUnsafeDisconnection(uint8 connectionHandle);

protected:
    bool _useNonUniqueAttributeUUIDMode;
	void _checkPointerTable(const void* start, void* comparison, int length);

	void _createDefaultChainLink();
	void _populateChainRSP();
	void _populateChainEVT();
	IBLE_COMMAND_CHAIN_LINK* _baseCommandChain;
};

#pragma warning(pop)

#endif

