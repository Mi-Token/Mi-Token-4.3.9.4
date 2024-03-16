#pragma comment(lib, "advapi32.lib")


#include "BLE_NamedPipe.h"
#include <stdio.h>
#include <stdlib.h>

#include "BLE_Structs.h"

#include <assert.h>

#include <Windows.h>
#include <AclAPI.h>

#include "DebugLogging.h"
#include "CurrentState.h"
#include "MiTokenBLE.h"

#include "GenericUtils.h"

#if USE_MEM_CHECK
#include "MainLib.h"
#endif

#define MESSAGE_FLAG_NORMAL (0x00)
#define MESSAGE_FLAG_BLE (0x01)
#define MESSAGE_FLAG_USER (0x02)

#define PIPE_BUFFER (4096)

#define COMMAND_FLAG_EXCLUSIVE (0x01)
#define EXCLUSIVE_MODE_ENABLE (0x00)
#define EXCLUSIVE_MODE_DISABLE (0x01)
#define EXCLUSIVE_MODE_UNABLE (0x02)

#define COMMAND_FLAG_CLIENT_COM (0x02)
#define CLIENT_COM_MODE_INUSE (0x00)
#define CLIENT_COM_MORE_DONE (0x01)

#define COMMAND_FLAG_DONT_FORWARD_BLE (0x03)


#define COMMAND_FLAG_DONT_FORWARD_BLE (0x03)


#define VALID_HANDLE(p) (( p != NULL) && (p != INVALID_HANDLE_VALUE))

MiTokenBLEChild NamedPipe::getRoot()
{
	return _root;
}

void NamedPipe::sendBLEMessage(unsigned char* data, int length, unsigned char* data2, unsigned int len2, int handleID)
{
	DEBUG_ENTER_FUNCTION;

	if(!BLEProcessingRequired())
	{
		DEBUG_END_FUNCTION;
	}

	unsigned char* newData;
	unsigned int newLength;

	int tempLength = (length + len2 + 8);
	unsigned char* tempData = new unsigned char[tempLength];

	memcpy(&tempData[0], &length, 4);
	memcpy(&tempData[4], data, length);
	memcpy(&tempData[4 + length], &len2, 4);
	memcpy(&tempData[8 + length], data2, len2);

#if USE_MEM_CHECK
	MemLeak_SetLocationID(500);
#endif

	packData(tempData, tempLength, MESSAGE_FLAG_BLE, newData, newLength);

#if USE_MEM_CHECK
	MemLeak_SetLocationID(501);
#endif

	sendData(newData, newLength, handleID);

#if USE_MEM_CHECK
	MemLeak_SetLocationID(502);
#endif


	delete[] tempData;
	delete[] newData;

	DEBUG_END_FUNCTION;
}

void NamedPipe::sendCommand(unsigned char* data, int length, int handleID)
{
	DEBUG_ENTER_FUNCTION;

	unsigned char* newData;
	unsigned int newLength;

	packData(data, length, MESSAGE_FLAG_NORMAL, newData, newLength);
	sendData(newData, newLength, handleID);

	delete[] newData;

	DEBUG_END_FUNCTION;
}

void NamedPipe::sendUserMessage(unsigned char* data, int length, int handleID)
{
	DEBUG_ENTER_FUNCTION;

	unsigned char* newData;
	unsigned int newLength;

	packData(data, length, MESSAGE_FLAG_USER, newData, newLength);
	sendData(newData, newLength, handleID);

	delete[] newData;

	DEBUG_END_FUNCTION;
}

void NamedPipe::packData(unsigned char* data, int length, unsigned char flag, unsigned char*& outData, unsigned int& outLength)
{
	DEBUG_ENTER_FUNCTION;

	outLength = length + 5; //4 byte length + 1 byte flag + data
	outData = new unsigned char[outLength];
	memcpy(outData, &outLength, 4);
	outData[4] = flag;
	memcpy(outData + 5, data, length);

	DEBUG_END_FUNCTION;
}

void NamedPipeClient::sendData(unsigned char* data, int dataLength, int handleID)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_LOG("Sending message over NamedPipe of length %d\r\n\t", dataLength);
	DEBUG_BYTES(data, dataLength);
	DEBUG_LOG("\r\n");

	if((myPipe != NULL) && ( myPipe != INVALID_HANDLE_VALUE))
	{
		DWORD bytesWritten;
		BOOL fSuccess = WriteFile(myPipe,
									data,
									dataLength,
									&bytesWritten,
									&writeOverlapped);
		if(!fSuccess)
		{
			//The pipe failed for some reason
			int lastError = GetLastError();
			DEBUG_LOG("The pipe failed, GLE = %d\r\n", lastError);
			printf("The pipe failed, GLE = %d\r\n", lastError);
			if(lastError == ERROR_IO_PENDING)
			{
				DEBUG_LOG("Waiting on Pipe to complete\r\n");
				if(GetOverlappedResult(myPipe, &writeOverlapped, &bytesWritten, TRUE) == false)
				{
					lastError = GetLastError();
					DEBUG_LOG("Writing to the Pipe failed with GLE = %d\r\n", lastError);
				}
				else
				{
					fSuccess = true;
				}
			}
		}
		else
		{
			//printf("The pipe succeeded (C->%d)\r\n", dataLength);
		}
	}
	
	DEBUG_END_FUNCTION;
}

bool NamedPipeClient::BLEProcessingRequired()
{
	//clients don't ever forward BLE Messages
	return false;
}

