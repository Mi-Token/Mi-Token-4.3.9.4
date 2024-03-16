#pragma once

#ifndef _UNITTEST_METADATA_
#define _UNITTEST_METADATA_

#include "BLE_API_Types.h"

class UnitTest_MetaData
{
public:
	UnitTest_MetaData();
	~UnitTest_MetaData();

	bool addNode(uint8 flag, uint8 dataLen, uint8* data);
	uint8* packAndReturn(int& packLen);

protected:
	struct Node
	{
		Node* nextNode;
		uint8 flag;
		uint8 dataLen;
		uint8* data;
	} _node;

	uint8* _packedData;
	int _packedLen;
};

#endif