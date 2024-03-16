#include "BLE_API.h"
#include "BLE_API_Types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>


#ifndef bd_addr
#define bd_addr mac_address_t
#endif

bd_addr addrFromHexMac(const char* HexMAC);
DWORD WINAPI PollThread(LPVOID lpParam);
void checkAllDevices();
int waitForDeviceID(bd_addr address);
char* getVersionOfDevice(bd_addr address, uint8& length);
int checkDeviceVersion(bd_addr address);
int checkDeviceVersionFromID(int deviceID);
int deviceIDofClosest(int requiredPollID);
int doClosestCheck(int requiredPollID);

int getDeviceCount();
int getDeviceInfo(int deviceID, DeviceInfo* pDevInfo);

bool printMode;
bool running = false;
const char* versionID = 0;
int useClosest = false;
bool loudmode;
bool pauseOnEnd = false;

HANDLE consoleHandle;

#define CONSOLE_ERROR 0x0E //Yellow on Black
#define CONSOLE_FAILURE 0x0C //Red on Black
#define CONSOLE_SUCCESS 0x0A //Green on black
#define CONSOLE_NORMAL 0x07 //White on Black


bool filterMode = false;
int filterCount;
DeviceData* filters;

void DoFilteredSearch();
int filteredCount;
DeviceInfo* filteredItems;


