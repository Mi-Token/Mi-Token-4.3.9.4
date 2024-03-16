#include "MiTokenBLE.h"
#include "CurrentState.h"
#include <stdio.h>
#include <stdlib.h>

#include "BLE_LowLevel.h"
#include "DebugLogging.h"
#include "MiTokenBLEConnection.h"
#include "BLE_Structs.h"
#include "BLE_LowLevel.h"
#include "MiTokenBLE_Commands.h"

#include "GenericUtils.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#define CHECK_POINTER(p) if(p == nullptr ) { return BLE_API_ERR_NULL_INTERNAL_INTERFACE; } 
#define CHECK_NP_POINTER(p) if(p == nullptr ) { return BLE_API_NP_ERR_NULL_INTERNAL_INTERFACE; } 

IMiTokenBLE::~IMiTokenBLE()
{

}

MiTokenBLE::MiTokenBLE()
{
#if USE_MEM_CHECK
	MemLeak_SetLocationID(1);
#endif
	_BLE_IO = nullptr;
	_rootPoll = nullptr;
	_connectedToCOM = false;
	_COMPort = nullptr;
	_curPollID = 0;
	_curConnection = nullptr;
	_LowLevel = nullptr;
	_rootConnection = nullptr;
	_nextRequestID = 1;
	_myState = nullptr;
	_childCount = 0;
	_allowLongServiceUUIDs = false; //this will be overwritten by MiTokenBLEV2_2 as it allows longServiceUUIDs, however MiTokenBLE will not allow it.
#if USE_MEM_CHECK
	MemLeak_SetLocationID(22);
#endif
	_connSettings.set =  false;
	_connSettings.interval_max = _connSettings.interval_min = _connSettings.latency = _connSettings.timeout = 0;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(21);
#endif
	_PollMetaDataLock = new ReadWriteLocker();

#if USE_MEM_CHECK
	MemLeak_SetLocationID(20);
#endif
	this->_registerMutex = CreateMutex(nullptr, FALSE, nullptr);

#if USE_MEM_CHECK
	MemLeak_SetLocationID(3);
#endif
	this->_myState = new CurrentState();
#if USE_MEM_CHECK
	MemLeak_SetLocationID(4);
#endif
	this->_BLE_IO = new Basic_BLE_IO(this);
#if USE_MEM_CHECK
	MemLeak_SetLocationID(5);
#endif
	this->_LowLevel = new BLE_LowLevel();
#if USE_MEM_CHECK
	MemLeak_SetLocationID(6);
#endif
	this->_LowLevel->SetIO(_BLE_IO);
#if USE_MEM_CHECK
	MemLeak_SetLocationID(2);
#endif
}

MiTokenBLE::~MiTokenBLE()
{
	waitUntilSafe();
	printf("Interface being deleted\r\n");
	delete _PollMetaDataLock;
	delete _LowLevel;
	delete _myState;
	if(_COMPort != nullptr)
	{
		delete[] _COMPort;
	}
}

BLE_API_RET MiTokenBLE::Initialize(const char* COMPort)
{
	return BLE_API_ERR_FUNCTION_NOT_IMPLEMENTED;
}

