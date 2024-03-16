#pragma once

#ifndef _READWRITE_LOCKER_H_
#define _READWRITE_LOCKER_H_

#include <Windows.h>
#include "ScopedLock.h"

class ReadLock;
class WriteLock;

class ReadWriteLocker
{
public:
	~ReadWriteLocker();
	ReadWriteLocker();
	void GetWriteLock();
	void GetReadLock();
	void ReleaseWriteLock();
	void ReleaseReadLock();

protected:
	HANDLE _mainMutex;
	HANDLE _canReadHandle, _canWriteHandle;
	int _readCount;
};


template <class T>
class ProtectedPointer
{
public:
	ProtectedPointer()
	{
		data = nullptr;
	}
	const T* GetRead(ReadWriteLocker* locker, ReadLock* readLock)
	{
		readLock->newLock(locker);
		return data;
	}
	T* GetWrite(ReadWriteLocker* locker, WriteLock* writeLock)
	{
		writeLock->newLock(locker);
		return data;
	}
	T** GetDataPtr(ReadWriteLocker* locker, WriteLock* writeLock)
	{
		writeLock->newLock(locker);
		return &data;
	}

protected:
	T* data;
};
#endif