//Argument format [file] {-checkall | -deviceID [MAC ADDRESS] | -useclose } [-version stringID] [-com COM11] -loudmode
int main(int argc, const char** argv)
{
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	printMode = true;
	loudmode = false;
	bool checkAll = false;
	useClosest = false;
	const char* COMPort = NULL;
	const char* DeviceMAC = NULL;
	bool waitForDevice = false;
	if(argc != 1)
	{
		for(int i = 1 ; i < argc ; i++)
		{
			if(strcmp(argv[i], "-checkall") == 0)
			{
				checkAll = true;
				printMode = false;
			}
			if(strcmp(argv[i], "-version") == 0)
			{
				if((i + 1) < argc)
				{
					versionID = argv[i + 1];
					i++;
				}
			}
			if(strcmp(argv[i], "-com") == 0)
			{
				if((i + 1) < argc)
				{
					COMPort = argv[i + 1];
					i++;
				}
			}
			if(strcmp(argv[i], "-deviceID") == 0)
			{
				if((i + 1) < argc)
				{
					DeviceMAC = argv[i + 1];
					waitForDevice = true;
					printMode = false;
					i++;
				}
			}
			if(strcmp(argv[i], "-useclose") == 0)
			{
				useClosest = true;
				printMode = false;
			}
			if(strcmp(argv[i], "-loudmode") == 0)
			{
				loudmode = true;
			}
			if(strcmp(argv[i], "-pause") == 0)
			{
				pauseOnEnd = true;
			}
			if(strcmp(argv[i], "-filter") == 0)
			{
				if((i + 1) < argc)
				{
					i++;
					filterCount = atoi(argv[i]);
					if((filterCount < 0) || (filterCount > 10))
					{
						printf("Invalid Command Line : -filter [count] requires that  0 > [count] > 10.\r\nFilter will be disabled\r\n");
						filterCount = 0;
					}
					else if ((i + (filterCount * 2)) >= argc)
					{
						printf("Invalid Command Line : -filter [count] {[ID] [value]} would require at least %d arguments. Recieved %d\r\nFilter will be disabled\r\n", (i + (filterCount * 2)), argc);
						filterCount = 0;
					}
					else
					{
						filters = new DeviceData[filterCount];
						for(int fid = 0 ; fid < filterCount ; ++fid)
						{
							i++;
							filters[fid].flag = (char)atoi(argv[i]);
							i++;
							filters[fid].length = (char)strlen(argv[i]);
							filters[fid].data = (unsigned char*)calloc(filters[fid].length, 1);
							memcpy(filters[fid].data, argv[i], filters[fid].length);
						}

						filterMode = true;
					}
				}
			}
		}
	}
	else
	{
		printf("Error - Missing argument : -com [COMPort]\r\nTerminating early\r\n");
		if(pauseOnEnd)
		{
			getchar();
		}
		return -1;
	}

	printf("Starting Mi-Token BLE Device Version Checker\r\nSettings:\r\n\tCheck Mode : %s\r\n\tVersion : %s\r\n", (checkAll ? "ALL" : (waitForDevice ? "SINGLE DEVICE" : (useClosest ? "CLOSEST" : "USER DEFINED"))), versionID);
	printf("Filter Mode : %s\r\n", filterMode ? "Enabled" : "Disabled");
	if(filterMode)
	{
		printf("Filter Count : %d\r\n", filterCount);
		for(int fid = 0 ; fid < filterCount ; ++fid)
		{
			printf("\tFilter %d\r\n\t\tFlag %d\r\n\t\tLength %d\r\n\t\tData : ", fid, (int)filters[fid].flag, (int)filters[fid].length);
			for(int fcc = 0 ; fcc < filters[fid].length ; ++fcc)
			{
				printf("%c", filters[fid].data[fcc]);
			}
			printf("\r\n");
		}
	}
	if((!checkAll) && (waitForDevice))
	{
		printf("\tDevice ID : ");
		bd_addr address = addrFromHexMac(DeviceMAC);
		for(int i = 0 ; i < 6; ++i)
		{
			printf("%02lX%s", address.addr[i], (i == 5 ? "" : ":"));
		}
		printf("\r\n");
	}
	if(loudmode)
	{
		printf("\tLoud Mode : ENABLED\r\n");
	}

	running = (BLE_Initialize(COMPort) == 0);
	if(!running)
	{
		printf("Error : Could not connect to COMPort : %s\r\nTerminating early\r\n", COMPort);
		if(pauseOnEnd)
		{
			getchar();
		}
		return -2;
	}
	else
	{
		CreateThread(NULL, 0, PollThread, NULL, 0, NULL);
	}

	if(checkAll)
	{
		//We will wait 5 seconds then get all the devices connected and check them
		Sleep(5000);
		checkAllDevices();
		if(pauseOnEnd)
		{
			getchar();
		}
		return 0;
	}

	if(waitForDevice)
	{
		bd_addr deviceAddr = addrFromHexMac(DeviceMAC);
		int deviceSearchID;
		while((deviceSearchID = waitForDeviceID(deviceAddr)) == -1)
		{
			Sleep(1000);
		}
		/*
		DeviceInfo devInfo;
		getDeviceInfo(deviceSearchID, &devInfo);
		bd_addr add;
		memcpy(add.addr, devInfo.address, 6);
		checkDeviceVersion(add, versionID);
		*/


		int ret = checkDeviceVersionFromID(deviceSearchID);
		if(pauseOnEnd)
		{
			getchar();
		}
		return ret;
	}

	while(running)
	{
		if(useClosest)
		{
			char c = getchar();
			if((c | 0x20) == 'q')
			{
				running = false;
			}
			while(c != 10)
			{
				c = getchar();
			}
		}
		else
		{
			//normal print mode
			char c = getchar();
			if((c >= '0') && (c <= '9'))
			{
				int deviceCount = getDeviceCount();
				int cint = (c - '0');
				if(cint >= deviceCount)
				{
					printf("Device %c doesn't exist\r\n", c);
				}
				else
				{
					printMode = false;
					checkDeviceVersionFromID(cint);
					printMode = true;
				}
			}
			if((c | 0x20) == 'q')
			{
				running = false;
			}

			while(c != 10)
			{
				c = getchar();
			}
			
		}

		Sleep(1000);	
	}

	if(pauseOnEnd)
	{
		getchar();
	}

	return 0;
}

