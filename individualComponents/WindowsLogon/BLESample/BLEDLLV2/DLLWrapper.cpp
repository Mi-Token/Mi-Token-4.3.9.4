#include "DLLWrapper.h"

#include <string>

FinalizedFilterObject::FinalizedFilterObject()
{
	count = 0;
	filters = NULL;
	addressCount = 0;
	addresses = NULL;
}

FilterObject::FilterObject()
{
	nextNode = NULL;
	flag = 0;
	len = 0;
	data = NULL;
	filterAddressCount = 0;
	filterAddresses = NULL;
}

int ConvertDeviceIntoToUChar(DeviceInfo devInfo, unsigned char* deviceInfo)
{
	/*
		deviceInfo format
			6 byte Address (Opposite endianness to the way it is stored in DeviceInfo struct)
			1 byte RSSI
			4 byte 'last seen ID'
	*/
	memcpy(deviceInfo, &(devInfo.address[0]), 6);
	deviceInfo[6] = devInfo.RSSI;
	memcpy(&(deviceInfo[7]), &(devInfo.lastSeen), 4);

	return 0;
}