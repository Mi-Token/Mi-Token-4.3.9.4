#pragma once

#ifndef _BLE_STRUCTS_H_
#define _BLE_STRUCTS_H_

#include "apitypes.h"

#include "BLE_API_Types.h"

#include <Windows.h>
#include "BLE_API.h"

class MiTokenBLE;

#include "ReadWriteLocker.h"

#define GetOffsetFromVoidPointers(dest, source) \
	((int)dest - (int)source)

#define GetOffsetFromVoidPointersV2(object, attribute) \
	GetOffsetFromVoidPointers(&(object->attribute), object)

#define GetOffsetFromVoidPointersForInheritedObject(object, attribute, childtype) \
	GetOffsetFromVoidPointersV2(object, attribute) - GetOffsetFromVoidPointers(dynamic_cast<childtype>(object), object)

/*
	Can be any value. Requests will start from BLE_CONN_NEW_REQUEST + 1 and therefore asking for request BLE_CONN_NEW_REQUEST
	will always fail to find a result and thus return a new ProfileRequest object
	-Best to leave it at 0
*/
/*
#define BLE_CONN_NEW_REQUEST 0

/*
	The return value of a request that couldn't be made (due to a request already being in the process of being made)
	-Best to leave it at BLE_CONN_NEW_REQUEST
*/
/*
#ifdef USE_APITYPES_HEADER
#define BLE_CONN_SUCCESS (BLE_CONN_NEW_REQUEST + 1)
#define BLE_CONN_BAD_REQUEST (BLE_CONN_NEW_REQUEST )
#define BLE_CONN_ERR_NOT_CONNECTED (BLE_CONN_NEW_REQUEST - 1)
#define BLE_CONN_ERR_NO_SUCH_CONNECTION (BLE_CONN_ERR_NOT_CONNECTED - 1)
#define BLE_CONN_ERR_NO_SUCH_SERVICE (BLE_CONN_ERR_NO_SUCH_CONNECTION - 1)
#define BLE_CONN_ERR_SERVICES_NOT_SCANNED (BLE_CONN_ERR_NO_SUCH_SERVICE - 1)
#define BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS (BLE_CONN_ERR_SERVICES_NOT_SCANNED - 1)
#define BLE_CONN_ERR_NO_SUCH_ATTRIBUTE (BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS - 1)
#define BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS (BLE_CONN_ERR_NO_SUCH_ATTRIBUTE - 1)
#define BLE_CONN_ERR_ATTRIBUTE_BEING_READ (BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS - 1)
#define BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA (BLE_CONN_ERR_ATTRIBUTE_BEING_READ - 1)
#define BLE_CONN_ERR_CONNECTION_TIMED_OUT (BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA - 1)

#define BLE_API_EXCLUSIVE_ALLOWED (BLE_CONN_SUCCESS)
#define BLE_API_EXCLUSIVE_ASKED (-20)
#define BLE_API_EXCLUSIVE_REJECTED (-21)
#define BLE_API_NP_ERR_NOT_CLIENT (-22)
#define BLE_API_EXCLUSIVE_DISABLED (-23)

#endif
*/

struct SingleLinkedList
{
	struct SingleLinkedList* nextNode;
	void* extraData;
};

class BaseLinkedList
{
public:
	BaseLinkedList* nextNode;

	static BaseLinkedList* getNodeBasedOnDataIfExists(BaseLinkedList* rootNode, int offset, int dataLen, void* data, BaseLinkedList** lastChecked);
	static BaseLinkedList* getNodeBasedOnDataOrCreateNew(BaseLinkedList* rootNode, int offset, int dataLen, void* data, void* initializeData);
	static void removeNodeFromList(BaseLinkedList* rootNode, int offset, int dataLen, void* data);
	virtual BaseLinkedList* createNewNode(void* initializeData) = 0;
	static void cleanUp(BaseLinkedList* baseNode);

protected:
	static BaseLinkedList* removeNode(BaseLinkedList* baseNodem, BaseLinkedList* nodeToRemove);
};


class FilteredSearch;

struct PollInit
{
	MiTokenBLE* parent;
	unsigned char* address;
};

class PollData : protected BaseLinkedList
{
public:

	friend class FilteredSearch; //needs access to (static)RootNode and (this)->nextNode;

	static PollData* findNodeWithAddress(unsigned char address[6], bool createNewIfNotExist, PollData** pRootNode, MiTokenBLE* _parent);
	unsigned char address[6];
	char RSSI;
	int lastSeen;
	int IIR_RSSI;
	int Total_PollRSSI;
	int Total_PollCount;

	int CalculateRSSI();

	static void GotPoll(unsigned char address[6], int rssi, int metaDataLength, void* metaData, MiTokenBLE* parent, PollData** pRootNode);
	static void GotRSSILessPoll(unsigned char address[6], int metaDataLength, void* metaData, PollData** pRootNode);
	static int GetDeviceCount(PollData* const * pRootNode = nullptr);
	static PollData* GetDeviceFromID(int deviceID, PollData* const * pRootNode = nullptr);