bool NamedPipeServer::BLEProcessingRequired()
{
	if(clientData == nullptr)
	{
		return false;
	}

	for(int pid = 0; pid < pipeCount; ++pid)
	{
		if(clientData[pid].forwardBLE)
		{
			return true;
		}
	}

	return false;
}
void NamedPipeServer::sendData(unsigned char* data, int dataLength, int handleID)
{
	DEBUG_ENTER_FUNCTION;

	if(clientData != NULL)
	{
		int minPipeID, maxPipeID;
		if(handleID == -1)
		{
			if(data[4] == MESSAGE_FLAG_BLE) //exclusive mode only matters for BLE Messages, all others will always be sent through
			{
				if(exclusiveMode == -1)
				{
					minPipeID = 0;
					maxPipeID = pipeCount;
				}
				else if(exclusiveMode == -2)
				{
					//we cannot actually send this data, so send from 0 to -1 (which will not send it to anyone)
					minPipeID = 0;
					maxPipeID = -1;
				}
				else
				{
					minPipeID = exclusiveMode;
					maxPipeID = minPipeID + 1;
				}
			}
			else
			{
				minPipeID = 0;
				maxPipeID = pipeCount;
			}

		}
		else
		{
			minPipeID = handleID;
			maxPipeID = minPipeID + 1;
		}

		for(int pipeID = minPipeID;  pipeID < maxPipeID; ++pipeID)
		{
			if((clientData[pipeID].pipeHandle != INVALID_HANDLE_VALUE) && (clientData[pipeID].messageClient))
			{
				if ((data[4] == MESSAGE_FLAG_BLE) && (clientData[pipeID].forwardBLE == false))
				{
					//we are sending through a BLE message, and this client doesn't want BLE Messages.
					continue;
				}

				DWORD bytesWritten;
				BOOL fSuccess = WriteFile(clientData[pipeID].pipeHandle,
											data,
											dataLength,
											&bytesWritten,
											&clientData[pipeID].writeOverlapped);
				if (!fSuccess)
				{
					//The pipe failed for some reason
					int lastError = GetLastError();
					DEBUG_LOG("The pipe failed, GLE = %d\r\n", lastError);
					printf("The pipe failed, GLE = %d\r\n", lastError);
					if (lastError == ERROR_IO_PENDING)
					{
						DEBUG_LOG("Waiting on Pipe to complete\r\n");
						if (GetOverlappedResult(myPipe, &clientData[pipeID].writeOverlapped, &bytesWritten, TRUE) == false)
						{
							lastError = GetLastError();
							DEBUG_LOG("Writing to the Pipe failed with GLE = %d\r\n", lastError);
						}
						else
						{
							fSuccess = true;
						}
					}
					else if (lastError == ERROR_NO_DATA)
					{
						//the pipe is being closed
						clientData[pipeID].messageClient = false;
						clientData[pipeID].forwardBLE = false;
					}
					else if (lastError == ERROR_INVALID_HANDLE)
					{
						clientData[pipeID].pipeHandle = INVALID_HANDLE_VALUE;
						clientData[pipeID].messageClient = false;
						clientData[pipeID].forwardBLE = false;
					}
				}
				else
				{
					//printf("The pipe succeeded (S->%d)%s\r\n", dataLength, (!fSuccess ? " [ASYNC]" : ""));
				}
			}
		}
	}
	DEBUG_END_FUNCTION;
}

DWORD WINAPI NamedPipeServer::pipeServerListener(LPVOID param)
{
	DEBUG_ENTER_FUNCTION;
	std::shared_ptr<NamedPipeServer>* pPtr = (std::shared_ptr<NamedPipeServer>*)param;
	std::shared_ptr<NamedPipeServer> server = pPtr[0];
	pPtr[0] = nullptr;
	delete[] pPtr;

	//NamedPipeServer* server = (NamedPipeServer*)param;
	MiTokenBLEChild root = MiTokenBLEChild(server->_root);
	
	//Create security desc for the named pipe
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pACL = NULL;
	SECURITY_ATTRIBUTES sa;
	bool useSA = false;

	
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if(NULL == pSD)
	{
		//oh no :/
	}
	else
	{
		if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			//more bad stuff
		}
		if(!SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE))
		{
			//again bad
		}
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = pSD;
		sa.bInheritHandle = FALSE;
		useSA = true;
	}

	OVERLAPPED overlap = {0};
	overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while(root.GetRoot()->GetState()->shouldAbort() == FALSE)
	{
		DEBUG_FUNCTION_SECTION;
		HANDLE myPipe = INVALID_HANDLE_VALUE;
		myPipe = CreateNamedPipeA(server->getPipeName(),
									PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
									PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
									PIPE_UNLIMITED_INSTANCES,
									PIPE_BUFFER,
									PIPE_BUFFER,
									0,
									(useSA ? &sa : NULL));

		if(myPipe == INVALID_HANDLE_VALUE)
		{
			printf("Pipe server failed to start up. GLE = %d\r\n",GetLastError());
			DEBUG_RETURN 0;
		}

		BOOL fconnected = ConnectNamedPipe(myPipe, &overlap);

		if (fconnected == FALSE)
		{
			int lastError = GetLastError();
			DEBUG_FUNCTION_SECTION;
			if (lastError == ERROR_IO_PENDING)
			{
				if(root.GetRoot()->GetState()->waitOnEvents(overlap.hEvent) == FALSE)
				{
					//We should abort
					fconnected = false;
					CloseHandle(myPipe);
				}
				else
				{
					fconnected = HasOverlappedIoCompleted(&overlap);
					DEBUG_FUNCTION_SECTION;
				}
			}
			else if (lastError == ERROR_PIPE_CONNECTED)
			{
				fconnected = TRUE;
				//fconnected = HasOverlappedIoCompleted(&overlap);
				DEBUG_FUNCTION_SECTION;
			}
			else
			{
				printf("Error : ConnectNamePipe returned error code %d", lastError);
				DEBUG_VERBOSE("Error : ConnectNamePipe returned error code %d", lastError);
			}
		}
		

		if(fconnected)
		{
			DEBUG_FUNCTION_SECTION;
			printf("A Client has connected to the NamedPipe\r\n");
			server->addNewClient(myPipe, NULL /*Don't pass down the overlapped structure - we will use it only for this function*/);
		}
	}

	if(pACL)
	{
		LocalFree(pACL);
	}
	if(pSD)
	{
		LocalFree(pSD);
	}

	DEBUG_END_FUNCTION;
}

