#include "BLE_Structs.h"
#include <string>
#include <Windows.h>

#include "BLE_Callbacks.h"
#include "BLE_LowLevel.h"

#include <assert.h>

#include "DebugLogging.h"
#include "MiTokenBLE.h"
#include "ScopedLock.h"
#include "GenericUtils.h"


//int CurrentPollID = 0;
int PollEnabled = 0;
int PollDeviceCount = 0;

//struct PollList* rootPollNode = NULL;
//struct SingleLinkedList* rootPollNodeV2 = NULL;
//struct SingleLinkedList* rootRequestNode = NULL;
//HANDLE profileRequestMutex = CreateMutex(NULL, FALSE, NULL);

PollData* PollData::rootNode = NULL;
int PollData::DeviceCount = 0;
PollData* PollData::_lastDeviceInfoNode = NULL;
int PollData::_lastDeviceInfoID = -1;

RequestData* RequestData::rootNode = NULL;
RequestData* RequestData::latestNode = NULL;
RequestData* RequestData::currentRequest = NULL;
HANDLE RequestData::profileRequestMutex = CreateEvent(NULL, FALSE, TRUE, NULL);

ConnectionData* ConnectionData::rootNode = NULL;


#define WriteToMovingPtr(pointer, offset, source, length) \
	memcpy(pointer + offset, source, length); \
	offset += length;
#define ReadFromMovingPtr(pointer, offset, dest, length) \
	memcpy(dest, pointer + offset, length); \
	offset += length;

/*

struct SingleLinkedList* findNodeIfExistFromInt(struct SingleLinkedList* rootNode, int value, struct SingleLinkedList** lastSearchedNode)
{
	struct SingleLinkedList* curSearchNode = rootNode;
	if(lastSearchedNode)
	{
		*lastSearchedNode = NULL;
	}
	while(curSearchNode != NULL)
	{
		if(lastSearchedNode)
		{
			*lastSearchedNode = curSearchNode;
		}
		
		if((*(int*)curSearchNode->extraData) == value)
		{
			DEBUG_RETURN curSearchNode ;
		}

		curSearchNode = curSearchNode->nextNode;
	}

	DEBUG_RETURN  NULL;
}

struct SingleLinkedList* findNodeIfExistFromBytePtr(struct SingleLinkedList* rootNode, uint8 length, const uint8* data, struct SingleLinkedList** lastSearchedNode)
{
	struct SingleLinkedList* curSearchNode = rootNode;
	if(*lastSearchedNode)
	{
		*lastSearchedNode = NULL;
	}
	while(curSearchNode != NULL)
	{
		if(*lastSearchedNode)
		{
			*lastSearchedNode = curSearchNode;
		}

		if(memcmp((uint8*)curSearchNode->extraData, data, length) == 0)
		{
			DEBUG_RETURN curSearchNode;
		}

		curSearchNode = curSearchNode->nextNode;
	}

	DEBUG_RETURN NULL;
}

struct SingleLinkedList* findNodeFromIndex(struct SingleLinkedList* rootNode, int index)
{
	struct SingleLinkedList* curSearchNode = rootNode;

	while((curSearchNode != NULL) && (index > 0))
	{
		index--;
		curSearchNode = curSearchNode->nextNode;
	}

	DEBUG_RETURN curSearchNode;
}

struct SingleLinkedList* findLastNode(struct SingleLinkedList* rootNode)
{
	struct SingleLinkedList* curSearchNode = rootNode;
	struct SingleLinkedList* lastSearchNode = NULL;
	while(curSearchNode != NULL)
	{
		lastSearchNode = curSearchNode;
		curSearchNode = curSearchNode->nextNode;
	}

	DEBUG_RETURN lastSearchNode;
}
*/
/*
struct SingleLinkedList* createNewPoll(struct SingleLinkedList* nodeToAppendTo, uint8 addressLength, const uint8* address)
{
	struct SingleLinkedList* newNode;
	struct PollListExtraData* pollData;

	PollDeviceCount++;

	newNode = (struct SingleLinkedList*)malloc(sizeof(struct SingleLinkedList));
	newNode->nextNode = NULL;
	newNode->extraData = malloc(sizeof(struct PollListExtraData));

	pollData = (struct PollListExtraData*)newNode->extraData;

	memcpy(pollData->address, address, 6);
	pollData->IIR_RSSI = 0;
	pollData->lastSeen = 0;
	pollData->RSSI = 0;
	pollData->Total_PollCount = 0;
	pollData->Total_PollRSSI = 0;

	if(nodeToAppendTo == NULL)
	{
		rootPollNodeV2 = newNode;
	}
	else
	{
		nodeToAppendTo->nextNode = newNode;
	}

	DEBUG_RETURN newNode;
}*/
/*
void POLL_GotPoll(uint8 addressLength, const uint8* address, int8 rssi)
{
	
	struct SingleLinkedList *node, *prenode;
	struct PollListExtraData* pollData;
	if(addressLength != 6)
	{
		//something is wrong :/
	}
	else
	{
		node = findNodeIfExistFromBytePtr(rootPollNodeV2, addressLength, address, &prenode);
		if(node == NULL)
		{
			node = createNewPoll(prenode, addressLength, address);
		}

		pollData = (PollListExtraData*)node->extraData;

		if(pollData->lastSeen != CurrentPollID)
		{
			pollData->Total_PollCount = pollData->Total_PollRSSI = 0;
		}
		
		pollData->Total_PollCount++;
		pollData->Total_PollRSSI += rssi;
		pollData->lastSeen = CurrentPollID;
	}
	
}

int POLL_GetDeviceCount()
{
	DEBUG_RETURN PollDeviceCount;
}
*/
/*
struct ProfileRequestExtraData* currentRequest = NULL;

struct ProfileRequestExtraData* getProfileRequest(int requestID)
{
	struct SingleLinkedList *curNode, *lastNode;
	struct ProfileRequestExtraData* extraData;
	curNode = findNodeIfExistFromInt(rootRequestNode, requestID, &lastNode);

	if((curNode == NULL) && (requestID == BLE_CONN_NEW_REQUEST))
	{
		curNode = (struct SingleLinkedList*)malloc(sizeof(struct SingleLinkedList));
		curNode->extraData = malloc(sizeof(struct ProfileRequestExtraData));
		extraData = (struct ProfileRequestExtraData*)curNode->extraData;
		extraData->ConnectionCompleted = 0;
		extraData->ConnectionID = 0;
		curNode->nextNode = NULL;
		if(lastNode == NULL)
		{
			extraData->RequestID = (BLE_CONN_NEW_REQUEST) + 1;
			rootRequestNode = curNode;
		}
		else
		{
			extraData->RequestID = ((struct ProfileRequestExtraData*)lastNode->extraData)->RequestID;
			lastNode->nextNode = curNode;
		}
	}
	else
	{
		extraData = (struct ProfileRequestExtraData*)curNode->extraData;
	}

	DEBUG_RETURN extraData;
}

void endRequest()
{
	currentRequest = NULL;
	ReleaseMutex(profileRequestMutex);
}

void finishRequest(uint8 connectionHandle)
{
	currentRequest->ConnectionID = connectionHandle;
	currentRequest->ConnectionCompleted = 1;
	endRequest();
}

struct ProfileRequestExtraData* startRequest()
{
	if(WaitForSingleObject(profileRequestMutex, 0) == WAIT_TIMEOUT)
	{
		DEBUG_RETURN NULL;
	}

	currentRequest = getProfileRequest(BLE_CONN_NEW_REQUEST);

	DEBUG_RETURN currentRequest;

}

*/
void BaseLinkedList::cleanUp(BaseLinkedList* baseNode)
{
	DEBUG_ENTER_FUNCTION;

	BaseLinkedList *nextNode;
	while(baseNode != NULL)
	{
		nextNode = baseNode->nextNode;
		delete baseNode;
		baseNode = nextNode;
	}

	DEBUG_END_FUNCTION;
}

