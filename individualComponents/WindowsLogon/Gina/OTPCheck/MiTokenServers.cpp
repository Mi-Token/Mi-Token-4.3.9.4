#include "MiTokenServers.h"


TCHAR* serverLocationList[] = {
	TEXT("Software\\Mi-Token\\Logon\\Servers"),
	TEXT("Software\\Mi-Token\\Logon")
};

#define SERVER_COUNT (sizeof(serverLocationList) / sizeof(TCHAR*))

MiTokenServers::MiTokenServers()
{
	currentRegLocation = 0;
	currentRegKey = 0;
	calcMaxSize();
	moveToFirstSpot();
}
void MiTokenServers::calcMaxSize()
{
	maxValueLength = 0;
	maxDataLength = 0;
	for(int i = 0 ; i < SERVER_COUNT ; ++i)
	{
		currentRegLocation = i;
		calcRegKeyCount();
		if(maxValueLength < curMaxValueLength)
		{
			maxValueLength = curMaxValueLength;
		}
		if(maxDataLength < curMaxDataLength)
		{
			maxDataLength = curMaxDataLength;
		}
	}
	currentRegLocation = 0;
}

void MiTokenServers::calcRegKeyCount()
{
	DWORD cValues = 0;
	HKEY key;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, serverLocationList[currentRegLocation], 0, KEY_READ, &key);
	if(key == NULL)
	{
		currentRegKeyCount = 0;
		curMaxValueLength = 0;
		curMaxDataLength = 0;
		return;
	}

	RegQueryInfoKey(key,           // key handle 
			NULL,                   // buffer for class name 
			NULL,                   // size of class string 
			NULL,                   // reserved 
			NULL,                   // number of subkeys 
			NULL,                   // longest subkey size 
			NULL,                   // longest class string 
			&currentRegKeyCount,               // number of values for this key 
			&curMaxValueLength,		// longest value name 
			&curMaxDataLength,		// longest value data 
			NULL,			        // security descriptor 
			NULL);	

	RegCloseKey(key);

}

void MiTokenServers::getMaxSizeRequired(int& maxValueLength, int& maxDataLength)
{
	maxValueLength = this->maxValueLength;
	maxDataLength = this->maxDataLength;
}

void MiTokenServers::moveToFirstSpot()
{
	currentRegLocation = 0;
	calcRegKeyCount();
	while(currentRegKeyCount == 0)
	{
		currentRegLocation++;
		if(currentRegLocation >= SERVER_COUNT)
		{
			break;
		}
		calcRegKeyCount();
	}
}
void MiTokenServers::moveToNextSpot()
{
	currentRegKey++;
	while((currentRegKey >= currentRegKeyCount) ||
		(currentRegKeyCount == 0))
	{
		currentRegLocation++;
		if(currentRegLocation >= SERVER_COUNT)
		{
			break;
		}
		calcRegKeyCount();
	}
}
bool MiTokenServers::hasMoreServers()
{
	return (currentRegLocation < SERVER_COUNT);
}

int MiTokenServers::getNextAPIServer(BYTE* server, int& serverLength, TCHAR* key, int& keyLength, bool& moreServers)
{
	bool loop = true;
	int ret = 0;
	while(loop && (moreServers = this->hasMoreServers()))
	{
		HKEY Regkey;

		loop = false;
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, serverLocationList[currentRegLocation], 0, KEY_READ, &Regkey);
		if(Regkey == NULL)
		{
			moveToNextSpot();
			return -1;
		}
		DWORD serverLen = serverLength;
		DWORD keyLen = keyLength;
		DWORD valType;
		ret = RegEnumValue(Regkey,
			currentRegKey,
			key,
			&keyLen,
			NULL,
			&valType,
			server,
			&serverLen);

		RegCloseKey(Regkey);
		keyLength = keyLen;
		serverLength = serverLen;
		moveToNextSpot();

		if(valType != REG_SZ)
		{
			loop = true;
		}
	
	}
	return ret;
}