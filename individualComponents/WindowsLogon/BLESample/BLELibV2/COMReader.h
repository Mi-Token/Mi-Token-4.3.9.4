#pragma once

#ifndef _COMREADER_H_
#define _COMREADER_H_

#include "IConstReader.h"
#include "MiTokenBLEChild.h"

class COMReader : public IConstReader
{
public:

	COMReader(HANDLE comHandle, MiTokenBLEChild root);
	~COMReader();

	virtual bool startReader();
	virtual bool stopReader();
	virtual void setWriteBuffer(MessageQueue* writeBuffer);
	virtual void setOverlappedStructure(LPOVERLAPPED lpOverlapped);

protected:
	HANDLE _comPortHandle;
	static DWORD WINAPI _COMReaderThreadStart(LPVOID param);
	DWORD _threadFunction();
	int readData(unsigned char* buffer, DWORD size, DWORD* read);
	MiTokenBLEChild _root;
	
};

#endif