#include "BLETokenLogin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>


enum DATA_FLAG : unsigned char
{
    LOGIN_ON = 0x01,
    LOGIN_DONE = 0x02,
    CHECK_FOR_MAC = 0x03,
    REQUEST_LOCK = 0x04,
    REQUEST_QUIT = 0x05,
    STORE_PASSWORD = 0x06,
    REQUEST_PASSWORD = 0x07,
    PASSWORD_VAL = 0x08,

    CP_ONLINE = 0x09,
    SET_USERNAME = 0x10,
    SET_USERANDPASS = 0x11,

};

BLETokenLogin* BLETokenLogin::MyInstance = nullptr;

void NamedPipeConnectedCallback(IMiTokenBLE* instance)
{
	printf("Pipe is now connected");
	instance->Pipe_DisableBLEForwarding();
	BLETokenLogin::getInstance()->SendOnlineMessage();
}

BLETokenLogin::BLETokenLogin()
{
	MyInstance = this;
	gotUserPassCallback = nullptr;
	_username = _password = nullptr;
	_instance = dynamic_cast<IMiTokenBLEV2_2*>(CreateNewMiTokenBLEInterface(USE_MITOKEN_BLE_API_V2_2));
	_instance->Initialize(nullptr, "\\\\.\\pipe\\Mi-TokenBLEV2", false, NamedPipeConnectedCallback);
	_instance->Pipe_SetNamedPipeMessageCallback(BLETokenLogin::Pipe_Callback);

}

BLETokenLogin* BLETokenLogin::getInstance()
{
	if(MyInstance == nullptr)
	{
		MyInstance = new BLETokenLogin();
	}

	return MyInstance;
}

void BLETokenLogin::SendOnlineMessage()
{
	unsigned char messageData[] = { DATA_FLAG::CP_ONLINE };

	_instance->Pipe_SendMessage(messageData, sizeof(messageData) / sizeof(unsigned char), -1);
}

void BLETokenLogin::SendLoggedInMessage()
{
	unsigned char messageData[] = {DATA_FLAG::LOGIN_DONE};
	_instance->Pipe_SendMessage(messageData, sizeof(messageData) / sizeof(unsigned char), -1);
}

void BLETokenLogin::Pipe_Callback(IMiTokenBLE* sender, unsigned char* data, int length, int connectionID)
{
	BLETokenLogin::getInstance()->gotMessage(data, length);
}

void BLETokenLogin::gotMessage(unsigned char* data, int length)
{
	printf("Got Message of Length : %d\r\n\t", length);
	for(int i = 0 ; i < length ; ++i)
	{
		printf("%02lX ", data[i]);
	}

	if(data[0] == DATA_FLAG::SET_USERANDPASS)
	{
		unsigned char usernameLen = data[1];
		unsigned char passwordLen = data[2 + usernameLen];
		unsigned char* username, *password;
		username = &(data[2]);
		password = &(data[3 + usernameLen]);

		_setUsernameAndPassword(username, usernameLen, password, passwordLen);
		if(gotUserPassCallback != nullptr)
		{
			gotUserPassCallback(username, usernameLen, password, passwordLen, false, _LPVOID);
		}
	}
	else if(data[0] == DATA_FLAG::SET_USERNAME)
	{
		unsigned char usernameLen = data[1];
		unsigned char* username = &(data[2]);
		_setUsernameAndPassword(username, usernameLen, nullptr, 0);
		if(gotUserPassCallback != nullptr)
		{
			gotUserPassCallback(username, usernameLen, nullptr, 0, true, _LPVOID);
		}
	}
}

void BLETokenLogin::SendUserPassMessage(char* username, int userLen, char* password, int passLen)
{
	int totalLength = (1) + (userLen + 4) + (passLen + 4);
	unsigned char* data = new unsigned char[totalLength];
	data[0] = DATA_FLAG::STORE_PASSWORD;
	int index = 1;
	(data[index]) = userLen;
	index += 1;
	memcpy(&(data[index]), username, userLen);
	index += userLen;
	(data[index]) = passLen;
	index += 1;
	memcpy(&(data[index]), password, passLen);
	index == passLen;

	_instance->Pipe_SendMessage(data, totalLength, -1);
}


void BLETokenLogin::SetGotUserPassCallback(void (*callback) (unsigned char* username, int userLen, unsigned char* password, int passLen, bool needPassword, void* LPVOID), void* LPVOID)
{
	gotUserPassCallback = callback;
	_LPVOID = LPVOID;
}

void BLETokenLogin::_setUsernameAndPassword(unsigned char* username, int usernameLen, unsigned char* password, int passwordLen)
{
	if(_username != nullptr)
	{
		delete[] _username;
	}
	_username = new char[usernameLen + 1];
	memcpy(_username, username, usernameLen);
	_username[usernameLen] = 0;
	if(_password != nullptr)
	{
		delete[] _password;
	}
	if(password == nullptr)
	{
		_password = nullptr;
	}
	else
	{
		_password = new char[passwordLen + 1];
		memcpy(_password, password, passwordLen);
		_password[passwordLen] = 0;
	}
}

bool BLETokenLogin::isSameUsername(char* username)
{
	return isSameString(username, _username);
}

bool BLETokenLogin::isSamePassword(char* password)
{
	return isSameString(password, _password);
}

bool BLETokenLogin::isSameString(char* str1, char* str2)
{
	if((str1 == nullptr) || (str2 == nullptr))
	{
		return false; //nullptr is always false
	}
	if(strcmp(str1, str2) == 0)
	{
		return true;
	}
	return false;
}

const char* BLETokenLogin::getUsername()
{
	return _username;
}

const char* BLETokenLogin::getPassword()
{
	return _password;
}