BaseLinkedList* BaseLinkedList::getNodeBasedOnDataIfExists(BaseLinkedList* rootNode, int offset, int dataLen, void* data, BaseLinkedList** lastChecked)
{
	DEBUG_ENTER_FUNCTION;

	BaseLinkedList* curNode;

	if(lastChecked != NULL)
	{
		*lastChecked = NULL;
	}
		
	if(rootNode == NULL)
	{
		DEBUG_RETURN NULL;
	}

	curNode = rootNode;
	while(curNode != NULL)
	{
		uint8* dataPos = (uint8*)curNode;
		dataPos += offset; //move to the location of the data
		if(memcmp(dataPos, data, dataLen) == 0)
		{
			DEBUG_RETURN curNode;
		}

		if(lastChecked != NULL)
		{
			*lastChecked = curNode;
		}
		curNode = curNode->nextNode;
	}

	DEBUG_RETURN NULL;
}

BaseLinkedList* BaseLinkedList::getNodeBasedOnDataOrCreateNew(BaseLinkedList* rootNode, int offset, int dataLen, void* data, void* initializeData)
{
	DEBUG_ENTER_FUNCTION;

	BaseLinkedList* lastNode = NULL;
	BaseLinkedList* result = NULL;
	result = getNodeBasedOnDataIfExists(rootNode, offset, dataLen, data, &lastNode);
	if(result != NULL)
	{
		DEBUG_RETURN result;
	}
	
	if(rootNode != NULL)
	{
		result = lastNode->createNewNode(initializeData);
	}

	DEBUG_RETURN result;	
}

BaseLinkedList* BaseLinkedList::removeNode(BaseLinkedList* rootNode, BaseLinkedList* node)
{
	DEBUG_ENTER_FUNCTION;

	//We DEBUG_RETURN the new rootNode (in all cases but where rootNode == node, this would just be rootNode)
	BaseLinkedList* nodeToDEBUG_RETURN = rootNode;

	if(rootNode == node)
	{
		nodeToDEBUG_RETURN = rootNode->nextNode;
		delete rootNode;
		DEBUG_RETURN nodeToDEBUG_RETURN;
	}

	BaseLinkedList* preNode = NULL;
	while((rootNode != node) && (rootNode != NULL))
	{
		preNode = rootNode;
		rootNode = rootNode->nextNode;
	}
	
	if(rootNode != NULL)
	{
		preNode->nextNode = node->nextNode;

		delete node;
	}
}

void BaseLinkedList::removeNodeFromList(BaseLinkedList* rootNode, int offset, int dataLen, void* data)
{
	DEBUG_ENTER_FUNCTION;

	BaseLinkedList* lastNode = NULL;
	BaseLinkedList* result = NULL;

	result = getNodeBasedOnDataIfExists(rootNode, offset, dataLen, data, &lastNode);
	if(result != NULL)
	{
		//check if it was rootNode (lastNode == NULL)
		if(lastNode == NULL) 
		{
			rootNode = result->nextNode;
		}
		else
		{
			//skip this node (last nodes next should be this nodes next)
			lastNode->nextNode = result->nextNode;
		}

		delete (result);
	}

	DEBUG_END_FUNCTION;
}



PollData::PollData(PollInit* init)
{
	DEBUG_ENTER_FUNCTION;

	memcpy(this->address, init->address, 6);
	this->RSSI = 0;
	this->lastSeen = 0;
	this->IIR_RSSI = 0;
	this->Total_PollCount = 0;
	this->Total_PollRSSI = 0;
	this->nextNode = NULL;
	//this->_metaData = NULL;
	this->_parent = init->parent;
	this->_metaDataCount = 0;
	DeviceCount++;

	DEBUG_END_FUNCTION;
}

BaseLinkedList* PollData::createNewNode(void* initializeData)
{
	DEBUG_ENTER_FUNCTION;

	PollData* newNode = new PollData((PollInit*)initializeData);
	this->nextNode = newNode;
	
	DEBUG_RETURN newNode;
}


void PollData::finalize()
{
	DEBUG_ENTER_FUNCTION;

	PollData* node = rootNode;
	rootNode = NULL;

	PollData* rnode;
	while(node)
	{
		rnode = node;
		node = static_cast<PollData*>(node->nextNode);
		delete rnode;
	}

	DeviceCount = 0;
	_lastDeviceInfoNode = NULL;

	DEBUG_END_FUNCTION;
}

PollData::~PollData()
{
	DEBUG_ENTER_FUNCTION;

	WriteLock lock;
	DeviceData** _pMetaData = _metaDataObj.GetDataPtr(_parent->GetPollMetaDataLock(), &lock);

	DeviceData* _metaData = *_pMetaData;
	_pMetaData = nullptr;

	if(_metaData)
	{
		delete[] _metaData;
	}

	DEBUG_END_FUNCTION;
}

PollData* PollData::findNodeWithAddress(unsigned char address[6], bool createNewIfNotExist, PollData** pRootNode, MiTokenBLE* _parent)
{
	DEBUG_ENTER_FUNCTION;
	if (pRootNode == nullptr)
	{
		pRootNode = &rootNode;
	}

	if (*pRootNode == NULL)
	{
		if(createNewIfNotExist)
		{
			PollInit pi;
			pi.address = address;
			pi.parent = _parent;
			*pRootNode = new PollData(&pi);
			DEBUG_RETURN *pRootNode;
		}
		else
		{
			DEBUG_RETURN NULL;
		}
	}


	int offset = GetOffsetFromVoidPointersV2((*pRootNode), address);
	
	if(createNewIfNotExist)
	{
		PollInit pi;
		pi.address = address;
		pi.parent = _parent;
		DEBUG_RETURN(PollData*)getNodeBasedOnDataOrCreateNew(*pRootNode, offset, 6, address, &pi);
	}
	else
	{
		DEBUG_RETURN(PollData*)getNodeBasedOnDataIfExists(*pRootNode, offset, 6, address, NULL);
	}
}

