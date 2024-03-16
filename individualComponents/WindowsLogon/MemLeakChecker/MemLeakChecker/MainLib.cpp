#include "MainLib.h"
#include <Windows.h>
#include <crtdbg.h>

#include "NamedPipeClient.h"

#define LOCATION_ID_NOT_SET 0xFFFFFFFF
#define LOCATION_ID_UNKNOWN 0xFFFFFFFE
#define LOCATION_ID_MEMMGMT 0xFFFFFFFD

int curLocationID = LOCATION_ID_UNKNOWN;

//unfinished definition
#define nNoMansLandSize 4
typedef struct _CrtMemBlockHeader
{
struct _CrtMemBlockHeader * pBlockHeaderNext;
struct _CrtMemBlockHeader * pBlockHeaderPrev;
char *                      szFileName;
int                         nLine;
size_t                      nDataSize;
int                         nBlockUse;
long                        lRequest;
unsigned char               gap[nNoMansLandSize];
} _CrtMemBlockHeader;

#if 0
typedef struct _MemBlock
{
	void* location;
	size_t allocSize;
	int blockUse;
	char filename[100];
	int fileline;
	bool inUse;
	bool refreshed;
} _memBlock;

typedef struct _MemBlockPack
{
	_memBlock memBlocks[128];
	struct _MemBlockPack * nextPack;
} _memBlockPack;

typedef struct _MemBlockFrom
{
	//Location ID
	int who;
	_memBlockPack firstPack;
} _memBlockFrom;

typedef struct _MemBlockCtrl
{
	_MemBlockFrom linkedBlock;
	_MemBlockCtrl* nextCtrlBlock;
} _memBlockCtrl;

_memBlockCtrl mainCtrl;

bool initMainCtrl()
{
	mainCtrl.linkedBlock.who = LOCATION_ID_MEMMGMT;
	mainCtrl.nextCtrlBlock = NULL;
	memset(&mainCtrl.linkedBlock.firstPack, 0, sizeof(_memBlockPack));
}

bool mainCtrlInit = initMainCtrl();

void startRefresh()
{
	_memBlockCtrl* pBlockCtrl = &mainCtrl;
	_memBlockPack* pPack;
	while(pBlockCtrl)
	{
		pPack = &(pBlockCtrl->linkedBlock.firstPack);
		while(pPack)
		{
			for(int i = 0 ; i < 128 ; ++i)
			{
				pPack->memBlocks[i].refreshed = false;
			}
			pPack = pPack->nextPack;
		}
		pBlockCtrl = pBlockCtrl->nextCtrlBlock;
	}

	return;
}

void endRefresh()
{
	_memBlockCtrl* pBlockCtrl = &mainCtrl;
	_memBlockPack* pPack;
	while(pBlockCtrl)
	{
		pPack = &(pBlockCtrl->linkedBlock.firstPack);
		while(pPack)
		{
			for(int i = 0 ; i < 128 ; ++i)
			{
				if((pPack->memBlocks[i].inUse) && (!pPack->memBlocks[i].refreshed))
				{
					pPack->memBlocks[i].inUse = false;
				}
			}
			pPack = pPack->nextPack;
		}
		pBlockCtrl = pBlockCtrl->nextCtrlBlock;
	}

	return;
}

_memBlockPack* getContainedBlockPack(void* memLocation)
{
	_memBlockCtrl* pCtrl = &mainCtrl;
	while(pCtrl)
	{
		if(pCtrl->linkedBlock.who != LOCATION_ID_NOT_SET)
		{
			_memBlockPack* pPack = &pCtrl->linkedBlock.firstPack;
			while(pPack)
			{
				for(int i = 0 ; i < 128 ; ++i)
				{
					if(pPack->memBlocks[i].inUse)
					{
						//the block is in use
						if(pPack->memBlocks[i].location == memLocation)
						{
							pPack->memBlocks[i].refreshed = true;
							return pPack;
						}
					}
				}
				pPack = pPack->nextPack;
			}
		}
	}
	return NULL;
}

_memBlockCtrl* getCtrlBlock(int currentLocationID)
{
	_memBlockCtrl* pCtrl = &mainCtrl;
	while(pCtrl)
	{
		if(pCtrl->linkedBlock.who == currentLocationID)
			return pCtrl;
		pCtrl = pCtrl->nextCtrlBlock;
	}

	return NULL;
}

void AddItemToBlock(_memBlockCtrl* block, void* memLocation)
{
	_memBlockPack *pPack;
	_memBlockPack *pLastBlock;

	pPack = &(block->linkedBlock.firstPack);
	pLastBlock = pPack;
	while(pPack)
	{
		for(int i = 0 ; i < 128 ; ++i)
		{
			if(!pPack->memBlocks[i].inUse)
			{
				pPack->memBlocks[i].inUse = true;
				pPack->memBlocks[i].location = memLocation;
				return;
			}
		}
		pLastBlock = pPack;
		pPack = pPack->nextPack;
	}

	//we need to allocate a new pack block
	pPack = (_memBlockPack*)calloc(1, sizeof(_memBlockPack));
	//note : we will need to add this pack to the LOCATION_ID_MEMMGMT block
	pLastBlock->nextPack = pPack;
	pPack->memBlocks[0].location = memLocation;
	pPack->memBlocks[0].inUse = true;

	//Add the section to the LOCATION_ID_MEMMGMT block
	void* pAdd = (void*)(((int)pPack) - sizeof(_CrtMemBlockHeader));
	_memBlockCtrl* pMGMTBlock = getCtrlBlock(LOCATION_ID_MEMMGMT);
	checkAndAddBlock(pAdd, pMGMTBlock);

	return;
}

