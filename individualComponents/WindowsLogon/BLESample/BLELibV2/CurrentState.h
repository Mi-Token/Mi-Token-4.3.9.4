#pragma once

#ifndef _CURRENT_STATE_H_
#define _CURRENT_STATE_H_

#include <Windows.h>

//Singleton class - This class doesn't work with V2.1 - We need to change it to an instanced class.

class CurrentState
{
public:
	CurrentState();
	~CurrentState();
	void abort();
	void restart();
	bool waitOnEvents(HANDLE otherEvent);
	bool shouldAbort();
	bool waitForOverlappedIO(HANDLE hFile, LPOVERLAPPED overlapped, DWORD& cbBytes);
	int waitForOverlappedIO(HANDLE hFile, LPOVERLAPPED readOverlapped, int handleCount, const HANDLE* handleList, DWORD& cbBytes);

	HANDLE getHandle();

protected:
	HANDLE abortEvent;
};

#endif