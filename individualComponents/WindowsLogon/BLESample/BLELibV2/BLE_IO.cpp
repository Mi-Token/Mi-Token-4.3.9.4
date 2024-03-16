#include "BLE_IO.h"
#include "BLE_LowLevel.h"
#include <Windows.h>
#include "apitypes.h"
#include <stdio.h>
#include "cmd_def.h"
#include "BLE_NamedPipe.h"

#include "BLE_Structs.h"
#include "BLE_ASYNC.h"

#include "DebugLogging.h"
#include "CurrentState.h"
#include "ScopedLock.h"

#include "MiTokenBLE.h"

#if USE_MEM_CHECK
#include "MainLib.h"
#endif

#ifndef VALID_HANDLE
#define VALID_HANDLE(p) ((p != NULL) && (p != INVALID_HANDLE_VALUE))
#endif


IBLE_IO::~IBLE_IO()
{
	DEBUG_ENTER_FUNCTION;
	DEBUG_END_FUNCTION;
}

IBLE_IO* IO_Interface = nullptr; // new Basic_BLE_IO();

#ifdef UNIT_TEST
void Set_IO_Interface(IBLE_IO* newInterface)
{
	delete IO_Interface;
	if(newInterface == NULL)
	{
		IO_Interface = new Basic_BLE_IO();
	}
	else
	{
		IO_Interface = newInterface;
	}
}
#endif

#ifdef _DEBUG
void (__stdcall *debugDataOut) (int amount);
void (__stdcall *debugDataIn) (int amount);
#endif


bool fallbackToCOM = false;

void genericServerGotData(NamedPipe* sender, unsigned char* data, unsigned int length)
{
	DEBUG_ENTER_FUNCTION;
	sender->getRoot().GetRoot()->GetIO()->serverGotData(data, length);
	DEBUG_END_FUNCTION;
}

//Use DEBUG_IO to printf a bunch of debug info regarding the Serial ports IO.
//#define DEBUG_IO

//NamedPipe* pipe = NULL;

#define PIPE_IS_SERVER ((pipe != NULL) && (pipe->isServer()))
#define PIPE_IS_CLIENT ((pipe != NULL) && (!pipe->isServer()))

#define ClientPipe (std::dynamic_pointer_cast<NamedPipeClient>(pipe))
#define ServerPipe (std::dynamic_pointer_cast<NamedPipeServer>(pipe))

//Serial port's handle
//HANDLE serial_handle;

//The function that is called to write data to the serial_port
void genericOutput(uint8 len1, uint8* data1, uint16 len2, uint8* data2)
{
	DEBUG_ENTER_FUNCTION;
	IO_Interface->output(len1, data1, len2, data2);
	DEBUG_END_FUNCTION;
}

//ASync IO OVERLAPPED structures
OVERLAPPED ReadOverlap = {0}, WriteOverlap = {0};

/*
Basic_BLE_IO::Basic_BLE_IO()
{
	DEBUG_ENTER_FUNCTION;
	fileLock = CreateMutex(NULL, FALSE, NULL);
	fallbackToCOM = false;
	pipe = NULL;
	serial_handle = NULL;
	memset(&ReadOverlap, 0, sizeof(ReadOverlap));
	memset(&WriteOverlap, 0, sizeof(WriteOverlap));
	async = new BLE_ASYNC(this, nullptr);
	DEBUG_END_FUNCTION;
}
/**/

#ifndef MiTokenBLE_ClasslessMode
Basic_BLE_IO::Basic_BLE_IO(MiTokenBLE* parent) : _root(parent)
{
	DEBUG_ENTER_FUNCTION;

	fileLock = CreateMutex(NULL, FALSE, NULL);
	fallbackToCOM = false;
	pipe = NULL;
	serial_handle = NULL;
	memset(&ReadOverlap, 0, sizeof(ReadOverlap));
	memset(&WriteOverlap, 0, sizeof(WriteOverlap));
	async = new BLE_ASYNC(this, parent);
	DEBUG_END_FUNCTION;
}