struct ClientThreadParam
{
	NamedPipeServer* pServer;
	int clientID;
	NamedPipeServer::ClientData* pClientData;

	ClientThreadParam(NamedPipeServer* pServer, int clientID, NamedPipeServer::ClientData* clientData)
	{
		this->pServer = pServer;
		this->clientID = clientID;
		this->pClientData = clientData;
	}
};

void NamedPipeServer::addNewClient(HANDLE clientPipe, OVERLAPPED* overlapped)
{
	DEBUG_ENTER_FUNCTION;


	
	if(_root.GetRoot()->GetState()->waitOnEvents(dataMutex) == FALSE)
	{
		//we need to abort - Don't make a new client
		DEBUG_END_FUNCTION;
		return;
	}


	DEBUG_FUNCTION_SECTION;
	bool found = false;
	for(int i = 0 ; ((i < pipeCount) && (!found)) ; ++i)
	{
		DEBUG_FUNCTION_SECTION;
		if(clientData[i].pipeHandle == INVALID_HANDLE_VALUE)
		{
			clientData[i].pipeHandle = clientPipe;
			ClientThreadParam* pClientParam = new ClientThreadParam(this, i, &(clientData[i]));
			clientData[i].messageClient = true;
			clientData[i].forwardBLE = true;
			clientData[i].clientThread = CreateNamedThread(NULL, 0, pipeClientThread, pClientParam, 0, NULL, "NSP: ClientThread");
			found = true;
		}
	}

	if(!found)
	{
		DEBUG_FUNCTION_SECTION;
		NamedPipeServer::ClientData* clients = new NamedPipeServer::ClientData[pipeCount + 1];
		memcpy(clients, clientData, sizeof(NamedPipeServer::ClientData) * pipeCount);

		if(clientData != NULL)
		{
			delete[] clientData;
		}
		
		clientData = clients;
		clients = NULL;

		NamedPipeServer::ClientData* myCD = &(clientData[pipeCount]);
		
		myCD->pipeHandle = clientPipe;
		ClientThreadParam* pClientParam = new ClientThreadParam(this, pipeCount, myCD);
		memset(&myCD->readOverlapped, 0, sizeof(OVERLAPPED));
		memset(&myCD->writeOverlapped, 0, sizeof(OVERLAPPED));

		myCD->readOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		myCD->writeOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		
		myCD->messageClient = true;
		myCD->forwardBLE = true;
		myCD->clientThread = CreateNamedThread(NULL, 0, pipeClientThread, pClientParam, 0, NULL, "NSP : ClientThread");

		pipeCount++;
	}
	ReleaseMutex(dataMutex);

	DEBUG_END_FUNCTION;
}

struct PipeClientThread
{
	NamedPipe* pClass;
	HANDLE pipeHandle;
};


DWORD WINAPI NamedPipeServer::pipeClientThread(LPVOID param)
{
	DEBUG_ENTER_FUNCTION;

	ClientThreadParam* threadParam = (ClientThreadParam*)param;

	HANDLE heap = GetProcessHeap();
	unsigned char* request = (unsigned char*)HeapAlloc(heap, 0, PIPE_BUFFER);
	unsigned char* reply = (unsigned char*)HeapAlloc(heap, 0, PIPE_BUFFER);

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	if(threadParam == NULL)
	{
		printf("Error : Did not pass on ClientThreadParam\r\n");
		DEBUG_RETURN (DWORD)-1;
	}
	if((request == NULL) || (reply == NULL))
	{
		printf("Erorr : Failed to allocate heaps\r\n");
		if(request != NULL) { HeapFree(heap, 0, request); }
		if(reply != NULL) { HeapFree(heap, 0, reply); }

		DEBUG_RETURN (DWORD)-1;
	}

	hPipe = threadParam->pClientData->pipeHandle;

	OVERLAPPED overlapped = threadParam->pClientData->readOverlapped;
	MiTokenBLEChild root = MiTokenBLEChild(threadParam->pServer->_root);

	while(root.GetRoot()->GetState()->shouldAbort() == FALSE)
	{
		DEBUG_FUNCTION_SECTION;
		DEBUG_VERBOSE("ReadFile Params (handle %d)\r\n", hPipe);
		fSuccess = ReadFile(hPipe,
							request,
							PIPE_BUFFER,
							&cbBytesRead,
							&overlapped);
		if(!fSuccess)
		{
			int lastError = GetLastError();
			DEBUG_FUNCTION_SECTION;
			if (lastError == ERROR_BROKEN_PIPE)
			{
				DEBUG_FUNCTION_SECTION;
				printf("Client Disconnected\r\n");
				threadParam->pServer->removeClient(hPipe);
				break;
			}
			else if (lastError == ERROR_IO_PENDING)
			{
				if(root.GetRoot()->GetState()->waitForOverlappedIO(hPipe, &overlapped, cbBytesRead) == FALSE)
				{
					fSuccess = false;
				}
				else
				{
					fSuccess = true;
				}
				
				/*
				try
				{
					DEBUG_FUNCTION_SECTION;
					if(GetOverlappedResult(hPipe, &overlapped, &cbBytesRead, TRUE) == false)
					{
						lastError = GetLastError();
						DEBUG_FUNCTION_SECTION;
						printf("Error Reading from client : GLE = %d\r\n", lastError);
						DEBUG_VERBOSE("Error Reading from client : GLE = %d\r\n", lastError);
					}
					else
					{
						DEBUG_FUNCTION_SECTION;
						//we managed to do the delayed read
						fSuccess = TRUE;
					}

					DEBUG_FUNCTION_SECTION;
				}
				catch(...)
				{
					DEBUG_FUNCTION_SECTION;
				}
				*/

			}
			else
			{
				DEBUG_VERBOSE("InstanceThread READFILE failed. GLE = %d (CB = %d)\r\n", lastError, cbBytesRead);
				printf("InstanceThread READFILE failed. GLE = %d\r\n", lastError);
				threadParam->pServer->removeClient(hPipe);
				break;
			}
			
		}
		

		if (fSuccess)
		{
			//printf("The pipe succeeded (S<-%d)\r\n", cbBytesRead);

			if(cbBytesRead > 0)
			{
				DEBUG_FUNCTION_SECTION;
				unsigned char* buffer = new unsigned char[cbBytesRead];
				memcpy(buffer, request, cbBytesRead);
				threadParam->pServer->processMessage(buffer, cbBytesRead, threadParam->pServer->getIndexFromPipe(hPipe));
				delete[] buffer;
			}
			else
			{
				DEBUG_FUNCTION_SECTION;
			}
		}
	}

	DEBUG_END_FUNCTION;
}