BLE_API_RET MiTokenBLE::Initialize(const char* COMPort, const char* NamedPipePort, bool isServer)
{
#if USE_MEM_CHECK
	MemLeak_SetLocationID(101);
#endif

	char str[80] = { 0 };
	BLE_API_RET retCode = BLE_API_ERR_UNKNOWN;
	bool COMOpened = false;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(102);
#endif

	if (COMPort != nullptr)
	{
		snprintf(str, sizeof(str)-1, "\\\\.\\%s", COMPort);
		int len = strlen(str) + 1;
		_COMPort = new char[len];
		memcpy(_COMPort, str, len);
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(103);
#endif

	if (isServer && (COMPort != nullptr))
	{
		retCode = _BLE_IO->OpenSerialHandle(str);
		if (retCode == 0)
		{
			COMOpened = true;
			_connectedToCOM = true;
		}
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(104);
#endif

	_BLE_IO->InitializeNamedPipe(NamedPipePort, isServer, nullptr);

#if USE_MEM_CHECK
	MemLeak_SetLocationID(107);
#endif

	if (isServer && COMOpened)
	{
		_BLE_IO->InitBLEDevice();
	}
	else if (!isServer)
	{
		Sleep(1000);
		if (!_BLE_IO->ClientConnectedToNP())
		{
			if (COMPort != nullptr)
			{
				retCode = _BLE_IO->OpenSerialHandle(str);

				if (retCode != BLE_API_SUCCESS)
				{
					return retCode;
				}

				_connectedToCOM = true;
				_BLE_IO->InitBLEDevice();
			}
		}
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(105);
#endif

	if (!isServer || COMOpened)
	{
		_BLE_IO->StartASyncRead();
	}
#if USE_MEM_CHECK
	MemLeak_SetLocationID(106);
#endif

	return BLE_API_SUCCESS;
}

bool MiTokenBLE::ConnectedToCOM() const
{
	if (_connectedToCOM)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BLE_API_RET MiTokenBLE::ReInitCOM()
{
	BLE_API_RET retCode = BLE_API_ERR_UNKNOWN;
	if (!_connectedToCOM)
	{
		retCode = _BLE_IO->OpenSerialHandle(_COMPort);
		if (retCode == BLE_API_SUCCESS)
		{
			_connectedToCOM = true;
			//BLE_LL_InitBLEDevice();
			_BLE_IO->InitBLEDevice();
			_BLE_IO->StartASyncRead();
		}
	}

	return retCode;
}

IMiTokenBLEConnection* MiTokenBLE::getLastConnection()
{
	return _curConnection;
}

void MiTokenBLE::DeviceConnected(mac_address address, uint8 connectionHandle)
{
	MiTokenBLEConnection* pConn = dynamic_cast<MiTokenBLEConnection*>(_rootConnection);
	while (pConn != nullptr)
	{
		mac_address temp;
		pConn->GetAddress(temp);
		if (memcmp(&temp, &address, sizeof(mac_address)) == 0)
		{
			pConn->gotConnection(connectionHandle);
			pConn = nullptr;
		}
		else
		{
			pConn = (MiTokenBLEConnection*)pConn->nextNode;
		}
	}

}

IMiTokenBLEConnection* MiTokenBLE::StartProfile(mac_address address, int& requestID)
{
//#error not yet implimented
	MiTokenBLEConnection* ret = nullptr;

	if(_rootConnection == nullptr)
	{
		if((dynamic_cast<MiTokenBLEV2_2*>(this)) != nullptr)
		{
			//V2.2+ interface
			_rootConnection = new MiTokenBLEConnectionV2_2(this);
		}
		else
		{
			//V2.1 interface
			_rootConnection = new MiTokenBLEConnection(this);
		}

		_rootConnection->Initialize(address);

		this->_curConnection = _rootConnection;
		requestID = _rootConnection->GetRequestID();
		return _rootConnection;
	}
	else
	{
		int offset = GetOffsetFromVoidPointersV2(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), _address);
		MiTokenBLEConnection* lastChecked = nullptr;
		ret = (MiTokenBLEConnection*)(BaseLinkedList::getNodeBasedOnDataOrCreateNew(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), offset, sizeof(address), &address, this));
		
		ret->Initialize(address);

		this->_curConnection = ret;
		requestID = ret->GetRequestID();
		return (IMiTokenBLEConnection*)ret;
	}
}

BLE_API_RET MiTokenBLE::RestartScanner()
{
	CHECK_POINTER(_LowLevel);
	
	return _LowLevel->RestartScanner();
}

bool MiTokenBLE::COMOwnerKnown() const
{
	CHECK_POINTER(_BLE_IO);

	return _BLE_IO->COMOwnerKnown();
}

BLE_API_RET MiTokenBLE::SetCOMWaitHandle(void* waitHandle)
{
	CHECK_POINTER(_BLE_IO);

	return _BLE_IO->SetCOMWaitHandle(waitHandle);
}

BLE_API_NP_RETURNS MiTokenBLE::ReleaseCOMPort()
{
	CHECK_NP_POINTER(_BLE_IO);

	return _BLE_IO->ReleaseCOMPort();
}

void MiTokenBLE::SetPollID(int newPollID)
{
	_curPollID = newPollID;
}

int MiTokenBLE::GetPollID() const
{
	return _curPollID;
}

int MiTokenBLE::GetDeviceFoundCount() const
{
	return PollData::GetDeviceCount(&_rootPoll);
}

BLE_API_RET CopyFromPollDataToDeviceInfo(PollData* data, struct DeviceInfo* pDeviceInfo)
{
	DEBUG_ENTER_FUNCTION;

	memcpy(pDeviceInfo->address, data->address, 6);
	pDeviceInfo->lastSeen = data->lastSeen;
	pDeviceInfo->RSSI = data->CalculateRSSI();

	DEBUG_RETURN BLE_API_SUCCESS;
}

BLE_API_RET MiTokenBLE::GetDeviceInfo(int deviceID, DeviceInfo* pDeviceInfo) const
{
	PollData* data = PollData::GetDeviceFromID(deviceID, &_rootPoll);
	if (data == nullptr)
	{
		return BLE_API_ERR_INVALID_DEVICE_ID;
	}

	return CopyFromPollDataToDeviceInfo(data, pDeviceInfo);
}

BLE_API_RET MiTokenBLE::GetDeviceMetaInfo(int deviceID, int flag, uint8* buffer, int* bufferLength) const
{
	if (bufferLength == nullptr)
	{
		return BLE_API_ERR_NULL_POINTER;
	}

	PollData* data = PollData::GetDeviceFromID(deviceID, &_rootPoll);
	if (data == nullptr)
	{
		return BLE_API_ERR_NO_SUCH_DEVICE;
	}

	int inputLength = *bufferLength;

	int indexCache = 0;
	if (!data->getMetaDataLength(flag, bufferLength, indexCache))
	{
		return BLE_API_ERR_NO_SUCH_META_FLAG;
	}

	if ((inputLength < *bufferLength) || (buffer == nullptr))
	{
		return BLE_API_MORE_DATA;
	}

	if (!data->getMetaData(flag, buffer, *bufferLength, indexCache))
	{
		return BLE_API_ERR_FAILED_GETTING_META_DATA;
	}

	return BLE_API_SUCCESS;
}

FilteredSearchObject MiTokenBLE::StartFilteredSearch(int filterCount, DeviceData* filters)
{
	return new FilteredSearch(filterCount, filters, this);
}

FilteredSearchObject MiTokenBLE::StartFilteredSearchEx(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray)
{
	return new FilteredSearch(filterCount, filters, addressCount, addressArray, this);
}

BLE_API_RET MiTokenBLE::ConvertFilteredSearchToByteStream(FilteredSearchObject filter, uint8* byteStream, int& length)
{
	FilteredSearch* pFilter = static_cast<FilteredSearch*>(filter);
	return pFilter->convertToByteStream(byteStream, length);
}

FilteredSearchObject MiTokenBLE::ConvertByteStreamToFilteredSearch(uint8* byteStream, int length)
{
	return new FilteredSearch(byteStream, length, this);
}

BLE_API_RET MiTokenBLE::ContinueFilteredSearch(FilteredSearchObject filter, DeviceInfo* pDeviceInfo)
{
	FilteredSearch* pFilter = (FilteredSearch*)filter;
	PollData* node = pFilter->findNextNode();
	if (node == nullptr)
	{
		return BLE_API_ERR_SEARCH_FINISHED;
	}

	return CopyFromPollDataToDeviceInfo(node, pDeviceInfo);
}

void MiTokenBLE::RestartFilteredSearch(FilteredSearchObject filter)
{
	FilteredSearch* pFilter = (FilteredSearch*)filter;
	pFilter->restart(_rootPoll);
}

void MiTokenBLE::FinishedFilteredSearch(FilteredSearchObject filter)
{
	delete ((FilteredSearch*)filter);
}

void MiTokenBLE::DEBUG_PrintDeiveMetaData(int deviceID) const
{
#ifndef _DEBUG
	//do nothing
	DEBUG_END_FUNCTION;
#else //_DEBUG is defined
	PollData* data = PollData::GetDeviceFromID(deviceID, &_rootPoll);

	if (data == NULL)
	{
		printf("Device with ID not found\r\n");
	}

	data->printMetaData(this);
#endif	
}

void MiTokenBLE::DEBUG_SetIODebugOutput(void(__stdcall *dataIn) (int amount), void(__stdcall *dataOut) (int amount))
{
#ifndef _DEBUG
	//do nothing
	DEBUG_END_FUNCTION;
#else
	debugDataIn = dataIn;
	debugDataOut = dataOut;
#endif
}


BLE_LowLevel* MiTokenBLE::getLowLevel()
{
	return _LowLevel;
}

PollData** MiTokenBLE::GetPRootPoll()
{
	return &_rootPoll;
}

int MiTokenBLE::GetNextRequestID()
{
	return _nextRequestID++;
}

IBLE_IO* MiTokenBLE::GetIO() const
{
	return _BLE_IO;
}

IMiTokenBLEConnection* MiTokenBLE::GetConnectionWithConnectionID(int connectionID)
{
	int offset = GetOffsetFromVoidPointersV2(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), _connectionID);
	return ((MiTokenBLEConnection*)MiTokenBLEConnection::getNodeBasedOnDataIfExists(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), offset, sizeof(connectionID), &connectionID, nullptr));
}

