#pragma once

#ifndef _UNITTEST_BLEIO_H_
#define _UNITTEST_BLEIO_H_

#include "BLE_IO.h"
class UnitTest_IO : public IBLE_IO
{
public:
	//Change MyState values to change how this interface will respond.
	//You should change MyState before calling any other methods
	class MyState
	{
		//UnitTest_IO can change these protected values to change current state, however the UnitTest processes cannot
		friend class UnitTest_IO;
	public:
		bool emulateCOMPortOpenFailure;
		bool useNamedPipe;
		bool emulateCOMOwnerKnown;
		bool emulatePipeConnection;
		bool enableAsyncReads;
	protected:
		bool _comPortOpen;
		bool _asyncEnabled;
	}myState;

	class Trace
	{
	public:
		void resetAll();
		bool OpenSerialHandle, CloseSerialHandle, InitializeNamedPipe, CloseNamedPipe, CancelIOEx, ReadMessage, ReleaseCOMPort;
		bool RequestExclusiveAccess, ReleaseExclusiveAccess, ExclusiveAccessStatus, SendMessageOverNP, SetCallback;
		bool COMOwnerKnown, SetCOMWaitHandle, ClientConnectedToNP, output, serverGotData;
		bool InitBLEDevice, StartASyncRead, StopASyncRead;
	}myTrace;

	class MessageStack
	{
	public:
		MessageStack();
		~MessageStack();
		void addMessage(uint8 len1, void* data1, uint16 len2, void* data2);
		void getMessage(uint8& len1, void*& data1, uint16& len2, void*& data2);
		void stop();
	protected:
		class node
		{
		public:
			uint8 len1;
			uint16 len2;
			void* data1;
			void* data2;
			node* nextNode;
		};
		bool _running;
		node* _rootNode;
		HANDLE _mutex;
		HANDLE _hasMessage;
		void lock();
		void unlock();

	}messageStack, outputStack;

	UnitTest_IO();
	virtual ~UnitTest_IO();

	
	virtual int OpenSerialHandle(const char* PORT);
	virtual int CloseSerialHandle();
	virtual int InitializeNamedPipe(const char* NamedPipe, bool isServer);
	virtual int CloseNamedPipe();
	//virtual int CancelIO();
	virtual int CancelIOEx();
	virtual int ReadMessage();
	virtual int ReleaseCOMPort();

	virtual BLE_API_NP_RETURNS RequestExclusiveAccess();
	virtual BLE_API_NP_RETURNS ReleaseExclusiveAccess();
	virtual BLE_API_NP_RETURNS ExclusiveAccessStatus();
	virtual BLE_API_NP_RETURNS SendMessageOverNP(unsigned char* data, int length, int connectionID);
	virtual BLE_API_NP_RETURNS SetCallback(void(*callback) (unsigned char* data, int length, int connectionID));

	virtual int COMOwnerKnown();
	virtual int SetCOMWaitHandle(void* waitHandle);
	virtual bool ClientConnectedToNP();
	virtual void output(uint8 len1, uint8* data, uint16 len2, uint8* data2);
	
	virtual void serverGotData(unsigned char* data, unsigned int length);

	virtual void InitBLEDevice();
	virtual void StartASyncRead();
	virtual void StopASyncRead();


	void inputMessage(uint8 len1, uint8* data, uint16 len2, uint8* data2);
protected:
	NamedPipe* pipe;
};
#endif