void PollData::GotRSSILessPoll(unsigned char address[6], int metaDataLength, void* metaData, PollData** pRootNode)
{
	DEBUG_ENTER_FUNCTION;
	/*
	PollData* data = findNodeWithAddress(address, false, pRootNode);
	if(data != NULL)
	{
		data->addMetaData(metaDataLength, metaData);
	}
	*/
	DEBUG_END_FUNCTION;
}

void PollData::GotPoll(unsigned char address[6], int rssi, int metaDataLength, void* metaData, MiTokenBLE* parent, PollData** pRootNode)
{
	DEBUG_ENTER_FUNCTION;

	PollData* data = findNodeWithAddress(address, true, pRootNode, parent);
	data->addMetaData(parent, metaDataLength, metaData);

	int CurrentPollID = parent->GetPollID();
	if(data->lastSeen != CurrentPollID)
	{
		data->Total_PollCount = data->Total_PollRSSI = 0;
	}

	data->Total_PollCount++;
	data->Total_PollRSSI += rssi;
	data->lastSeen = CurrentPollID;
	
	char sbuffer[250];
	sprintf(sbuffer, "Got poll on %02lX:%02lX:%02lX:%02lX:%02lX:%02lX with RSSI of %d\r\n\tcurrent TotalRSSI = %d, pollCount = %d, ave = %d\r\n",
		address[0], address[1], address[2], address[3], address[4], address[5],
		rssi,
		data->Total_PollRSSI, data->Total_PollCount, (data->Total_PollRSSI / data->Total_PollCount));

	//printf("%s", sbuffer);
	OutputDebugStringA(sbuffer);
	/*
	DEBUG_LOG("%s", sbuffer);
	*/
	DEBUG_END_FUNCTION;
}

int PollData::GetDeviceCount(PollData* const* pRootNode)
{
	DEBUG_ENTER_FUNCTION;
	PollData* pRoot = rootNode;
	if (pRootNode != nullptr)
	{
		pRoot = *pRootNode;
	}

	int devCount = 0;
	while (pRoot != nullptr)
	{
		pRoot = (PollData*)pRoot->nextNode;
		devCount++;
	}

	DEBUG_RETURN devCount;
}

int PollData::CalculateRSSI()
{
	DEBUG_ENTER_FUNCTION;

	if(Total_PollCount == 0)
	{
		return 0;
	}
	DEBUG_RETURN Total_PollRSSI / Total_PollCount;
}

PollData* PollData::GetDeviceFromID(int deviceID, PollData* const * pRootNode)
{
	DEBUG_ENTER_FUNCTION;

	PollData* baseNode = rootNode;

	if (pRootNode != nullptr)
	{
		baseNode = *pRootNode;
		_lastDeviceInfoID = -1;
	}

	int searchLeft = deviceID;

	if((_lastDeviceInfoID != -1) && (_lastDeviceInfoID < deviceID))
	{
		//we can skip the first [_lastDeviceInfoID] nodes by skipping straight to [_lastDeviceInfoNode]
		baseNode = _lastDeviceInfoNode;
		searchLeft -= _lastDeviceInfoID;
	}

	while((searchLeft > 0) && (baseNode != NULL))
	{
		searchLeft -= 1;
		baseNode = (PollData*)baseNode->nextNode;
	}

	_lastDeviceInfoNode = baseNode;
	_lastDeviceInfoID = deviceID;

	DEBUG_RETURN baseNode;
}

bool PollData::getMetaDataLength(int flag, int* length, int& index)
{
	DEBUG_ENTER_FUNCTION;

	ReadLock lock;

	const DeviceData* _metaData = _metaDataObj.GetRead(_parent->GetPollMetaDataLock(), &lock);

	if(_metaDataCount != 0)
	{
		for(int i = 0 ; i < _metaDataCount ; ++i)
		{
			if(_metaData[i].flag == flag)
			{
				index = i;
				*length = _metaData[i].length;
				DEBUG_RETURN true;
			}
		}
	}
	DEBUG_RETURN false;
}
bool PollData::getMetaData(int flag, uint8* buffer, int length, int index)
{
	DEBUG_ENTER_FUNCTION;

	ReadLock lock;

	const DeviceData* _metaData = _metaDataObj.GetRead(_parent->GetPollMetaDataLock(), &lock);

	if(_metaDataCount != 0)
	{
		if(_metaData[index].flag != flag)
		{
			//invalid index value, research
			for(int i = 0 ; i < _metaDataCount; i++)
			{

				if(_metaData[i].flag == flag)
				{
					index = i;
					break;
				}
			}
		}

		//verify that the index is correct now
		if(_metaData[index].flag == flag)
		{
			if(_metaData[index].length <= length)
			{
				memcpy(buffer, _metaData[index].data, _metaData[index].length);
				DEBUG_RETURN true;
			}
		}
	}
	DEBUG_RETURN false;
}


