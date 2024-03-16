#pragma once

#ifndef _BLE_IO_H_
#define _BLE_IO_H_

#include "BLE_API_Types.h"

#include "BLE_NamedPipe.h"
#include <Windows.h>
#include "BLE_API.h"
#include "MiTokenBLEChild.h"
#include <memory>

/*
#ifdef __cplusplus
extern "C" {
#endif
*/


//Class used by IBLE_IO
class BLE_ASYNC;



//basic IO interface
class IBLE_IO
{
public:
	virtual ~IBLE_IO(); //we need a virtual destructor
	virtual BLE_API_RET OpenSerialHandle(const char* PORT) = 0;
	virtual int CloseSerialHandle() = 0;
	virtual int InitializeNamedPipe(const char* NamedPipe, bool isServer, void (*pipeConnectedCallback) (IMiTokenBLE* instance)) = 0;
	virtual int CloseNamedPipe() = 0;
	//virtual int CancelIO() = 0;
	virtual int CancelIOEx() = 0;
	virtual int ReadMessage() = 0;
	virtual BLE_API_NP_RETURNS ReleaseCOMPort() = 0;

	virtual BLE_API_NP_RETURNS RequestExclusiveAccess() = 0;
	virtual BLE_API_NP_RETURNS ReleaseExclusiveAccess() = 0;
	virtual BLE_API_NP_RETURNS ExclusiveAccessStatus() = 0;
	virtual BLE_API_NP_RETURNS SendMessageOverNP(unsigned char* data, int length, int connectionID) = 0;
	virtual BLE_API_NP_RETURNS SetCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID)) = 0;
	virtual BLE_API_NP_RETURNS DisableBLEForwarding() = 0;

	virtual bool COMOwnerKnown() = 0;
	virtual BLE_API_RET SetCOMWaitHandle(void* waitHandle) = 0;
	virtual bool ClientConnectedToNP() = 0;
	virtual void output(uint8 len1, uint8* data, uint16 len2, uint8* data2) = 0;
	virtual void serverGotData(unsigned char* data, unsigned int length) = 0;

	virtual void InitBLEDevice() = 0;
	virtual void StartASyncRead() = 0;
	virtual void StopASyncRead() = 0;

	virtual std::shared_ptr<NamedPipe> getPipe() = 0;

protected:
	BLE_ASYNC* async;
};

extern IBLE_IO* IO_Interface;

class Basic_BLE_IO : public IBLE_IO
{
public:
	//Basic_BLE_IO();
#ifndef MiTokenBLE_ClasslessMode
	Basic_BLE_IO(MiTokenBLE* parent);
#endif
	virtual ~Basic_BLE_IO();

	virtual BLE_API_RET OpenSerialHandle(const char* PORT);
	virtual int CloseSerialHandle();
	virtual int InitializeNamedPipe(const char* NamedPipe, bool isServer, void (*pipeConnectedCallback) (IMiTokenBLE* instance) = nullptr);
	virtual int CloseNamedPipe();
	//virtual int CancelIO();
	virtual int CancelIOEx();
	virtual int ReadMessage();
	virtual BLE_API_NP_RETURNS ReleaseCOMPort();

	virtual BLE_API_NP_RETURNS RequestExclusiveAccess();
	virtual BLE_API_NP_RETURNS ReleaseExclusiveAccess();
	virtual BLE_API_NP_RETURNS ExclusiveAccessStatus();
	virtual BLE_API_NP_RETURNS SendMessageOverNP(unsigned char* data, int length, int connectionID);
	virtual BLE_API_NP_RETURNS SetCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID));

	virtual BLE_API_NP_RETURNS DisableBLEForwarding();

	virtual bool COMOwnerKnown();
	virtual BLE_API_RET SetCOMWaitHandle(void* waitHandle);
	virtual bool ClientConnectedToNP();
	virtual void output(uint8 len1, uint8* data, uint16 len2, uint8* data2);
	virtual void serverGotData(unsigned char* data, unsigned int length);

	virtual void InitBLEDevice();
	virtual void StartASyncRead();
	virtual void StopASyncRead();

	virtual std::shared_ptr<NamedPipe> getPipe();

protected:
	int readData(unsigned char* buffer, DWORD size, DWORD* read);
	void openCOM(const char* COMPort);
	void closeCOM();
	bool fallbackToCOM;
	
	std::shared_ptr<NamedPipe> pipe;
	HANDLE serial_handle;
	OVERLAPPED ReadOverlap, WriteOverlap;
	HANDLE fileLock;
	

#ifndef MiTokenBLE_ClasslessMode
	MiTokenBLEChild _root;
	//MiTokenBLE * _parent;
#endif
};
/*
int BLE_IO_OpenSerialHandle(const char* PORT);
int BLE_IO_CloseSerialHandle();

int BLE_IO_InitializeNamedPipe(const char* NamedPipe, bool server);
int BLE_IO_CloseNamedPipe();

int BLE_IO_CancelIOEx();
int BLE_IO_ReadMessage();

int BLE_IO_ReleaseCOMPort();

BLE_API_NP_RETURNS BLE_IO_RequestExclusiveAccess();
BLE_API_NP_RETURNS BLE_IO_ReleaseExclusiveAccess();
BLE_API_NP_RETURNS BLE_IO_ExclusiveAccessStatus();

BLE_API_NP_RETURNS BLE_IO_NP_SendMessage(unsigned char * data, int length, int connectionID);
BLE_API_NP_RETURNS BLE_IO_NP_SetCallback(void (*callback) (unsigned char* data, int length, int connectionID));

int BLE_IO_NP_COMOwnerKnown();
int BLE_IO_NP_SetCOMWaitHandle(void* waitHandle);


bool BLE_IO_NP_clientConnected();
*/

#ifdef _DEBUG
extern void (__stdcall *debugDataIn) (int amount);
extern void (__stdcall *debugDataOut) (int amount);
#endif



#ifdef UNIT_TEST
//These functions may be called by UNIT TESTing processes
void Set_IO_Interface(IBLE_IO* newInterface);
void genericOutput(uint8 len1, uint8* data1, uint16 len2, uint8* data2);
void genericServerGotData(unsigned char* data, unsigned int length);
#endif
/*
#ifdef __cplusplus
}
#endif
*/

#endif
