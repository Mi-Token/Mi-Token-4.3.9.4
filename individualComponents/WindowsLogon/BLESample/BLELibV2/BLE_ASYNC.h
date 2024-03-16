#pragma once

#ifndef _BLE_ASYNC_H_
#define _BLE_ASYNC_H_


#include <Windows.h>
#include "BLE_IO.h"

class BLE_ASYNC
{
public:
	BLE_ASYNC(IBLE_IO* io_interface, MiTokenBLE* parent);
	~BLE_ASYNC();

	static DWORD WINAPI ReadPollMain(LPVOID param);

	int Start();
	int End();

protected:
	HANDLE waitHandle;
	DWORD _ReadPoll(MiTokenBLEChild& _childRef);
	HANDLE readPollThread;
	bool runReadPoll;
	IBLE_IO* _io_interface;
	MiTokenBLEChild _root;
};

/*
int BLE_ASYNC_AllPollsEnd();

int BLE_ASYNC_ReadPollStart();
int BLE_ASYNC_ReadPollEnd();
*/


#endif