#endif
Basic_BLE_IO::~Basic_BLE_IO()
{
	DEBUG_ENTER_FUNCTION;
	CloseNamedPipe();
	
	CloseSerialHandle();
	delete async;

	DEBUG_END_FUNCTION;
}

//Opens the COM Port supplied in COMPort as an OVERLAPPED file. Also initializes the OVERLAPPED structure's completed event handlers.
void Basic_BLE_IO::openCOM(const char* COMPort)
{
	DEBUG_ENTER_FUNCTION;
#if USE_MEM_CHECK
	
	ID(110);
#endif

	if(ReadOverlap.hEvent == 0)
	{
		ReadOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	}
	if(WriteOverlap.hEvent == 0)
	{
		WriteOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(111);
#endif

	serial_handle = INVALID_HANDLE_VALUE;
	for(int i = 0 ; i < 10 && (serial_handle == INVALID_HANDLE_VALUE) ; ++i)
	{
		SetLastError(0);
		serial_handle = CreateFileA(COMPort,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);

		if(serial_handle == INVALID_HANDLE_VALUE)
		{
			printf("Error : Could not open COM Port %s. GLE = %d\r\n", COMPort, GetLastError());
		}
	}

	DEBUG_END_FUNCTION;
}

void Basic_BLE_IO::closeCOM()
{
	DEBUG_ENTER_FUNCTION;

	if(VALID_HANDLE(serial_handle))
	{
		CloseHandle(serial_handle);
		serial_handle = INVALID_HANDLE_VALUE;
	}

	DEBUG_END_FUNCTION;
}

void DebugDataInstream(int amount)
{
	DEBUG_ENTER_FUNCTION;

#ifdef _DEBUG
	if(debugDataIn != NULL)
	{
		debugDataIn(amount);
	}
#endif
	DEBUG_END_FUNCTION;
}

void DebugDataOutstream(int amount)
{
	DEBUG_ENTER_FUNCTION;

#ifdef _DEBUG
	if(debugDataOut != NULL)
	{
		debugDataOut(amount);
	}
#endif

	DEBUG_END_FUNCTION;
}

//Set the write function for the BLE API to the output function. Also open the COM Port
BLE_API_RET Basic_BLE_IO::OpenSerialHandle(const char* COMPort)
{
	DEBUG_ENTER_FUNCTION;

	BLE_LL_SetWriteFunction(&genericOutput);
	
	openCOM(COMPort);
	if(serial_handle == INVALID_HANDLE_VALUE)
	{
		DEBUG_RETURN BLE_API_ERR_FAILED_TO_OPEN_COM_PORT;
	}

	/*
	//reset the COM Port
	BLE_LL_ResetSystem();
	CloseHandle(serial_handle);

	openCOM(COMPort);

	if(serial_handle == INVALID_HANDLE_VALUE)
	{
		int x = GetLastError();
		DEBUG_RETURN -1;
	}
	*/
	
	if(PIPE_IS_CLIENT)
	{
		//we are connecting to the COM despite being a client... we mustn't be able to reach the server and are instead falling back to COM
		ClientPipe->clientBoundToCOM = true;
		fallbackToCOM = true;
	}

	DEBUG_RETURN BLE_API_SUCCESS;
}

int Basic_BLE_IO::InitializeNamedPipe(const char* pipeName, bool server, void (*pipeConnectedCallback) (IMiTokenBLE* instance))
{
	DEBUG_ENTER_FUNCTION;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(112);
#endif

	//make sure the WriteFunction is set
	BLE_LL_SetWriteFunction(&genericOutput);

	if(ReadOverlap.hEvent == 0)
	{
		ReadOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	}
	if(WriteOverlap.hEvent == 0)
	{
		WriteOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(113);
#endif
	if(server)
	{
		pipe = std::dynamic_pointer_cast<NamedPipe>(std::make_shared<NamedPipeServer>(pipeName, &genericServerGotData, _root));
		ServerPipe->StartListener(ServerPipe);
	}
	else
	{
		pipe = std::dynamic_pointer_cast<NamedPipe>(std::make_shared<NamedPipeClient>(pipeName, _root, pipeConnectedCallback));
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(114);
#endif
	DEBUG_RETURN 0;
}

int Basic_BLE_IO::CloseSerialHandle()
{
	DEBUG_ENTER_FUNCTION;

	closeCOM();
	DEBUG_RETURN 0;
}

int Basic_BLE_IO::CloseNamedPipe()
{
	DEBUG_ENTER_FUNCTION;

	pipe = nullptr;
	/*
	NamedPipe* tempPipe = pipe;
	pipe = nullptr;
	

	if(tempPipe != NULL)
	{
		tempPipe->close();
	}
	delete tempPipe;
	*/
	DEBUG_RETURN 0;
}

//Synchronous wrapping of an ASynchronous read.
//Note : DO NOT use Synchronous reading, this will make the COM Port break randomly - hence why we use ASync reads and waiting for them to complete.
int Basic_BLE_IO::readData(unsigned char* buffer, DWORD size, DWORD* read)
{
	DEBUG_ENTER_FUNCTION;

	std::shared_ptr<NamedPipeClient> client = NULL;
	if(pipe != NULL)
	{
		if(PIPE_IS_CLIENT)
		{
			if(!fallbackToCOM)
			{
				client = ClientPipe;
			}
		}
	}

#ifdef DEBUG_IO
	printf("IRD\r\n");
#endif
	bool needMoreData = true;
	int ret;
	if(client != NULL)
	{
		client->readNextBlock(buffer, size, read, ReadOverlap.hEvent);
	}
	else
	{
		if(VALID_HANDLE(serial_handle))
		{
			DEBUG_FUNCTION_SECTION;
			ret = ReadFile(serial_handle, buffer, size, read, &ReadOverlap);
		}
		else
		{
			DEBUG_FUNCTION_SECTION;
			DEBUG_RETURN -1;
		}
	}
	
	DWORD bytesTransfered;
		
#ifdef DEBUG_IO
	printf("%08lX\r\n", ret);
	if(ret != 1)
	{
		printf("\tGLE : %d\r\n", GetLastError());
	}
	printf("\tRFFS\r\n");

	
#endif

	DEBUG_FUNCTION_SECTION;
	while(needMoreData && (client == NULL))
	{
		
#ifdef DEBUG_IO
		printf("\tNMD\r\n");
#endif
		DEBUG_FUNCTION_SECTION;
		if(_root.GetRoot()->GetState()->waitForOverlappedIO(serial_handle, &ReadOverlap, bytesTransfered) == FALSE)
		{
			DEBUG_RETURN -1;
		}
		else
		{
			needMoreData = false;
		}


		/*
		if(GetOverlappedResult(serial_handle, &ReadOverlap, &bytesTransfered, TRUE) == false)
		{
			DEBUG_FUNCTION_SECTION;
#ifdef DEBUG_IO
			printf("\t\tORF\r\n");
#endif
			if(GetLastError() != ERROR_IO_INCOMPLETE)
			{
				DEBUG_RETURN GetLastError();
			}
		}
		else
		{
			DEBUG_FUNCTION_SECTION;
#ifdef DEBUG_IO
			printf("\t\tORT [%d]\r\n", bytesTransfered);
#endif
			needMoreData = false;
		}
		*/

	}

	DEBUG_FUNCTION_SECTION;

	if(client == NULL)
	{	
		*read = bytesTransfered;
	}
	
#ifdef DEBUG_IO
	printf("ORD [");
	for(int i = 0 ; i < size;  i++)
	{
		printf("%02lX ", ((uint8*)buffer)[i]);
	}
	printf("\r\n");
#endif
	DEBUG_RETURN 0;
}

std::shared_ptr<NamedPipe> Basic_BLE_IO::getPipe()
{
	return pipe;
}

//Read in a message from the COM Port. Uses readData to handle the reading synchronously.
int Basic_BLE_IO::ReadMessage()
{
	DEBUG_ENTER_FUNCTION;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(400);
#endif

	//Sleep(250);
	DWORD rread, tread = 0;
	const struct ble_msg *apimsg;
	struct ble_header apihdr = {0};
	unsigned data[256] = {0};
	int ret;
#ifdef DEBUG_IO
	printf("RP1\r\n");
#endif

	if(PIPE_IS_CLIENT && !fallbackToCOM)
	{

#if USE_MEM_CHECK
	MemLeak_SetLocationID(401);
#endif
		DEBUG_FUNCTION_SECTION;
		unsigned char buffer[300] = {0};
		DWORD readSize;
		bool readBlock = ClientPipe->getNextBlock(buffer, _countof(buffer), &readSize, ReadOverlap.hEvent);
		if(!readBlock)
		{
			DEBUG_LOG("ERROR!! : Client->getNextBlock : returned false\r\n");
			return 0;
		}
		DWORD buffer1Size, buffer2Size;
		memcpy(&buffer1Size, buffer, 4);
		memcpy(&buffer2Size, &(buffer[4 + buffer1Size]), 4);
		
		memcpy(&apihdr, &(buffer[4]), 4);
		if(apihdr.lolen)
		{
			if(apihdr.lolen != buffer2Size)
			{
				DEBUG_LOG("Error : APIHDR.LOLEN != BUFFER2Size (%d vs %d)\r\n", apihdr.lolen, buffer2Size);
			}

			memcpy((unsigned char*)data, &(buffer[8 + buffer1Size]), apihdr.lolen);
		}

		apimsg = ble_get_msg_hdr(apihdr);
		if(!apimsg)
		{
			printf("ERROR : MessageID Not Found [%d] [%d]\r\n", (int)apihdr.cls, (int)apihdr.command);
			//DEBUG_RETURN -1;
			DEBUG_RETURN 0;
		}
		DEBUG_LOG("Got message from NP [%02lX][%02lX][%02lX][%02lX]\r\n", apihdr.type_hilen, apihdr.cls, apihdr.command, apihdr.lolen);
	}
	else
	{
#if USE_MEM_CHECK
	MemLeak_SetLocationID(402);
#endif
		DEBUG_FUNCTION_SECTION;
		ret = readData((unsigned char*)&apihdr, 4, &rread);
		if(ret != 0)
		{
			DEBUG_RETURN ret;
		}
		DebugDataInstream(rread);

	
		tread = rread;
		if(!rread)
		{
			//STUFF WENT WRONG!!!!
			printf("ARGH THE UNIVERSE\r\n");
			DEBUG_RETURN 0;
		}

		DWORD rread2 = 0;
		if(apihdr.lolen)
		{
	#ifdef DEBUG_IO
			printf("RP2\r\n");
	#endif
		
			ret = readData((unsigned char*)data, apihdr.lolen, &rread2);
			DebugDataInstream(rread);
			if(ret != 0)
			{
				DEBUG_RETURN ret;
			}
		

			tread += rread;
		}
	
#if USE_MEM_CHECK
	MemLeak_SetLocationID(403);
#endif
		if(PIPE_IS_SERVER)
		{
			pipe->sendBLEMessage((unsigned char*)&apihdr, rread, (unsigned char*)data, rread2);
		}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(404);
#endif
		apimsg = ble_get_msg_hdr(apihdr);
		if(!apimsg)
		{
			printf("ERROR : MessageID Not Found [%d] [%d]\r\n", (int)apihdr.cls, (int)apihdr.command);
			//DEBUG_RETURN -1;
			DEBUG_RETURN 0;
		}
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(405);
#endif
	//If the pipe is a Server Pipe, it is in Exclusive mode, and the exclusive mode isn't so only the server can read (selfExclusive mode)
	if(PIPE_IS_SERVER && ServerPipe->isExclusive() && (!ServerPipe->isSelfExclusive()))
	{
		//we cannot handle the data if we're the server and the pipe is in exclusive mode.
		//printf("Read data but is in exclusive mode\r\n");
	}
	else
	{
#ifndef MiTokenBLE_ClasslessMode
		//Class mode, we pass down the _parent
#if USE_MEM_CHECK
	MemLeak_SetLocationID(406);
#endif
		apimsg->handler(_root.GetRoot(), data);
#else
		//Classless mode, just pass the data
		apimsg->handler(data);
#endif
	}

	DEBUG_RETURN 0;
}

//Cancel all pending IO on the serial port
int Basic_BLE_IO::CancelIOEx()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN CancelIoEx(serial_handle, NULL);
}

//HANDLE fileLock = CreateMutex(NULL, FALSE, NULL);

//Output data given to us from the BLE API to the COM Port.
//Note : This uses ASynchronous writing but waits for it to make it synchronous.
//			Do NOT make write calls synchronous (even though we want this function to be synchronous) because that causes the COM Port IO breaks randomly.
void Basic_BLE_IO::output(uint8 len1, uint8* data1, uint16 len2, uint8* data2)
{
	DEBUG_ENTER_FUNCTION;

	bool locked;
	ScopedMutex scopeFileLock(fileLock, locked);
	if(!locked)
	{
		//we must be getting told to close down
		return;
	}


	static int writeID = 0;
	
	int uwid = writeID++;
	DWORD written, written2;
#ifdef DEBUG_IO
	printf("[%d] Writing %d bytes in blocks of [%d] and [%d]\r\n", uwid, len1 + len2, len1, len2);
#endif
	if(PIPE_IS_CLIENT && !fallbackToCOM)
	{
		pipe->sendBLEMessage(data1, len1, data2, len2);
		//pipe->sendBLEMessage(data2, len2);
	}
	else
	{
		if(!WriteFile(serial_handle, data1, len1, &written, &WriteOverlap))
		{
			if(GetLastError() != ERROR_IO_PENDING)
			{
				printf("ERROR: Writing Data %d\r\n", (int)GetLastError());
				DEBUG_LOG("ERROR WRITING DATA : WE ARE ABOUT TO EXIT(-1)!!!\r\n");
				exit(-1);
			}
		}

		if(_root.GetRoot()->GetState()->waitForOverlappedIO(serial_handle, &WriteOverlap, written2) == FALSE)
		{
			return;
		}

		//GetOverlappedResult(serial_handle, &WriteOverlap, &written2, TRUE);
		DebugDataOutstream(written2);
		if(!WriteFile(serial_handle, data2, len2, &written, &WriteOverlap))
		{
			if(GetLastError() != ERROR_IO_PENDING)
			{
				printf("ERROR: Writing Data %d\r\n", (int)GetLastError());
				DEBUG_LOG("ERROR WRITING DATA : WE ARE ABOUT TO EXIT(-2)!!!\r\n");
				exit(-2);
			}
		}
		//GetOverlappedResult(serial_handle, &WriteOverlap, &written2, TRUE);
		if(_root.GetRoot()->GetState()->waitForOverlappedIO(serial_handle, &WriteOverlap, written2) == FALSE)
		{
			return;
		}

		DebugDataOutstream(written2);
	}
#ifdef DEBUG_IO
	printf("[%d] Completed\r\n", uwid);
#endif

	DEBUG_END_FUNCTION;
}

void Basic_BLE_IO::serverGotData(unsigned char* data, unsigned int length)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_LOG("Writing %d bytes (s)\r\n", length);
	
	DEBUG_BYTES(data, length);

	DEBUG_LOG("\r\n");

	if(VALID_HANDLE(serial_handle))
	{
		DEBUG_FUNCTION_SECTION;
		bool locked;
		ScopedMutex(fileLock, locked);
		if(!locked)
		{
			//we must be closing down, so just return
			return;
		}


		int len[2] = { 0 };
		unsigned char* pData[2] = { 0 };

		memcpy(&len[0], data, 4);
		pData[0] = &(data[4]);
		memcpy(&len[1], &(data[4 + len[0]]), 4);
		pData[1] = &(data[8 + len[0]]);

		DEBUG_LOG("Data is split in %d and %d blocks\r\n", len[0], len[1]);
		DEBUG_BYTES(pData[0], len[0]);
		DEBUG_LOG("\r\n");
		DEBUG_BYTES(pData[1], len[1]);
		DEBUG_LOG("\r\n");

		for (int i = 0; i < 2; ++i)
		{
			DWORD written, written2;
			if (!WriteFile(serial_handle, pData[i], len[i], &written, &WriteOverlap))
			{
				if (GetLastError() != ERROR_IO_PENDING)
				{
					printf("ERROR: Writing Data %d\r\n", (int)GetLastError());
					exit(-1);
				}
			}
			if(_root.GetRoot()->GetState()->waitForOverlappedIO(serial_handle, &WriteOverlap, written2) == FALSE)
			{
				//we are closing down
				return;
			}
			DebugDataOutstream(written2);
		}

	}
	else
	{
		DEBUG_FUNCTION_SECTION;
	}
	DEBUG_END_FUNCTION;
}


BLE_API_NP_RETURNS Basic_BLE_IO::RequestExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_SERVER)
	{
		if(ServerPipe->makeServerExclusive())
		{
			return BLE_API_NP_SUCCESS;
		}
		else
		{
			return BLE_API_NP_BAD_EXCLUSIVE_STATE;
		}
	}

	DEBUG_RETURN ClientPipe->requestExclusiveAccess();
}

