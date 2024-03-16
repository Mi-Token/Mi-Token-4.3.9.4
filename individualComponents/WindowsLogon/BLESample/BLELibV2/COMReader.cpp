#include "COMReader.h"
#include "cmd_def.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "MiTokenBLE.h"
#include "CurrentState.h"
#include "GenericUtils.h"


COMReader::COMReader(HANDLE comHandle, MiTokenBLEChild root) : _root(root)
{
	_comPortHandle = comHandle;
	_threadHandle = INVALID_HANDLE_VALUE;
	_abortThreadHandle = INVALID_HANDLE_VALUE;
}

COMReader::~COMReader()
{
	if(_threadHandle != INVALID_HANDLE_VALUE)
	{
		stopReader();
	}
	
	CloseHandle(_abortThreadHandle);
}

bool COMReader::startReader()
{
	if(_threadHandle != INVALID_HANDLE_VALUE)
	{
		stopReader();
	}

	if(_abortThreadHandle != INVALID_HANDLE_VALUE)
	{
		ResetEvent(_abortThreadHandle);
	}
	else
	{
		_abortThreadHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	if(_writeToBuffer.GetRoot() == nullptr)
	{
		return false; //we cannot start while writeToBuffer is nullptr
	}

	_threadHandle = CreateNamedThread(nullptr, 0, COMReader::_COMReaderThreadStart, this, 0, nullptr, "COMReader : ThreadStart");

	return true;
}

bool COMReader::stopReader()
{
	if(_threadHandle)
	{
		SetEvent(_abortThreadHandle);
		WaitForSingleObject(_threadHandle, INFINITE);
	}

	CloseHandle(_threadHandle);
	_threadHandle = nullptr;
	return true;
}

void COMReader::setWriteBuffer(MessageQueue* writeBuffer)
{
	_writeToBuffer = writeBuffer;
}

void COMReader::setOverlappedStructure(LPOVERLAPPED lpOverlapped)
{
	_lpOverlapped = lpOverlapped;
}

DWORD COMReader::_COMReaderThreadStart(LPVOID param)
{
	COMReader *pReader = (COMReader*)param;
	MiTokenBLEChild localRoot = pReader->_root;
	DWORD ret = ((COMReader*)param)->_threadFunction();

	return ret;
}

DWORD COMReader::_threadFunction()
{
	while(true)
	{
		DWORD rread = 0, tread = 0;
		ble_header apihdr = {0};

		DWORD ret = 0;
		unsigned char data[256 + sizeof(apihdr) + 4] = {0};
	

		ret = readData((unsigned char*)&(data[4]), 4, &rread);

		if(ret != 0)
		{
			return ret;
		}

		tread = rread;

		if(!rread)
		{
			printf("Something went wrong reading from the COM Port!\r\n");
			return 0;
		}

		DWORD rread2 = 0;
		if(apihdr.lolen)
		{
			ret = readData((unsigned char*)&data[sizeof(apihdr) + 4], apihdr.lolen, &rread2);

			if(ret != 0)
			{
				return ret;
			}

			tread += rread2;
		}

		uint32 len = tread;
		memcpy(&(data[0]), &len, 4);

		_writeToBuffer.GetRoot()->addMessage(data, len);
	}
}

int COMReader::readData(unsigned char* buffer, DWORD size, DWORD* read)
{
	bool needMoreData = true;
	int ret;
	if(VALID_HANDLE(_comPortHandle))
	{
		ret = ReadFile(_comPortHandle, buffer, size, read, _lpOverlapped);
	}

	DWORD bytesTransferred;

	while(needMoreData)
	{
		const HANDLE handles[] = { _root.GetRoot()->GetState()->getHandle(), _abortThreadHandle, _lpOverlapped->hEvent };
		
		int handleID = _root.GetRoot()->GetState()->waitForOverlappedIO(_comPortHandle, _lpOverlapped, 3, handles, bytesTransferred);

		//handleID will be the index of the handle that returned (or -1 if it failed, or handleCount if the read wasn't async)
		//handleID will be either (handleCount == 3) or (index of lpOverlapped == 2) in the case of success
		if((handleID == 3) || (handleID == 2))
		{
			needMoreData = false;
		}
		else
		{
			//handleID is either 0 ==> main abort set
			//or 1 ==> _abortThreadHandle set
			return -1;
		}

	}

	*read = bytesTransferred;

	return 0;
}