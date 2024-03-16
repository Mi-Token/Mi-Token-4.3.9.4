#include "stdafx.h"

#include "UnitTest_MetaData.h"

#include <string>
#include <assert.h>

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] p; p = NULL; }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }
#endif

UnitTest_MetaData::UnitTest_MetaData()
{
	_packedData = NULL;
	_packedLen = 0;
	_node.nextNode = NULL;
	_node.data = NULL;
	_node.flag = 0;
	_node.dataLen = 0;
}

UnitTest_MetaData::~UnitTest_MetaData()
{
	Node* pnode = _node.nextNode;
	Node* pcur;
	while(pnode)
	{
		pcur = pnode;
		pnode = pnode->nextNode;
		SAFE_DELETE_ARRAY(pcur->data);
		SAFE_DELETE(pcur);
	}

	SAFE_DELETE_ARRAY(_node.data);
	SAFE_DELETE_ARRAY(_packedData);
}

bool UnitTest_MetaData::addNode(uint8 flag, uint8 dataLen, uint8* data)
{
	if(_packedData)
	{
		return false; //we have packed the data, do not allow changes anymore
	}

	Node* pnode = &_node;
	while(pnode->data)
	{
		if(pnode->nextNode == NULL)
		{
			pnode->nextNode = new Node();
			pnode = pnode->nextNode;
			pnode->data = NULL;
			pnode->dataLen = 0;
			pnode->flag = 0;
		}
		else
		{
			pnode = pnode->nextNode;
		}
	}

	pnode->flag = flag;
	pnode->dataLen = dataLen + 1; //dataLen is len(data) + 1 (for flag)
	pnode->data = new uint8[dataLen];
	memcpy(pnode->data, data, dataLen);

	return true;
}

uint8* UnitTest_MetaData::packAndReturn(int& packLen)
{
	if(_packedData)
	{
		packLen = _packedLen;
		return _packedData;
	}

	_packedLen = 0;
	Node* pnode = &_node;
	while((pnode) && (pnode->data))
	{
		_packedLen += 1 + pnode->dataLen; //datalen byte + dataLen (which is 1 (for flag) + length of data)
		pnode = pnode->nextNode;
	}

	_packedData = new uint8[_packedLen];
	pnode = &_node;
	int pos = 0 ;
	while((pnode) && (pnode->data))
	{
		_packedData[pos] = pnode->dataLen;
		pos++;
		_packedData[pos] = pnode->flag;
		pos++;
		memcpy(&(_packedData[pos]), pnode->data, pnode->dataLen - 1);
		pos += pnode->dataLen - 1;

		pnode = pnode->nextNode;
	}

	assert(_packedLen == pos);

	packLen = _packedLen;
	return _packedData;
}