void PollData::addMetaData(MiTokenBLE* parent, int metaDataLength, void* metaData)
{
	DEBUG_ENTER_FUNCTION;
	WriteLock lock;

	DeviceData** _pMetaData = _metaDataObj.GetDataPtr(_parent->GetPollMetaDataLock(), &lock);
	DeviceData* const _metaData = *_pMetaData;

	if((_metaDataCount != 0) || (metaDataLength == 0))
	{
		if(metaDataLength == 0)
		{
			//there is no meta data
			DEBUG_END_FUNCTION;
		}

		int currentLocation = 0;
		unsigned char* byteData = (unsigned char*)metaData;
		unsigned int metaCount = 0;
		while(currentLocation < metaDataLength)
		{
			unsigned char len = byteData[currentLocation];
			unsigned char flag = byteData[currentLocation + 1];
			bool found = false;
			for(int i = 0 ; (!found) && (i < _metaDataCount) ; ++i)
			{
				if(_metaData[i].flag == flag)
				{
					if(_metaData[i].length != (len - 1))
					{
						//Allocate a new byte array and copy the data across
						BYTE* newData = new BYTE[len - 1];
						memcpy(newData, byteData + currentLocation + 2, len - 1);
						BYTE* temp = _metaData[i].data;
						_metaData[i].data = newData;
						_metaData[i].length = len - 1;
						delete temp;
					}
					else
					{
						if(memcmp(_metaData[i].data, byteData + currentLocation + 2, _metaData[i].length) != 0)
						{
							memcpy(_metaData[i].data, byteData + currentLocation + 2, _metaData[i].length);
						}
					}
					found = true;
				}
			}
			if(!found)
			{
				metaCount++;
			}

			currentLocation += 1 + len;
		}

		if(metaCount == 0)
		{
			//no new data
			DEBUG_END_FUNCTION;
		}

		DeviceData* newMeta = new DeviceData[_metaDataCount + metaCount];
		int newMetaCount = metaCount + _metaDataCount;


		//Copy over old data
		for(int index = 0 ; index < _metaDataCount ; ++index)
		{
			newMeta[index].flag = _metaData[index].flag;
			newMeta[index].length = _metaData[index].length;
			newMeta[index].data = _metaData[index].data;
			_metaData[index].data = NULL;
		}

		currentLocation = 0;
		int curIndex = _metaDataCount;		
		while(currentLocation < metaDataLength)
		{
			unsigned char len, flag;
			len = byteData[currentLocation];
			flag = byteData[currentLocation + 1];
			bool found = false;
			for(int i = 0 ; (!found) && (i < _metaDataCount) ; ++i)
			{
				if(_metaData[i].flag == flag)
				{
					found = true;
				}
			}
			if(!found)
			{
				newMeta[curIndex].flag = flag;
				newMeta[curIndex].length = len - 1;
				unsigned char* data;
				data = new unsigned char[len - 1];
				memcpy(data, &(byteData[currentLocation + 2]), len - 1);
				newMeta[curIndex].data = data;

				curIndex++;
			}

			currentLocation += 1 + len;
		}

		delete[] _metaData;
		*_pMetaData = newMeta;
		_metaDataCount = newMetaCount;

		DEBUG_END_FUNCTION;
		//we already have metadata or there is none
		//DEBUG_RETURN;
	}
	else
	{
		int currentLocation = 0;
		//Quickly scan how many items we have
		//Format is [len][flag][data] where [len] = sizeof([flag] + [data]) and sizeof[flag] is always 1 (all elements are bytes)
		unsigned char* byteData = (unsigned char*)metaData;
		unsigned int metaCount = 0;
		while(currentLocation < metaDataLength)
		{
			metaCount++;
			unsigned char len = byteData[currentLocation];
			currentLocation += 1 + len; //move forward 1 to move to flag element, then move forward [len] elements ([len] = sizeof([flag]) + sizeof([data])
		}
		*_pMetaData = new DeviceData[metaCount];
		const_cast<DeviceData*>(_metaData) = *_pMetaData;
		_metaDataCount = metaCount;
		currentLocation = 0;
		metaCount = 0;
		while(currentLocation < metaDataLength)
		{
			unsigned char len, flag;
			len = byteData[currentLocation];
			flag = byteData[currentLocation + 1];
			unsigned char* data;
			data = new unsigned char[len - 1]; //len includes sizeof([flag]) which is 1, so get rid of that
			memcpy(data, &(byteData[currentLocation + 2]), len - 1);

			_metaData[metaCount].length = len - 1; //we will store the length of data, not the length of data + flag
			_metaData[metaCount].flag = flag;
			_metaData[metaCount].data = data;

			metaCount++;

			currentLocation += 1 + len;
		}
	}

	DEBUG_END_FUNCTION;
}

#ifdef _DEBUG
void PollData::printMetaData(const MiTokenBLE* parent)
{
	ReadLock lock;

	const DeviceData* _metaData = _metaDataObj.GetRead(_parent->GetPollMetaDataLock(), &lock);

	printf("Meta Data : \r\n\tCount : %d\r\n", _metaDataCount);
	for(int i = 0 ; i < _metaDataCount ; ++i)
	{
		printf("\t\tID : %d\r\n\t\tFlag : %d\r\n\t\tLength : %d\r\n\t\tData : ", i, _metaData[i].flag, _metaData[i].length);
		for(int j = 0 ; j < _metaData[i].length ; ++j)
		{
			printf("%02lX ", (unsigned int)_metaData[i].data[j]);
		}
		printf("\r\n\t\t : \"");
		for(int j = 0 ; j < _metaData[i].length ; ++j)
		{
			printf("%c", (_metaData[i].data[j] > 0x20 ? _metaData[i].data[j] : ' '));
		}
		printf("\"\r\n");
	}
}
#endif
bool PollData::checkFilter(DeviceData* filter)
{
	DEBUG_ENTER_FUNCTION;

	ReadLock lock;
	const DeviceData* _metaData = _metaDataObj.GetRead(_parent->GetPollMetaDataLock(), &lock);


	for(int i = 0 ; i < _metaDataCount ; ++i)
	{
		if(_metaData[i].flag == filter->flag)
		{
			if(filter->length == _metaData[i].length)
			{
				if(memcmp(filter->data, _metaData[i].data, _metaData[i].length) == 0)
				{
					//found flag with same data
					DEBUG_RETURN true;
				}
			}
		}
	}

	//never found flag with same data
	DEBUG_RETURN false;
}


RequestData::RequestData(int myID)
{
	DEBUG_ENTER_FUNCTION;

	this->ID = myID;
	this->linkedConnectionID = 0;
	this->connected = false;
	this->nextNode = NULL;
	latestNode = this;
	timedOut = false;

	DEBUG_END_FUNCTION;
}

RequestData::~RequestData()
{
	DEBUG_ENTER_FUNCTION;

	//Nothing is alloced so nothing needs to be freed

	DEBUG_END_FUNCTION;
}

BaseLinkedList* RequestData::createNewNode(void* initializeData)
{
	DEBUG_ENTER_FUNCTION;

	RequestData* newNode = new RequestData(*((int*)initializeData));
	this->nextNode = newNode;
	DEBUG_RETURN newNode;
}

RequestData* RequestData::findNodeWithID(unsigned int ID)
{
	DEBUG_ENTER_FUNCTION;

	if(rootNode == NULL)
	{
		DEBUG_RETURN NULL;
	}

	int offset = GetOffsetFromVoidPointers(&(rootNode->ID), rootNode);

	RequestData* lastNode = NULL;
	RequestData* ret = (RequestData*)getNodeBasedOnDataIfExists(rootNode, offset, sizeof(ID), &ID, (BaseLinkedList**)&lastNode);
	DEBUG_RETURN ret;
}

RequestData* RequestData::startRequest()
{
	DEBUG_ENTER_FUNCTION;

	//a request is already going on
	if(WaitForSingleObject(profileRequestMutex, 0) == WAIT_TIMEOUT)
	{
		DEBUG_RETURN NULL;
	}

	currentRequest = createNewRequest();

	DEBUG_RETURN currentRequest;
}