void NamedPipe::processMessage(unsigned char* message, int length, int pipeHandle)
{
	DEBUG_ENTER_FUNCTION;

	int msgLength;
	memcpy(&msgLength, message, 4);
	if(msgLength < length)
	{
		printf("Error : Message length should be %d bytes, only got %d bytes\r\n", msgLength, length);
		DEBUG_END_FUNCTION;
	}
	unsigned char flag = message[4];
	if(flag == MESSAGE_FLAG_BLE)
	{
		gotBLEMEssage(message + 5, length - 5);
	}
	else if(flag == MESSAGE_FLAG_NORMAL)
	{
		gotCommandMessage(message + 5, length - 5, pipeHandle);
	}
	else if(flag == MESSAGE_FLAG_USER)
	{
		if(userMessageCallback != NULL)
		{
			//Keep a handle to the IMiTokenBLE interface while we are calling out with it
			MiTokenBLEChild troot = _root;
			userMessageCallback(troot.GetRoot(), message + 5, length - 5, pipeHandle);
		}
	}
	else
	{
		printf("Error : Invalid Flag ID of %d\r\n", flag);
	}

	DEBUG_END_FUNCTION;
}

BLE_API_NP_RETURNS NamedPipeClient::requestExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	if(_exclusiveStatus == EXCLUSIVE_ALLOWED)
	{
		DEBUG_RETURN BLE_API_NP_BAD_EXCLUSIVE_STATE;
	}

	unsigned char command[] = {COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_ENABLE};

	sendCommand(command, sizeof(command));

	_exclusiveStatus = EXCLUSIVE_ASKED;

	DEBUG_RETURN BLE_API_NP_SUCCESS;
}

BLE_API_NP_RETURNS NamedPipeClient::releaseExclusiveAccess()
{
	DEBUG_ENTER_FUNCTION;

	if(_exclusiveStatus != EXCLUSIVE_ALLOWED)
	{
		DEBUG_RETURN BLE_API_NP_BAD_EXCLUSIVE_STATE;
	}

	unsigned char command[] = {COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_DISABLE};

	sendCommand(command, sizeof(command));

	_exclusiveStatus = EXCLUSIVE_ASKED;

	DEBUG_RETURN BLE_API_NP_SUCCESS;
}

BLE_API_NP_RETURNS NamedPipeClient::requestNoBLEForwarding()
{
	DEBUG_ENTER_FUNCTION;

	unsigned char command[] = { COMMAND_FLAG_DONT_FORWARD_BLE };
	sendCommand(command, sizeof(command));

	DEBUG_RETURN BLE_API_NP_SUCCESS;
}


void NamedPipe::gotBLEMEssage(unsigned char* message, int length)
{
	DEBUG_ENTER_FUNCTION;

	if(namedPipeBLERead != NULL)
	{
		namedPipeBLERead(this, message, length);
	}

	DEBUG_END_FUNCTION;
}

bool NamedPipeServer::makeServerExclusive()
{
	bool locked = false;
	if(_root.GetRoot()->GetState()->waitOnEvents(dataMutex) == FALSE)
	{
		return false;
	}
	if(exclusiveMode == -1)
	{
		exclusiveMode = -2;
		locked = true;
	}

	ReleaseMutex(dataMutex);
	return locked;
}

bool NamedPipeServer::releaseServerExclusive()
{
	bool unlocked = false;
	if(_root.GetRoot()->GetState()->waitOnEvents(dataMutex) == FALSE)
	{
		return false;
	}
	if(exclusiveMode == -2)
	{
		exclusiveMode = -1;
		unlocked = true;
	}

	ReleaseMutex(dataMutex);
	return unlocked;
}

bool NamedPipeServer::requestExclusiveMode(int pipeIndex)
{
	bool locked = false;
	if(_root.GetRoot()->GetState()->waitOnEvents(dataMutex) == FALSE)
	{
		return false;
	}
	if(exclusiveMode == -1)
	{
		exclusiveMode = pipeIndex;
		locked = true;
	}
	ReleaseMutex(dataMutex);

	return locked;
}

bool NamedPipeServer::releaseExclusiveMode(int pipeIndex)
{
	bool unlocked = false;
	if(_root.GetRoot()->GetState()->waitOnEvents(dataMutex) == FALSE)
	{
		return false;
	}
	if(exclusiveMode == pipeIndex)
	{
		exclusiveMode = -1;
		unlocked = true;
	}
	ReleaseMutex(dataMutex);

	return unlocked;
}