BLE_API_NP_RETURNS Basic_BLE_IO::ReleaseExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_SERVER)
	{
		//DEBUG_RETURN BLE_API_NP_ERR_NOT_CLIENT;
		if(ServerPipe->releaseServerExclusive())
		{
			DEBUG_RETURN BLE_API_NP_SUCCESS;
		}
		else
		{
			DEBUG_RETURN BLE_API_NP_BAD_EXCLUSIVE_STATE;
		}
	}

	DEBUG_RETURN ClientPipe->releaseExclusiveAccess();
}

BLE_API_NP_RETURNS Basic_BLE_IO::ExclusiveAccessStatus()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_SERVER)
	{
		if(ServerPipe->isSelfExclusive())
		{
			DEBUG_RETURN BLE_API_EXCLUSIVE_ALLOWED;
		}
		else
		{
			DEBUG_RETURN BLE_API_EXCLUSIVE_REJECTED;
		}
	}

	if(fallbackToCOM)
	{
		DEBUG_RETURN BLE_API_NP_COM_FALLBACK;
	}

	switch(ClientPipe->exclusiveStatus())
	{
	case NamedPipeClient::EXCLUSIVE_ALLOWED:
		DEBUG_RETURN BLE_API_EXCLUSIVE_ALLOWED;
	case NamedPipeClient::EXCLUSIVE_ASKED:
		DEBUG_RETURN BLE_API_EXCLUSIVE_ASKED;
	case NamedPipeClient::EXCLUSIVE_DENIED:
		DEBUG_RETURN BLE_API_EXCLUSIVE_REJECTED;
	case NamedPipeClient::EXCLUSIVE_DISABLED:
		DEBUG_RETURN BLE_API_EXCLUSIVE_DISABLED;
	}

	DEBUG_RETURN BLE_API_NP_INTERNAL_ERROR;
}