DWORD WINAPI RequestTimeout(LPVOID lpParam)
{
	DEBUG_ENTER_FUNCTION;

	Sleep(60000);
	RequestData::timeout(lpParam);
	DEBUG_RETURN 0;
}

void RequestData::timeout(LPVOID lpParam)
{
	DEBUG_ENTER_FUNCTION;

	RequestData* pRequest = (RequestData*)lpParam;
	if(pRequest->connected == false)
	{
		//Timeout
		BLE_LL_EndProcedure();
		endRequest();
		pRequest->timedOut = true;
	}
}

bool RequestData::hasRequest()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN currentRequest != NULL;
}

RequestData* RequestData::createNewRequest()
{
	DEBUG_ENTER_FUNCTION;

	RequestData* lastNode;

	lastNode = latestNode;
	int ID = 1;

	if((lastNode == NULL) && (rootNode != NULL))
	{
		lastNode = rootNode;
		while(lastNode->nextNode != NULL)
		{
			lastNode = (RequestData*)lastNode->nextNode;
		}
	}

	if(lastNode != NULL)
	{
		ID = lastNode->ID + 1;
	}
	
	RequestData* newRequest = new RequestData(ID);

	if(lastNode != NULL)
	{
		lastNode->nextNode = newRequest;
	}
	else
	{
		//if latestNode was NULL, we must have no nodes
		rootNode = newRequest;
	}
	
	//Create a timeout thread
	CreateNamedThread(NULL, 0, RequestTimeout, newRequest, 0, NULL, "Request: NewRequest");

	DEBUG_RETURN newRequest;
}

void RequestData::endRequest()
{
	DEBUG_ENTER_FUNCTION;

	currentRequest = NULL;
	SetEvent(profileRequestMutex);

	DEBUG_END_FUNCTION;
}

void RequestData::finishRequest(uint8 connectionID)
{
	DEBUG_ENTER_FUNCTION;

	RequestData* pCR = currentRequest;
	if(pCR != NULL)
	{
		pCR ->linkedConnectionID = connectionID;
		pCR ->connected = true;
		ConnectionData* conn = ConnectionData::findNodeWithID(connectionID, false);
		conn->linkedRequestID = pCR->ID;
		conn->callbacks.connectionClosed = BLE_CALLBACK_ConnectionClosed;
		endRequest();

	}
	DEBUG_END_FUNCTION;
}

void RequestData::closeRequest(unsigned int ID)
{
	DEBUG_ENTER_FUNCTION;

	RequestData* node = findNodeWithID(ID);
	if(node != NULL)
	{
		if(node == latestNode)
		{
			latestNode = NULL;
		}
		//BaseLinkedList::removeNode DEBUG_RETURNs the new rootNode (which would change iff rootNode == node)
		rootNode = (RequestData*)BaseLinkedList::removeNode(rootNode, node);
	}

	DEBUG_END_FUNCTION;
}

ConnectionData::ConnectionData(int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	memset(&this->callbacks, 0, sizeof(this->callbacks));
	this->connectionID = connectionID;
	this->nextNode = NULL;
	this->rootAttributeNode = NULL;
	this->rootServiceNode = NULL;
	this->_serviceScanCompleted = false;
	this->_attributeScanCompleted = true;
	this->_currentRSSICount = 0;
	this->_currentRSSIPollID = 0;
	this->_totalRSSI = 0;
	this->doingLongWrite = false;
	this->doingWrite = false;
	this->_extendedWriteAttributeID = this->_extendedWriteCurrentOffset = this->_extendedWriteDataLength = 0;
	this->_extendedWriteData = NULL;
	this->_extendedWriteBuffer = new MessageBuffer();

	DEBUG_END_FUNCTION;
}

ConnectionData* ConnectionData::findNodeWithID(int connectionID, bool createNewNode)
{
	DEBUG_ENTER_FUNCTION;

	if(rootNode == NULL)
	{
		if(createNewNode)
		{
			rootNode = new ConnectionData(connectionID);
			DEBUG_RETURN rootNode;
		}
		else
		{
			DEBUG_RETURN NULL;
		}
	}

	int offset = GetOffsetFromVoidPointers(&(rootNode->connectionID), rootNode);

	if(createNewNode)
	{
		DEBUG_RETURN (ConnectionData*)getNodeBasedOnDataOrCreateNew(rootNode, offset, sizeof(connectionID), &connectionID, &connectionID);
	}
	else
	{
		DEBUG_RETURN (ConnectionData*)getNodeBasedOnDataIfExists(rootNode, offset, sizeof(connectionID), &connectionID, NULL);
	}

}

void ConnectionData::removeNodeWithID(int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	if(rootNode != NULL)
	{
		int offset = GetOffsetFromVoidPointers(&(rootNode->connectionID), rootNode);
		removeNodeFromList(rootNode, offset, sizeof(connectionID), &connectionID);
	}

	DEBUG_END_FUNCTION;
}

ConnectionAttributes* ConnectionData::getAttributeFromCharacteristicsHandle(uint16 handle)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* ret = rootAttributeNode;
	while(ret != NULL)
	{
		if(ret->characteristicsHandle == handle)
		{
			DEBUG_RETURN ret;
		}
		ret = (ConnectionAttributes*)ret->nextNode;
	}

	DEBUG_RETURN ret;
}

BaseLinkedList* ConnectionData::createNewNode(void* initializeData)
{
	DEBUG_ENTER_FUNCTION;

	this->nextNode = new ConnectionData(*((int*)initializeData));
	DEBUG_RETURN this->nextNode;
}

void ConnectionData::CleanupData()
{
	DEBUG_ENTER_FUNCTION;

	cleanUp(rootNode);

	DEBUG_END_FUNCTION;
}

ConnectionServices* ConnectionData::getService(uint16 uuid, bool createNewService)
{
	DEBUG_ENTER_FUNCTION;

	if(rootServiceNode == NULL)
	{
		if(createNewService)
		{
			rootServiceNode = new ConnectionServices(uuid);
			DEBUG_RETURN rootServiceNode;
		}
		else
		{
			DEBUG_RETURN NULL;
		}
	}
	else
	{
		int offset = GetOffsetFromVoidPointers(&(rootServiceNode->ID), rootServiceNode);
		if(createNewService)
		{
			DEBUG_RETURN (ConnectionServices*)getNodeBasedOnDataOrCreateNew(rootServiceNode, offset, sizeof(uuid), &uuid, &uuid);
		}
		else
		{
			DEBUG_RETURN (ConnectionServices*)getNodeBasedOnDataIfExists(rootServiceNode, offset, sizeof(uuid), &uuid, NULL);
		}
	}
}

