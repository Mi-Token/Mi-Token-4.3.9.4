#pragma once

#include "common.h"




bool unpackAll(PackedData* pData, BYTE *packedBuffer, long bufferLength, int& embeddedFileType);
long findMSIStart(unsigned char* buffer, long bufferLength);