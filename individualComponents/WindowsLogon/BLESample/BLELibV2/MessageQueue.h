#pragma once

#ifndef _MESSAGEBUFFER_H_
#define _MESSAGEBUFFER_H_

#include <Windows.h>
#include "MiTokenBLEChild.h"
#include "SafeObject.h"

class MessageQueue : public BaseSafeObject
{
public:
	MessageQueue(MiTokenBLEChild root);
	~MessageQueue();

	void addMessage(void* message, int length);
	int getNextMessageLength();
	bool getNextMessage(void* data, int maxSize, int& actualSize);
	bool setWaitHandle(HANDLE waitHandle);

protected:
	void _lengthenStore(int minimumFreeSpace);
	int _storeSize();
	int _blockSize();
	int _freeSpace();
	unsigned char* _storedData;
	int _storeStart;
	int _storeLength;
	int _storeMaxLength;
	HANDLE _storeMutex;
	HANDLE _waitEvent;
	MiTokenBLEChild _root;
private:
	//Copy constructor and assignment operator are not allowed
	MessageQueue(MessageQueue& ref);
	void operator= (MessageQueue& ref);


};

#endif