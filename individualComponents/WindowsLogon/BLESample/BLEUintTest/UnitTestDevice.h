#pragma once

#ifndef _UNIT_TEST_DEVICE_H_
#define _UNIT_TEST_DEVICE_H_

#include "BLE_API.h"
#include "BLE_IO.h"
#include "UnitTest_BLEIO.h"

//Allows the UnitTester to emulate devices
class UnitTestDevice
{
public:
	UnitTestDevice();
	~UnitTestDevice();

	void setMetaData(void* data, int len);

	void sendPollOff(UnitTest_IO* pIO_Instance, uint8 RSSIToSend);

	void setMac(mac_address newMac);
	bool sameMac(mac_address* refMac);

	void giveRandomMac();
protected:
	mac_address _myMac;
	void* _metaData;
	int _metaLength;

};

#endif