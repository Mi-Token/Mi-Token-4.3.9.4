#include "CurrentState.h"
#include <stdio.h>

CurrentState::CurrentState()
{
	abortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CurrentState::~CurrentState()
{
	printf("Current State destructor called\r\n");
}

void CurrentState::abort()
{
	SetEvent(abortEvent);
}

void CurrentState::restart()
{
	ResetEvent(abortEvent);
}

bool CurrentState::waitOnEvents(HANDLE otherEvent)
{
	HANDLE events[2] = {abortEvent, otherEvent};

	int eventID = WaitForMultipleObjects(2, &(events[0]), FALSE, INFINITE);

	if(eventID == WAIT_OBJECT_0)
	{
		return false;
	}

	return true;
}

bool CurrentState::shouldAbort()
{
	return (WaitForSingleObject(abortEvent, 0) == WAIT_OBJECT_0);
}

bool CurrentState::waitForOverlappedIO(HANDLE hFile, LPOVERLAPPED overlapped, DWORD& cbBytes)
{
	DWORD lastError;
	if(GetOverlappedResult(hFile, overlapped, &cbBytes, FALSE) == FALSE)
	{
		lastError = GetLastError();
		if((lastError == ERROR_IO_PENDING) || (lastError == ERROR_IO_INCOMPLETE))
		{
			if(waitOnEvents(overlapped->hEvent) == FALSE)
			{
				//waitOnEvents = false ==> we should abort
				return false;
			}
			else
			{
				//waitOnEvents = true ==> we have the data now
				GetOverlappedResult(hFile, overlapped, &cbBytes, FALSE);
				return true;
			}
		}	
	}
	return true;
}

int CurrentState::waitForOverlappedIO(HANDLE hFile, LPOVERLAPPED readOverlapped, int handleCount, const HANDLE* handleList, DWORD& cbBytes)
{
	DWORD lastError;
	if(GetOverlappedResult(hFile, readOverlapped, &cbBytes, FALSE) == FALSE)
	{
		lastError = GetLastError();
		if((lastError == ERROR_IO_PENDING) || (lastError == ERROR_IO_INCOMPLETE))
		{
			int objIndex = WaitForMultipleObjects(handleCount, handleList, FALSE, INFINITE);
			if(handleList[objIndex - WAIT_OBJECT_0] == readOverlapped->hEvent)
			{
				GetOverlappedResult(hFile, readOverlapped, &cbBytes, FALSE);
			}

			return objIndex;
		}
		return -1;
	}
	return (handleCount);
}

HANDLE CurrentState::getHandle()
{
	return abortEvent;

}