#pragma once

#ifndef _SCOPED_LOCK_H_
#define _SCOPED_LOCK_H_

#include <Windows.h>

#include "ReadWriteLocker.h"
class ReadWriteLocker;

class ScopedMutex
{
public:
	ScopedMutex(HANDLE mutex, bool& locked);
	void EarlyRelease();
	void NewLock(HANDLE mutex);
	~ScopedMutex();

protected:
	HANDLE _mutex;
};


class ScopedEvent
{
public:
	ScopedEvent(HANDLE evt, HANDLE mutex);
	void EarlyRelease();
	void Newlock(HANDLE evt);
	~ScopedEvent();
protected:
	HANDLE _evt;
};

class ReadLock
{
public:
	//ReadLock();
	ReadLock(ReadWriteLocker* state = nullptr);
	~ReadLock();

	void newLock(ReadWriteLocker* state);
	void earlyRelease();
	
protected:
	ReadWriteLocker* _state;
};

class WriteLock
{
public:
	//WriteLock();
	WriteLock(ReadWriteLocker* state = nullptr);
	~WriteLock();

	void newLock(ReadWriteLocker* state);
	void earlyRelease();
protected:
	ReadWriteLocker* _state;
};

#endif