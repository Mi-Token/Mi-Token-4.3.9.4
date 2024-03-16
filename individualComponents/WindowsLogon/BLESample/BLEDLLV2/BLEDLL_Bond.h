#pragma once
#include "BLEDLLV2.h"


BLEDLLV2_API BLE_BOND_RET WINAPI MiTokenBLEV2_2_CONN_Bond_AddBondToToken(IMiTokenBLEV2_2* bleInstance, IMiTokenBLEConnectionV2_2* bleConnection, uint8* sha1Hash, uint8* bondData);
BLEDLLV2_API bool WINAPI MiTokenBLEV2_2_CONN_Bond_RemoveAll(IMiTokenBLEV2_2* bleInstance, IMiTokenBLEConnectionV2_2* bleConnection);