bd_addr addrFromHexMac(const char* HexMAC)
{
	bd_addr ret;

	int slen = strlen(HexMAC);
	int curSpot = 0;
	int curNibble = 0;
	int curValue = 0;

	for(int i = 0 ; i < slen ; ++i)
	{
		char c = HexMAC[i];

		if((c >= '0') && (c <= '9'))
		{
			curValue <<= 4;
			curValue += c - '0';
			curNibble++;
		}
		c |= 0x20;
		if((c >= 'a') && (c <= 'f'))
		{
			curValue <<= 4;
			curValue += c - 'a' + 10;
			curNibble++;
		}

		if(curNibble == 2)
		{
			ret.addr[curSpot] = curValue;		
			curNibble = 0;
			curValue = 0;
			curSpot++;
			if(curSpot == 6)
			{
				break;
			}
		}
	}
	
	return ret;
}

#define POLL_SPEED 1000

DWORD WINAPI PollThread(LPVOID lpParam)
{
	int pollID = 0;
	while(running)
	{
		pollID++;
		BLE_SetPollID(pollID);
		Sleep(POLL_SPEED);

		if(printMode)
		{
			int deviceCount = getDeviceCount();
			printf("----------------------------------------\r\nBLE Device Count : %d\r\n\tCurrent Poll ID : %d\r\n", deviceCount, pollID);
			for(int i = 0 ; i < deviceCount ; ++i)
			{
				DeviceInfo devInfo;
				getDeviceInfo(i, &devInfo);

				printf("\tDevice %d\r\n\t\tMac Address : ", i);
				for(int j = 0 ; j < 6 ; ++j)
				{
					printf("%02lX%s", devInfo.address[j], (j == 5 ? "" : ":"));
				}
				printf("\r\n\t\tRSSI : %d\r\n\t\tLast Poll : %d [%d ago]\r\n", devInfo.RSSI, devInfo.lastSeen, pollID - devInfo.lastSeen);
			}
			printf("----------------------------------------\r\n\r\n");
		}

		if(useClosest)
		{
			doClosestCheck(pollID);
		}
	}

	return 0;
}

void checkAllDevices()
{
	int devCount = getDeviceCount();
	for(int i = 0 ; i < devCount ; ++i)
	{
		checkDeviceVersionFromID(i);
	}
}

int waitForDeviceID(bd_addr address)
{
	int devCount = getDeviceCount();
	for(int i = 0 ; i < devCount ; ++i)
	{
		DeviceInfo devInfo;
		getDeviceInfo(i, &devInfo);
		if(memcmp(address.addr, devInfo.address, 6) == 0)
		{
			return i;
		}
	}
	return -1;
}


enum Stages
{
	STAGE_CONNECTING,
	STAGE_SCANNING_SERVICES_DEVICE_INFORMATION,
	STAGE_SCANNING_REQUEST_VERSION_ID,
	STAGE_SCANNING_SERVICES_READ_VERSION_ID,
	STAGE_DISCONNECT,
};

#define DEVICE_INFORMATION_UUID 0x180A
#define DEVICE_INFORMATION_FIRMWARE_VERSION 0x2A26


const char* RequestErrorCodeToString(int requestErrorCode)
{
	switch(requestErrorCode)
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
	default:
		return "UNKNOWN ERROR CODE";
	}
}

