#include "BLEDLL_Bond.h"
#include "BLE_BondingLib.h"

BLEDLLV2_API BLE_BOND_RET WINAPI MiTokenBLEV2_2_CONN_Bond_AddBondToToken(IMiTokenBLEV2_2* bleInstance, IMiTokenBLEConnectionV2_2* bleConnection, uint8* sha1Hash, uint8* bondData)
{
	BLE_BondingLib bonder;
	BLE_BOND_RET ret = bonder.addBondToToken(bleInstance, bleConnection, sha1Hash);
	if(ret == BOND_RET_SUCCESS)
	{
		if(bonder.bondLength > 0)
		{
			memcpy(bondData, bonder.bondData, bonder.bondLength);
		}
	}
	return ret;
}

BLEDLLV2_API bool WINAPI MiTokenBLEV2_2_CONN_Bond_RemoveAll(IMiTokenBLEV2_2* bleInstance, IMiTokenBLEConnectionV2_2* bleConnection)
{
	BLE_BondingLib bonder;
	return bonder.removeAllBondsFromToken(bleInstance, bleConnection);
}