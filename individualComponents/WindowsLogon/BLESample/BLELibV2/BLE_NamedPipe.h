#pragma once

#ifndef _BLE_NAMED_PIPE_H_
#define _BLE_NAMED_PIPE_H_

#include <Windows.h>

#include "BLE_API_Types.h"
#include "BLE_API.h"
#include "MiTokenBLEChild.h"
#include <memory>



class NamedPipe
{
public:
	void sendBLEMessage(unsigned char* data, int length, unsigned char* data2, unsigned int len2, int handleID = -1);
	void sendCommand(unsigned char* data, int length, int handleID = -1);
	void NamedPipe::sendUserMessage(unsigned char* data, int length, int handleID = -1);

	virtual bool isServer() = 0;
	virtual void close() = 0;

	
	void (*userMessageCallback) (IMiTokenBLE* sender, unsigned char* data, int length, int handleID);
	MiTokenBLEChild getRoot();

	virtual ~NamedPipe();

protected:
	NamedPipe(MiTokenBLEChild root);
	
	virtual bool BLEProcessingRequired() = 0;
	void (*namedPipeBLERead)(NamedPipe* sender, unsigned char* data, unsigned int length);
	HANDLE myPipe;
	void packData(unsigned char* data, int length, unsigned char flag, unsigned char*& outData, unsigned int& outLength);
	virtual void sendData(unsigned char* data, int length, int handleID) = 0;
	void gotBLEMEssage(unsigned char* data, int length);

	void processMessage(unsigned char* message, int length, int pipeHandle);
	virtual void gotCommandMessage(unsigned char* data, int length, int pipeIndex) =0;
	virtual void removeClient(HANDLE clientPipe) =0;
	
	MiTokenBLEChild _root;
	
};

class NamedPipeServer : public NamedPipe
{
public:
	NamedPipeServer(const char* pipeServer, void (*namedPipeBLERead) (NamedPipe* sender, unsigned char* data, unsigned int length), MiTokenBLEChild root);

	const char* getPipeName();

	virtual bool isServer();
	virtual void close();

	bool isExclusive();
	bool isSelfExclusive();

	class ClientData
	{
	public:
		HANDLE pipeHandle;
		HANDLE clientThread;
		OVERLAPPED readOverlapped;
		OVERLAPPED writeOverlapped;
		bool messageClient;
		bool forwardBLE;
	};

	bool clientHasCOM();
	void SetClientCOMReleaseWaitHandle(HANDLE waitHandle);

	bool makeServerExclusive();
	bool releaseServerExclusive();

	~NamedPipeServer();
	void StartListener(std::shared_ptr<NamedPipeServer> mySharedPtr);
protected:

	virtual bool BLEProcessingRequired();
	volatile bool _clientWithCOMKnown;
	volatile HANDLE _clientWithCOMKnownWait;

	ClientData* clientData;
	

	HANDLE* pipeHandles;
	static DWORD WINAPI pipeServerListener(LPVOID param);
	void addNewClient(HANDLE clientPipe, OVERLAPPED* overlapped);
	virtual void removeClient(HANDLE clientPipe);
	
	HANDLE ClientListenerThread;
	HANDLE* clientThreads;
	HANDLE dataMutex;
	int pipeCount;
	int exclusiveMode;
	char* pipeName;

	virtual void sendData(unsigned char* data, int length, int handleID);
	
	virtual void gotCommandMessage(unsigned char* data, int length, int pipeHandle);

	int getIndexFromPipe(HANDLE pipeHandle);
	static DWORD WINAPI pipeClientThread(LPVOID param);

	bool requestExclusiveMode(int pipeIndex);
	bool releaseExclusiveMode(int pipeIndex);
	

};

class NamedPipeClient : public NamedPipe
{
public:
	NamedPipeClient(const char* pipeServer, MiTokenBLEChild root, void (*pipeConnectedCallback) (IMiTokenBLE* instance) = nullptr);
	bool attemptConnection();
	virtual bool isServer();
	bool readNextBlock(unsigned char* buffer, DWORD size, DWORD* read, HANDLE waitHandle);
	bool getNextBlock(unsigned char* buffer, DWORD maxSize, DWORD* size, HANDLE waitHandle);

	virtual void close();

	BLE_API_NP_RETURNS requestExclusiveAccess();
	BLE_API_NP_RETURNS releaseExclusiveAccess();
	BLE_API_NP_RETURNS requestNoBLEForwarding();
	void (*pipeConnectedCallback) (IMiTokenBLE* instance);

	enum ExclusiveAccessStatus
	{
		EXCLUSIVE_DISABLED,
		EXCLUSIVE_ASKED,
		EXCLUSIVE_ALLOWED,
		EXCLUSIVE_DENIED
	};

	ExclusiveAccessStatus exclusiveStatus();
	bool isConnected;
	bool clientBoundToCOM;

	void InformServerOfClosedCOMPort();

	~NamedPipeClient();
protected:

	virtual bool BLEProcessingRequired();
	ExclusiveAccessStatus _exclusiveStatus;
	bool tryConnect;
	bool runPipe;
	char* pipeName;
	void initConnection();
	DWORD asyncConnect(LPVOID param);
	static DWORD WINAPI asyncConnectFunc(LPVOID param);
	HANDLE asyncConnectThread;
	virtual void sendData(unsigned char* data, int length, int handleID);
	virtual void gotCommandMessage(unsigned char* data, int length, int pipeHandle);
	virtual void removeClient(HANDLE clientPipe);
	OVERLAPPED writeOverlapped;

	class DataStore
	{
	public:
		DataStore(MiTokenBLEChild root);
		void addDataToStore(void* data, int length);
		int getNextDataBlockLength();
		bool getDataFromStore(void* data, int maxSize, int& size);
		bool setWaitHandle(HANDLE waitHandle);
		void dispose(void);

	protected:
		
		void lengthenStore(int minimumFreeSpace);
		int storeSize();
		int blockSize();
		int freeSpace();
		unsigned char* storedData;
		int storeStart;
		int storeLength;
		int storeMaxLength;
		HANDLE storeMutex;
		HANDLE waitHandle;
		MiTokenBLEChild _root;
	}myDataStore;


};


#endif