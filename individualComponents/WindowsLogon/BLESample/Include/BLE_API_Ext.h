#pragma once

#ifdef BLEAPI_USEEX

#ifndef _BLE_API_EXT_H_
#define _BLE_API_EXT_H_

#include "BLE_API.h"
#include "aes.h"


int EXT_MakeStoreBlock(int length, const uint8* data, int& outlength, uint8*& outdata);
int EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata);
int EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata);

#endif

#endif