void NamedPipeServer::gotCommandMessage(unsigned char* message, int length, int pipeIndex)
{
	DEBUG_ENTER_FUNCTION;

	if(length <= 0)
	{
		printf("Error : gotCommandMessage with length 0\r\n");
		DEBUG_END_FUNCTION;
	}

	unsigned char messageFlag = message[0];

	switch(messageFlag)
	{
	case COMMAND_FLAG_EXCLUSIVE:
		{
			if(length != 2)
			{
				printf("Error : Got COMMAND;EXCLUSIVE message with invalid length of %d, expected %d\r\n", length, 2);
				DEBUG_END_FUNCTION;
			}
			unsigned char mode = message[1];
			switch(mode)
			{
			case EXCLUSIVE_MODE_ENABLE:
				{
					printf("EME : Requested for port %d\r\n", pipeIndex);
					bool unableToLock = false;
					
					unableToLock = !requestExclusiveMode(pipeIndex);
					/*
					WaitForSingleObject(dataMutex, INFINITE);
					if(exclusiveMode == -1)
					{
						exclusiveMode = pipeIndex;
					}
					else
					{
						unableToLock = true;
					}
					ReleaseMutex(dataMutex);
					*/

					if(unableToLock)
					{
						unsigned char commandBlock[] = { COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_UNABLE};
						sendCommand(commandBlock, sizeof(commandBlock), pipeIndex);
					}
					else
					{
						unsigned char commandBlock[] = {COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_ENABLE};
						sendCommand(commandBlock, sizeof(commandBlock), pipeIndex);
					}
				}
				break;
			case EXCLUSIVE_MODE_DISABLE:
				{
					printf("EMD : Requested for port %d\r\n", pipeIndex);
					bool unableToUnlock = false;

					unableToUnlock = !releaseExclusiveMode(pipeIndex);

					/*
					WaitForSingleObject(dataMutex, INFINITE);
					if(exclusiveMode == pipeIndex)
					{
						exclusiveMode = -1;
					}
					else
					{
						unableToUnlock = true;
					}
					ReleaseMutex(dataMutex);
					*/
					
					unsigned char lockedBlock[] = {COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_DISABLE};
					unsigned char failedBlock[] = {COMMAND_FLAG_EXCLUSIVE, EXCLUSIVE_MODE_UNABLE};


					//if this assert fails, you need to change how the following sendCommand works
					assert(sizeof(lockedBlock) == sizeof(failedBlock));

					sendCommand((unableToUnlock ? failedBlock : lockedBlock), sizeof(lockedBlock), pipeIndex);
				}
				break;
			}
		}
	case COMMAND_FLAG_CLIENT_COM:
		{
			if(length != 2)
			{
				printf("Error : Got COMMAND;CLIENT_COM message with invalid length of %d, expected %d\r\n", length, 2);
				DEBUG_END_FUNCTION;
			}
			unsigned char mode = message[1];
			switch(mode)
			{
			case CLIENT_COM_MODE_INUSE:
				//a client has the COM port, and we know who it is
				_clientWithCOMKnown = true;
				break;
			case CLIENT_COM_MORE_DONE:
				//the client who had the COM port is done with it, so we can now get access to it
				_clientWithCOMKnown = false;
				if(_clientWithCOMKnownWait != NULL)
				{
					SetEvent(_clientWithCOMKnownWait);
				}
				break;
			}
		}
	case COMMAND_FLAG_DONT_FORWARD_BLE:
		{
			clientData[pipeIndex].forwardBLE = false;
		}
	}
	DEBUG_END_FUNCTION;
}




NamedPipeServer::NamedPipeServer(const char* pipeServer, void(*namedPipeBLERead) (NamedPipe* sender, unsigned char* data, unsigned int length), MiTokenBLEChild root) : NamedPipe(root), pipeName(nullptr)
{
#if USE_MEM_CHECK
	MemLeak_SetLocationID(120);
#endif

	DEBUG_ENTER_FUNCTION;
	
	ClientListenerThread = NULL;
	dataMutex = CreateMutex(NULL, FALSE, NULL);
	pipeCount = 0;	
	clientThreads = NULL;
	pipeHandles = NULL;
	exclusiveMode = -1;
	clientData = NULL;

	int pipeNameLen = strlen(pipeServer) + 1;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(121);
#endif
	
	pipeName = new char[pipeNameLen];

#if USE_MEM_CHECK
	MemLeak_SetLocationID(122);
#endif

	memcpy(pipeName, pipeServer, pipeNameLen);

	this->namedPipeBLERead = namedPipeBLERead;
	this->_clientWithCOMKnown = false;

#if USE_MEM_CHECK
	MemLeak_SetLocationID(123);
#endif

}

void NamedPipeServer::StartListener(std::shared_ptr<NamedPipeServer> mySharedPtr)
{
	std::shared_ptr<NamedPipeServer>* pPtr = new std::shared_ptr<NamedPipeServer>[1];
	pPtr[0] = mySharedPtr;
	ClientListenerThread = CreateNamedThread(NULL, 0, pipeServerListener, pPtr, 0, NULL, "NPS : Listener");
	if(ClientListenerThread == NULL)
	{
		printf("Pipe server listener thread failed. GLE = %d\r\n", GetLastError());
	}
	DEBUG_END_FUNCTION;

}

bool NamedPipeClient::isServer()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN false;
}

bool NamedPipeServer::isServer()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN true;
}

int NamedPipeServer::getIndexFromPipe(HANDLE pipeHandle)
{
	DEBUG_ENTER_FUNCTION;

	for(int i = 0 ; i < pipeCount ; ++i)
	{
		if(clientData[i].pipeHandle == pipeHandle)
		{
			DEBUG_RETURN i;
		}
	}

	DEBUG_RETURN -1;
}

void NamedPipeServer::removeClient(HANDLE clientPipe)
{
	DEBUG_ENTER_FUNCTION;

	int index = getIndexFromPipe(clientPipe);
	if(index != -1)
	{
		clientData[index].messageClient = false;
		clientData[index].forwardBLE = false;

		//Release Exclusive Mode incase the Client still had it.
		releaseExclusiveMode(index);

		try
		{
			CloseHandle(clientData[index].pipeHandle);
		}
		catch (...)
		{}


		clientData[index].clientThread = INVALID_HANDLE_VALUE;
		clientData[index].pipeHandle = INVALID_HANDLE_VALUE;
		//TerminateThread(clientData[index].clientThread, 0);
	}
	
	DEBUG_END_FUNCTION;
}


const char* NamedPipeServer::getPipeName()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN pipeName;
}

//TODO : Actually impliment these

