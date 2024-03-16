#include "ReadWriteLocker.h"
#include "ScopedLock.h"
#include <stdio.h>

ReadWriteLocker::ReadWriteLocker()
{
	_mainMutex = CreateMutex(nullptr, FALSE, nullptr);
	_canReadHandle = CreateEvent(nullptr, true, true, nullptr);
	_canWriteHandle = CreateEvent(nullptr, true, true, nullptr);
	_readCount = 0;
}

ReadWriteLocker::~ReadWriteLocker()
{
	CloseHandle(_mainMutex);
	CloseHandle(_canReadHandle);
	CloseHandle(_canWriteHandle);
}

void ReadWriteLocker::GetReadLock()
{
	bool hasReadAccess = false;
	
	bool locked;
	ScopedMutex lock(_mainMutex, locked);

	while(!hasReadAccess)
	{
		if(WaitForSingleObject(_canReadHandle, 0) == WAIT_TIMEOUT)
		{
			//we cannot go yet, release the _mainMutex and wait for _canReadHandle
			lock.EarlyRelease();
			WaitForSingleObject(_canReadHandle, INFINITE);

			//now lock the _mainMutex again
			lock.NewLock(_mainMutex);
		}
		else
		{
			hasReadAccess = true;
		}
	}


	//We have read access
	if(_readCount == 0)
	{
		ResetEvent(_canWriteHandle);
	}

	_readCount++;
}

void ReadWriteLocker::GetWriteLock()
{
	bool hasWriteAccess = false;
	bool locked;
	ScopedMutex lock(_mainMutex, locked);

	while(!hasWriteAccess)
	{
		if(WaitForSingleObject(_canWriteHandle, 0) == WAIT_TIMEOUT)
		{
			//we cannot go yet, release _mainMutex and wait for _canWriteHandle
			lock.EarlyRelease();
			WaitForSingleObject(_canWriteHandle, INFINITE);

			//now lock the _mainMutex again
			lock.NewLock(_mainMutex);
		}
		else
		{
			hasWriteAccess = true;
		}
	}

	ResetEvent(_canWriteHandle);

	int x = GetLastError();
	ResetEvent(_canReadHandle);

	return;
}

void ReadWriteLocker::ReleaseReadLock()
{
	bool locked;
	ScopedMutex lock(_mainMutex, locked);
	_readCount--;
	if(_readCount == 0)
	{
		SetEvent(_canWriteHandle);
	}

	return;
}

void ReadWriteLocker::ReleaseWriteLock()
{
	bool locked;
	ScopedMutex lock(_mainMutex, locked);
	{
		SetEvent(_canWriteHandle);
		SetEvent(_canReadHandle);
	}

	return;
}

/*
ProtectedBytePointer::ProtectedBytePointer()
{
	data = nullptr;
}

const BYTE* ProtectedBytePointer::GetRead(ReadWriteLocker* locker, ReadLock* readLock)
{
	readLock->newLock(locker);

	return data;
}

BYTE* ProtectedBytePointer::GetWrite(ReadWriteLocker* locker, WriteLock* writeLock)
{
	writeLock->newLock(locker);

	return data;
}

BYTE** ProtectedBytePointer::GetDataPtr(ReadWriteLocker* locker, WriteLock* writeLock)
{
	writeLock->newLock(locker);

	return &data;
}*/