ConnectionAttributes* ConnectionData::getAttribute(uint16 uuid, bool createNewAttribute)
{
	DEBUG_ENTER_FUNCTION;

	if(rootAttributeNode == NULL)
	{
		if(createNewAttribute)
		{
			rootAttributeNode = new ConnectionAttributes(uuid);
			DEBUG_RETURN rootAttributeNode;
		}
		else
		{
			DEBUG_RETURN NULL;
		}
	}
	else
	{
		int offset = GetOffsetFromVoidPointers(&(rootAttributeNode->ID), rootAttributeNode);
		if(createNewAttribute)
		{
			DEBUG_RETURN (ConnectionAttributes*)getNodeBasedOnDataOrCreateNew(rootAttributeNode, offset, sizeof(uuid), &uuid, &uuid);
		}
		else
		{
			DEBUG_RETURN (ConnectionAttributes*)getNodeBasedOnDataIfExists(rootAttributeNode, offset, sizeof(uuid), &uuid, NULL);
		}
	}
}

void ConnectionData::setAttributeValue(uint16 uuid, uint8 length, const uint8* data, bool createNewAttribute)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* attribute = getAttribute(uuid, createNewAttribute);
	attribute->setValue(length, data);

	DEBUG_END_FUNCTION;
}

bool ConnectionData::getAttributeValue(uint16 uuid, uint8& value)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* attribute = getAttribute(uuid, false);
	if(attribute == NULL)
	{
		DEBUG_RETURN false;
	}
	if((attribute->data = NULL) || (attribute->dataLength < 1))
	{
		DEBUG_RETURN false;
	}

	value = attribute->data[0];
	DEBUG_RETURN true;
}

bool ConnectionData::getAttributeValue(uint16 uuid, uint16& value)
{
	DEBUG_ENTER_FUNCTION;

	ConnectionAttributes* attribute = getAttribute(uuid, false);
	if(attribute == NULL)
	{
		DEBUG_RETURN false;
	}
	if((attribute->data = NULL) || (attribute->dataLength < 2))
	{
		DEBUG_RETURN false;
	}

	value = ((uint16*)attribute->data)[0];
	DEBUG_RETURN true;
}


bool ConnectionData::beginAttributeScan()
{
	DEBUG_ENTER_FUNCTION;

	_attributeScanCompleted = false;
	DEBUG_RETURN true;
}

void ConnectionData::finishServiceScan()
{
	DEBUG_ENTER_FUNCTION;

	_serviceScanCompleted = true;

	DEBUG_END_FUNCTION;
}

void ConnectionData::finishAttributeScan()
{
	DEBUG_ENTER_FUNCTION;

	_attributeScanCompleted = true;

	DEBUG_END_FUNCTION;
}

bool ConnectionData::attributeScanCompleted() const
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN _attributeScanCompleted;
}

bool ConnectionData::serviceScanCompleted()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN _serviceScanCompleted;
}

void ConnectionData::closeConnection(ConnectionData*& connection)
{
	DEBUG_ENTER_FUNCTION;

	rootNode = (ConnectionData*)BaseLinkedList::removeNode(rootNode, connection);
	connection = NULL;

	DEBUG_END_FUNCTION;
}

int ConnectionData::getAverageRSSI()
{
	DEBUG_ENTER_FUNCTION;

	if(_currentRSSICount == 0)
	{
		DEBUG_RETURN 0;
	}

	DEBUG_RETURN (_totalRSSI / _currentRSSICount);
}

void ConnectionData::addPoll(int pollID, int rssi)
{
	DEBUG_ENTER_FUNCTION;

	if(_currentRSSIPollID != pollID)
	{
		_currentRSSIPollID = pollID;
		_totalRSSI = 0;
		_currentRSSICount = 0;
	}

	if(rssi <= -103) //-103 is the RSSI given when 
	{
		DEBUG_END_FUNCTION;
	}
	_totalRSSI += rssi;
	_currentRSSICount++;

	DEBUG_END_FUNCTION;
}

void ConnectionData::SetNextExtendedWrite(uint16 attributeID, uint16 dataLength, const void* data)
{
	DEBUG_ENTER_FUNCTION;

	_extendedWriteData = malloc(dataLength);
	memcpy(_extendedWriteData, data, dataLength);
	_extendedWriteAttributeID = attributeID;
	_extendedWriteCurrentOffset = 0;
	_extendedWriteDataLength = dataLength;

	DEBUG_END_FUNCTION;
}

void ConnectionData::AppendToExtendedWrite(MessageBuffer* buffer)
{
	DEBUG_ENTER_FUNCTION;

	this->_extendedWriteBuffer->append(buffer);

	DEBUG_END_FUNCTION;
}

bool ConnectionData::GetNextExtendedWrite(uint16& attributeID, uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft)
{
	DEBUG_ENTER_FUNCTION;

	if(_extendedWriteBuffer->hasDataWaiting())
	{
		bool lastWrite = _extendedWriteBuffer->getWaitingData(attributeID, offset, dataLength, data, totalSizeLeft);

	}
	else
	{
		if(_extendedWriteData == NULL)
		{
			DEBUG_RETURN false;
		}
		bool lastWrite = true;
		attributeID = _extendedWriteAttributeID;
		int realDataLength = (_extendedWriteDataLength - _extendedWriteCurrentOffset);
		if(realDataLength > dataLength)
		{
			lastWrite = false;
			realDataLength = dataLength;
		}

		dataLength = realDataLength;
		
		offset = _extendedWriteCurrentOffset;
		memcpy(data, &(((uint8*)_extendedWriteData)[_extendedWriteCurrentOffset]), dataLength);
		_extendedWriteCurrentOffset += dataLength;

		if(lastWrite)
		{
			free(_extendedWriteData);
			_extendedWriteData = NULL;
			_extendedWriteAttributeID = _extendedWriteDataLength = _extendedWriteCurrentOffset = 0;
		}
	}

	DEBUG_RETURN true;
}

ConnectionServices::ConnectionServices(uint16 ID)
{
	DEBUG_ENTER_FUNCTION;

	this->ID = ID;
	this->start = 0;
	this->end = 0;
	this->nextNode = NULL;

	this->longID = nullptr;
	this->longIDLen = 0;
	DEBUG_END_FUNCTION;
}

ConnectionServices::ConnectionServices(const uint8* longID, uint8 longIDLen)
{
	DEBUG_ENTER_FUNCTION;

	this->ID = 0x0000;
	this->start = 0;
	this->end = 0;
	this->nextNode = nullptr;

	this->longID = new uint8[longIDLen];
	memcpy(this->longID, longID, longIDLen);
	this->longIDLen = longIDLen;

	DEBUG_END_FUNCTION;
}

BaseLinkedList* ConnectionServices::createNewNode(void *initializeData)
{
	DEBUG_ENTER_FUNCTION;

	this->nextNode = new ConnectionServices(*((uint16*)initializeData));
	DEBUG_RETURN this->nextNode;
}