NamedPipeClient::NamedPipeClient(const char* NamedPipe, MiTokenBLEChild root, void (*pipeConnectedCallback) (IMiTokenBLE* instance)) : NamedPipe(root), myDataStore(root), pipeName(nullptr), asyncConnectThread(INVALID_HANDLE_VALUE)
{
	DEBUG_ENTER_FUNCTION;
	this->pipeConnectedCallback = pipeConnectedCallback;
	int pipeLen = strlen(NamedPipe) + 1;
	pipeName = new char[pipeLen];
	memcpy(pipeName, NamedPipe, pipeLen);
	memset(&writeOverlapped, 0, sizeof(writeOverlapped));

	initConnection();

	clientBoundToCOM = false;
	DEBUG_END_FUNCTION;
}

void NamedPipeClient::initConnection()
{
	DEBUG_ENTER_FUNCTION;

	tryConnect = true;
	runPipe = true;
	isConnected = false;
	if(!VALID_HANDLE(writeOverlapped.hEvent))
	{
		writeOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	_exclusiveStatus = EXCLUSIVE_DISABLED;
	asyncConnectThread = CreateNamedThread(NULL, 0, NamedPipeClient::asyncConnectFunc, this, 0, NULL, "NPC : ASyncConnect");

	DEBUG_END_FUNCTION;
}
NamedPipeClient::DataStore::DataStore(MiTokenBLEChild root) : _root(root), storedData(nullptr)
{
	DEBUG_ENTER_FUNCTION;
	storedData = new unsigned char[PIPE_BUFFER * 2];
	DEBUG_FUNCTION_SECTION;
	storeStart = 0;
	DEBUG_FUNCTION_SECTION;
	storeLength = 0;
	DEBUG_FUNCTION_SECTION;
	storeMaxLength = PIPE_BUFFER * 2;
	DEBUG_FUNCTION_SECTION;
	storeMutex = CreateMutex(NULL, FALSE, NULL);
	DEBUG_FUNCTION_SECTION;
	waitHandle = NULL;
	DEBUG_FUNCTION_SECTION;

	DEBUG_END_FUNCTION;
}

void NamedPipeClient::DataStore::addDataToStore(void* data, int length)
{
	DEBUG_ENTER_FUNCTION;

	if(_root.GetRoot()->GetState()->waitOnEvents(storeMutex) == FALSE)
	{
		DEBUG_END_FUNCTION;
		return;
	}

	DEBUG_VERBOSE("Storing %d bytes in the Client DataStore\r\n", length);

	int freeSpace = this->freeSpace();

	if(freeSpace < length)
	{
		lengthenStore(freeSpace);
	}

	memcpy(&storedData[storeLength], data, length);
	storeLength += length;

	if(waitHandle && blockSize())
	{
		//we have another block, so trigger the wait handle
		SetEvent(waitHandle);
		//we triggered the handle, so remove it
		waitHandle = NULL;
	}
	
	ReleaseMutex(storeMutex);

	DEBUG_END_FUNCTION;
}

int NamedPipeClient::DataStore::getNextDataBlockLength()
{
	DEBUG_ENTER_FUNCTION;

	int len = 0;
	if(_root.GetRoot()->GetState()->waitOnEvents(storeMutex) == FALSE)
	{
		DEBUG_RETURN 0;
	}

	len = blockSize();
	ReleaseMutex(storeMutex);

	DEBUG_RETURN len;
}

bool NamedPipeClient::DataStore::getDataFromStore(void* data, int maxSize, int& size)
{
	DEBUG_ENTER_FUNCTION;

	size = 0;
	bool pulledData = false;
	if(_root.GetRoot()->GetState()->waitOnEvents(storeMutex) == FALSE)
	{
		DEBUG_RETURN false;
	}

	DEBUG_VERBOSE("Grabbing a maximum of %d bytes from the store\r\n", maxSize);

	size = blockSize();
	if(size > 0)
	{
		if(maxSize >= size)
		{
			DEBUG_VERBOSE("There is enough room, grabbing data\r\n");
			//move part the [length] and [flag] header
			storeStart += 5;
			memcpy(data, &(storedData[storeStart]), size);
			storeStart += size;
			pulledData = true;
		}
	}


	ReleaseMutex(storeMutex);

	DEBUG_RETURN pulledData;
}

bool NamedPipeClient::DataStore::setWaitHandle(HANDLE waitHandle)
{
	DEBUG_ENTER_FUNCTION;

	bool retCode = false;

	if(_root.GetRoot()->GetState()->waitOnEvents(storeMutex) == FALSE)
	{
		DEBUG_RETURN false;
	}

	//if we already have a block, don't even bother setting the wait handle, just trigger it now
	if(blockSize())
	{
		DEBUG_VERBOSE("We already have data, setting event\r\n");
		SetEvent(waitHandle);
		retCode = true;
	}
	else
	{
		DEBUG_VERBOSE("We must wait for data, resetting event\r\n");
		this->waitHandle = waitHandle;
		ResetEvent(waitHandle);
	}

	ReleaseMutex(storeMutex);

	DEBUG_RETURN retCode;
}

void NamedPipeClient::DataStore::lengthenStore(int minimumFreeSpace)
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_VERBOSE("Lengthening store to allow at least %d free bytes\r\n", minimumFreeSpace);
	int freeSpace = this->freeSpace();
	int newFreeSpace = (freeSpace + (2 * PIPE_BUFFER));
	if(newFreeSpace < minimumFreeSpace)
	{
		//we need more even free space
		newFreeSpace = (minimumFreeSpace + (PIPE_BUFFER));
	}

	int spaceNeeded = (newFreeSpace - freeSpace);

	int newLength = storeMaxLength + spaceNeeded - storeStart;

	unsigned char* newData = new unsigned char[newLength];
	memcpy(newData, &(storedData[storeStart]), (storeLength - storeStart));

	storeLength = storeLength - storeStart;
	storeStart = 0;
	storeMaxLength = newLength;
	delete[] storedData;
	storedData = newData;
	newData = NULL;

	DEBUG_END_FUNCTION;
}

int NamedPipeClient::DataStore::storeSize()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN (storeLength - storeStart);
}

