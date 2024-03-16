#include "GenericUtils.h"


const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;
	LPCSTR szName;
	DWORD dwThreadID;
	DWORD dwReservedFlags;
}THREADNAME_INFO;
#pragma pack(pop)

void NameThread(LPCSTR name)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000; //needs to be this
	info.szName = name;
	info.dwThreadID = -1; //caller
	info.dwReservedFlags = 0;

	//now to change the thread name by rasing an exception
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{}
}

struct tempThreadStartParam
{
public:
	DWORD (WINAPI *threadStart) (LPVOID);
	LPVOID param;
	LPCSTR threadName;
};


DWORD WINAPI tempThreadStart(LPVOID param)
{
	tempThreadStartParam* pTTSP = (tempThreadStartParam*)param;
	LPTHREAD_START_ROUTINE routine = pTTSP->threadStart;
	LPVOID innerParam = pTTSP->param;

	char* name = new char[strlen(pTTSP->threadName) + 1];
	memcpy(name, pTTSP->threadName, strlen(pTTSP->threadName) + 1);

	NameThread(name);

	delete pTTSP; //no longer needed
	
	DWORD ret = routine(innerParam); //call the thread start

	delete[] name;

	return ret;
}

HANDLE CreateNamedThread(__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,__in SIZE_T dwStackSize, __in LPTHREAD_START_ROUTINE lpStartAddress, __in_opt __deref __drv_aliasesMem LPVOID lpParameter, __in DWORD dwCreationFlags, __out_opt LPDWORD lpThreadId, __in LPCSTR ThreadName)
{
	tempThreadStartParam* pTTSP = new tempThreadStartParam;
	pTTSP->threadStart = lpStartAddress;
	pTTSP->param = lpParameter;
	pTTSP->threadName = ThreadName;

	return CreateThread(lpThreadAttributes, dwStackSize, tempThreadStart, pTTSP, dwCreationFlags, lpThreadId);
}