ConnectionAttributes::ConnectionAttributes(uint16 ID)
{
	DEBUG_ENTER_FUNCTION;

	this->ID = ID;
	this->characteristicsHandle = 0;
	this->dataLength = 0;
	this->data = NULL;
	this->nextNode = NULL;
	this->reading = false;
	this->longread = false;
	this->attributeIsGUID = false;
	this->serviceIsGUID = false;

	DEBUG_END_FUNCTION;
}

ConnectionAttributes* ConnectionAttributes::getNextNode()
{
	return (ConnectionAttributes*)nextNode;
}

void ConnectionAttributes::setValue(uint8 length, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	if((this->data == NULL) || (this->dataLength != length))
	{
		if(this->data != NULL)
		{
			free(this->data);
		}

		this->data = (uint8*)malloc(length);
	}

	memcpy(this->data, data, length);

	DEBUG_END_FUNCTION;
}

BaseLinkedList* ConnectionAttributes::createNewNode(void* initializeData)
{
	DEBUG_ENTER_FUNCTION;

	this->nextNode = new ConnectionAttributes(*((uint16*)initializeData));
	DEBUG_RETURN this->nextNode;
}


volatile int currentPollID = 0;

FilteredSearch::FilteredSearch(int filterCount, DeviceData* filters, MiTokenBLE* parent)
{
	DEBUG_ENTER_FUNCTION;

	init(filterCount, filters, 0, NULL, parent);

	DEBUG_END_FUNCTION;
}

FilteredSearch::FilteredSearch(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray, MiTokenBLE* parent)
{
	DEBUG_ENTER_FUNCTION;

	init(filterCount, filters, addressCount, addressArray, parent);

	DEBUG_END_FUNCTION;
}

FilteredSearch::FilteredSearch(uint8* byteStream, int length, MiTokenBLE* parent)
{
	DEBUG_ENTER_FUNCTION;

	//Byte stream format is
	/*
		//Filter version FBS[00])

		Version [4 bytes] {FBS[00]}
		Filter Count [4 bytes]
		Address Count [4 bytes]
		{ [[foreach filter]]
			flag [1 byte]
			length [1 byte]
			data [length]
		}
		{ [[foreach address]]
			address [6 bytes]
		}
	*/
	uint8 version[] = {'F', 'B', 'S', 0x00};
	uint8 rversion[4];

	int readOffset = 0;

	ReadFromMovingPtr(byteStream, readOffset, &(rversion[0]), 4);
	if(memcmp(rversion, version, 4) != 0)
	{
		//There is an error - make a blank filter
		init(0, NULL,  0, NULL, parent);
		DEBUG_END_FUNCTION;
		return;
	}

	ReadFromMovingPtr(byteStream, readOffset, &_filterCount, 4);
	ReadFromMovingPtr(byteStream, readOffset, &_filterAddressCount, 4);

	_filters = new DeviceData[_filterCount];
	for(int fc = 0 ; fc < _filterCount ; ++fc)
	{
		ReadFromMovingPtr(byteStream, readOffset, &(_filters[fc].flag), 1);
		ReadFromMovingPtr(byteStream, readOffset, &(_filters[fc].length), 1);
		_filters[fc].data = new uint8[_filters[fc].length];
		ReadFromMovingPtr(byteStream, readOffset, _filters[fc].data, _filters[fc].length);
	}

	_filterAddressArray = new uint8[6 * _filterAddressCount];
	ReadFromMovingPtr(byteStream, readOffset, _filterAddressArray, (6 * _filterAddressCount));

	_filterAddresses = (_filterAddressCount > 0);

	assert(readOffset == length);
	
	_currentNode = *parent->GetPRootPoll();

	DEBUG_END_FUNCTION;
}

BLE_API_RET FilteredSearch::convertToByteStream(uint8* byteStream, int& length)
{
	DEBUG_ENTER_FUNCTION;

	//Byte stream format is
	/*
		//Filter version FBS[00])

		Version [4 bytes] {FBS[00]}
		Filter Count [4 bytes]
		Address Count [4 bytes]
		{ [[foreach filter]]
			flag [1 byte]
			length [1 byte]
			data [length]
		}
		{ [[foreach address]]
			address [6 bytes]
		}
	*/

	int expectedLength = 12; //Version + FilterCount + Address
	for(int fc = 0 ; fc < _filterCount ; ++fc)
	{
		expectedLength += _filters[fc].length + 2; //flag + length + [length]
	}

	expectedLength += _filterAddressCount * 6; //6 bytes per address

	if((byteStream == NULL) || (length < expectedLength))
	{
		length = expectedLength;
		DEBUG_RETURN BLE_API_MORE_DATA;
	}

	//We have enough room, so copy it all over
	length = expectedLength;
	uint8 version[] = {'F', 'B', 'S', 0x00};

	int writeLoc = 0;
	WriteToMovingPtr(byteStream, writeLoc, version, 4);
	WriteToMovingPtr(byteStream, writeLoc, &_filterCount, 4);
	WriteToMovingPtr(byteStream, writeLoc, &_filterAddressCount, 4);

	for(int fc = 0 ; fc < _filterCount ; ++fc)
	{
		WriteToMovingPtr(byteStream, writeLoc, &_filters[fc].flag, 1);
		WriteToMovingPtr(byteStream, writeLoc, &_filters[fc].length, 1);
		WriteToMovingPtr(byteStream, writeLoc, _filters[fc].data, _filters[fc].length);
	}

	WriteToMovingPtr(byteStream, writeLoc, _filterAddressArray, (6 * _filterAddressCount));

	assert(writeLoc == expectedLength);
	
	DEBUG_RETURN BLE_API_SUCCESS;
}

FilteredSearch::~FilteredSearch()
{
	DEBUG_ENTER_FUNCTION;

	for(int i = 0 ; i < _filterCount ; ++i)
	{
		delete [] _filters[i].data;
	}

	delete [] _filters;

	if(_filterAddressArray != NULL)
	{
		delete[] _filterAddressArray;
	}

	DEBUG_END_FUNCTION;
}

void FilteredSearch::init(int filterCount, DeviceData* filters, int addressCount, uint8* addressArray, MiTokenBLE* parent)
{
	DEBUG_ENTER_FUNCTION;

	_filterCount = filterCount;
	_filters = new DeviceData[filterCount];
	_filterAddresses = ((addressCount > 0) && (addressArray != NULL));
	_filterAddressArray = (_filterAddresses ? new uint8[addressCount * 6] : NULL);

	for(int i = 0 ; i < _filterCount ; ++i)
	{
		_filters[i].flag = filters[i].flag;
		_filters[i].length = filters[i].length;
		_filters[i].data = new unsigned char [_filters[i].length];
		memcpy(_filters[i].data, filters[i].data, _filters[i].length);
	}

	if(_filterAddresses)
	{
		_filterAddressCount = addressCount;
		_filterAddressArray = new uint8[addressCount * 6];
		memcpy(_filterAddressArray, addressArray, (addressCount * 6));
	}
	else
	{
		_filterAddressCount = 0;
		_filterAddressArray = NULL;
	}

	_currentNode = *parent->GetPRootPoll();

	DEBUG_END_FUNCTION;
}