int NamedPipeClient::DataStore::blockSize()
{
	DEBUG_ENTER_FUNCTION;

	int len = 0;
	//we need at least 4 bytes to get the next messages length
	if(storeSize() > 4)
	{
		//copy the length header over to len
		memcpy(&len, &(storedData[storeStart]), 4);

		//check there is at least len bytes in the store - otherwise we don't have the entire message yet
		if(storeSize() < len)
		{
			DEBUG_VERBOSE("Entire message has not been sent over the named pipe yet\r\n");
			len = 0;
		}
		else
		{
			//len is the block length, the data length is the block length - 5, so subtract 5 from len
			len -= 5;
		}
	}

	DEBUG_RETURN len;
}

int NamedPipeClient::DataStore::freeSpace()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_VERBOSE("DataStore currently contains %d bytes\r\n", storeLength - storeStart);
	DEBUG_RETURN (storeMaxLength - storeLength);
}

void NamedPipeClient::DataStore::dispose(void)
{
	if (storedData != nullptr)
	{
		delete[] storedData;
	}
}

DWORD WINAPI NamedPipeClient::asyncConnectFunc(LPVOID param)
{
	NamedPipeClient* pClient = reinterpret_cast<NamedPipeClient*>(param);	

	if(pClient->_root.GetRoot()->GetState()->shouldAbort())
	{
		//delete pClient as it is no longer needed
		delete pClient;
		return 0;
	}

	return pClient->asyncConnect(pClient);
}

DWORD NamedPipeClient::asyncConnect(LPVOID param)

{
	DEBUG_ENTER_FUNCTION;

	//Create security desc for the named pipe
	PSID pEveryoneSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	EXPLICIT_ACCESS ea[1];
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pACL = NULL;
	SECURITY_ATTRIBUTES sa;
	bool useSA = false;

	
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if(NULL == pSD)
	{
		//oh no :/
	}
	else
	{
		if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		{
			//more bad stuff
		}
		if(!SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE))
		{
			//again bad
		}
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = pSD;
		sa.bInheritHandle = FALSE;
		useSA = true;
	}

	NamedPipeClient* pClient = (NamedPipeClient*)param;
	MiTokenBLEChild root = MiTokenBLEChild(pClient->_root);

	while(pClient->tryConnect)
	{
		//pipe isn't setup yet
		if(root.GetRoot()->GetState()->shouldAbort())
		{
			DEBUG_RETURN 0;
		} 

		pClient->myPipe = CreateFileA(pClient->pipeName,
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							(useSA ? &sa : NULL),
							OPEN_EXISTING,
							FILE_FLAG_OVERLAPPED,
							NULL);

		if(pClient->myPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}
		if(GetLastError() ==  ERROR_FILE_NOT_FOUND)
		{
			//pipe isn't setup yet
			if(root.GetRoot()->GetState()->shouldAbort())
			{
				DEBUG_RETURN 0;
			}

			printf("Could not open client pipe : FILE_NOT_FOUND. Trying again in 1 second\r\n");
			Sleep(1000);
			continue;
		}
		else if(GetLastError() != ERROR_PIPE_BUSY)
		{

			printf("Could not open client pipe : GLE = %d\r\n", GetLastError());
			DEBUG_RETURN -1;
		}

		if( !WaitNamedPipeA(pClient->pipeName, 20000) )
		{
			int d = GetLastError();
			printf("Could not open pipe : 20 second wait timed out. [GLE = %d]", d);
			DEBUG_RETURN -2;
		}

	}

	pClient->isConnected = true;
	if(pClient->pipeConnectedCallback != nullptr)
	{
		pClient->pipeConnectedCallback(pClient->getRoot().GetRoot());
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(pClient->myPipe,
											&dwMode,
											NULL,
											NULL);
	if(!fSuccess)
	{
		printf("Failed to SetNamedPipeHandleState, GLE = %d\r\n", GetLastError());
		DEBUG_RETURN -3;
	}

	HANDLE heap = GetProcessHeap();
	unsigned char* request = (unsigned char*)HeapAlloc(heap, 0, PIPE_BUFFER);

	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD bytesRead;

	if(pClient->clientBoundToCOM)
	{
		uint8 commandData[] = {COMMAND_FLAG_CLIENT_COM, CLIENT_COM_MODE_INUSE};
		pClient->sendCommand(commandData, sizeof(commandData), -1);
	}
	while(pClient->runPipe)
	{
		fSuccess = ReadFile(
			pClient->myPipe,
			request,
			PIPE_BUFFER,
			&bytesRead,
			&overlapped);


		int lastError = GetLastError();
		if( !fSuccess && lastError != ERROR_MORE_DATA)
		{
			if(lastError == ERROR_IO_PENDING)
			{
				if(root.GetRoot()->GetState()->waitForOverlappedIO(pClient->myPipe, &overlapped, bytesRead) == FALSE)
				{
					//we are to abort
					break;
				}

				/*
				if(GetOverlappedResult(pClient->myPipe, &overlapped, &bytesRead, FALSE) == FALSE)
				{
					lastError = GetLastError();
					if((lastError == ERROR_IO_PENDING) || (lastError == ERROR_IO_INCOMPLETE))
					{
						if(!CurrentState::getInstance()->waitOnEvents(overlapped.hEvent))
						{
							//we are to cancel - we recieved an abort call from the currentState.
							break;
						}
						else
						{
							GetOverlappedResult(pClient->myPipe, &overlapped, &bytesRead, FALSE);
						}

					}
				}

				/*
				if(GetOverlappedResult(pClient->myPipe, &overlapped, &bytesRead, TRUE) == false)
				{
					lastError = GetLastError();
					DEBUG_VERBOSE("Error reading from Pipe : GLE = %d\r\n",lastError);
					printf("Error reading from Pipe : GLE = %d\r\n",lastError);
				}
				*/

			}
			else
			{
				break;
			}
		}

		

		//printf("The pipe succeeded (C<-%d)\r\n", bytesRead);

		if(bytesRead > 0)
		{
			if(bytesRead < 5)
			{
				//we should always read at least 5 bytes
				printf("Error, only read %d bytes\r\n", bytesRead);
				assert(false);
			}

			unsigned char * curRequest = request;
			while(bytesRead)
			{
			
				int length;
				char flag;
				memcpy(&length, curRequest, 4);
				flag = curRequest[4];

				assert(bytesRead >= length);

				if(flag == MESSAGE_FLAG_BLE)
				{
					pClient->myDataStore.addDataToStore(curRequest, length);
					curRequest += length;
					bytesRead -= length;
				}
				else if(flag == MESSAGE_FLAG_NORMAL)
				{
					curRequest += 5;
					bytesRead -= 5;
					length -= 5;
					assert(length > 0);
					char normalFlag = curRequest[0];
					switch(normalFlag)
					{
					case COMMAND_FLAG_EXCLUSIVE:
						{
							assert(length == 2);
							char newMode = curRequest[1];
							switch(newMode)
							{
							case EXCLUSIVE_MODE_DISABLE:
								pClient->_exclusiveStatus = EXCLUSIVE_DISABLED;
								break;
							case EXCLUSIVE_MODE_ENABLE:
								pClient->_exclusiveStatus = EXCLUSIVE_ALLOWED;
								break;
							case EXCLUSIVE_MODE_UNABLE:
								pClient->_exclusiveStatus = EXCLUSIVE_DENIED;
								break;
							}
						}
					}

					curRequest += length;
					bytesRead -= length;
				}
				else if(flag == MESSAGE_FLAG_USER)
				{
					if(pClient->userMessageCallback != NULL)
					{
						pClient->userMessageCallback(root.GetRoot(), curRequest + 5, length - 5, -1);
					}

					curRequest += length;
					bytesRead -= length;
				}

			}
		}
		else
		{
			//the pipe has closed itself
			pClient->runPipe = false;
		}
	}

	CloseHandle(pClient->myPipe);

	
	if(!root.GetRoot()->GetState()->shouldAbort())
	{
		//we have not been told to abort, so restart the connection
		pClient->initConnection();
	}
	

	DEBUG_RETURN 0;
}