void checkAndAddBlock(void* memLocation, _memBlockCtrl* curBlock)
{
	//check if it doesn't already exist
	if(getContainedBlockPack(memLocation) == NULL)
	{
		//add the item to the current block
		AddItemToBlock(curBlock, memLocation);
	}
}



#endif


//We will send this data via Named Pipes to a C# App that will actually handle the data for us

char sendBuffer[8192]; //8k NamedPipe send buffer
int sendBufferPos;
void GetFirstHeader();

NamedPipeClient NPC;
#ifdef UNICODE
void MemLeak_Init(wchar_t* pipeName)
#else
void MemLeak_Init(char* pipeName)
#endif

{
	NPC.Connect(pipeName, NULL);
	sendBufferPos = 0;
	GetFirstHeader();
}

void writeToSendBuffer(void* data, int length)
{
	//if we are trying to append more then 8k bytes, break it into 2 seperate requests, one 8K large, the other the rest of the size
	if(length > 8192)
	{
		writeToSendBuffer(data, 8192);
		data = (void*)((int)data + 8192);
		writeToSendBuffer(data, length - 8192);
		return;
	}

	if(sendBufferPos + length > 8192)
	{
		NPC.SendBytes((unsigned char*)sendBuffer, sendBufferPos);
		sendBufferPos = 0;
	}

	memcpy(&sendBuffer[sendBufferPos], data, length);
	sendBufferPos += length;	
}

void sendOffBuffer()
{
	if(sendBufferPos)
	{
		NPC.SendBytes((unsigned char*)sendBuffer, sendBufferPos);
		sendBufferPos = 0;
	}
}

void startMemUpdate()
{
	unsigned char header[] = {'U', 'P', 'M', 'D'};
	writeToSendBuffer((void*)header, sizeof(header));
}

void endMemUpdate()
{
	unsigned char header[] = {'E', 'U', 'P', 'D'};
	writeToSendBuffer((void*)header, sizeof(header));
}

void setLocationID(int LID)
{
	unsigned char updateLID[8] = {'U', 'L', 'I', 'D', 0};
	memcpy(&(updateLID[4]), &LID, 4);
	writeToSendBuffer((void*)updateLID, sizeof(updateLID));
}

typedef struct _sendMemBuffer
{
	unsigned char header[4];
	int sectionSize;
	int ptrSize;
	void* location;
	int allocSize;
	int fileLine;
	int blockUseID;
	char fileName[100];
} sendMemBuffer;

void sendMemSection(_CrtMemBlockHeader* pBlock)
{
	sendMemBuffer buffer = {0};
	buffer.header[0] = 'M'; buffer.header[1] = 'E'; buffer.header[2] = 'M'; buffer.header[3] = 'S';
	buffer.sectionSize = sizeof(sendMemBuffer);
	buffer.ptrSize = sizeof(void*);
	buffer.location = pBlock;
	buffer.allocSize = pBlock->nDataSize;
	buffer.fileLine = pBlock->nLine;
	buffer.blockUseID = pBlock->nBlockUse;
	int slen = 0;
	if(pBlock->szFileName)
	{
		slen = strlen(pBlock->szFileName);
	}
	if(slen >= sizeof(buffer.fileName))
		slen = sizeof(buffer.fileName);
	memcpy(&(buffer.fileName[0]), pBlock->szFileName, slen);
	writeToSendBuffer((void*)&buffer, sizeof(buffer));
}

//Commit the memory changes, then set the new location ID
void MemLeak_SetLocationID(int locationID)
{
	MemLeak_CheckMemChanges();
	//curLocationID = locationID;
	setLocationID(locationID);
}

_CrtMemState * oldState = NULL;
_CrtMemBlockHeader* memStart = NULL;


void GetFirstHeader()
{
	//Improved version that uses the _CrtMemState to get the first block
	_CrtMemState curState = {0};
	_CrtMemCheckpoint(&curState);
	memStart = curState.pBlockHeader;

	/*
	//allocate some data so we can hook into the MemBlock
	void* vpData = malloc(1);
	memStart = reinterpret_cast<_CrtMemBlockHeader*>( vpData );
	--memStart;
	bool moreFound = false;
	while(memStart->pBlockHeaderNext != NULL)
	{
		moreFound = true;
		memStart = memStart->pBlockHeaderNext;
	}
	if(!moreFound)
	{
		//we need to keep vpData around for memStart - so leak it)
	}
	else
	{
		//free the allocated data
		free(vpData);
	}
	*/
}
void AddChanges()
{
	startMemUpdate();
	GetFirstHeader();
	_CrtMemBlockHeader * curCrtBlock = memStart;
	while(curCrtBlock)
	{
		sendMemSection(curCrtBlock);
		curCrtBlock = curCrtBlock->pBlockHeaderNext;
	}

	endMemUpdate();

	sendOffBuffer();

}


void MemLeak_CheckMemChanges(void)
{
	/*
	if(memStart == NULL)
		GetFirstHeader();

	_CrtMemState * memState;
	_CrtMemState * diff;
	_CrtMemCheckpoint(memState);

	if(oldState)
	{
		_CrtMemDifference(diff, oldState, memState);
	}
	else
	{
		diff = memState;
	}

	AddChanges(diff);
	*/

	AddChanges();


}

void* Mem_GetTable(void)
{
	return NULL;
}