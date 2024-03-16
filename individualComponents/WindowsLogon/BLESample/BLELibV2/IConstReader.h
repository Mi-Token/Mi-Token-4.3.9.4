#pragma once

#ifndef _ICONST_READER_H_
#define _ICONST_READER_H_


#include <Windows.h>
#include "SafeObject.h"
#include "MessageQueue.h"

#ifndef VALID_HANDLE
#define VALID_HANDLE(p) ((p != nullptr) && (p != INVALID_HANDLE_VALUE))
#endif

class IConstReader
{
public:

	virtual bool startReader() = 0;
	virtual bool stopReader() = 0;
	
	virtual void setWriteBuffer(SafeObject<MessageQueue> writeBuffer) = 0;
	virtual void setOverlappedStructure(LPOVERLAPPED lpOverlapped) = 0;
protected:
	HANDLE _threadHandle;
	SafeObject<MessageQueue> _writeToBuffer;
	
	HANDLE _abortThreadHandle;
	LPOVERLAPPED _lpOverlapped;
};

#endif