#pragma once

#include "NamedPipeClient.h"


enum SCCred_CARD_RESULT
{
	SCCred_NewDefaultCard,
	SCCred_NotDefaultCard,
	SCCred_AutoLoginUser,
};

enum SCCred_CARD_RESULTV2
{
	SCCred_CardExists,
	SCCred_CardDoesntExist,
};

BOOL SCCreds_Init();
SCCred_CARD_RESULT SCCreds_CardScanned(unsigned long CardID);
BOOL SCCreds_AddNameAndPass(unsigned char* username, int usernameLength, unsigned char* password, int passwordLength);
HRESULT SCCreds_GetUsernameAndPass(unsigned char* username, int* usernameLength, unsigned char* password, int* passwordLength);
void SCCreds_SendData();
void SCCreds_Finished();
//BOOL TryReadSerialNumber(unsigned long* serialNumber);
BOOL SCCreds_HasCachedCreds();
//void SCCreds_Beep();



SCCred_CARD_RESULTV2 SCCreds_CardScannedV2(__int64 CardID);
void SCCreds_SetCardID(__int64 CardID);
BOOL SCCreds_AddNameAndPassV2(unsigned char* username, int usernameLength, unsigned char* password, int passwordLength);
HRESULT SCCreds_GetUsernameAndPassV2(unsigned char* username, int* usernameLength, unsigned char* password, int* passwordLength);
void SCCreds_SendDataV2();
void SCCreds_FininishedV2();
