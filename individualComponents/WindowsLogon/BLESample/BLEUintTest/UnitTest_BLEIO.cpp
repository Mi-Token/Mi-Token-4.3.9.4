#include "stdafx.h"

#include "BLE_IO.h"

#include "UnitTest_BLEIO.h"
#include "BLE_LowLevel.h"
#include "BLE_NamedPipe.h"
#include "BLE_ASYNC.h"

#include "cmd_def.h"



#define PIPE_IS_SERVER ((pipe != NULL) && (pipe->isServer()))
#define PIPE_IS_CLIENT ((pipe != NULL) && (!pipe->isServer()))

#define ClientPipe (static_cast<NamedPipeClient*>(pipe))
#define ServerPipe (static_cast<NamedPipeServer*>(pipe))


void UnitTest_IO::Trace::resetAll()
{
	//set all booleans back to false - memset is the easiest way to do it
	memset(this, 0, sizeof(*this));
}

UnitTest_IO::UnitTest_IO()
{
	myState.emulateCOMOwnerKnown = false;
	myState.emulateCOMPortOpenFailure = false;
	myState.useNamedPipe = false;
	myState._comPortOpen = false;
	myState.emulatePipeConnection = false;
	myTrace.resetAll();
	async = new BLE_ASYNC(this);
	pipe = NULL;
}

UnitTest_IO::~UnitTest_IO()
{
	if(myState._asyncEnabled)
	{
		delete async;
		myState._asyncEnabled = false;
	}
}


int UnitTest_IO::OpenSerialHandle(const char* PORT)
{
	myTrace.OpenSerialHandle = true;

	BLE_LL_SetWriteFunction(&genericOutput);
	//just pretend we opened a COM port
	if(myState.emulateCOMPortOpenFailure)
	{
		return -1;
	}
	myState._comPortOpen = true;
	return 0;
}

int UnitTest_IO::CloseSerialHandle()
{
	myTrace.CloseSerialHandle = true;

	myState._comPortOpen = false;
	return 0;
}

int UnitTest_IO::InitializeNamedPipe(const char* NamedPipe, bool isServer)
{
	myTrace.InitializeNamedPipe = true;

	BLE_LL_SetWriteFunction(&genericOutput);

	if(!myState.useNamedPipe)
	{
		return 0;
	}

	if(isServer)
	{
		pipe = new NamedPipeServer(NamedPipe, &genericServerGotData);
	}
	else
	{
		pipe = new NamedPipeClient(NamedPipe);
	}

	return 0;
}

int UnitTest_IO::CloseNamedPipe()
{
	myTrace.CloseNamedPipe = true;

	//TODO : is this correct?
	if(pipe)
	{
		delete pipe;
		pipe = NULL;
	}

	return 0;
}

int UnitTest_IO::CancelIOEx()
{
	myTrace.CancelIOEx = true;

	if(myState.enableAsyncReads)
	{
		messageStack.stop();
	}

	return 1;
}

int UnitTest_IO::ReadMessage()
{
	myTrace.ReadMessage = true;

	if(myState.enableAsyncReads)
	{
		uint8 l1;
		uint16 l2;
		void *d1, *d2;

		messageStack.getMessage(l1, d1, l2, d2);

		//if d1 and d2 are both NULL, than we have been told to shutdown
		if((d1 == NULL) && (d2 == NULL))
		{
			return 0; 
		}

		struct ble_header apihdr ={0};
		unsigned data[256] = {0};

		memcpy(&apihdr, d1, 4);
		if(apihdr.lolen)
		{
			memcpy(&data[0], d2, l2);
		}

		const struct ble_msg* apimsg;
		apimsg = ble_get_msg_hdr(apihdr);
		if(!apimsg)
		{
		}
		else
		{
			apimsg->handler(data);
		}
	}
	return 0;
}

int UnitTest_IO::ReleaseCOMPort()
{
	myTrace.ReleaseCOMPort = true;

	return 0;
}

BLE_API_NP_RETURNS UnitTest_IO::RequestExclusiveAccess()
{
	myTrace.RequestExclusiveAccess = true;

	return BLE_API_NP_BAD_EXCLUSIVE_STATE;
}

BLE_API_NP_RETURNS UnitTest_IO::ReleaseExclusiveAccess()
{
	myTrace.ReleaseExclusiveAccess = true;

	return BLE_API_NP_BAD_EXCLUSIVE_STATE;
}

BLE_API_NP_RETURNS UnitTest_IO::ExclusiveAccessStatus()
{
	myTrace.ExclusiveAccessStatus = true;

	return BLE_API_NP_BAD_EXCLUSIVE_STATE;
}

BLE_API_NP_RETURNS UnitTest_IO::SendMessageOverNP(unsigned char* data, int length, int connectioNID)
{
	myTrace.SendMessageOverNP = true;

	return BLE_API_NP_BAD_EXCLUSIVE_STATE;
}