	void addMetaData(MiTokenBLE* parent, int metaDataLength, void* metaData);

	bool checkFilter(DeviceData* filter);
	static void finalize();

	bool getMetaDataLength(int flag, int* length, int& index);
	bool getMetaData(int flag, uint8* buffer, int length, int index);

	
#ifdef _DEBUG
	void printMetaData(const MiTokenBLE* parent);
#endif
private:
	//Use to speed up GetDeviceFromID
	static PollData* _lastDeviceInfoNode;
	static int _lastDeviceInfoID;

	//Constructor
	PollData(PollInit* init);

	//Destructor
	~PollData();

	//Copy Data [should not be used]
	PollData(PollData& reference);
	void operator = (PollData& reference);

	//Other stuff we don't want people touching
	static int DeviceCount;
	static PollData* rootNode;
	virtual BaseLinkedList* createNewNode(void* initializeData);

	int _metaDataCount;
	//DeviceData * _metaData;
	ProtectedPointer<DeviceData> _metaDataObj;
	MiTokenBLE* _parent;
	
};

class RequestData : protected BaseLinkedList
{
public:
	static RequestData* findNodeWithID(unsigned int ID);
	static RequestData* createNewRequest();
	unsigned int ID;
	unsigned int linkedConnectionID;
	bool connected;
	static RequestData* getRequestFromID(unsigned int ID);
	
	static RequestData* startRequest();
	static void finishRequest(uint8 connectionID);

	static void closeRequest(unsigned int ID);
	static bool hasRequest();
	static void timeout(LPVOID lpParam);

	bool timedOut;
private:

	//Constructor
	RequestData(int myID);

	//Destructor
	~RequestData();

	//Copy Data [should not be used]
	RequestData(RequestData& reference);
	void operator = (RequestData& reference);

	//Other stuff
	static RequestData* rootNode;
	static RequestData* latestNode;
	static RequestData* currentRequest;
	static HANDLE profileRequestMutex;
	virtual BaseLinkedList* createNewNode(void* initializeData);
	static void endRequest();
	
	
	
};

class ConnectionData;

struct Callbacks
{
	//Data callbacks
	void (*serviceFound) (ConnectionData* myData, uint8 serviceHandleLength, const uint8* serviceHandleData, uint16 start, uint16 end);
	void (*serviceInformationFound) (ConnectionData* myData, uint16 characteristicsHandle, uint8 dataLen, const uint8* data);
	void (*handleValue) (ConnectionData* myData, uint8 type, uint16 attributeHandle, uint8 dataLen, const uint8* data);
	void (*RSSIValue) (ConnectionData* myData, int8 rssi);
	void (*prepareWriteResponse) (ConnectionData* myData, uint16 result);


	//Completed callbacks
	void (*serviceSearchFinished) (ConnectionData* myData);
	void (*serviceInformationFinished) (ConnectionData* myData);
	void (*writeAttributeFinished) (ConnectionData* myData);
	void (*longWriteSectionCompleted) (ConnectionData* myData);
	void (*longReadCompleted) (ConnectionData* myData, uint16 handle);
	void (*extendedWriteCompleted) (ConnectionData* myData);
	void (*connectionClosed) (ConnectionData* myData);
};



class MessageBuffer
{
public:
	MessageBuffer();
	~MessageBuffer();
	void addMessage(uint16 handle, uint16 dataLength, const uint8* data);
	void append(MessageBuffer* bufferToAppend);
	bool hasDataWaiting();
	bool getWaitingData(uint16& attributeID, uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft);
	int getWaitingDataSize();
protected:

	
	class Nodes : public BaseLinkedList
	{
	public:

		friend class MessageBuffer;

		Nodes();
		Nodes(Nodes* baseNode);
		~Nodes();
		uint16 handle;
		uint16 dataLength;
		uint8* data;

	protected:
		virtual BaseLinkedList* createNewNode(void* initializeData);

	};

	uint16 currentNodeOffset;

	Nodes* rootNode;
	Nodes* lastNode;
};




class ConnectionServices : protected BaseLinkedList
{
	friend class ConnectionData;
	friend class MiTokenBLEConnection;
public:
	uint16 ID;
	uint16 start;
	uint16 end;
	uint8* longID;
	uint8 longIDLen;

protected:
	ConnectionServices(uint16 ID);
	ConnectionServices(const uint8* longID, uint8 longIDLen);
	virtual BaseLinkedList* createNewNode(void* initializeData);
};

class MiTokenBLEConnection;

class ConnectionAttributes : protected BaseLinkedList
{
	friend class ConnectionData;
	friend class MiTokenBLEConnection;

public:
	bool attributeIsGUID;
	uint16 serviceUUID;
	bool serviceIsGUID;
	uint16 ID;
	uint16 characteristicsHandle;
	uint8 dataLength;
	uint8* data;
	volatile bool reading;
	bool longread;
	bool writing;
	ConnectionAttributes* getNextNode();
protected:
	void setValue(uint8 length, const uint8* data);
	ConnectionAttributes(uint16 ID);
	virtual BaseLinkedList* createNewNode(void* initializeData);
};

