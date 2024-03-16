#include "BLE_API.h"
#include <string>
#include <Windows.h>

#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#define DATA_FLAG_REQUEST_LOCK (0x04)
#define DATA_FLAG_REQUEST_QUIT (0x05)


IMiTokenBLEV2_2* bleInstance;

bool running = true;

void pipeConnected(IMiTokenBLE* instance)
{
	instance->Pipe_DisableBLEForwarding();
}

void gotNPMessage(IMiTokenBLE* sender, uint8* data, int length, int connID)
{
	switch(data[0])
	{
	case DATA_FLAG_REQUEST_LOCK:
		//	Check if multi user; otherwise single
		if (data[1])
			WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE,WTS_CURRENT_SESSION, FALSE);
		else
			LockWorkStation();
		break;
	case DATA_FLAG_REQUEST_QUIT:
		running = false;
		break;
	}
}


#ifdef _CONSOLE
int main()
#else if _WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	bleInstance = dynamic_cast<IMiTokenBLEV2_2*>(CreateNewMiTokenBLEInterface(USE_MITOKEN_BLE_API_V2_2));

	bleInstance->Initialize(NULL, "\\\\.\\pipe\\Mi-TokenBLEV2", false, &pipeConnected);
	bleInstance->Pipe_SetNamedPipeMessageCallback(&gotNPMessage);

	

	while(running)
	{
#ifdef _CONSOLE
		char c = getchar();
		if(c == 'l')
		{
			uint8 data[] = {DATA_FLAG_REQUEST_LOCK};
			printf("Sending lock message");
			gotNPMessage(bleInstance, (uint8*)data, sizeof(data), -1);
		}
#endif
		Sleep(1000);
	}
}