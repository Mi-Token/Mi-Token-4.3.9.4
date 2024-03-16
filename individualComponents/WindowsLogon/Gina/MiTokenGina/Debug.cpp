#pragma once

#include <windows.h>
#include "Debug.h"
#include <stdio.h>
#include <time.h>

DWORD GetFormattedMessage(LPWSTR pBuffer, DWORD bufferSize, LPWSTR pMessage, ...);

void PrintError(LPCWSTR function, LPCWSTR message)
{
	WCHAR errorBuffer[128];
	WCHAR msgBuffer[256];

	LPCWSTR output = NULL;

	DWORD ret;
	DWORD err = GetLastError();

	memset(errorBuffer, 0, sizeof(errorBuffer));
	memset(msgBuffer, 0, sizeof(msgBuffer));
	
	ret = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, err, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR) &errorBuffer, sizeof(errorBuffer) / sizeof(errorBuffer[0]), NULL);
	if (ret == 0)
	{
		ret = GetFormattedMessage((LPWSTR) errorBuffer, sizeof(errorBuffer) / sizeof(errorBuffer[0]), L"Unknown error 0x%1!x!",
				err);
	}

	if (ret > 0)
	{
		ret = GetFormattedMessage((LPWSTR) msgBuffer, sizeof(msgBuffer) / sizeof(msgBuffer[0]), L"%1 (%2) [%3]",
				message, errorBuffer, function);
	}

	if (ret == 0)
		output = L"failed to format message!";
	else
		output = msgBuffer;


	PrintDebug(output);
}

DWORD GetFormattedMessage(LPWSTR pBuffer, DWORD bufferSize, LPWSTR pMessage, ...)
{
	DWORD ret = 0;
    va_list args = NULL;
    va_start(args, pMessage);

    ret = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                  pMessage, 
                  0,  // ignored
                  0,  // ignored
                  (LPWSTR) pBuffer,
                  bufferSize, 
                  &args);

    va_end(args);

    return ret;
}

void PrintDebug(LPCWSTR message)
{
	/*
	static int prid = 0; //pseudo-random id
	if(prid == 0)
	{
		srand(time(NULL));
		prid = rand();
	}
	
	FILE* fp;
	fp = fopen("C:\\MiTokenGina.txt", "a+");
	if(fp != NULL)
	{
		char buffer[80];
		time_t rawtime;
		struct tm* timeinfo;
		time ( &rawtime );
		timeinfo = localtime( &rawtime );
		strftime(buffer, 80, "%X", timeinfo);
		fprintf(fp, "[%04lX] %s : ", prid, buffer);
		fwprintf(fp, L"%s\n", message);
		fclose(fp);
	}
	/**/
	OutputDebugString(message);
}