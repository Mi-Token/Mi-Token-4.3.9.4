#include "ScopedLock.h"
#include "CurrentState.h"


ScopedMutex::ScopedMutex(HANDLE mutex, bool& locked)
{
	locked = false;
	_mutex = NULL;

	//NewLock(mutex);
	WaitForSingleObject(mutex, INFINITE);
	//if(CurrentState::waitOnEvents(mutex))
	{
		_mutex = mutex;
		locked = true;
	}
}

void ScopedMutex::EarlyRelease()
{
	if(_mutex != NULL)
	{
		ReleaseMutex(_mutex);
		_mutex = NULL;
	}
}

void ScopedMutex::NewLock(HANDLE mutex)
{
	EarlyRelease();
	
	WaitForSingleObject(mutex, INFINITE);
	//if(CurrentState::waitOnEvents(mutex))
	{
		_mutex = mutex;
	}
}

ScopedMutex::~ScopedMutex()
{
	EarlyRelease();
	
}

ReadLock::ReadLock(ReadWriteLocker* state)
{
	_state = nullptr;
	newLock(state);
}

ReadLock::~ReadLock()
{
	earlyRelease();
}

void ReadLock::newLock(ReadWriteLocker* state)
{
	earlyRelease();
	if(state == nullptr)
	{
		return;
	}

	_state = state;

	_state->GetReadLock();
}

void ReadLock::earlyRelease()
{
	if(_state != nullptr)
	{
		_state->ReleaseReadLock();
		_state = nullptr;
	}
}

WriteLock::WriteLock(ReadWriteLocker* state)
{
	_state = nullptr;
	newLock(state);
}

WriteLock::~WriteLock()
{
	earlyRelease();
}

void WriteLock::newLock(ReadWriteLocker* state)
{
	earlyRelease();
	if(state != nullptr)
	{
		_state = state;

		_state->GetWriteLock();
	}
}

void WriteLock::earlyRelease()
{
	if(_state != nullptr)
	{
		_state->ReleaseWriteLock();
		_state = nullptr;
	}
}