bool NamedPipeServer::isExclusive()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN (exclusiveMode != -1);
}

bool NamedPipeServer::isSelfExclusive()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN (exclusiveMode == -2);
}

bool NamedPipeClient::getNextBlock(unsigned char* buffer, DWORD maxSize, DWORD* size, HANDLE waitHandle)
{
	DEBUG_ENTER_FUNCTION;

	int bread;
	bool gotData = false;

	if(!myDataStore.setWaitHandle(waitHandle))
	{
		if(_root.GetRoot()->GetState()->waitOnEvents(waitHandle) == FALSE)
		{
			//we should abort
			DEBUG_RETURN false;
		}
	}
	gotData = myDataStore.getDataFromStore(buffer, maxSize, bread);

	*size = bread;

	DEBUG_RETURN gotData;
}

bool NamedPipeClient::readNextBlock(unsigned char* buffer, DWORD size, DWORD* read, HANDLE waitHandle)
{
	DEBUG_ENTER_FUNCTION;

	int bread;
	bool gotData = false;

	while(!gotData)
	{
		if(!myDataStore.setWaitHandle(waitHandle))
		{
			if(_root.GetRoot()->GetState()->waitOnEvents(waitHandle) == FALSE)
			{
				DEBUG_RETURN false;
			}
		}
		gotData = myDataStore.getDataFromStore(buffer, size, bread);
	} 

	*read = bread;

	DEBUG_RETURN true;
}


bool NamedPipeServer::clientHasCOM()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN this->_clientWithCOMKnown;
}

void NamedPipeServer::SetClientCOMReleaseWaitHandle(HANDLE waitHandle)
{
	DEBUG_ENTER_FUNCTION;

	this->_clientWithCOMKnownWait = waitHandle;

	DEBUG_END_FUNCTION;
}



NamedPipeClient::ExclusiveAccessStatus NamedPipeClient::exclusiveStatus()
{
	DEBUG_ENTER_FUNCTION;

	DEBUG_RETURN _exclusiveStatus;
}


NamedPipe::NamedPipe(MiTokenBLEChild root) : _root(root)
{
	DEBUG_ENTER_FUNCTION;

	namedPipeBLERead = NULL;
	userMessageCallback = NULL;
	
	DEBUG_END_FUNCTION;
}

NamedPipeServer::~NamedPipeServer()
{
	close();
	delete[] clientData;

	if (pipeName != nullptr)
	{
		delete pipeName;
		pipeName = nullptr;
	}
}

NamedPipeClient::~NamedPipeClient()
{
	if (pipeName != nullptr)
	{
		delete pipeName;
		pipeName = nullptr;
	}

	myDataStore.dispose();
}

NamedPipe::~NamedPipe()
{

}

void NamedPipeServer::close()
{
	for(int i = 0 ; i < pipeCount; ++i)
	{
		clientData[i].messageClient = false;
		if(clientData[i].pipeHandle != INVALID_HANDLE_VALUE)
		{
			FlushFileBuffers(clientData[i].pipeHandle);
			DisconnectNamedPipe(clientData[i].pipeHandle);
			CloseHandle(clientData[i].pipeHandle);
			clientData[i].pipeHandle = INVALID_HANDLE_VALUE;
		}
	}
}

void NamedPipeClient::close()
{

}

void NamedPipeClient::gotCommandMessage(unsigned char* data, int length, int pipeHandle)
{

}

void NamedPipeClient::removeClient(HANDLE clientPipe)
{

}

void NamedPipeClient::InformServerOfClosedCOMPort()
{
	DEBUG_ENTER_FUNCTION;

	uint8 command[] = { COMMAND_FLAG_CLIENT_COM, CLIENT_COM_MORE_DONE};
	sendCommand(command, sizeof(command), -1);

	DEBUG_END_FUNCTION;
}