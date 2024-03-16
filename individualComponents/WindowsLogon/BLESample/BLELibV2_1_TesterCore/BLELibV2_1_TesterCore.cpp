#include "BLELibV2_1_TesterCore.h"
#include "BLE_API.h"
#include <Windows.h>
#include <stdio.h>
#include "LibTesterHelperFuncs.h"
#include "MiTokenBLE_UUIDs.h"
#include "BLE_BondingLib.h"

void DeviceHashTest(IMiTokenBLE* instance, mac_address addr);

#define IGNORE_DEV_META

#define LOWER_CASE(charV) ((charV) | 0x20)
#define IS_NUM(charV) ((charV >= '0') && (charV <= '9'))

#define POLL_SPEED 1000
bool showPolls = true;
volatile bool running;
DWORD WINAPI pollThread(LPVOID param)
{
	IMiTokenBLE* instance = static_cast<IMiTokenBLE*>(param);
	int pollID = 0;
	int j = 0;

	while (running)
	{
		pollID++;
		instance->SetPollID(pollID);
		Sleep(POLL_SPEED);
		
		if (showPolls && running)
		{
			int deviceCount = instance->GetDeviceFoundCount();
			printf("----------------------------------------\r\nBLE Device Count : %d\r\n\tCurrent Poll ID : %d\r\n", deviceCount, pollID);
			for (int i = 0; i < deviceCount; ++i)
			{
				DeviceInfo devInfo;
				instance->GetDeviceInfo(i, &devInfo);
				
				printf("\tDevice %d\r\n\t\tMacAddress : ", i);
				for (j = 0; j < 6; ++j)
				{
					printf("%02lX%s", devInfo.address[j], (j == 5 ? "" : ":"));
				}
				printf("\r\n\t\tRSSI : %d\r\n\t\tLast Poll : %d\r\n", devInfo.RSSI, devInfo.lastSeen);
#ifndef IGNORE_DEV_META
				instance->DEBUG_PrintDeiveMetaData(i);
#endif
			}
		}
	}

	return 0;
}

void startMainTest(IMiTokenBLE* instance)
{
	running = true;
	CreateThread(NULL, 0, pollThread, instance, 0, nullptr);

	IMiTokenBLEV2_2* v2_2Instance = dynamic_cast<IMiTokenBLEV2_2*>(instance);

	while(running)
	{
		char c = getchar();

		if(LOWER_CASE(c) == 'q')
		{
			showPolls = false;
			running = false;
		}
		if(LOWER_CASE(c) == 'b')
		{
			if(v2_2Instance == nullptr)
			{
				printf("ERROR : Bonding required a BLE V2.2 Instance!\r\n");
			}
			else
			{
				c = getchar();
				if(IS_NUM(c))
				{
					int deviceCount = instance->GetDeviceFoundCount();
					int cint = (c - '0');
					if(cint >= deviceCount)
					{
						printf("Device %c doesn't exist\r\n", c);
					}
					else
					{
						printf("Creating bond for device\r\n");
						showPolls = false;
						Sleep(2000);
						printf("Getting Device Info : ");
						DeviceInfo devInfo;
						if(instance->GetDeviceInfo(cint, &devInfo) == BLE_API_SUCCESS)
						{
							printf("Success\r\n");
							mac_address addr;
							memcpy(&addr, &devInfo.address[0], sizeof(mac_address));
							REQUEST_ID reqID;
							IMiTokenBLEConnectionV2_2* conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(v2_2Instance->StartProfile(addr, reqID));
							BLE_BondingLib bondingLib;
							uint8* hash = nullptr;
						
							bondingLib.addBondToToken(v2_2Instance, conn, hash);
						}
					}
				}
			}
			while(c != 10) { c = getchar(); }
			getchar();
		}
		else if((c >= '0') && (c <= '9'))
		{
			int deviceCount = instance->GetDeviceFoundCount();
			int cint = (c - '0');
			if(cint >= deviceCount)
			{
				printf("Device %c doesn't exists\r\n", c);
			}
			else
			{
				printf("Doing device hash test\r\n");
				showPolls = false;
				DeviceInfo devInfo;
				printf("Getting Device info\r\n");
				if(instance->GetDeviceInfo(cint, &devInfo) == BLE_API_SUCCESS)
				{
					mac_address addr;
					memcpy(&addr, &devInfo.address[0], sizeof(mac_address));
					DeviceHashTest(instance, addr);
					showPolls = true;
				}
				else
				{
					printf("Failed to get device info!\r\n");
				}

			}
		}

		while(c != 10)
		{
			c = getchar();
		}


	}

	
}

enum DeviceHashTestStages
{
	DeviceHashStage_Start,

};