IMiTokenBLEConnection* MiTokenBLE::GetConnectionWithMACAddress(mac_address address)
{
	int offset = GetOffsetFromVoidPointersV2(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), _address);

	offset = GetOffsetFromVoidPointersForInheritedObject(dynamic_cast<MiTokenBLEConnection*>(_rootConnection), _address, BaseLinkedList*);
	return ((MiTokenBLEConnection*)MiTokenBLEConnection::getNodeBasedOnDataIfExists(dynamic_cast<BaseLinkedList*>(_rootConnection), offset, sizeof(address), &address, nullptr));
}

ReadWriteLocker* MiTokenBLE::GetPollMetaDataLock()
{
	return _PollMetaDataLock;
}

CurrentState* MiTokenBLE::GetState()
{
	return _myState;
}

void MiTokenBLE::RegisterNewChild()
{
	WaitForSingleObject(_registerMutex, INFINITE);
	_childCount++;
	printf("MiTokenBLE : Child Registered\r\n");
	ReleaseMutex(_registerMutex);
}

void MiTokenBLE::UnregisterChild()
{
	WaitForSingleObject(_registerMutex, INFINITE);
	_childCount--;
	printf("MiTokenBLE : Child Unregistered\r\n");
	if ((_childCount == 0) && (_safeToDelete != nullptr))
	{
		SetEvent(_safeToDelete);
	}
	ReleaseMutex(_registerMutex);
}