char* getVersionOfDevice(bd_addr address, uint8& readAttributeLength)
{
	if(loudmode)
	{
		printf("Getting Version of Device : ");
	}
	for(int i = 0 ; i < 6 ; ++i)
	{
		printf("%02lX%s", address.addr[i], (i == 5 ? "" : ":"));
	}
	if(loudmode)
	{
		printf("\r\n");
	}
	int requestID = 0;
	int oldreqID = 0;
	
	int stage = STAGE_SCANNING_SERVICES_DEVICE_INFORMATION;
	int resp = 0;

	requestID = BLE_CONN_StartProfile(address);
	oldreqID = requestID;
	readAttributeLength = 0;
	char* readAttributeBuffer;

	bool runFunction = true;
	while(runFunction)
	{
		if((oldreqID == BLE_CONN_BAD_REQUEST))
		{
			printf("Error : StartProfile returned BAD_REQUEST\r\n");
			return NULL;
		}
		if(oldreqID == BLE_CONN_ERR_CONNECTION_TIMED_OUT)
		{
			printf("Error : Connection Timed out\r\n");
			BLE_CONN_Disconnect(requestID);
			return NULL;
		}

		if(loudmode)
		{
			printf(".");
		}
		Sleep(100);
		switch(stage)
		{
		case STAGE_SCANNING_SERVICES_DEVICE_INFORMATION:
			resp = BLE_CONN_ScanServicesInRange(requestID, DEVICE_INFORMATION_UUID, 0x20);
			if(resp == requestID)
			{
				if(loudmode)
				{
					printf("\r\nScanning Services from DEVICE_INFORMATION_UUID [%04lX]\r\n", DEVICE_INFORMATION_UUID);
				}
				stage = STAGE_SCANNING_REQUEST_VERSION_ID;
			}
			else
			{
				if(resp != oldreqID)
				{
					if(loudmode)
					{
						printf("\r\nCannot start scanning yet : error code [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
					}
				}
			}

			if(resp == BLE_CONN_ERR_NO_SUCH_SERVICE)
			{
				readAttributeBuffer = NULL;
				readAttributeLength = 0;
				stage = STAGE_DISCONNECT;
			}
			oldreqID = resp;
			break;
		case STAGE_SCANNING_REQUEST_VERSION_ID:
			resp = BLE_CONN_StartGetAttribute(requestID, DEVICE_INFORMATION_FIRMWARE_VERSION);
			if(resp == requestID)
			{
				if(loudmode)
				{
					printf("\r\nRequesting FIRMWARE_VERSION from device\r\n");
				}
				stage = STAGE_SCANNING_SERVICES_READ_VERSION_ID;
			}
			else
			{
				if(resp != oldreqID)
				{
					if(loudmode)
					{
						printf("\r\nCannot start scanning yet : error code [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
					}
				}
			}
			oldreqID = resp;
			break;

		case STAGE_SCANNING_SERVICES_READ_VERSION_ID:
			resp = BLE_CONN_GetAttribute(requestID, DEVICE_INFORMATION_FIRMWARE_VERSION, 0, readAttributeLength, NULL);
			if(resp == requestID)
			{
				readAttributeBuffer = (char*)malloc(readAttributeLength);
				resp = BLE_CONN_GetAttribute(requestID, DEVICE_INFORMATION_FIRMWARE_VERSION, readAttributeLength, readAttributeLength, (uint8*)readAttributeBuffer);
				if(resp == requestID)
				{
					if(loudmode)
					{
						printf("\r\nRead Value from FIRMWARE_VERSION [%04lX] of Size [%d]\r\n", DEVICE_INFORMATION_FIRMWARE_VERSION, readAttributeLength);
					}
					stage = STAGE_DISCONNECT;
				}
				else
				{
					if(loudmode)
					{
						printf("\r\nError : First attempt of GetAttribute Succeeded while the second one failed with : [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
					}
				}
			}
			else
			{
				if(resp != oldreqID)
				{
					if(loudmode)
					{
						printf("\r\nCannot read attribute yet : error code [%d] [%s]\r\n", resp, RequestErrorCodeToString(resp));
					}
				}
			}
			oldreqID = resp;
			break;
		case STAGE_DISCONNECT:
			BLE_CONN_Disconnect(requestID);
			runFunction = false;
			break;
		}
	}

	return readAttributeBuffer;
}

int checkDeviceVersion(bd_addr address)
{
	uint8 length;
	char* version = getVersionOfDevice(address, length);

	if(loudmode)
	{
		printf("Device Version Check");
	}

	printf(" : ");
	if(version == NULL)
	{
		SetConsoleTextAttribute(consoleHandle, CONSOLE_ERROR);
		printf("ERROR\r\n");
		SetConsoleTextAttribute(consoleHandle, CONSOLE_NORMAL);
		return -1;
	}

	if((length != strlen(versionID)) || (memcmp(versionID, version, strlen(versionID) != 0)))
	{
		SetConsoleTextAttribute(consoleHandle, CONSOLE_FAILURE);
		printf("FAILED [");
		for(int i = 0 ; i < length ; ++i)
		{
			printf("%c", version[i]);
		}
		printf("]\r\n");
		SetConsoleTextAttribute(consoleHandle, CONSOLE_NORMAL);
		free(version);
		return 0;
	}

	free(version);
	SetConsoleTextAttribute(consoleHandle, CONSOLE_SUCCESS);
	printf("SUCCESS\r\n");
	SetConsoleTextAttribute(consoleHandle, CONSOLE_NORMAL);
	return 1;
}

int checkDeviceVersionFromID(int deviceID)
{
	DeviceInfo devInfo;
	getDeviceInfo(deviceID, &devInfo);
	
	bd_addr addr;
	memcpy(addr.addr, devInfo.address, 6);

	int ret = checkDeviceVersion(addr);

	BLE_RestartScanner();

	return ret;
}

int deviceIDofClosest(int requiredPollID)
{
	int DeviceCount = getDeviceCount();
	int maxPoll = requiredPollID;
	char maxRSSI = -127;
	int deviceID = -1;
	for(int i = 0 ; i < DeviceCount ; ++i)
	{
		DeviceInfo devInfo;
		getDeviceInfo(i, &devInfo);
		if(devInfo.lastSeen > maxPoll)
		{
			maxPoll = devInfo.lastSeen;
			maxRSSI = devInfo.RSSI;
			deviceID = i;
		}
		else if(devInfo.lastSeen == maxPoll)
		{
			if(devInfo.RSSI > maxRSSI)
			{
				maxRSSI = devInfo.RSSI;
				deviceID = i;
			}
		}
	}

	return deviceID;
}

int doClosestCheck(int requiredPollID)
{
	int ID = deviceIDofClosest(requiredPollID);
	if(ID != -1)
	{
		checkDeviceVersionFromID(ID);		
	}
	return 0;
}

int getDeviceCount()
{
	if(filterMode)
	{
		DoFilteredSearch();
		return filteredCount;
	}
	else
	{
		return BLE_GetDeviceFoundCount();
	}
}

int getDeviceInfo(int deviceID, DeviceInfo* pDevInfo)
{
	if(filterMode)
	{
		memcpy(pDevInfo, &(filteredItems[deviceID]), sizeof(DeviceInfo));	
		return 0;
	}
	else
	{
		return BLE_GetDeviceInfo(deviceID, pDevInfo);
	}
}

void DoFilteredSearch()
{
	FilteredSearchObject searcher = BLE_StartFilteredSearch(filterCount, filters);
	int currentBufferSize = 20;
	if(filteredItems != NULL)
	{
		delete[] filteredItems; 
		filteredItems = NULL;
	}

	filteredItems = new DeviceInfo[20];

	DeviceInfo devInfo;
	filteredCount = 0;

	while(BLE_ContinueFilteredSearch(searcher, &devInfo) == 0)
	{
		if(filteredCount >= currentBufferSize)
		{
			//create a new buffer
			DeviceInfo* newBuffer = new DeviceInfo[currentBufferSize + 20];
			memcpy(newBuffer, filteredItems, currentBufferSize * sizeof(DeviceInfo));
			currentBufferSize += 20;
			delete[] filteredItems;
			filteredItems = newBuffer;
			newBuffer = NULL;
		}
		memcpy(&(filteredItems[filteredCount]), &devInfo, sizeof(devInfo));
		filteredCount++;
	}
	BLE_FinishFilteredSearch(searcher);
}