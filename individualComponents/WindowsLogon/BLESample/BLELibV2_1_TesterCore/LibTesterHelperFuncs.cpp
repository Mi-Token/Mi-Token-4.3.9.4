#include "LibTesterHelperFuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "MiTokenBLE_UUIDs.h"

int pseudoSync_ScanServicesNearUUID(IMiTokenBLEConnection* conn, uint16 UUID)
{
	/*
	int oldresp = BLE_CONN_SUCCESS;
	bool retry = true;
	while(retry)
	{
		int resp = conn->ScanServicesInRange(UUID, 0x20);

		if(resp == BLE_CONN_SUCCESS)
		{
			retry = false;
		}
		else
		{
			if(resp != oldresp)
			{
				printf("Scan services Temporary Holdup : [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
				oldresp = resp;
			}
			Sleep(100);
		}
	}
	return BLE_CONN_SUCCESS;
	*/

	IMiTokenBLEConnectionV2_2* v2_2Conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(conn);
	return v2_2Conn->SyncScanServiesInRange(UUID, 0x20, INFINITE);
}

int pseudoSync_ReadBuffer(IMiTokenBLEConnection* conn, uint16 UUID, uint8& readLen, uint8*& readPtr)
{
	bool needRequest = true, needRead = true;
	int oldresp = BLE_CONN_SUCCESS;
	IMiTokenBLEConnectionV2_2* v2_2Conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(conn);
	while((needRequest) | (needRead))
	{
		if(needRequest)
		{
			/*
			int resp = conn->StartGetAttribute(UUID);
			if(resp == BLE_CONN_SUCCESS)
			{
				needRequest = false;
				oldresp =  resp;
			}
			else
			{
				if(resp != oldresp)
				{
					printf("Wait for State Buffer Temp Holdup : [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
					oldresp = resp;
				}
				Sleep(100);
			}
			*/

			v2_2Conn->SyncRequestAttribute(UUID, INFINITE);
			needRequest = false;
		}
		else
		{
			{
				readLen = 0;
				if(readPtr != nullptr)
				{
					delete[] readPtr;
					readPtr = nullptr;
				}
				int resp1 = BLE_CONN_SUCCESS, resp2 = BLE_CONN_SUCCESS;
				if(resp1 = conn->GetAttribute(UUID, readLen, readLen, readPtr), resp1 == BLE_CONN_SUCCESS)
				{
					readPtr = new uint8[readLen];
					if(resp2 = conn->GetAttribute(UUID, readLen, readLen, readPtr), resp2 == BLE_CONN_SUCCESS)
					{
						needRead = false;
					}
					else
					{
						Sleep(100);
					}		
				}
			}
		}
	}
	return BLE_CONN_SUCCESS;
}

	


int pseudoSync_WaitForStateBuffer(IMiTokenBLEConnection* conn, uint16 UUID, uint8 successID, uint8 waitID)
{
	bool needRead = true;
	while(needRead)
	{
		uint8 readLen = 0;
		uint8* readPtr = nullptr;
		pseudoSync_ReadBuffer(conn, UUID, readLen, readPtr);
	
		if(readLen != 1)
		{
			printf("Wait For State Buffer : Strange data length of %d bytes\r\n", readLen);
			Sleep(100);
		}
		else
		{
			if(readPtr[0] == successID)
			{
				needRead = false;
			}
			else if(readPtr[0] == waitID)
			{
				printf("Wait For State Buffer : Got 'Wait' Message. Waiting\r\n");
				Sleep(1000);
			}
			else
			{
				printf("Wait For State Buffer : Got Unknown value.\r\n");
				Sleep(100);
			}
		}

		delete[] readPtr;
		readPtr = nullptr;
	}	
	return BLE_CONN_SUCCESS;
}

int pseudoSync_GetDeviceVersion(IMiTokenBLEConnection* conn, DeviceVersion& version)
{
	IMiTokenBLEConnectionV2_2* v2_2Conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(conn);
	v2_2Conn->SyncScanServiesInRange(BLE_UUID_SERVICE_DEVICE_INFO, 0x20, INFINITE);

	//pseudoSync_ScanServicesNearUUID(conn, BLE_UUID_SERVICE_DEVICE_INFO);

	bool needRead = true;
	while(needRead)
	{
		uint8 readLen = 0;
		uint8* readPtr = nullptr;

		pseudoSync_ReadBuffer(conn, BLE_UUID_ATTRIBUTE_DEVICE_INFO_FIRMWARE_VERSION, readLen, readPtr);

		printf("Read Device Firmware Version [%d] bytes\r\n\t", readLen);
		for(int i = 0 ; i < readLen ; ++i)
		{
			printf("%02lX ", readPtr[i]);
		}
		printf("\r\n\t");
		for(int i = 0 ; i < readLen ; ++i)
		{
			printf("%c", readPtr[i]);
		}
		printf("\r\n");

		int versionStart = 0, versionEnd = -1;
		for(int i = 0 ; (i < readLen) && (versionEnd == -1) ; ++i)
		{
			if(readPtr[i] == '_')
			{
				if(versionStart == -1)
				{
					versionStart = i + 1;
				}
				else
				{
					versionEnd = i;
				}
			}
		}

		needRead = false;
		if(versionStart == -1)
		{
			printf("Error : No '_' were discovered, cannot determine the version!\r\n");
		}
		else
		{
			if(versionEnd == -1)
			{
				versionEnd = readLen;
			}

			int versionLength = (versionEnd - versionStart) + 1;
			char* versionBuffer;
			char stackBuffer[40];
			if(versionLength > sizeof(stackBuffer))
			{
				versionBuffer = new char[versionLength];
			}
			else
			{
				versionBuffer = (char*)&(stackBuffer);
			}

			memcpy(versionBuffer, &(readPtr[versionStart]),  (versionLength - 1));
			versionBuffer[versionLength - 1] = '\0';

			if(strcmp("Prox-Logon", versionBuffer) == 0)
			{
				//running Version 1.0
				printf("Running version Prox-Logon : No MAC_ADDRESS before hashing\r\n");
				version = DevVersion1_0;
			}
			else if(strcmp("Blu", versionBuffer) == 0)
			{
				printf("Running version Blu : MAC_ADDRESS before hashing\r\n");
				version = DevVersion1_1;
			}
			else
			{
				printf("Running unknown version \r\n");
				version = DevVersion_Unknown;	
			}

			if(versionBuffer != (char*)&stackBuffer)
			{
				delete versionBuffer;
			}
		}

		delete[] readPtr;
		readPtr = nullptr;
	}

	return BLE_CONN_SUCCESS;
}

int pseudoSync_WriteToUUID(IMiTokenBLEConnection* conn, uint16 UUID, int length, const char* data)
{
	/*
	bool needRewrite = true;
	int oldresp = BLE_CONN_SUCCESS;
	while(needRewrite)
	{
		int resp = conn->SetAttribute(UUID, length, (uint8*)data);

		if(resp == BLE_CONN_SUCCESS)
		{
			needRewrite = false;
		}
		else
		{
			if(oldresp != resp)
			{
				printf("Scan services Temporary Holdup : [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
				oldresp = resp;
			}
			Sleep(100);
		}
	}
	
	return BLE_CONN_SUCCESS;
	*/

	IMiTokenBLEConnectionV2_2* v2_2Conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(conn);

	return v2_2Conn->SyncSetAttribute(UUID, length, (uint8*)data, INFINITE);
}