DWORD WINAPI MiTokenBLE::deleteThread(LPVOID instance)
{

	MiTokenBLE* ble = (MiTokenBLE*)instance;

	delete ble;

	return 0;
	/*
	while (true)
	{
		WaitForSingleObject(ble->_registerMutex, INFINITE);
		if (ble->_childCount == 0)
		{
			printf("MiTokenBLE Deleted\r\n");
			delete ble;
			ble = nullptr;
			return 0;
		}
		if(ble->_safeToDelete == nullptr)
		{
			ble->_safeToDelete = CreateEvent(nullptr, false, false, nullptr);
		}

		ReleaseMutex(ble->_registerMutex);

		WaitForSingleObject(ble->_safeToDelete, INFINITE);
	}
	*/
}

BLE_API_RET MiTokenBLE::SafeDelete()
{

	if (_myState->shouldAbort())
	{
		return BLE_API_ERR_INTERFACE_TO_BE_DELETED;
	}

	//Signal children to abort
	_myState->abort();

	//Close everything we can down
	_connectedToCOM = false;

	_BLE_IO->ReleaseCOMPort();
	_BLE_IO->StopASyncRead();
	_BLE_IO->CloseSerialHandle();
	_BLE_IO->CloseNamedPipe();

	delete _BLE_IO;

	PollData::finalize();

	//Signal self to wait for all children to be aborted
	CreateNamedThread(nullptr, 0, MiTokenBLE::deleteThread, this, 0, nullptr, "BLE: Delete Thread");

	return BLE_API_SUCCESS;
}


BLE_API_NP_RETURNS MiTokenBLE::Pipe_DisableBLEForwarding()
{
	return GetIO()->DisableBLEForwarding();
}

BLE_API_NP_RETURNS MiTokenBLE::Pipe_HasExclusiveAccess() const
{
	return GetIO()->ExclusiveAccessStatus();
}
BLE_API_NP_RETURNS MiTokenBLE::Pipe_ReleaseExclusiveAccess()
{
	return GetIO()->ReleaseExclusiveAccess();
}
BLE_API_NP_RETURNS MiTokenBLE::Pipe_RequestExclusiveAccess()
{
	return GetIO()->RequestExclusiveAccess();
}
BLE_API_NP_RETURNS MiTokenBLE::Pipe_SendMessage(unsigned char* data, int length, int connectionID)
{
	return GetIO()->SendMessageOverNP(data, length, connectionID);
}
BLE_API_NP_RETURNS MiTokenBLE::Pipe_SetNamedPipeMessageCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID))
{
	return GetIO()->SetCallback(callback);
}

