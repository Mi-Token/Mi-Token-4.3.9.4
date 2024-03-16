#include "BLE_API.h"
#include "BLELibV2_1_TesterCore.h"
#include <Windows.h>
#include <stdio.h>


#define COM_PORT "COM11"


int main()
{
	IMiTokenBLE* mitokenBLE = CreateNewMiTokenBLEInterface();
	printf("Starting Client Tester\r\n");
	mitokenBLE->Initialize(COM_PORT, "\\\\.\\pipe\\LibTestPipe", false);

	startMainTest(mitokenBLE);

	return 0;
}