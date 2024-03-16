#include "MessageQueue.h"
#include "MiTokenBLE.h"

#define DEFAULT_BUFFER (4096)

MessageQueue::MessageQueue(MiTokenBLEChild root) : _root(root)
{
	_storedData = new unsigned char[DEFAULT_BUFFER];
	_storeStart = 0;
	_storeLength = 0;
	_storeMaxLength = DEFAULT_BUFFER;
	_storeMutex = CreateMutex(nullptr, FALSE, nullptr);
	_waitEvent = INVALID_HANDLE_VALUE;
}

MessageQueue::~MessageQueue()
{
	CloseHandle(_storeMutex);
	CloseHandle(_waitEvent);
	delete _storedData;
}

void MessageQueue::addMessage(void* message, int length)
{
	if (_root.GetRoot()->GetState()->waitOnEvents(_storeMutex) == FALSE)
	{
		return;
	}

	int freeSpace = this->_freeSpace();
	if (freeSpace < length)
	{
		_lengthenStore(length);
	}

	memcpy(&_storedData[_storeLength], message, length);
	_storeLength += length;

	if (_waitEvent && _blockSize())
	{
		SetEvent(_waitEvent);
		_waitEvent = nullptr;
	}

	ReleaseMutex(_storeMutex);
}

int MessageQueue::getNextMessageLength()
{
	int len = 0;
	if (_root.GetRoot()->GetState()->waitOnEvents(_storeMutex) == FALSE)
	{
		return 0;
	}

	len = _blockSize();
	ReleaseMutex(_storeMutex);
}

bool MessageQueue::getNextMessage(void* data, int maxSize, int& actualSize)
{

	actualSize = 0;
	bool pulledData = false;
	if (_root.GetRoot()->GetState()->waitOnEvents(_storeMutex) == FALSE)
	{
		return false;
	}

	actualSize = _blockSize();
	if (actualSize > 0)
	{
		if (maxSize >= actualSize)
		{
			_storeStart += 5;
			memcpy(data, &(_storedData[_storeStart]), actualSize);
			_storeStart += actualSize;
			pulledData = true;
		}
	}

	ReleaseMutex(_storeMutex);

	return pulledData;
}

bool MessageQueue::setWaitHandle(HANDLE waitHandle)
{
	bool retCode = false;

	if (_root.GetRoot()->GetState()->waitOnEvents(_storeMutex) == FALSE)
	{
		return false;
	}

	if (_blockSize())
	{
		SetEvent(waitHandle);
		retCode = true;
	}
	else
	{
		this->_waitEvent = waitHandle;
		ResetEvent(waitHandle);
	}

	ReleaseMutex(_storeMutex);

	return retCode;
}

void MessageQueue::_lengthenStore(int minimumFreeSpace)
{
	int freeSpace = this->_freeSpace();
	int newFreeSpace = (freeSpace + (2 * DEFAULT_BUFFER));
	if (newFreeSpace < minimumFreeSpace)
	{
		newFreeSpace = (minimumFreeSpace + DEFAULT_BUFFER);
	}

	int spaceNeeded = (newFreeSpace - freeSpace);
	int newLength = _storeMaxLength + spaceNeeded - _storeStart;

	unsigned char* newData = new unsigned char[newLength];
	memcpy(newData, &(_storedData[_storeStart]), (_storeLength - _storeStart));

	_storeLength = _storeLength - _storeStart;
	_storeStart = 0;
	_storeMaxLength = newLength;

	delete[] _storedData;
	_storedData = newData;
	newData = nullptr;

	return;
}

int MessageQueue::_storeSize()
{
	return (_storeLength - _storeStart);
}

int MessageQueue::_blockSize()
{
	int len = 0;
	int size = _storeSize();
	if (size > 4)
	{
		memcpy(&len, &(_storedData[_storeStart]), 4);
		if (size < len)
		{
			len = 0;
		}
		else
		{
			len -= 5;
		}
	}

	return len;
}

int MessageQueue::_freeSpace()
{
	return (_storeMaxLength - _storeLength);
}