bool MiTokenBLE::connectionUpdateRequired(uint16 interval_min, uint16 interval_max, uint16 latency, uint16 timeout, bool justCheck)
{
	if((_connSettings.set == false) ||
		(_connSettings.interval_min != interval_min) ||
		(_connSettings.interval_max != interval_max) ||
		(_connSettings.latency != latency) ||
		(_connSettings.timeout != timeout))
	{
		if(!justCheck)
		{
			_connSettings.interval_min = interval_min;
			_connSettings.interval_max = interval_max;
			_connSettings.latency = latency;
			_connSettings.timeout = timeout;
			_connSettings.set = true;
		}

		return true;
	}

	return false;
}

void MiTokenBLE::forceConnectionUpdateRequired()
{
	_connSettings.set = false;
}

bool MiTokenBLE::allowLongServiceUUIDs()
{
	return _allowLongServiceUUIDs;
}

MiTokenBLEV2_2::MiTokenBLEV2_2() : MiTokenBLE(), _baseCommandChain(nullptr)
{
	_useNonUniqueAttributeUUIDMode = false;
	_allowLongServiceUUIDs = true; //MiTokenBLEV2_2 supports longServiceUUIDs, while MiTokenBLE does not (they didn't exist when it was finalized)

	_createDefaultChainLink();
}

MiTokenBLEV2_2::~MiTokenBLEV2_2()
{
	//Destroy the DefaultChainLink here
	if (_baseCommandChain != nullptr)
	{
		delete _baseCommandChain;
	}

	
}

BLE_API_RET MiTokenBLEV2_2::Initialize(const char* COMPort, const char* NamedPipePort, bool isServer, void (*pipeConnectedCallback) (IMiTokenBLE* instance))
{
	char str[80] = { 0 };
	BLE_API_RET retCode = BLE_API_ERR_UNKNOWN;
	bool COMOpened = false;

	if (COMPort != nullptr)
	{
		snprintf(str, sizeof(str)-1, "\\\\.\\%s", COMPort);
		int len = strlen(str) + 1;
		_COMPort = new char[len];
		memcpy(_COMPort, str, len);
	}

	if (isServer && (COMPort != nullptr))
	{
		retCode = _BLE_IO->OpenSerialHandle(str);
		if (retCode == 0)
		{
			COMOpened = true;
			_connectedToCOM = true;
		}
	}

	printf("Connecting to pipe %s\r\n", NamedPipePort);
	_BLE_IO->InitializeNamedPipe(NamedPipePort, isServer, pipeConnectedCallback);

	if (isServer && COMOpened)
	{
		_BLE_IO->InitBLEDevice();
	}
	else if (!isServer)
	{
		Sleep(1000);
		if (!_BLE_IO->ClientConnectedToNP())
		{
			if (COMPort != nullptr)
			{
				retCode = _BLE_IO->OpenSerialHandle(str);

				if (retCode != BLE_API_SUCCESS)
				{
					return retCode;
				}

				_connectedToCOM = true;
				_BLE_IO->InitBLEDevice();
			}
		}
	}

	if (!isServer || COMOpened)
	{
		_BLE_IO->StartASyncRead();
	}

	return BLE_API_SUCCESS;
}


void MiTokenBLEV2_2::EnableNonUniqueAttributeUUIDMode()
{
	_useNonUniqueAttributeUUIDMode = true;
}

const IBLE_COMMAND_CHAIN_LINK* MiTokenBLEV2_2::GetBaseCommandChain()
{
	return _baseCommandChain;
}

void MiTokenBLEV2_2::_checkPointerTable(const void* start, void* comparison, int length)
{
	int ptrlen = sizeof(void*);
	int ptrcount = length / ptrlen;
	const uint8* pstart = reinterpret_cast<const uint8*>(start);
	uint8* pcomp = reinterpret_cast<uint8*>(comparison);

	for (int i = 0; i < ptrcount ; ++i)
	{
		//check if the 2 pointers point to the same location
		if (memcmp(pstart + (i * ptrlen), pcomp + (i * ptrlen), ptrlen) == 0)
		{
			//they do, so remove the comparison pointer and have it point to nullptr instead
			memset((pcomp + (i * ptrlen)), 0, ptrlen);
		}
	}
}

