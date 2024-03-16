#include "BLE_API.h"
#include "BLELibV2_1_TesterCore.h"
#include <Windows.h>
#include <stdio.h>
#include "MainLib.h"

#define COM_PORT "COM11"
//#define COM_PORT "COM3"


void performLeakTest()
{
	MemLeak_Init(L"\\\\.\\pipe\\MTMemDebug");
	MemLeak_CheckMemChanges();
	while(true)
	{
		printf("Performing a leak test\r\n");
		for(int i = 0 ; i < 10; ++i)
		{
//			MemLeak_SetLocationID(0);
			IMiTokenBLE* IMiTokenBLE = CreateNewMiTokenBLEInterface(USE_MITOKEN_BLE_API_V2_2);
//			MemLeak_SetLocationID(100);
			IMiTokenBLE ->Initialize(COM_PORT, "\\\\.\\pipe\\Mi-TokenBLEV2", true);		
//			MemLeak_SetLocationID(200);
			IMiTokenBLE->SafeDelete();
			printf(".");
//			MemLeak_CheckMemChanges();
		}
		printf("Leak test completed!");
		char c = getchar();
		while(c != 10)
		{
			c = getchar();
		}
	}
}

int main()
{
	performLeakTest();

	IMiTokenBLE* mitokenBLE = CreateNewMiTokenBLEInterface(USE_MITOKEN_BLE_API_V2_2);
	printf("Starting Server Tester\r\n");
	mitokenBLE->Initialize(COM_PORT, "\\\\.\\pipe\\Mi-TokenBLEV2", true);

	startMainTest(mitokenBLE);


	mitokenBLE->SafeDelete();

	getchar();
	return 0;
}