PollData* FilteredSearch::findNextNode()
{
	DEBUG_ENTER_FUNCTION;

	PollData* ret = NULL;
	while((_currentNode != NULL) && (ret == NULL))
	{
		if(_currentNodeValid())
		{
			ret = _currentNode;
		}

		_currentNode = (PollData*)_currentNode->nextNode;
	}

	DEBUG_RETURN ret;
}

void FilteredSearch::restart(PollData* rootNode)
{
	DEBUG_ENTER_FUNCTION;

	if (rootNode != nullptr)
	{
		_currentNode = rootNode;
	}
	else
	{
		_currentNode = PollData::rootNode;
	}
	DEBUG_END_FUNCTION;
}

bool FilteredSearch::_currentNodeValid()
{
	DEBUG_ENTER_FUNCTION;

	if(_currentNode == NULL)
	{
		DEBUG_RETURN false;
	}

	if(_filterAddresses)
	{
		bool inarray = false;
		for(int i = 0 ;  i < _filterAddressCount ; ++i)
		{
			if(memcmp(_currentNode->address, &(_filterAddressArray[i * 6]), 6) == 0)
			{
				inarray = true;
				break;
			}
		}

		if(!inarray)
		{
			DEBUG_RETURN false;
		}
	}

	for(int i = 0 ; i < _filterCount ; ++i)
	{
		if(_currentNode->checkFilter(&(_filters[i])) == false)
		{
			//only allow if they pass all the filters
			DEBUG_RETURN false;
		}
	}

	//it passed all the filters so it is valid
	DEBUG_RETURN true;
}

MessageBuffer::MessageBuffer()
{
	DEBUG_ENTER_FUNCTION;

	rootNode = lastNode = NULL;
	currentNodeOffset = 0;

	DEBUG_END_FUNCTION;
}

MessageBuffer::~MessageBuffer()
{
	DEBUG_ENTER_FUNCTION;

	Nodes* currentNode = rootNode;
	Nodes* nextNode;
	while(currentNode != NULL)
	{
		nextNode = (Nodes*)currentNode->nextNode;
		if(currentNode->data)
		{
			free(currentNode->data);
		}

		currentNode = nextNode;
	}

	DEBUG_END_FUNCTION;
}

void MessageBuffer::addMessage(uint16 handle, uint16 dataLength, const uint8* data)
{
	DEBUG_ENTER_FUNCTION;

	if(rootNode == NULL)
	{
		rootNode = lastNode = new Nodes();

	}
	else
	{
		lastNode->nextNode = new Nodes();
		lastNode = (Nodes*)lastNode->nextNode;
	}
	
	lastNode->handle = handle;
	lastNode->dataLength = dataLength;
	lastNode->data = (uint8*)malloc(dataLength);
	memcpy(lastNode->data, data, dataLength);
	lastNode->nextNode = NULL;

	DEBUG_END_FUNCTION;
}

MessageBuffer::Nodes::Nodes()
{
	DEBUG_ENTER_FUNCTION;

	this->handle = 0;
	this->dataLength = 0;
	this->data = NULL;
	this->nextNode = NULL;

	DEBUG_END_FUNCTION;
}

MessageBuffer::Nodes::Nodes(MessageBuffer::Nodes* baseNode)
{
	DEBUG_ENTER_FUNCTION;

	this->handle = baseNode->handle;
	this->dataLength = baseNode->dataLength;
	this->data = (uint8*)malloc(baseNode->dataLength);
	memcpy(this->data, baseNode->data, baseNode->dataLength);
	this->nextNode = NULL;

	DEBUG_END_FUNCTION;
}

MessageBuffer::Nodes::~Nodes()
{
	DEBUG_ENTER_FUNCTION;

	free(this->data);

	DEBUG_END_FUNCTION;
}

BaseLinkedList* MessageBuffer::Nodes::createNewNode(void* data)
{
	DEBUG_ENTER_FUNCTION;

	Nodes* ret ;
	if(data == NULL)
	{
		ret = new Nodes();
	}
	else
	{
		ret = new Nodes((Nodes*)data);
	}

	DEBUG_RETURN ret;
}

void MessageBuffer::append(MessageBuffer* bufferToAppend)
{
	DEBUG_ENTER_FUNCTION;

	Nodes* startNode = bufferToAppend->rootNode;
	while(startNode != NULL)
	{
		if(lastNode == NULL)
		{
			lastNode = rootNode = new Nodes(startNode);
		}
		else
		{
			lastNode = (Nodes*)(lastNode->nextNode = (Nodes*)lastNode->createNewNode(startNode));
		}
		startNode = (Nodes*)startNode->nextNode;	
	}

	DEBUG_END_FUNCTION;
}

bool MessageBuffer::hasDataWaiting()
{
	DEBUG_ENTER_FUNCTION;

	if(rootNode == NULL)
	{
		DEBUG_RETURN false;
	}

	DEBUG_RETURN true;
}

bool MessageBuffer::getWaitingData(uint16& attributeID, uint16& offset, uint8& dataLength, void* data, uint16& totalSizeLeft)
{
	DEBUG_ENTER_FUNCTION;

	int realSizeLeft = rootNode->dataLength - this->currentNodeOffset;
	totalSizeLeft = realSizeLeft;

	bool lastWrite = true;
	if(realSizeLeft > dataLength)
	{
		lastWrite = false;
		realSizeLeft = dataLength;
	}

	dataLength = realSizeLeft;
	offset = currentNodeOffset;
	attributeID = rootNode->handle;
	memcpy(data, rootNode->data + this->currentNodeOffset, dataLength);
	if(lastWrite)
	{
		Nodes* pnode = rootNode;
		rootNode = (Nodes*)rootNode->nextNode;
		delete pnode;
		currentNodeOffset = 0;
		if(rootNode == NULL)
		{
			lastNode = NULL;
		}
	}
	else
	{
		currentNodeOffset += dataLength;
	}

	DEBUG_RETURN hasDataWaiting();

}

int MessageBuffer::getWaitingDataSize()
{
	if(rootNode == nullptr)
	{
		return 0;
	}
	int realSizeLeft = rootNode->dataLength - this->currentNodeOffset;
	return realSizeLeft;
}