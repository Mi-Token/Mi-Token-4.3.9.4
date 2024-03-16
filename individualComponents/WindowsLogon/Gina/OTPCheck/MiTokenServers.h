#pragma once

#ifndef _MITOKEN_SERVERS_
#define _MITOKEN_SERVERS_

#include <string.h>
#include <tchar.h>
#include <windows.h>


class MiTokenServers
{
public:
	MiTokenServers();
	bool hasMoreServers();
	int getNextAPIServer(BYTE* server, int& serverLength, TCHAR* key, int& keyLength, bool& moreServers);
	void getMaxSizeRequired(int& maxValueLength, int& maxDataLength);

private:
	int currentRegLocation;
	DWORD currentRegKey;
	DWORD currentRegKeyCount;
	void calcRegKeyCount();
	void calcMaxSize();
	DWORD curMaxValueLength;
	DWORD curMaxDataLength;
	int maxValueLength;
	int maxDataLength;
	void moveToNextSpot();
	void moveToFirstSpot();
};
#endif