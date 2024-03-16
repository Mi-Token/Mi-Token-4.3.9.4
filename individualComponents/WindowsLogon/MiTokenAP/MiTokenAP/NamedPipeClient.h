#pragma once

#ifndef _NAMED_PIPE_CLIENT_H_
#define _NAMED_PIPE_CLIENT_H_

#include <tchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Windows.h>

#define CLIENT_BUF_SIZE 512
class NamedPipeClient
{
public:
	NamedPipeClient();
	int Connect(LPTSTR pipeName, wchar_t* senderName); //, void (*receivedData) (NamedPipeClient* sender, unsigned char* data, int length));
	int SendPipeMessage(wchar_t * message);//, va_list _ArgList);
	int SendMessageF(wchar_t* format, ...);
	int SendMessageCF(char* format, ...);
	int SendBytes(unsigned char* data, int length);
	bool NamedPipeClient::connected();
	void readPipeBytes(unsigned char* data, int maxlength, DWORD* bytesRead);
	void close();
	//static DWORD WINAPI threadFunction(void* data);

protected:
private:
	HANDLE _hPipe;
	TCHAR _buffer[CLIENT_BUF_SIZE];
	//void (*_receivedData) (NamedPipeClient* sender, unsigned char * data, int length);
	
};

#endif