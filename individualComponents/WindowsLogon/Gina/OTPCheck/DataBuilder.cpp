#include "DataBuilder.h"
#include <stdlib.h>
#include <string>

class DataNode
{
public: 
	void* data;
	int length;
	DataNode* nextNode;

	DataNode()
	{
		data = NULL;
		length = 0;
		nextNode = NULL;
	}
};

DataBuilder::DataBuilder()
{
	this->firstNode = new DataNode();
	this->lastNode = this->firstNode;
}

DataBuilder::~DataBuilder()
{
	DataNode* node = this->firstNode;
	void* cnode;
	while(node->nextNode != NULL)
	{
		free(node->data);
		cnode = node;
		node = node->nextNode;
		free(cnode);
	}
	free(node);
}

void DataBuilder::AppendData(void* data, int length)
{
	this->lastNode->data = malloc(length);
	memcpy(this->lastNode->data, data, length);
	this->lastNode->length = length;
	this->lastNode->nextNode = new DataNode();
	this->lastNode = this->lastNode->nextNode;
}

bool DataBuilder::GetData(void* data, int* length)
{
	if(length == NULL)
		return false; //nothing we can do

	

	//get the size and set length to it
	int tlen = 0;
	DataNode* pNode = this->firstNode;
	while(pNode->nextNode != NULL)
	{
		tlen += pNode->length;
		pNode = pNode->nextNode;
	}

	

	if(data == NULL)
	{
		*length = tlen;
		//we need to return the lenght
		return false;
	}

	if(*length < tlen)
	{
		*length = tlen;
		return false;
	}

	*length = tlen;
	tlen = 0;


    pNode = this->firstNode;
	while(pNode->nextNode != NULL)
	{
		memcpy((char*)data + tlen, pNode->data, pNode->length);
		tlen += pNode->length;
		pNode = pNode->nextNode;
	}

	return true;
}