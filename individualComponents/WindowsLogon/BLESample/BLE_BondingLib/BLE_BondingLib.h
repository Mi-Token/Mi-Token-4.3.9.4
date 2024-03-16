#pragma once

#include "BLE_API.h"

class BLE_BondingLib
{
public:
	BLE_BondingLib(void);
	~BLE_BondingLib(void);

	BLE_BOND_RET addBondToToken(IMiTokenBLEV2_2* v2_2Interface, IMiTokenBLEConnectionV2_2* v2_2Connection, uint8* sha1Hash);
	bool removeAllBondsFromToken(IMiTokenBLEV2_2* v2_2Interface, IMiTokenBLEConnectionV2_2* v2_2Connection);

	uint8* bondData;
	uint8 bondLength;
};