class ConnectionData : protected BaseLinkedList
{
public:
	int connectionID;
	Callbacks callbacks;

	static ConnectionData* findNodeWithID(int connectionID, bool createNewNode = true);
	static void removeNodeWithID(int connectionID);
	static void CleanupData();

	ConnectionServices* getService(uint16 uuid, bool createNewService = true);
	ConnectionAttributes* getAttribute(uint16 uuid, bool createNewAttribute = true);
	void setAttributeValue(uint16 uuid, uint8 length, const uint8* data, bool createNewAttribute = false);
	bool getAttributeValue(uint16 uuid, uint8& value);
	bool getAttributeValue(uint16 uuid, uint16& value);

	int serviceCount();
	int attributeCount();

	bool attributeScanCompleted() const;
	bool serviceScanCompleted();

	bool beginAttributeScan();

	void finishServiceScan();
	void finishAttributeScan();

	static void closeConnection(ConnectionData*& connection);
	int getAverageRSSI();
	void addPoll(int pollID, int rssi);

	void (__stdcall *buttonPushed) (int requestID, int buttonID);

	int linkedRequestID;
	bool doingLongWrite;
	bool doingWrite;
	uint16 lastError;
	void SetNextExtendedWrite(uint16 attributeID, uint16 dataLength, const void* data);
	bool GetNextExtendedWrite(uint16& attributeID, uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft);
	void AppendToExtendedWrite(MessageBuffer* messageBuffer);

	ConnectionAttributes* getAttributeFromCharacteristicsHandle(uint16 handle);
private:

	MessageBuffer* _extendedWriteBuffer;

	void* _extendedWriteData;
	uint16 _extendedWriteDataLength;
	uint16 _extendedWriteAttributeID;
	uint16 _extendedWriteCurrentOffset;

	ConnectionServices* rootServiceNode;
	ConnectionAttributes* rootAttributeNode;
	int _servicesScanned;
	int _attributesScanned;
	bool _attributeScanCompleted;
	bool _serviceScanCompleted;


	ConnectionData(int connectionID);
	~ConnectionData();
	ConnectionData(ConnectionData& reference);
	void operator= (ConnectionData& reference);

	int _totalRSSI;
	int _currentRSSICount;
	int _currentRSSIPollID;

	static ConnectionData* rootNode;
	virtual BaseLinkedList* createNewNode(void* initializeData);
};


class FilteredSearch
{
public:
	FilteredSearch(int filterCount, DeviceData* filters, MiTokenBLE* parent);
	FilteredSearch(uint8* byteStream, int byteStreamLength, MiTokenBLE* parent);
	FilteredSearch::FilteredSearch(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray, MiTokenBLE* parent);
	~FilteredSearch();
	PollData* findNextNode();
	void restart(PollData* rootNode = nullptr);
	BLE_API_RET convertToByteStream(uint8* byteStream, int& length);

protected:

	void init(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray, MiTokenBLE* parent);
	bool _filterAddresses;
	int _filterAddressCount;
	uint8* _filterAddressArray;
	int _filterCount;
	DeviceData* _filters;
	PollData* _currentNode;
	bool _currentNodeValid();

private:


};

//extern struct SingleLinkedList* rootPollNodeV2;
//extern struct SingleLinkedList* rootRequestNode;
/*
struct SingleLinkedList* findNodeIfExistFromInt(struct SingleLinkedList* rootNode, int value, struct SingleLinkedList** lastSearchedNode);
struct SingleLinkedList* findNodeIfExistFromBytePtr(struct SingleLinkedList* rootNode, uint8 length, const uint8* data, struct SingleLinkedList** lastSearchedNode);
struct SingleLinkedList* findLastNode(struct SingleLinkedList* rootNode);
struct SingleLinkedList* findNodeFromIndex(struct SingleLinkedList* rootNode, int index);
*/

/*
struct ConnectionData
{
	struct ConnectionData* nextNode;
	int connectionID;
	struct Callbacks callbacks;
	void* extraData;
};

*/


//Non-Connected POLL Data
void POLL_GotPoll(uint8 addressLength, const uint8* address, int8 rssi);
int POLL_GetDeviceCount();

struct PollList
{
	struct PollList* nextNode;
	unsigned char MacAddress[6];
	char RSSI;
	int PollLastSeen;
	int IIR_RSSI;
	int Total_PollRSSI;
	int Total_PollCount;
};


struct PollListExtraData
{
	unsigned char address[6];
	char RSSI;
	int lastSeen;
	int IIR_RSSI;
	int Total_PollRSSI;
	int Total_PollCount;
};

struct ProfileRequestExtraData
{
	int RequestID;
	uint8 ConnectionID;
	int ConnectionCompleted;
};



extern struct PollList * rootPollNode;

//extern int CurrentPollID;
extern int PollEnabled;

struct PollList* getNodeOfAddress(const uint8 *MacAddress);

//struct ProfileRequestExtraData* startRequest();
//void finishRequest(uint8 connectionHandle);

extern volatile int currentPollID;

#endif