BLE_API_RET MiTokenBLEV2_2::AppendCommandChain(IBLE_COMMAND_CHAIN_LINK* newLink, bool checkForAndRemoveDuplicatePointers)
{
	if (checkForAndRemoveDuplicatePointers && (newLink->isLinkModular() == false))
	{
		BLE_COMMAND_CHAIN_LINK_FULL* pfull = dynamic_cast<BLE_COMMAND_CHAIN_LINK_FULL*>(newLink);
		const IBLE_COMMAND_CHAIN_LINK* pchainNode = _baseCommandChain;
		while (pchainNode != nullptr)
		{
			if (pchainNode->isLinkModular())
			{
			}
			else
			{
				const BLE_COMMAND_CHAIN_LINK_FULL* fullPtr = dynamic_cast<const BLE_COMMAND_CHAIN_LINK_FULL*>(pchainNode);
				_checkPointerTable(&fullPtr->ble, &pfull->ble, sizeof(fullPtr->ble));
				_checkPointerTable(&fullPtr->evt, &pfull->evt, sizeof(fullPtr->evt));
				_checkPointerTable(&fullPtr->rsp, &pfull->rsp, sizeof(fullPtr->rsp));
			}
			pchainNode = pchainNode->getNextLink();
		}
		BLE_COMMAND_CHAIN_LINK_FULL nullChain;
		if ((memcmp(&nullChain.ble, &pfull->ble, sizeof(nullChain.ble)) == 0) &&
			(memcmp(&nullChain.evt, &pfull->evt, sizeof(nullChain.evt)) == 0) &&
			(memcmp(&nullChain.rsp, &pfull->rsp, sizeof(nullChain.rsp)) == 0))
		{
			//The chain is completely empty now
			return BLE_API_SUCCESS;
		}

	}

	_baseCommandChain->appendLink(newLink);
	return BLE_API_SUCCESS;
}

bool MiTokenBLEV2_2::useNonUniqueAttributeUUIDMode()
{
	return _useNonUniqueAttributeUUIDMode;
}

void MiTokenBLEV2_2::_createDefaultChainLink()
{
	if (_baseCommandChain != nullptr)
	{
		return; //it already exists
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(10);
#endif
	_baseCommandChain = new BLE_COMMAND_CHAIN_LINK_FULL();
	_populateChainEVT();
	_populateChainRSP();

#if USE_MEM_CHECK
	MemLeak_SetLocationID(2);
#endif	
}

void MiTokenBLEV2_2::_populateChainRSP()
{
	BLE_COMMAND_CHAIN_LINK_FULL* chain = dynamic_cast<BLE_COMMAND_CHAIN_LINK_FULL*>(_baseCommandChain);

	if(chain != nullptr)
	{
		chain->rsp.attclient.prepare_write = &RSP_ATTCLIENT_PREPARE_WRITE;

		chain->rsp.connection.disconnect = &RSP_CONNECTION_DISCONNECT;
		chain->rsp.connection.get_rssi =  &RSP_CONNECTION_GET_RSSI;
		
	}	
}

void MiTokenBLEV2_2::_populateChainEVT()
{
	BLE_COMMAND_CHAIN_LINK_FULL* chain = dynamic_cast<BLE_COMMAND_CHAIN_LINK_FULL*>(_baseCommandChain);

	if(chain != nullptr)
	{
		chain->evt.attclient.attribute_value = &EVT_ATTCLIENT_ATTRIBUTE_VALUE;
		chain->evt.attclient.find_information_found = &EVT_ATTCLIENT_FIND_INFORMATION_FOUND;
		chain->evt.attclient.group_found = &EVT_ATTCLIENT_GROUP_FOUND;
		chain->evt.attclient.procedure_completed = &EVT_ATTCLIENT_PROCEDURE_COMPLETED;

		chain->evt.connection.status = &EVT_CONNECTION_STATUS;
		chain->evt.connection.disconnected = &EVT_CONNECTION_DISCONNECTED;
	}

}

void MiTokenBLEV2_2::ForceUnsafeDisconnection(uint8 connectionHandle)
{
	this->_LowLevel->Disconnect(connectionHandle);
}