BLE_API_NP_RETURNS Basic_BLE_IO::SendMessageOverNP(unsigned char * data, int length, int connectionID)
{
	DEBUG_ENTER_FUNCTION;

	pipe->sendUserMessage(data, length, connectionID);
	DEBUG_RETURN BLE_API_NP_SUCCESS;
}

BLE_API_NP_RETURNS Basic_BLE_IO::SetCallback(void (*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID))
{
	DEBUG_ENTER_FUNCTION;

	pipe->userMessageCallback = callback;
	DEBUG_RETURN BLE_API_NP_SUCCESS;
}

BLE_API_NP_RETURNS Basic_BLE_IO::DisableBLEForwarding()
{
	DEBUG_ENTER_FUNCTION;

	if (PIPE_IS_SERVER)
	{
		DEBUG_RETURN BLE_API_NP_ERR_NOT_CLIENT;
	}

	DEBUG_RETURN ClientPipe->requestNoBLEForwarding();
}


bool Basic_BLE_IO::ClientConnectedToNP()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_CLIENT)
	{
		DEBUG_RETURN ClientPipe->isConnected;
	}

	DEBUG_RETURN false;
}

bool Basic_BLE_IO::COMOwnerKnown()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_SERVER)
	{
		if(ServerPipe->clientHasCOM())
		{
			DEBUG_RETURN true;
		}
		DEBUG_RETURN false;
	}
	else
	{
		DEBUG_RETURN false;
	}
}

