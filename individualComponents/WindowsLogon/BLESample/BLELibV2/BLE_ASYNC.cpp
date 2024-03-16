#include "BLE_ASYNC.h"
#include "BLE_IO.h"

#include "DebugLogging.h"
#include "CurrentState.h"
#include "MiTokenBLE.h"
#include "GenericUtils.h"


#ifndef VALID_HANDLE
#define VALID_HANDLE(p) ((p != NULL) && (p != INVALID_HANDLE_VALUE))
#endif

BLE_ASYNC::BLE_ASYNC(IBLE_IO* io_interface, MiTokenBLE* parent) : _root(parent)
{
	DEBUG_ENTER_FUNCTION;
	
	_io_interface = io_interface;
	
	runReadPoll = false;
	readPollThread = NULL;
	waitHandle = NULL;

	DEBUG_END_FUNCTION;
}

BLE_ASYNC::~BLE_ASYNC()
{
	DEBUG_ENTER_FUNCTION;

	End();

	DEBUG_END_FUNCTION;
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; //must be 0x1000;
	LPCSTR szName; //Pointer to name (in user addr space)
	DWORD dwThreadID; //Thread ID (-1 = caller thread)
	DWORD dwFlags; //Reservered for future use, must be 0
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char* threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

DWORD WINAPI BLE_ASYNC::ReadPollMain(LPVOID param)
{

#if USE_MEM_CHECK
	MemLeak_SetLocationID(304);
#endif

	DEBUG_ENTER_FUNCTION;
	SetThreadName(-1, "ASYNC - Read Poll");

	BLE_ASYNC* pAsync = reinterpret_cast<BLE_ASYNC*>(param);
#if USE_MEM_CHECK
	MemLeak_SetLocationID(305);
#endif
	MiTokenBLEChild root = pAsync->_root; //we will store our own child to the root object.
	std::shared_ptr<NamedPipe> pipe = root.GetRoot()->GetIO()->getPipe();
	DWORD ret = pAsync->_ReadPoll(root);

	/*
	if(pAsync->_root.GetRoot()->GetState()->shouldAbort())
	{
		//we are aborting, so delete the pAsync
		delete pAsync;
	}*/
	
	DEBUG_RETURN ret;
}

int BLE_ASYNC::Start()
{
	DEBUG_ENTER_FUNCTION;
#if USE_MEM_CHECK
	MemLeak_SetLocationID(300);
#endif

	if(VALID_HANDLE(readPollThread))
	{

#if USE_MEM_CHECK
	MemLeak_SetLocationID(301);
#endif
		if(!runReadPoll)
		{
			DEBUG_RETURN ERROR_ALREADY_EXISTS;
		}

		runReadPoll = FALSE;
		if(_io_interface->CancelIOEx() == 0)
		{
			if(GetLastError() == ERROR_INVALID_HANDLE)
			{}
			else
			{
				DEBUG_RETURN GetLastError();
			}
		}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(302);
#endif

		waitHandle = readPollThread;
		
		readPollThread = CreateNamedThread(NULL, 0, BLE_ASYNC::ReadPollMain, this, 0, NULL, "ASYNC : ReadPollMain");
	}
	else
	{

#if USE_MEM_CHECK
	MemLeak_SetLocationID(303);
#endif
		runReadPoll = true;
		readPollThread = CreateNamedThread(NULL, 0, BLE_ASYNC::ReadPollMain, this, 0, NULL, "ASYNC : ReadPollMain");
	}

	return 0;
}

int BLE_ASYNC::End()
{
	DEBUG_ENTER_FUNCTION;


	int ret = -1;
	if(VALID_HANDLE(readPollThread))
	{
		runReadPoll = false;
		if(_io_interface->CancelIOEx() == 0)
		{
			ret = GetLastError();
			if(ret != 0x490)
			{
				//DEBUG_RETURN ret;
			}
			else
			{
				DEBUG_RETURN ERROR_SUCCESS;
			}
		}
		if(WaitForSingleObject(readPollThread, 2000) == WAIT_TIMEOUT)
		{
			ret = WAIT_TIMEOUT;
		}

		CloseHandle(readPollThread);
		readPollThread = INVALID_HANDLE_VALUE;
	}

	DEBUG_RETURN ret;
}

DWORD BLE_ASYNC::_ReadPoll(MiTokenBLEChild& _root)
{
	DEBUG_ENTER_FUNCTION;

	int ret;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(306);
#endif

	if(VALID_HANDLE(waitHandle))
	{
		
		if(_root.GetRoot()->GetState()->waitOnEvents(waitHandle) == FALSE)
		{
			DEBUG_RETURN 0;
		}
		CloseHandle(waitHandle);
		waitHandle = NULL;
		runReadPoll = true;
	}

#if USE_MEM_CHECK
	MemLeak_SetLocationID(307);
#endif

	ret = 0;
	while((runReadPoll) && (ret == 0) && (_root.GetRoot()->GetState()->shouldAbort() == FALSE))
	{

#if USE_MEM_CHECK
	MemLeak_SetLocationID(308);
#endif

		ret = _io_interface->ReadMessage();
	}

	DEBUG_RETURN 0;
}


//READ POLL
HANDLE readPollThread = INVALID_HANDLE_VALUE;
int runReadPoll = FALSE;

DWORD WINAPI ReadPollMain(LPVOID param)
{
	DEBUG_ENTER_FUNCTION;

	int ret;

	if((param != NULL) && (param != INVALID_HANDLE_VALUE))
	{
		WaitForSingleObject(param, INFINITE);
		CloseHandle(param);
		runReadPoll = TRUE;
	}

	ret = 0;
	while((runReadPoll) && (ret == 0))
	{
		ret = IO_Interface->ReadMessage();
	}

	DEBUG_RETURN 0;
}
int BLE_ASYNC_ReadPollStart()
{
	DEBUG_ENTER_FUNCTION;

	if(readPollThread != INVALID_HANDLE_VALUE)
	{
		if(!runReadPoll)
		{
			DEBUG_RETURN ERROR_ALREADY_EXISTS;
		}

		runReadPoll = FALSE;
		if(IO_Interface->CancelIOEx() == 0)
		{
			if(GetLastError() ==  ERROR_INVALID_HANDLE)
			{}
			else
			{
				DEBUG_RETURN GetLastError();
			}
		}

		readPollThread = CreateNamedThread(NULL, 0, ReadPollMain, (LPVOID)readPollThread, 0, NULL, "ASYNC : ReadPollMain");
	}
	else
	{
		runReadPoll = TRUE;
		readPollThread = CreateNamedThread(NULL, 0, ReadPollMain, NULL, 0, NULL, "ASYNC : ReadPollMain");
	}

	return 0;
}


int BLE_ASYNC_ReadPollEnd()
{
	DEBUG_ENTER_FUNCTION;

	int ret = ERROR_SUCCESS;
	if(readPollThread != INVALID_HANDLE_VALUE)
	{
		runReadPoll = FALSE;
		if(IO_Interface->CancelIOEx() == 0)
		{
			ret = GetLastError();
			if(ret != 0x490)
			{
				DEBUG_RETURN ret;
			}
			else
			{
				ret = ERROR_SUCCESS;
			}
		}
		if(WaitForSingleObject(readPollThread, 2000) ==  WAIT_TIMEOUT)
		{
			ret = WAIT_TIMEOUT;
		}

		CloseHandle(readPollThread);
		readPollThread = INVALID_HANDLE_VALUE;
	}

	DEBUG_RETURN ret;
}