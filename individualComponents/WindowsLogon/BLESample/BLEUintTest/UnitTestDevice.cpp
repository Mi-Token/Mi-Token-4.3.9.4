#include "stdafx.h"


#include "UnitTestDevice.h"

#include <string>
#include "BLE_API.h"

#include "cmd_def.h"

UnitTestDevice::UnitTestDevice()
{
	_metaData = NULL;
	_metaLength = 0;
}

UnitTestDevice::~UnitTestDevice()
{
	if(_metaData)
	{
		delete[] _metaData;
	}
}

void UnitTestDevice::setMetaData(void* data, int len)
{
	_metaData = new uint8[len];
	memcpy(_metaData, data, len);
	_metaLength = len;

	return;
}

void UnitTestDevice::sendPollOff(UnitTest_IO* pIO_Instance, uint8 RSSIToSend)
{
	ble_header hdr = {0};

	hdr.type_hilen |= ble_msg_type_evt;
	hdr.cls = 6;
	hdr.command = 0;
	
	uint8* pU8 = new uint8[256];
	ble_msg_gap_scan_response_evt_t* msg = reinterpret_cast<ble_msg_gap_scan_response_evt_t*>(pU8);
	hdr.lolen = sizeof(msg);

	msg->address_type = 0;
	msg->bond = 0;
	msg->packet_type = 4;
	msg->rssi = RSSIToSend;
	memcpy(&msg->sender.addr, &_myMac.addr, sizeof(mac_address));


	msg->data.len = this->_metaLength;

	memcpy(&msg->data.data, _metaData, _metaLength);

	int realLength = sizeof(*msg) + _metaLength; //msg includes a 1byte metaData array
	hdr.lolen = realLength;

	pIO_Instance->inputMessage(sizeof(hdr), reinterpret_cast<uint8*>(&hdr), realLength, reinterpret_cast<uint8*>(msg));

	delete[] pU8;

}

void UnitTestDevice::setMac(mac_address newMac)
{
	memcpy(&(_myMac.addr), &(newMac.addr), sizeof(mac_address));
}

void UnitTestDevice::giveRandomMac()
{
	for(int i = 0 ; i < sizeof(mac_address) ; ++i)
	{
		_myMac.addr[i] = rand() & 0xFF;
	}
}

bool UnitTestDevice::sameMac(mac_address* refMac)
{
	if(memcmp(&(_myMac.addr), &(refMac->addr), sizeof(mac_address)) == 0)
	{
		return true;
	}
	return false;
}

