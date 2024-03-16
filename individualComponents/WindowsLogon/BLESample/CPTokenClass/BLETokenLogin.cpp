#include "BLETokenLogin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] p; p = nullptr; }
#endif

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
	SET_WATCHER_STAGE = 0x12,

	CP_PASS_CHANGE = 0x20,

};

enum WATCHER_STAGES : unsigned char
{
	STAGE_LookingForToken = 0x01,
	STAGE_AuthenticatingToken = 0x02,
	STAGE_WaitingForUsername = 0x03,
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
	_myStage = StageEnums::Initializing;
	gotUserPassCallback = nullptr;
	_username = _password = nullptr;
	_instance = dynamic_cast<IMiTokenBLEV2_2*>(CreateNewMiTokenBLEInterface(USE_MITOKEN_BLE_API_V2_2));
	_instance->Initialize(nullptr, "\\\\.\\pipe\\Mi-TokenBLEV2", false, NamedPipeConnectedCallback);
	_instance->Pipe_SetNamedPipeMessageCallback(BLETokenLogin::Pipe_Callback);
	_wusername = _wpassword = nullptr;

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
	for (int i = 0; i < length; ++i)
	{
		printf("%02lX ", data[i]);
	}

	if (data[0] == DATA_FLAG::SET_USERANDPASS)
	{
		unsigned char usernameLen = data[1];
		unsigned char passwordLen = data[2 + usernameLen];
		if (_wusername != nullptr)
		{
			delete[] _wusername;
			_wusername = nullptr;
		}
		if (_wpassword != nullptr)
		{
			delete[] _wpassword;
			_wpassword = nullptr;
		}

		_wusername = new wchar_t[(usernameLen / 2) + 1];
		_wpassword = new wchar_t[(passwordLen / 2) + 1];
		memcpy(_wusername, &data[2], usernameLen);
		_wusername[usernameLen / 2] = 0;
		memcpy(_wpassword, &data[3 + usernameLen], passwordLen);
		/*
		wchar_t* username, *password;
		username = &(data[2]);
		password = &(data[3 + usernameLen]);

		_setUsernameAndPassword(username, usernameLen, password, passwordLen);
		*/
		if (gotUserPassCallback != nullptr)
		{
			gotUserPassCallback(nullptr, 0, nullptr, 0, false, _LPVOID);
		}
	}
	else if (data[0] == DATA_FLAG::SET_USERNAME)
	{
		unsigned char usernameLen = data[1];
		SAFE_DELETE_ARRAY(_wusername);
		SAFE_DELETE_ARRAY(_wpassword);
		_wusername = new wchar_t[usernameLen + 1];
		memcpy(_wusername, &data[2], usernameLen);
		//_setUsernameAndPassword(nullptr, 0, nullptr, 0);
		if (gotUserPassCallback != nullptr)
		{
			gotUserPassCallback(nullptr, 0, nullptr, 0, true, _LPVOID);
		}
	}
	else if(data[0] == DATA_FLAG::SET_WATCHER_STAGE)
	{
		switch(data[1])
		{
		case WATCHER_STAGES::STAGE_AuthenticatingToken:
			_myStage = StageEnums::AuthenticatingToken;
			break;
		case WATCHER_STAGES::STAGE_LookingForToken:
			_myStage = StageEnums::LookingForToken;
			break;
		case WATCHER_STAGES::STAGE_WaitingForUsername:
			_myStage = StageEnums::WaitingForUsername;
			break;
		}
	}

}

StageEnums BLETokenLogin::getCurrentStage()
{
	return _myStage;
}

bool BLETokenLogin::hasPassword(wchar_t* password)
{
	if (password != nullptr)
	{
		if(_wpassword != nullptr)
		{
			memcpy(password, _wpassword, (wcslen(_wpassword) + 1) * 2);
		}
	}

	return _wpassword != nullptr;
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

void BLETokenLogin::SendUserPassMessage(wchar_t* username, wchar_t* password)
{
	int userLen = 2 * (wcslen(username) + 1);
	int passLen = 2 * (wcslen(password) + 1);

	int totalLength = (1) + (userLen + 1) + (passLen + 1);
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



bool BLETokenLogin::hasUsername(wchar_t* ppUsername)
{
	if (ppUsername != nullptr)
	{
		if(_wusername != nullptr)
		{
			memcpy(ppUsername, _wusername, (wcslen(_wusername) + 1) * 2);
		}
	}

	return _wusername != nullptr;
}

bool BLETokenLogin::canAutoLogin()
{
	return ((_wusername != nullptr) && (_wpassword != nullptr));
}

bool BLETokenLogin::isSameUsername(wchar_t* username)
{
	if((username == nullptr) || (_wusername == nullptr))
	{
		return false;
	}

	int l1 = wcslen(_wusername);
	if(wcslen(username) != l1)
	{
		return false;
	}

	if(memcmp(username, _wusername, l1 * sizeof(wchar_t)) == 0)
	{
		return true;
	}

	return false;
}

bool BLETokenLogin::hasDevice()
{
	if((_myStage == StageEnums::AuthenticatingToken) ||
		(_myStage == StageEnums::WaitingForUsername))
	{
		return true;
	}

	return false;
}

void BLETokenLogin::clearUsernamePassword()
{
	SAFE_DELETE_ARRAY(_wusername);
	SAFE_DELETE_ARRAY(_wpassword);
}

void BLETokenLogin::passwordMustChange()
{
	SAFE_DELETE_ARRAY(_wusername);
	SAFE_DELETE_ARRAY(_wpassword);

	unsigned char data[1] = {DATA_FLAG::CP_PASS_CHANGE};

	_instance->Pipe_SendMessage(data, 1, -1);
}

void BLETokenLogin::passwordHasBeenChanged()
{
	SAFE_DELETE_ARRAY(_wpassword);
	SAFE_DELETE_ARRAY(_wusername);
	unsigned char data[] = { DATA_FLAG::CP_ONLINE };

	_instance->Pipe_SendMessage(data, sizeof(data) / sizeof(unsigned char), -1);
}