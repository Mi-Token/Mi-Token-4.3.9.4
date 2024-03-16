//Allows NamedPipe Communications

#include "NamedPipeClient.h"
#include <stdio.h>
#include <Windows.h>


/*
DWORD WINAPI NamedPipeClient::threadFunction(void* data)
{
	NamedPipeClient* pNPC = (NamedPipeClient*)data;
	
	BYTE buffer[1024] = {0};

	while(pNPC->_hPipe != INVALID_HANDLE_VALUE)
	{
		DWORD bytesRead;
		if(!(ReadFile(pNPC->_hPipe, buffer, 1024, &bytesRead, 0)))
		{
			printf("Error %d", GetLastError());
		}
		else
		{
			pNPC->_receivedData(pNPC, buffer, bytesRead);
		}
	}
	return 0;
}

*/
NamedPipeClient::NamedPipeClient()
{
	_hPipe = INVALID_HANDLE_VALUE;
}
void NamedPipeClient::readPipeBytes(unsigned char* data, int maxlength, DWORD* bytesRead)
{

	if(_hPipe != INVALID_HANDLE_VALUE)
	{
		if(!(ReadFile(_hPipe, data, maxlength, bytesRead, 0)))
		{
			printf("Error %d", GetLastError());
		}
		else
		{
			
		}
	}
	return;
}
int NamedPipeClient::Connect(LPTSTR pipeName, wchar_t* senderName)//, void (*receivedData) (NamedPipeClient* sender, unsigned char* data, int length))
{
	_hPipe= CreateFile( 
		pipeName,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,  // allow read/write sharing
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 
 
	// Break if the pipe handle is valid. 
 
	if (_hPipe == INVALID_HANDLE_VALUE) 
	{
 
		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
 
		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			int x = GetLastError();
			if(x == 0)
				return -1;
			return x;
		}
 
		// All pipe instances are busy, so wait for 20 seconds. 
 
		return -2;
	}

	if(senderName)
		this->SendPipeMessage(senderName);


	return 0;
}

bool NamedPipeClient::connected()
{
	return !(_hPipe == INVALID_HANDLE_VALUE);
}
int NamedPipeClient::SendPipeMessage(wchar_t * message) //, va_list _ArgList)
{
	UNREFERENCED_PARAMETER(message);
	/*
	DWORD cbToWrite, cbWritten;
	BOOL fSuccess;
	
	if(_hPipe == INVALID_HANDLE_VALUE)
		return -1;

	{
		//wchar_t dest[250];
		//cbToWrite = sizeof(TCHAR) * _stprintf_s(dest, 250, message, _ArgList);
		cbToWrite = (lstrlen(message)+1)*sizeof(TCHAR);
   
		fSuccess = WriteFile( 
			_hPipe,                  // pipe handle 
			message,             // message 
			cbToWrite,              // message length 
			&cbWritten,             // bytes written 
			NULL);                  // not overlapped 

		if (!fSuccess) 
		{
			if(GetLastError() == 232) //the pipe is being closed
			{
				CloseHandle(_hPipe);
				_hPipe = INVALID_HANDLE_VALUE;
				return -3;
			}
			return -2;
		}
		return 0;
	}
	*/
	return -5;
}

int NamedPipeClient::SendMessageF(wchar_t* format, ...)
{
	if(_hPipe != INVALID_HANDLE_VALUE)
	{
		
		wchar_t *buffer;
		va_list args;
		va_start(args, format);		
		int len = _vscwprintf(format, args) + 1;
		buffer = (wchar_t*)calloc(len, sizeof(wchar_t));
		vswprintf_s(buffer, len, format, args);
		va_end(args);
		int ret = this->SendPipeMessage(buffer);
		free(buffer);
		return ret;

	}
	else
		return -1;
}

int NamedPipeClient::SendBytes(unsigned char* data, int length)
{
	if(_hPipe != INVALID_HANDLE_VALUE)
	{
		BOOL fSuccess;
		DWORD cbWritten;

		fSuccess = WriteFile( 
			_hPipe,                  // pipe handle 
			data,             // message 
			length,              // message length 
			&cbWritten,             // bytes written 
			NULL);                  // not overlapped 

		if (!fSuccess) 
		{
			if(GetLastError() == 232) //the pipe is being closed
			{
				CloseHandle(_hPipe);
				_hPipe = INVALID_HANDLE_VALUE;
				return -3;
			}
			return -2;
		}
		return 0;
	}
	return -1;
}
int NamedPipeClient::SendMessageCF(char* format, ...)
{
	if(_hPipe != INVALID_HANDLE_VALUE)
	{
		va_list args;
		int len;
		char* cbuffer;
		va_start(args, format);
		len = _vscprintf(format, args) + 1;
		cbuffer = (char*)calloc(len, sizeof(char));
		vsprintf_s(cbuffer, len, format, args);

		int ret = this->SendBytes((unsigned char*)cbuffer, len);
		/*
		wchar_t * buffer = (wchar_t*)calloc(len, sizeof(wchar_t));
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, buffer, len, cbuffer, len);
		int ret = this->SendPipeMessage(buffer);
		free(buffer);
		*/
		free(cbuffer);
		return ret;
	}
	else
		return -1;
}

void NamedPipeClient::close()
{
	if(_hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hPipe);
	}
}