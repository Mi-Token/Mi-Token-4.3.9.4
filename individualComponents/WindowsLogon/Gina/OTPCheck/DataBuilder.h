#pragma once

class DataNode;

class DataBuilder
{
public:
	DataBuilder();
	~DataBuilder();
	void AppendData(void* data, int length);
	bool GetData(void* data, int* length);
private:
	DataNode* firstNode;
	DataNode* lastNode;
};