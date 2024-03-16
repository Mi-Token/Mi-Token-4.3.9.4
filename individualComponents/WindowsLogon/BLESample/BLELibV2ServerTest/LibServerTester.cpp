#include "BLE_API.h"

#include <stdlib.h>
#include <stdio.h>

#include <Windows.h>


void GotNPMessage(uint8* data, int length, int connectionID)
{
	printf("Got message via NP from [%d] of length [%d]\r\n\t", connectionID, length);
	for(int i = 0 ; i < length ; ++i)
	{
		printf("%c", data[i]);
	}
	printf("\r\n");

	BLE_NP_SendMessage(data, length, connectionID);


}

int main()

{
	const char* COMPort = "COM11";
	const char* NamedPipe = "\\\\.\\pipe\\Mi-TokenBLE";
	bool running = false;

	if(BLE_InitializeEx(COMPort, NamedPipe, true) != 0)
	{
		printf("Error : InitializeEx on server returned != 0\r\n");
		getchar();
		return -1;
	}


	while(!running)
	{
		
		running = (BLE_ConnectedToCOM() == 1);
		
		if(!running)
		{
			printf("Not connected to COM - Retrying in 1 second\r\n");
			Sleep(1000);

			if(BLE_COMOwnerKnown() == 1)
			{
				printf("Couldn't connect to COM port : Owner is known\r\n");
				HANDLE evt = CreateEvent(NULL, TRUE, FALSE, NULL);
				if(BLE_SetCOMWaitHandle((void*)evt) == 1)
				{
					//we can now wait for the event
					printf("Waiting on the COMWait Event\r\n");
					WaitForSingleObject(evt, INFINITE);
				}
				CloseHandle(evt);
			}
			else
			{
				BLE_ReinitCOM(COMPort);
			}
			
		}
	}
	if(running)
	{
		BLE_NP_SetMessageCallback(GotNPMessage);
	}
	while(running)
	{
		char c = getchar();
		if((c | 0x20) == 'q')
		{
			running = false;
		}
	}


	BLE_Finalize();
	
}