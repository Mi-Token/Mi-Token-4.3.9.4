#pragma once

#ifndef _NAMED_PIPE_CLIENT_H_
#define _NAMED_PIPE_CLIENT_H_

#include <stdlib.h>
#include <stdarg.h>
#include <Windows.h>
#include <tchar.h>

#define CLIENT_BUF_SIZE 512
class NamedPipeClient
{
public:
	int Connect(LPTSTR pipeName, wchar_t* senderName);
	int SendPipeMessage(wchar_t * message);//, va_list _ArgList);
	int SendMessageF(wchar_t* format, ...);
	int SendMessageCF(char* format, ...);
	int SendBytes(unsigned char* data, int length);
	bool NamedPipeClient::connected();
protected:
private:
	HANDLE _hPipe;
	TCHAR _buffer[CLIENT_BUF_SIZE];
};

#endif