BLE_API_RET Basic_BLE_IO::SetCOMWaitHandle(void* waitHandle)
{
	DEBUG_ENTER_FUNCTION;

	bool setEvent = true;
	if(PIPE_IS_SERVER)
	{
		if(ServerPipe->clientHasCOM())
		{
			ServerPipe->SetClientCOMReleaseWaitHandle((HANDLE)waitHandle);
			if(ServerPipe->clientHasCOM())
			{
				setEvent = false;
			}
		}
	}

	if(setEvent)
	{
		SetEvent((HANDLE)waitHandle);
		DEBUG_RETURN BLE_API_SUCCESS;
	}

	DEBUG_RETURN BLE_API_ERR_CLIENT_STILL_HAS_COM;
}

BLE_API_NP_RETURNS Basic_BLE_IO::ReleaseCOMPort()
{
	DEBUG_ENTER_FUNCTION;

	if(PIPE_IS_CLIENT)
	{
		if(ClientPipe->isConnected)
		{
			if(fallbackToCOM)
			{
				//we are connected and have fallbackToCOM, disconnect COM
				CloseSerialHandle();
				//inform the server that we closed the COM Port
				ClientPipe->InformServerOfClosedCOMPort();
				//turn off fallbackToCOM
				fallbackToCOM = false;
				//restart the reading thread
				async->Start();
				//DEBUG_RETURN SUCCESS
				DEBUG_RETURN BLE_API_NP_SUCCESS;
			}

			DEBUG_RETURN BLE_API_NP_SUCCESS;
		}
		else
		{
			DEBUG_RETURN BLE_API_NP_ERR_NOT_CONNECTED;
		}
		

		DEBUG_RETURN BLE_API_NP_ERR_UNKNOWN;
	}

	if(PIPE_IS_SERVER)
	{
		CloseSerialHandle();
		DEBUG_RETURN BLE_API_NP_SUCCESS;
	}

	DEBUG_RETURN BLE_API_NP_ERR_UNKNOWN;
}


void Basic_BLE_IO::StartASyncRead()
{
	DEBUG_ENTER_FUNCTION;

	async->Start();

	DEBUG_END_FUNCTION;
}

void Basic_BLE_IO::StopASyncRead()
{
	DEBUG_ENTER_FUNCTION;

	if(async->End() == -1) //-1 means there was no thread to stop, so kill async
	{}

	DEBUG_END_FUNCTION;
}

void Basic_BLE_IO::InitBLEDevice()
{
	DEBUG_ENTER_FUNCTION;

	_root.GetRoot()->getLowLevel()->InitBLEDevice();
	//BLE_LL_InitBLEDevice();

	DEBUG_END_FUNCTION;
}
