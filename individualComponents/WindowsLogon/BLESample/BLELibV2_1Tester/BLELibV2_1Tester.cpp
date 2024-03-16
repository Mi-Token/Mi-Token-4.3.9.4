#include <stdio.h>
#include <stdlib.h>
#include "BLE_API.h"
#include <Windows.h>




int main()
{
	IMiTokenBLE* BLEInterface;
	
	BLEInterface = CreateNewMiTokenBLEInterface();
	if(BLEInterface == nullptr)
	{
		printf("Error : Failed to create BLE Interface.\r\n");
		return -1;
	}

	BLEInterface->Initialize("COM3", "\\\\.\\pipe\\MiTokenBLE", false);


	bool running = true;
	while(running)
	{
		char c = getchar();

		//switch for non-alpha characters
		switch(c)
		{

		}

		//will convert all chars to lower case
		switch(c | 0x20)
		{
		case 'q':
			running = false;
			break;

		}


		while(c != 10) { c = getchar(); }

	}
	
	BLEInterface->SafeDelete();
	

	running = true;
	while(running)
	{
		getchar();
		running = false;
	}
}