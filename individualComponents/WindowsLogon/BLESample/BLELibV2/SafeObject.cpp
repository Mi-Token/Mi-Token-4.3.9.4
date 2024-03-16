#include "SafeObject.h"
#include "ScopedLock.h"

#include <stdio.h>


BaseSafeObject::BaseSafeObject()
{
	_refMutex = CreateMutex(nullptr, false, nullptr);
	_refCount = 0;
	_safeToDel = nullptr;
}

void BaseSafeObject::AddRef()
{
	bool locked;
	ScopedMutex mut(_refMutex, locked);
	_refCount++;
	printf("Added Reference [%d] [%08lX]\r\n", _refCount, this);
}

void BaseSafeObject::RemRef()
{
	bool locked;
	ScopedMutex mut(_refMutex, locked);
	_refCount--;
	printf("Removed Reference [%d] [%08lX]\r\n", _refCount, this);

	if ((_refCount == 0) && (_safeToDel != nullptr))
	{
		SetEvent(_safeToDel);
	}
}

void BaseSafeObject::waitUntilSafe()
{
	printf("Waiting for Safe Object [%08lX]\r\n", this);
	WaitForSingleObject(_refMutex, INFINITE);
	while (_refCount != 0)
	{
		if (_safeToDel == nullptr)
		{
			_safeToDel = CreateEvent(nullptr, false, false, nullptr);
		}

		ReleaseMutex(_refMutex);
		WaitForSingleObject(_safeToDel, INFINITE);
	}


	if (_safeToDel != nullptr)
	{
		CloseHandle(_safeToDel);
		_safeToDel = nullptr;
	}

	printf("Safe Object [%08lX] is safe to be deleted\r\n", this);
}

BaseSafeObject::~BaseSafeObject()
{
	waitUntilSafe();
	CloseHandle(_refMutex);
	_refMutex = nullptr;
}
