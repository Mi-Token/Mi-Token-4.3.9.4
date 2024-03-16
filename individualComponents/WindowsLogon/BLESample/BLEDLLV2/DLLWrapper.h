#pragma once

#ifndef _DLLWRAPPER_H_
#define _DLLWRAPPER_H_

#include <BLE_API.h>

int ConvertDeviceIntoToUChar(DeviceInfo devInfo, unsigned char* deviceInfo);

class FinalizedFilterObject
{
public:
	FinalizedFilterObject();
	int count;
	DeviceData* filters;
	int addressCount;
	uint8* addresses;
};

class FilterObject
{
public:
	FilterObject();
	FilterObject* nextNode;
	int flag;
	int len;
	unsigned char* data;

	int filterAddressCount;
	unsigned char* filterAddresses;

};

#endif