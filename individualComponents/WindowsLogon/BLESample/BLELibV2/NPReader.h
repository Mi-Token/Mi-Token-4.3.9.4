#pragma once

#ifndef _NPREADER_H_
#define _NPREADER_H_

#include "IConstReader.h"
#include "MiTokenBLEChild.h"
#include "BLE_NamedPipe.h"

class NPReader : public IConstReader
{
public:
	NPReader(NamedPipeClient* npClient, MiTokenBLEChild root);
	~NPReader();

	virtual bool startReader();
	virtual bool stopReader();
	virtual void setWriteBuffer(MessageQueue* writeBuffer);
	virtual void setOverlappedStructure(LPOVERLAPPED lpOverlapped);

protected:
	NamedPipeClient* _pClient;
	static DWORD WINAPI _NPReaderThreadStart(LPVOID param);
	DWORD _threadFunction();
	MiTokenBLEChild _root;
};

#endif