void DeviceHashTest(IMiTokenBLE* instance, mac_address addr)
{
	BLE_API_NP_RETURNS npret = instance->Pipe_RequestExclusiveAccess();
	if (npret != BLE_API_NP_SUCCESS)
	{
		printf("\tFailed to get exclusive access\r\n");
		return;
	}

	bool waitForExclusive = true;
	while (waitForExclusive)
	{
		Sleep(100);
		npret = instance->Pipe_HasExclusiveAccess();
		switch (npret)
		{
		case BLE_API_EXCLUSIVE_ALLOWED:
			waitForExclusive = false;
			printf("Exclusive Access Allowed\r\n");
			break;
		case BLE_API_NP_COM_FALLBACK:
			waitForExclusive = false;
			printf("Using COM : Exclusive Not Required\r\n");
			break;
		case BLE_API_EXCLUSIVE_ASKED:
			printf("Asked for Exclusive Access\r\n");
			break;
		case BLE_API_EXCLUSIVE_DISABLED:
			printf("Exclusive Mode Disabled\r\n");
			break;
		case BLE_API_EXCLUSIVE_REJECTED:
			printf("Rejected from Exclusive Access\r\n");
			return;
		}
	}

	int requestID;
	IMiTokenBLEConnectionV2_2* conn = dynamic_cast<IMiTokenBLEConnectionV2_2*>(instance->StartProfile(addr, requestID));
	

	if (conn == nullptr)
	{
		printf("Start Profile failed or wasn't a IMiTokenConnectionV2_2 object\r\n");
		instance->Pipe_ReleaseExclusiveAccess();
		return;
	}

	conn->SyncWaitForConnection(INFINITE);

	/*
	pseudoSync_ScanServicesNearUUID(conn, 0x180A);
	{
		uint8 readLen;
		uint8* readPtr = nullptr;
		pseudoSync_ReadBuffer(conn, 0x2A23, readLen, readPtr);
	}
	*/
	DeviceVersion versionID;
	pseudoSync_GetDeviceVersion(conn, versionID);
	if(versionID == DevVersion_Unknown)
	{
		printf("Error : Unknown Device Version : Cannot Proceed\r\n");
	}
	else
	{
		uint16 uuid_hash_service = (versionID == DevVersion1_0 ? BLE_UUID_SERVICE_HASH_OLD : BLE_UUID_SERVICE_HASH);
		uint16 uuid_hash_buffer = (versionID == DevVersion1_0 ? BLE_UUID_ATTRIBUTE_HASH_BUFFER_OLD : BLE_UUID_ATTRIBUTE_HASH_BUFFER);
		uint16 uuid_hash_status = (versionID == DevVersion1_0 ? BLE_UUID_ATTRIBUTE_HASH_STATUS_OLD : BLE_UUID_ATTRIBUTE_HASH_STATUS);

		pseudoSync_ScanServicesNearUUID(conn, uuid_hash_service);

		char hash[20] = { 0 };
		printf("Generating Hash of %d bytes\r\n\t", sizeof(hash));
		for (int i = 0; i < sizeof(hash); ++i)
		{
			hash[i] = rand();
			printf("%02lX ", (unsigned char)hash[i]);
		}
		printf("\r\n");

		char hashV1_1[sizeof(hash) + 16];
		int loc = 0;
		while(loc < 16)
		{
			memcpy(&hashV1_1[loc], &addr.addr[0], sizeof(mac_address));
			loc += sizeof(mac_address);
		}

		memcpy(&hashV1_1[16], hash, sizeof(hash));

		if(versionID == DevVersion1_0)
		{
			pseudoSync_WriteToUUID(conn, uuid_hash_buffer, sizeof(hash), hash);
		}
		else
		{
			pseudoSync_WriteToUUID(conn, uuid_hash_buffer, sizeof(hashV1_1), hashV1_1);
		}

		pseudoSync_WaitForStateBuffer(conn, uuid_hash_status, 0, 1);

		uint8 readLen = 0;
		uint8* readPtr = nullptr;

		pseudoSync_ReadBuffer(conn, uuid_hash_buffer, readLen, readPtr);

		printf("Read hash of %d bytes\r\n\t", readLen);
		for(int i = 0 ; i < readLen ; ++i)
		{
			printf("%02lX ", readPtr[i]);
		}
		printf("\r\n");
	}

	printf("Type 'q' to quit\r\n");
	char c;
	while( c = getchar(), c != 'q')
	{}

	conn->Disconnect();

	instance->Pipe_ReleaseExclusiveAccess();
}



const char* RequestErrorCodeToString(int requestErrorCode)
{
	switch (requestErrorCode)
	{
	case BLE_CONN_BAD_REQUEST:
		return "BAD REQUEST";
	case BLE_CONN_ERR_NOT_CONNECTED:
		return "NOT CONNECTED TO DEVICE";
	case BLE_CONN_ERR_NO_SUCH_CONNECTION:
		return "NO SUCH CONNECTION EXISTS";
	case BLE_CONN_ERR_NO_SUCH_SERVICE:
		return "NO SUCH SERVICE EXISTS";
	case BLE_CONN_ERR_SERVICES_NOT_SCANNED:
		return "SERVICES NOT YET SCANNED";
	case BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS:
		return "ATTRIBUTE SCAN IN PROGRESS";
	case BLE_CONN_ERR_NO_SUCH_ATTRIBUTE:
		return "NO SUCH ATTRIBUTE EXISTS";
	case BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS:
		return "LONG WRITE IN PROGRESS";
	case BLE_CONN_ERR_ATTRIBUTE_BEING_READ:
		return "ATTRIBUTE BEING READ";
	case BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA:
		return "ATTRIBUTE HAS MORE DATA";
	case BLE_CONN_ERR_CONNECTION_TIMED_OUT:
		return "CONNECTION TIMED OUT";

	default:
		return "UNKNOWN ERROR CODE";
	}
}