BLE_API_NP_RETURNS UnitTest_IO::SetCallback(void(*callback) (IMiTokenBLE* sender, unsigned char* data, int length, int connectionID))
{
	myTrace.SetCallback = true;

	return BLE_API_NP_BAD_EXCLUSIVE_STATE;
}

int UnitTest_IO::COMOwnerKnown()
{
	myTrace.COMOwnerKnown = true;

	return myState.emulateCOMOwnerKnown;
}

int UnitTest_IO::SetCOMWaitHandle(void* waitHandle)
{
	myTrace.SetCOMWaitHandle = true;

	return 0;
}

bool UnitTest_IO::ClientConnectedToNP()
{
	myTrace.ClientConnectedToNP = true;

	if(myState.useNamedPipe)
	{
		if(PIPE_IS_SERVER)
		{
			return false;
		}
		if(PIPE_IS_CLIENT)
		{
			return ClientPipe->isConnected;
		}

		return false;
	}

	return myState.emulatePipeConnection;
}

void UnitTest_IO::output(uint8 len1, uint8* data, uint16 len2, uint8* data2)
{
	myTrace.output = true;

	if(myState.enableAsyncReads)
	{
		outputStack.addMessage(len1, data, len2, data2);
	}
}

void UnitTest_IO::serverGotData(unsigned char* data, unsigned int length)
{
	myTrace.serverGotData = true;
}

void UnitTest_IO::InitBLEDevice()
{
	myTrace.InitBLEDevice = true;

	return;
}

void UnitTest_IO::StartASyncRead()
{
	myTrace.StartASyncRead = true;

	if(myState.enableAsyncReads)
	{
		async->Start();
		myState._asyncEnabled = true;
	}
	return;
}

void UnitTest_IO::StopASyncRead()
{
	myTrace.StopASyncRead = true;

	if(myState.enableAsyncReads)
	{
		if(myState._asyncEnabled)
		{
			async->End();
		}
	}
}

void UnitTest_IO::inputMessage(uint8 len1, uint8* data, uint16 len2, uint8* data2)
{
	messageStack.addMessage(len1, data, len2, data2);
}


UnitTest_IO::MessageStack::MessageStack()
{
	_rootNode = NULL;
	_mutex = CreateMutex(NULL, FALSE, NULL);
	_hasMessage = CreateEvent(NULL, TRUE, FALSE, NULL);
	_running = true;
}

UnitTest_IO::MessageStack::~MessageStack()
{
	_running = false;

	while(_rootNode)
	{
		node* pNode = _rootNode;
		_rootNode = _rootNode->nextNode;
		delete [] pNode->data1;
		delete [] pNode->data2;
		delete pNode;
	}

	ReleaseMutex(_mutex);
	CloseHandle(_hasMessage);

}

void UnitTest_IO::MessageStack::addMessage(uint8 len1, void* data1, uint16 len2, void* data2)
{
	lock();
	node* pNode;
	if(_rootNode == NULL)
	{
		_rootNode = new node();
		pNode = _rootNode;
	}
	else
	{
		pNode = _rootNode;
		while(pNode->nextNode)
		{
			pNode = pNode->nextNode;
		}

		pNode->nextNode = new node();
		pNode = pNode->nextNode;
	}
	pNode->nextNode = NULL;
	pNode->len1 = len1;
	pNode->len2 = len2;
	pNode->data1 = new uint8[len1];
	pNode->data2 = new uint8[len2];
	memcpy(pNode->data1, data1, len1);
	memcpy(pNode->data2, data2, len2);

	SetEvent(_hasMessage);
	unlock();
}

void UnitTest_IO::MessageStack::getMessage(uint8& len1, void*& data1, uint16& len2, void*& data2)
{
	lock();
	if(_rootNode == NULL)
	{
		ResetEvent(_hasMessage);
		unlock();
		WaitForSingleObject(_hasMessage, INFINITE);
		lock();
	}

	if(!_running)
	{
		len1 = 0;
		len2 = 0 ;
		data1 = data2 = NULL;
		return;
	}

	node* pNode = _rootNode;
	_rootNode = _rootNode->nextNode;
	len1 = pNode->len1;
	len2 = pNode->len2;
	data1 = pNode->data1;
	data2 = pNode->data2;
	delete pNode;
	unlock();
}

void UnitTest_IO::MessageStack::stop()
{
	lock();
	_running = false;
	SetEvent(_hasMessage);
	unlock();
}

void UnitTest_IO::MessageStack::lock()
{
	WaitForSingleObject(this->_mutex, INFINITE);
}

void UnitTest_IO::MessageStack::unlock()
{
	ReleaseMutex(this->_mutex);
}