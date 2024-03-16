#pragma once

#ifndef NO_HOT_SWAP

#include "Passthroughs.h"
//DLL Hot swapping functions
int APIENTRY DLL_Pack(unsigned char** data, int* cbytes);
int APIENTRY DLL_Unpack(unsigned char* data, int cbytes);
int APIENTRY DLL_FREE(unsigned char* data);
#endif