bool NormalStageOutput(int response, int& oldResponse, const char* successMessage, const char* failMessage, int& stage, int nextStage)
{
	bool ret = false;
	if (response == BLE_CONN_SUCCESS)
	{
		printf("%s\r\n", successMessage);
		stage = nextStage;
		ret = true;
	}
	else
	{
		if (response != oldResponse)
		{
			printf("%s : error code [%d] [%s]\r\n", failMessage, response, RequestErrorCodeToString(response));
		}
	}

	oldResponse = response;
	return ret;
}
/*
int ScanServicesNearUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, UUID, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int ScanServicesNearKeyChangeUUID(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, BLE_UUID_SERVICE_PUSH_BUTTON, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from KEY_CHANGE_UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int ScanServicesNearSecretServiceUUID(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, BLE_UUID_SERVICE_SECRET, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from KEY_SECRET_SERVICE_UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int WriteDataToUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID, uint8 dataLength, uint8* dataPtr)
{
	int resp = BLE_CONN_SetAttribute(requestID, UUID, dataLength, dataPtr);
	NormalStageOutput(resp, lastResponse, "Writing data to UUID", "Cannot write to UUID", stage, nextStage);
	return resp;
}

int SetSecretAttribute(int requestID, int& stage, int& lastResponse, int nextStage, MessageBufferObject& messageBuffer)
{
	int resp = BLE_CONN_WriteMessageBuffer(requestID, messageBuffer);
	if (NormalStageOutput(resp, lastResponse, "Set Attribute for KEY_SECRET_ATTRIBUTE_UUID", "Cannot set attribute yet", stage, nextStage))
	{
		messageBuffer = NULL;
	}
	return resp;
}

int SetNotifyAttribute(int requestID, int& stage, int& lastResponse, int nextStage)
{
	static uint8 notify[] = { 0x01, 0x00 };
	int resp = BLE_CONN_SetAttribute(requestID, BLE_UUID_ATTRIBUTE_PUSH_BUTTON_CHARACTERISTIC_CONFIG, sizeof(notify), notify);
	if (NormalStageOutput(resp, lastResponse, "Set Attribute for KEY_PRESSED_NOTIFY_UUID to Enabled", "Cannot set attribute yet", stage, nextStage))
	{
		/*
		buttonTurnedOff = false;
		BLE_CONN_ButtonPressedCallback(requestID, buttonPushed);
		*//*
	}
	return resp;
}

int GetSecretCharacterHandle(int requestID, int& stage, int& lastResponse, int nextStage, uint16& characteristicHandle)
{
	int resp = BLE_CONN_GetCharacteristicHandle(requestID, BLE_UUID_ATTRIBUTE_SECRET_BUFFER, characteristicHandle);
	NormalStageOutput(resp, lastResponse, "Got Characteristic Handle for KEY_SECRET_ATTRIBUTE_UUID", "Cannot get Attribute Handle yet", stage, nextStage);
	return resp;
}

int LongReadSecret(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ReadLong(requestID, BLE_UUID_ATTRIBUTE_SECRET_BUFFER);
	NormalStageOutput(resp, lastResponse, "Long Read for KEY_SECRET_ATTRIBUTE_UUID", "Cannot long read yet", stage, nextStage);
	return resp;
}

int StartReadAttribute(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID)
{
	int resp = BLE_CONN_StartGetAttribute(requestID, attributeUUID);
	NormalStageOutput(resp, lastResponse, "Start reading attribute", "Cannot start reading attribute yet", stage, nextStage);
	return resp;
}

int ReadAttributeBuffer(int requestID, int& stage, int& lastResponse, int nextStage, uint16 attributeUUID, uint8& readLength, uint8*& readBuffer)
{
	int resp = BLE_CONN_GetAttribute(requestID, attributeUUID, 0, readLength, readBuffer);
	if (NormalStageOutput(resp, lastResponse, "Read data length from attribute", "Cannot long read from attribute yet", stage, stage))
	{
		readBuffer = new uint8[readLength];
		resp = BLE_CONN_GetAttribute(requestID, attributeUUID, readLength, readLength, readBuffer);
		if (NormalStageOutput(resp, lastResponse, "Read data from attribute", "Error : 2nd attribute read failed : Weird ", stage, nextStage))
		{
			printf("Read %d bytes from handle [%04lX]\r\n", readLength, attributeUUID);
		}
	}
	return resp;
}
*/