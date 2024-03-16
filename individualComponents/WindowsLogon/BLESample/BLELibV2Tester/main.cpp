#include "BLE_API.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
//#include "apitypes.h"
//#include "BLE_Structs.h"

#include <time.h>

#include "MiTokenBLE_UUIDs.h"

#ifdef USE_EX
#include "aes.h"
#endif

#ifdef USE_APITYPES_HEADER
#include "BLE_Structs.h" //data from this should be included if USE_APITYPES_HEADER isn't defined
#endif

bool running;
volatile bool useFilter = false;

#define POLL_SPEED 1000
bool showPolls = true;


#define USE_FILTER_BYTEARRAY
#ifdef USE_FILTER_BYTEARRAY
/*
Filter Converted!
        Length : 27
        Bin Data :46 42 53 00 01 00 00 00 01 00 00 00 09 07 4D 54 2D 44 45 42 55
 3C 1C 43 E3 BA 72
----------------------------------------
*/
//Variables for the Filter bytearray
uint8 filterByteArrData[27] = {
								0x46, 0x42, 0x53, 0x00, 
								0x01, 0x00, 0x00, 0x00, 
								0x01, 0x00, 0x00, 0x00, 
								0x09, 0x07, 0x4D, 0x54,
								0x2D, 0x44, 0x45, 0x42,
								0x55, 0x3C, 0x1C, 0x43,
								0xE3, 0xBA, 0x72
							  };
int filterByteArrLength = 27;
#endif
bool doneBinArrConversion = false;


DWORD WINAPI PollThread(LPVOID lpParam)
{
	printf("Poll Started\r\n");
	int i,j, deviceCount, pollID;
	DeviceInfo devInfo;
	FilteredSearchObject deviceSearcher = NULL;
	DeviceData filter;
	
	filter.flag = 9;
	filter.length = 7;
	filter.data = new unsigned char[8];
	memcpy(filter.data, "MT-DEBUG", 8);
	
	uint8 addrr[] = { 0x3C, 0x1C, 0x43, 0xE3, 0xBA, 0x72};

	pollID = 0;
	while(running)
	{
		pollID++;
		BLE_SetPollID(pollID);
		Sleep(POLL_SPEED);
		if(showPolls)
		{
			if(!useFilter)
			{
				deviceCount = BLE_GetDeviceFoundCount();
			}
			else
			{
				//deviceSearcher = BLE_StartFilteredSearch(1, &filter);
#ifdef USE_FILTER_BYTEARRAY
				deviceSearcher = BLE_CovnertByteStreamToFilteredSearch(filterByteArrData, filterByteArrLength);
#else
				deviceSearcher = BLE_StartFilteredSearchEx(1, &filter, sizeof(addrr) / 6, addrr);
				uint8* filterBinArray = NULL;
				int filterBinLen = 0;

				if(!doneBinArrConversion)
				{
					if(BLE_ConvertFilteredSearchToByteStream(deviceSearcher, filterBinArray, filterBinLen) != BLE_API_MORE_DATA)
					{
						printf("Error : Strange Ret code from ConvertFilteredSearchToByteStream!\r\n");
					}
					else
					{
						filterBinArray = new uint8[filterBinLen];
						if(BLE_ConvertFilteredSearchToByteStream(deviceSearcher, filterBinArray, filterBinLen) != BLE_API_SUCCESS)
						{
							printf("Error : Strange Ret code from ConvertFilteredSearchToByteStream (2)!\r\n");
						}
						else
						{
							printf("Filter Converted!\r\n\tLength : %d\r\n\tBin Data :", filterBinLen);
							for(int fbid = 0 ; fbid < filterBinLen ; ++fbid)
							{
								printf("%02lX ", filterBinArray[fbid]);
							}
							printf("\r\n");
						}
					}

					doneBinArrConversion = true;
				}
#endif			 

				while(BLE_ContinueFilteredSearch(deviceSearcher, &devInfo) == 0)
				{
				}
				BLE_RestartFilteredSearch(deviceSearcher);
				deviceCount = 999;
			}
			printf("----------------------------------------\r\nBLE Device Count : %d\r\n\tCurrent Poll ID : %d\r\n", (useFilter ? -1 : deviceCount), pollID);
			for(i = 0 ; i < deviceCount ; ++i)
			{
				if(!useFilter)
				{
					BLE_GetDeviceInfo(i, &devInfo);
				}
				else
				{
					if(BLE_ContinueFilteredSearch(deviceSearcher, &devInfo) != 0)
					{
						break;
					}
				}

				printf("\tDevice %d\r\n\t\tMacAddress : ", i);
				for(j = 0 ; j < 6 ; ++j)
				{
					printf("%02lX%s", devInfo.address[j], (j == 5 ? "" : ":"));
				}
				printf("\r\n\t\tRSSI : %d\r\n\t\tLast Poll : %d\r\n", devInfo.RSSI, devInfo.lastSeen);
				BLE_DEBUG_PrintDeviceMetaData(i);
			}
			printf("----------------------------------------\r\n\r\n");
			if(deviceSearcher)
			{
				BLE_FinishFilteredSearch(deviceSearcher);
				deviceSearcher = NULL;
			}
		}
	}

	
	return 0;
}

#define KEY_CHANGE_UUID 0xFFE0
#define KEY_SECRET_SERVICE 0xA600
#define KEY_PRESSED_READ_UUID 0xFFE1
#define KEY_PRESSED_NOTIFY_UUID 0x2902
#define KEY_SECRET_ATTRIBUTE_UUID 0xA602
#define KEY_ANALYSE_ATTRIBUTE_UUID 0xA400
#define KEY_ANALYSE_ANALYSE_STATUS 0xA401
#define KEY_ANALYSE_ANALYSE_BUFFER 0xA402

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

struct RSSIThreadData
{
	bool run;
	int requestID;
};

DWORD WINAPI RSSIPollThread(LPVOID param)
{
	int pollInterval = 100;
	int pollID = 1;
	int pollNumber = 0;
	printf("RSSI Poll Started\r\n");
	RSSIThreadData* rtd = (RSSIThreadData*)param;

	BLE_SetPollID(pollID);

	while(rtd->run)
	{
		if(pollNumber >= (1000 / pollInterval))
		{
			pollID++;
			BLE_SetPollID(pollID);

			int rssi = 0;
			int ret = BLE_CONN_GetRSSI(rtd->requestID, rssi);
			if(ret != rtd->requestID)
			{
				printf("Error : Could not get average RSSI : Reason [%d] : %s\r\n", ret, RequestErrorCodeToString(ret));
			}
			else
			{
				printf("Average RSSI of %d\r\n", rssi);
			}
			pollNumber = 0;
		}

		pollNumber++;

		BLE_CONN_PollRSSI(rtd->requestID);
		Sleep(pollInterval);
	}

	free(rtd);

	return 0;
}

bool buttonTurnedOff;
void __stdcall buttonPushed(IMiTokenBLEConnection* sender, int buttonID)
{
	printf("Request %d Informed of Button press %d\r\n", sender->GetRequestID(), buttonID);
	if(buttonID & 1)
	{
	//	buttonTurnedOff = true;
	}
}

#ifdef USE_EX
bool EncryptData(const int inputLength, const uint8* inputData, const int keyLength, const uint8* keyData, const int maxOutputLength, int& outputLength, uint8* outputData, uint8* inIvec = NULL, bool dontDoCBCWithCBC = false)
{
	AES_KEY key;
	AES_set_encrypt_key(keyData, keyLength * 8, &key);
	bool newInput = false;
	const uint8* cinput = inputData;
	int newSize = inputLength + 1;
	
	if((inputLength % 0x10) != 0)
	{
		int paddID = (0x10 - (inputLength % 0x10));
		newInput = true;
		newSize = (inputLength + (0x10 - (inputLength % 0x10))) + 1;

		uint8* cinput2 = new uint8[newSize];
		memset(cinput2, paddID, newSize);
		memcpy(cinput2, inputData, inputLength);
		cinput = cinput2;
	}

	outputData[0] = ((newSize - 1) / 0x10);
	outputLength = newSize;

	uint8 ivec[0x10] = {0};
	memset(ivec, 0xCD, 0x10);
	uint8* vector = (inIvec == NULL ? ivec : inIvec);

	if(!dontDoCBCWithCBC)
	{
		AES_cbc_encrypt(cinput, &outputData[1], newSize - 1, &key, (inIvec == NULL ? ivec : inIvec), 1);
	}
	else
	{
		uint8 backupIV[0x10];
		memcpy(backupIV, vector, 0x10);
		for(int i = 0 ;  i < (newSize - 1) ; i += 16)
		{
			memcpy(vector, backupIV, 0x10);
			AES_cbc_encrypt(&cinput[i], &outputData[i + 1], 16, &key, vector, 1);
		}
	}
	
	if(newInput)
	{
		delete[] cinput;
	}
	
	/*
	if((inputLength % 0x10) == 0)
	{
		AES_cbc_encrypt(inputData, outputData + 1, inputLength, &key, (uint8*)ivec, 1);
		outputLength = inputLength + 1;
		outputData[0] = inputLength / 0x10;
	}
	else
	{
		int newSize = (inputLength + (0x10 - (inputLength % 0x10)));
		uint8* tempInput = new uint8[newSize];
		memset(tempInput, 0, newSize);
		memcpy(tempInput, inputData, inputLength);
		AES_cbc_encrypt(tempInput, (outputData + 1), newSize, &key, (uint8*)ivec, 1);
		outputData[0] = newSize / 0x10;
		outputLength = newSize + 1;
	}
	*/
	return true;
}
#endif

#ifdef USE_EX
uint8* EncryptDataV2(const int inputLength, const uint8* inputData, const int keyLength, const uint8* keyData, int blockSize, int& outputSize, bool manualCBC = false, bool dontDoCBCBWithCBC = false, uint8* ivector = NULL)
{
	uint8 ivec[0x10] = {0};
	memset(ivec, 0xCD, 0x10);
	uint8* vector = (ivector == NULL ? ivec : ivector);
	uint8* output = new uint8[blockSize * 2 + 1];
	int outputLen;
	for(int i = 0 ; i < inputLength ; i += blockSize)
	{
		EncryptData((i + blockSize > inputLength ? inputLength - i :  blockSize), &inputData[i], keyLength, keyData, blockSize * 2 + 1, outputLen, (uint8*)output, vector, dontDoCBCBWithCBC);
		printf("\r\nEncrypt Part %02lX\r\n\t[%d] Blocks [%d] : \r\n\t\t", i, outputLen, output[0]);
		for(int j = 1 ; j < outputLen ; ++j)
		{
			printf("%02lX ", output[j]);
			if(j % 0x10 == 0)
			{
				printf("\r\n\t\t");
			}
		}
		if(manualCBC)
		{
			printf("\r\n\tIV :      ");
			for(int j = 0 ; j < 16; ++j)
			{
				ivec[j] = output[outputLen - 16 + j];
				printf("%02lX ", ivec[j]);
			}
		}
	}
	outputSize = outputLen;
	return output;

	return NULL;
}
#endif

const uint8 key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
const uint8 iv[16] = { 0x00 };

const uint8 unencDataToSend[] = {
				0x01, //write
				
				0x38, 0x31, 0x32, 0x33, //block 1
				0x34, 0x35, 0x36, 0x37, 
				0x38, 0x39, 0x61, 0x62, 
				0x63, 0x64, 0x65, 0x66, 

				0x67, 0x68, 0x69, 0x6a, //block 2
				0x6b, 0x6c, 0x6d, 0x6e, 
				0x6f, 0x70, 0x71, 0x72, 
				0x73, 0x74, 0x75, 0x76};


uint8 unencDataEx[] = {
				0x39, 0x31, 0x32, 0x33, //block 1
				0x34, 0x35, 0x36, 0x37, 
				0x38, 0x39, 0x61, 0x62, 
				0x63, 0x64, 0x65, 0x66, 

				0x67, 0x68, 0x69, 0x6a, //block 2
				0x6b, 0x6c, 0x6d, 0x6e, 
				0x6f, 0x70, 0x71, 0x72, 
				0x73, 0x74, 0x75, 0x76};


static uint8 HexToByte(char hexDigit)
{
	if((hexDigit >= '0') && (hexDigit <= '9'))
	{
		return hexDigit - '0';
	}
	//set to lower case
	hexDigit |= 0x20;
	if((hexDigit >= 'a') && (hexDigit <= 'f'))
	{
		return hexDigit - 'a' + 10;
	}
	
	return 0;
}

static uint8* stringToUint8(const char* string)
{
	int slen = strlen(string);
	int blen = slen / 2;
	uint8* ret = new uint8[blen];

	for(int i = 0 ; i < blen ; i++)
	{
		uint8 b1 = HexToByte(string[i * 2]);
		uint8 b2 = HexToByte(string[i * 2 + 1]);
		ret[i] = (b1 << 4) | (b2);
	}

	return ret;
}


enum Stages
{
	STAGE_CONNECTING,
	STAGE_SCANNING_SERVICES_KEY_CHANGE,
	STAGE_SCANNING_SERVICES_KEY_SECRET,
	STAGE_SCANNING_SERVICES_KEY_ANALYSE,
	STAGE_SET_SECRET,
	STAGE_SET_NOTIFY,
	STAGE_WAITING,
	STAGE_GETTING_MESSAGE_BUFFER,
	STAGE_GETTING_CHARACTER_HANDLE,
	STAGE_SETTING_LONG_WRITE,
	STAGE_READ_DATA,
	STAGE_LONG_READ,
	STAGE_GET_READ_BUFFER,
	STAGE_HASH_DATA,
	STAGE_REQUEST_HASH,
	STAGE_READ_HASH,
	STAGE_CHECK_HASH_REQUEST,
	STAGE_CHECK_HASH_STATE,
	STAGE_SCANNING_SERVICES_DEVICE_INFO,
	STAGE_START_READ_DEVICE_SYSTEM_ID,
	STAGE_READING_DEVICE_SYSTEM_ID,
	STAGE_REQUEST_FIRMWARE_VERSION,
	STAGE_READ_FIRMWARE_VERSION,
};


bool NormalStageOutput(int response, int& oldResponse, const char* successMessage, const char* failMessage, int& stage, int nextStage)
{
	bool ret = false;
	if(response ==BLE_CONN_SUCCESS)
	{
		printf("%s\r\n", successMessage);
		stage = nextStage;
		ret = true;
	}
	else
	{
		if(response != oldResponse)
		{
			printf("%s : error code [%d] [%s]\r\n", failMessage, response, RequestErrorCodeToString(response));
		}
	}

	oldResponse = response;
	return ret;
}

int ScanServicesNearUUID(int requestID, int& stage, int& lastResponse, int nextStage, uint16 UUID)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, UUID, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int ScanServicesNearKeyChangeUUID(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, KEY_CHANGE_UUID, 0x20);
	NormalStageOutput(resp, lastResponse, "Scanning services from KEY_CHANGE_UUID", "Cannot start scanning yet", stage, nextStage);
	return resp;
}

int ScanServicesNearSecretServiceUUID(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ScanServicesInRange(requestID, KEY_SECRET_SERVICE, 0x20);
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
	if(NormalStageOutput(resp, lastResponse, "Set Attribute for KEY_SECRET_ATTRIBUTE_UUID", "Cannot set attribute yet", stage, nextStage))
	{
		messageBuffer = NULL;
	}
	return resp;
}

int SetNotifyAttribute(int requestID, int& stage, int& lastResponse, int nextStage)
{
	static uint8 notify[] = {0x01, 0x00};
	int resp = BLE_CONN_SetAttribute(requestID, KEY_PRESSED_NOTIFY_UUID, sizeof(notify), notify);
	if(NormalStageOutput(resp, lastResponse, "Set Attribute for KEY_PRESSED_NOTIFY_UUID to Enabled", "Cannot set attribute yet", stage, nextStage))
	{
		buttonTurnedOff = false;
		BLE_CONN_ButtonPressedCallback(requestID, buttonPushed);
	}
	return resp;
}

int GetSecretCharacterHandle(int requestID, int& stage, int& lastResponse, int nextStage, uint16& characteristicHandle)
{
	int resp = BLE_CONN_GetCharacteristicHandle(requestID, KEY_SECRET_ATTRIBUTE_UUID, characteristicHandle);
	NormalStageOutput(resp, lastResponse, "Got Characteristic Handle for KEY_SECRET_ATTRIBUTE_UUID", "Cannot get Attribute Handle yet", stage, nextStage);
	return resp;
}

int LongReadSecret(int requestID, int& stage, int& lastResponse, int nextStage)
{
	int resp = BLE_CONN_ReadLong(requestID, KEY_SECRET_ATTRIBUTE_UUID);
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
	if(NormalStageOutput(resp, lastResponse, "Read data length from attribute", "Cannot long read from attribute yet", stage, stage))
	{
		readBuffer = new uint8[readLength];
		resp = BLE_CONN_GetAttribute(requestID, attributeUUID, readLength, readLength, readBuffer);
		if(NormalStageOutput(resp, lastResponse, "Read data from attribute", "Error : 2nd attribute read failed : Weird ", stage, nextStage))
		{
			printf("Read %d bytes from handle [%04lX]\r\n", readLength, attributeUUID);
		}
	}
	return resp;
}

int DecryptBuffer(uint8 readLength, uint8* readBuffer, int& outLength, uint8*& outBuffer)
{
	int ret;
	outLength = 0;
	outBuffer = NULL;

	ret = BLE_EXT_DecryptReadBlock(readLength, readBuffer, outLength, outBuffer);
	if(ret == -1) //-1 == need buffer of 'outlength'
	{
		outBuffer = new uint8[outLength];
		ret = BLE_EXT_DecryptReadBlock(readLength, readBuffer, outLength, outBuffer);
		if(ret == 0)
		{
			return ret;
		}
		else
		{
			printf("Error : Could not decrypt the buffer : ret code of %d\r\n", ret);
		}
	}
	else
	{
		printf("Error : Could not get required buffer length : ret code of %d\r\n", ret);
	}

	return ret;
}

char ConnectV2(mac_address address)
{
	bool useNoCBC = false;
	int requestID = 0;
	int oldreqID = 0;
	uint16 characterHandle;
	int stage = STAGE_SCANNING_SERVICES_KEY_CHANGE;
	int resp = 0;
	MessageBufferObject messageBuffer;

	printf("Using V2 Connection Mode\r\n");


	printf("Getting Exclusive Mode\r\n");
	if(BLE_NP_RequestExclusiveAccess() != BLE_CONN_SUCCESS)
	{
		printf("\tFailed To request Exclusive Mode \r\n");
		return 0;
	}

	bool waitForExclusive = true;
	while(waitForExclusive)
	{
		Sleep(100);
		printf("Checking Exclusive Mode : ");
		switch(BLE_NP_HasExclusiveAccess())
		{
		case BLE_API_EXCLUSIVE_ALLOWED:
			printf("Allowed\r\n");
			waitForExclusive = false;
			break;
		case BLE_API_NP_COM_FALLBACK:
			printf("Using COM\r\n");
			waitForExclusive = false;
			break;
		case BLE_API_EXCLUSIVE_ASKED:
			printf("Asked\r\n");
			break;
		case BLE_API_EXCLUSIVE_DISABLED:
			printf("Disabled\r\n");
			break;
		case BLE_API_EXCLUSIVE_REJECTED:
			printf("Rejected\r\n");
			return 0;
		}
	}

	if(BLE_CONN_StartProfile(address, requestID) != BLE_CONN_SUCCESS)
	{
		printf("Error : StartProfile failed\r\n");
		BLE_NP_ReleaseExclusiveAccess();
		return 0;
	}
	oldreqID = requestID;
	char c = 0;
	static uint8 notify[] = {0x01, 0x00};

	//						   num blocks [2], 1 = store, [data]
	//						   num blocks [1], 3 = read, [junk - bin0s]
	static uint8 secret[33] = {0x02,0x01, 0xDE, 0xAD, 0xBE, 0xEF};

	uint8 dataToSend[0x100];
	int dataSendLength;
	int outputLen;

	/*
	printf("\r\nNo CBC\r\n");
	delete [] EncryptDataV2(sizeof(unencDataToSend), unencDataToSend, sizeof(key), key, 16, outputLen);
	printf("\r\nCBC\r\n");
	delete [] EncryptDataV2(sizeof(unencDataToSend), unencDataToSend, sizeof(key), key, 16, outputLen, true);
	printf("\r\nWhole Block\r\n");
	delete [] EncryptDataV2(sizeof(unencDataToSend), unencDataToSend, sizeof(key), key, sizeof(unencDataToSend), outputLen);
	printf("\r\nWhole Block No CBC\r\n");


	printf("\r\n\r\nEXAMPLE DATA! \r\n\r\n");
	static uint8* exampleVector = stringToUint8("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710");
	static uint8* exampleOutput = stringToUint8("7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b273bed6b8e3c1743b7116e69e222295163ff1caa1681fac09120eca307586e1a7");
	static uint8* exampleKey = stringToUint8("2b7e151628aed2a6abf7158809cf4f3c");
	static uint8* exampleIV = stringToUint8("000102030405060708090a0b0c0d0e0f");
	uint8* exampleTest = EncryptDataV2(64, exampleVector, 16, exampleKey, 64, outputLen, false, false, exampleIV);
	if(memcmp(exampleTest + 1, exampleOutput, 64) == 0)
	{
		printf("\r\nExample Data passed\r\n");
	}
	else
	{
		printf("\r\nERROR : Example Data FAILED\r\n");
	}
	
	

	uint8* dataNoCBC = EncryptDataV2(sizeof(unencDataToSend), unencDataToSend, sizeof(key), key, sizeof(unencDataToSend), outputLen, false, true);
	*/

	dataSendLength = sizeof(dataToSend);
	uint8* ptrBuffer = (uint8*)dataToSend;
	int ret;

	srand(time(NULL));

	printf("Randomizing Data : ");
	for(int irnd = 0 ; irnd < 4; ++irnd)
	{
		unencDataEx[irnd] = (rand() % 0x0A) + 0x30;
		printf("%c", unencDataEx[irnd]);
	}
	printf("\r\n");

	if((ret = BLE_EXT_MakeStoreBuffer(sizeof(unencDataEx), unencDataEx, dataSendLength, ptrBuffer)) == -1)
	{
		ptrBuffer = new uint8[dataSendLength];
		ret = BLE_EXT_MakeStoreBuffer(sizeof(unencDataEx), unencDataEx, dataSendLength, ptrBuffer);
	}
	
	if(ret != 0)
	{
		printf("BLE_EXT_MakeStoreBuffer failed with return code %d\r\n", ret);
		//return ' ';
	}

	uint8* reqReadBuffer = NULL;
	int reqReadBufferLength = 0;
	if((ret = BLE_EXT_MakeReadBlock(0, NULL, reqReadBufferLength, reqReadBuffer)) == -1)
	{
		reqReadBuffer = new uint8[reqReadBufferLength];
		ret = BLE_EXT_MakeReadBlock(0, NULL, reqReadBufferLength, reqReadBuffer);
	}

	if(ret != 0)
	{
		printf("BLE_EXT_MakeReadBuffer failed with return code %d\r\n", ret);
		//return ' ';
	}
	
	bool hashVersion1_1 = false;
	bool runFunction = true;
	RSSIThreadData *bp = NULL;
	bool hasBeenConnected = false;
	while(runFunction)
	{
		if(oldreqID == BLE_CONN_BAD_REQUEST)
		{
			break;
		}

		IMiTokenBLEConnection* pConnection = BLE_GetConnectionV2Wrapper();
		if(!pConnection->IsConnected())
		{
			if(hasBeenConnected)
			{
				runFunction = false;
			}
		}
		else
		{
			hasBeenConnected = true;
		}

		printf(".");
		Sleep(100);
		switch(stage)
		{
		case STAGE_SCANNING_SERVICES_KEY_CHANGE:
			ScanServicesNearKeyChangeUUID(requestID, stage, oldreqID, STAGE_SCANNING_SERVICES_DEVICE_INFO);
			break;

		case STAGE_SCANNING_SERVICES_KEY_ANALYSE:
			//TODO - which between new and old depending on hashVersion1_1
			{
				uint16 UUID = (hashVersion1_1 ? BLE_UUID_SERVICE_HASH : BLE_UUID_SERVICE_HASH_OLD);
				ScanServicesNearUUID(requestID, stage, oldreqID, STAGE_HASH_DATA, UUID);
			}
			break;

		case STAGE_SCANNING_SERVICES_KEY_SECRET:
			ScanServicesNearSecretServiceUUID(requestID, stage, oldreqID, STAGE_SCANNING_SERVICES_DEVICE_INFO);
			break;

		case STAGE_HASH_DATA:
			{
				static uint8 dataToHash[20] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
				static uint8 dataToHash_Blu[20 + 16] = {0};

				if(hashVersion1_1)
				{
					IMiTokenBLEConnection* pConnection = BLE_GetConnectionV2Wrapper();

					mac_address maddr;
					pConnection->GetAddress(maddr);

					/*
					for(int i = 0 ; i < 3; ++i)
					{
						uint8 temp = maddr.addr[i];
						maddr.addr[i] = maddr.addr[5 - i];
						maddr.addr[5 - i] = temp;
					}
					/**/

					int location = 0;
					while(location < 16)
					{
						memcpy(dataToHash_Blu + location, &maddr, sizeof(maddr));
						location += sizeof(maddr);
					}
					
//					memset(dataToHash_Blu, 0, 3);
					memcpy(dataToHash_Blu + 16, dataToHash, 20);

					WriteDataToUUID(requestID, stage, oldreqID, STAGE_CHECK_HASH_REQUEST, BLE_UUID_ATTRIBUTE_HASH_BUFFER, sizeof(dataToHash_Blu), dataToHash_Blu);
				}
				else
				{
					WriteDataToUUID(requestID, stage, oldreqID, STAGE_CHECK_HASH_REQUEST, BLE_UUID_ATTRIBUTE_HASH_BUFFER_OLD, sizeof(dataToHash), dataToHash);
				}
			}
			break;

		case STAGE_CHECK_HASH_REQUEST:
			{
				uint16 UUID = (hashVersion1_1 ? BLE_UUID_ATTRIBUTE_HASH_STATUS : BLE_UUID_ATTRIBUTE_HASH_STATUS_OLD);
				StartReadAttribute(requestID, stage, oldreqID, STAGE_CHECK_HASH_STATE, UUID);
			}
			break;
			
		case STAGE_CHECK_HASH_STATE:
			{
				uint16 UUID = (hashVersion1_1 ? BLE_UUID_ATTRIBUTE_HASH_STATUS : BLE_UUID_ATTRIBUTE_HASH_STATUS_OLD);
				uint8 readLen = 0;
				uint8* readPtr = NULL;
				if(ReadAttributeBuffer(requestID, stage, oldreqID, STAGE_CHECK_HASH_REQUEST, UUID, readLen, readPtr) == BLE_CONN_SUCCESS)
				{
					if(readLen == 1)
					{
						if(readPtr[0] == 0)
						{
							stage = STAGE_REQUEST_HASH;
						}
					}
					else
					{
						stage = STAGE_CHECK_HASH_REQUEST;
					}

					delete[] readPtr;
				
				}
			}
			break;

		case STAGE_REQUEST_HASH:
			{
				uint16 UUID = (hashVersion1_1 ? BLE_UUID_ATTRIBUTE_HASH_BUFFER: BLE_UUID_ATTRIBUTE_HASH_BUFFER_OLD);
				StartReadAttribute(requestID, stage, oldreqID, STAGE_READ_HASH, UUID);
			}
			break;

		case STAGE_READ_HASH:
			{
				//Sleep(5000);
				uint16 UUID = (hashVersion1_1 ? BLE_UUID_ATTRIBUTE_HASH_BUFFER: BLE_UUID_ATTRIBUTE_HASH_BUFFER_OLD);
				uint8 readLen = 0;
				uint8* readPtr = NULL;
				if(ReadAttributeBuffer(requestID, stage, oldreqID, STAGE_GETTING_MESSAGE_BUFFER, UUID, readLen, readPtr) == BLE_CONN_SUCCESS)
				{
					for(int hashi = 0 ; hashi < readLen;  ++hashi)
					{
						printf("%02lX ", readPtr[hashi]);
					}
					printf("\r\n");
				}
			}
			break;

		case STAGE_GETTING_MESSAGE_BUFFER:
			stage = STAGE_SET_NOTIFY;
			break;

			messageBuffer = BLE_CONN_CreateNewMessageBuffer();
			stage = STAGE_GETTING_CHARACTER_HANDLE;
			break;
		
		case STAGE_GETTING_CHARACTER_HANDLE:
			GetSecretCharacterHandle(requestID, stage, oldreqID, STAGE_SETTING_LONG_WRITE, characterHandle);
			break;

		case STAGE_SETTING_LONG_WRITE:
			BLE_CONN_AddMessageToMessageBuffer(messageBuffer, characterHandle, dataSendLength, dataToSend);
			stage = STAGE_SET_SECRET;
			break;

		case STAGE_SET_SECRET:
			SetSecretAttribute(requestID, stage, oldreqID, STAGE_SET_NOTIFY, messageBuffer);
			break;

		case STAGE_SET_NOTIFY:
			SetNotifyAttribute(requestID, stage, oldreqID, STAGE_WAITING);
			break;

		case STAGE_READ_DATA:
			messageBuffer = BLE_CONN_CreateNewMessageBuffer();			
			BLE_CONN_AddMessageToMessageBuffer(messageBuffer, characterHandle, reqReadBufferLength, reqReadBuffer);
			BLE_CONN_WriteMessageBuffer(requestID, messageBuffer);
			messageBuffer = NULL;
			stage = STAGE_LONG_READ;
			break;

		case STAGE_LONG_READ:
			Sleep(1000);
			
			LongReadSecret(requestID, stage, oldreqID, STAGE_GET_READ_BUFFER);
			break;

		case STAGE_GET_READ_BUFFER:
			{
			
				uint8 readLength = 0;
				uint8* readBuffer = NULL;
				
				if(ReadAttributeBuffer(requestID, stage, oldreqID, STAGE_SCANNING_SERVICES_DEVICE_INFO, KEY_SECRET_ATTRIBUTE_UUID, readLength, readBuffer) == BLE_CONN_SUCCESS)
				{
					int decryptedLength = 0;
					uint8* decryptedData = NULL;
					
					if(DecryptBuffer(readLength, readBuffer, decryptedLength, decryptedData) == 0)
					{
						printf("Buffer successfully decrypted : Found %d bytes\r\n\t", decryptedLength);
						for(int dbufi = 0 ; dbufi < decryptedLength; ++dbufi)
						{
							printf("%02lX ", decryptedData[dbufi]);
						}
						printf("\r\n");
					}
				}
			}
			break;

		case STAGE_SCANNING_SERVICES_DEVICE_INFO:
			ScanServicesNearUUID(requestID, stage, oldreqID, STAGE_START_READ_DEVICE_SYSTEM_ID, BLE_UUID_SERVICE_DEVICE_INFO);
			break;

		case STAGE_START_READ_DEVICE_SYSTEM_ID:
			StartReadAttribute(requestID, stage, oldreqID, STAGE_READING_DEVICE_SYSTEM_ID, BLE_UUID_ATTRIBUTE_DEVICE_INFO_SYSTEM_ID);
			break;
		case STAGE_READING_DEVICE_SYSTEM_ID:
			{
				uint8 readLen = 0;
				uint8* readPtr = NULL;
				if(ReadAttributeBuffer(requestID, stage, oldreqID, STAGE_REQUEST_FIRMWARE_VERSION, BLE_UUID_ATTRIBUTE_DEVICE_INFO_SYSTEM_ID, readLen, readPtr) == BLE_CONN_SUCCESS)
				{
					printf("Read Device System ID [%d] bytes\r\n\t", readLen);
					for(int i = 0 ; i < readLen ; ++i)
					{
						printf("%02lX ", readPtr[i]);
					}
					printf("\r\n");
				}
			}
			break;


		case STAGE_REQUEST_FIRMWARE_VERSION:
			StartReadAttribute(requestID, stage, oldreqID, STAGE_READ_FIRMWARE_VERSION, BLE_UUID_ATTRIBUTE_DEVICE_INFO_FIRMWARE_VERSION);
			break;

		case STAGE_READ_FIRMWARE_VERSION:
			{
				uint8 readLen = 0;
				uint8* readPtr = nullptr;
				if(ReadAttributeBuffer(requestID, stage, oldreqID, STAGE_SCANNING_SERVICES_KEY_ANALYSE, BLE_UUID_ATTRIBUTE_DEVICE_INFO_FIRMWARE_VERSION, readLen, readPtr) == BLE_CONN_SUCCESS)
				{
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
						}
						else if(strcmp("Blu", versionBuffer) == 0)
						{
							printf("Running version Blu : MAC_ADDRESS before hashing\r\n");
							hashVersion1_1 = true;
						}
						else
						{
							printf("Running unknown version \r\n");
							stage = STAGE_WAITING;
						}

						if(versionBuffer != (char*)&stackBuffer)
						{
							delete versionBuffer;
						}
					}

				}
			}
			break;
		case STAGE_WAITING:
			printf("Type 'Q' to quit\r\n");
			{
				while(((c = getchar()) | 0x20) != 'q')
				{
					if((c | 0x20) == 'p')
					{
						if(bp == NULL)
						{
							bp = (RSSIThreadData*)malloc(sizeof(RSSIThreadData));
							bp->requestID = requestID;
							bp->run = true;
							CreateThread(NULL, 0, RSSIPollThread, bp, NULL, NULL);
						}
					}

					if(buttonTurnedOff)
					{
						break;
					}
				};
				if(bp != NULL)
				{
					bp->run = false;
				}
				BLE_CONN_Disconnect(requestID);
				runFunction = false;
			}
		
			break;
		}
	}

	BLE_NP_ReleaseExclusiveAccess();

	return c;
}

void GotNPMessage(uint8* data, int length, int connectionID)
{
	printf("Got message via NP from [%d] of length [%d]\r\n\t", connectionID, length);
	for(int i = 0 ; i < length ; ++i)
	{
		printf("%c", data[i]);
	}
	printf("\r\n");

}


int main()
{
	const char* COMPort = "COM3";
	const char* NamedPipe = "\\\\.\\pipe\\Mi-TokenBLE";

	running = (BLE_InitializeEx(COMPort, NamedPipe, false) == 0);

	//BLE_NP_SetMessageCallback(GotNPMessage);

	if(running)
	{
		CreateThread(NULL, 0, PollThread, NULL, 0, NULL);
	}

	while(running)
	{
		printf("Running\r\n");
		char c = getchar();

		if((c | 0x20) == 'q')
		{
			showPolls = false;
			running = false;
		}
		if((c >= '0') && (c <= '9'))
		{
			int deviceCount = BLE_GetDeviceFoundCount();
			int cint = (c - '0');
			if(cint >= deviceCount)
			{
				printf("Device %c doesn't exist\r\n", c);
			}
			else
			{
				printf("Connecting to device %c\r\n", c);
				showPolls = false;
				DeviceInfo devInfo;
				BLE_GetDeviceInfo(cint, &devInfo);
				mac_address add;
				memcpy(add.addr, devInfo.address, 6);
				c = ConnectV2(add);
				//running = false;
				showPolls = true;
				Sleep(1000);
				printf("Attempting to restart the scanner\r\n");
				BLE_RestartScanner();
			}
		}
		if((c | 0x20) == 't')
		{
			c = getchar();
			if((c >= '0') && (c <= '9'))
			{
				int cint = (c - '0');
				int deviceCount = BLE_GetDeviceFoundCount();
				if(cint >= deviceCount)
				{
					printf("Device %c doesn't exist\r\n", c);
				}
				else
				{
					printf("Informing watcher to watch device %c\r\n", c);
					DeviceInfo devInfo;
					BLE_GetDeviceInfo(cint, &devInfo);
					mac_address add;
					memcpy(add.addr, devInfo.address, 6);
					uint8 command[] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
					memcpy(command + 1, add.addr, 6);
					BLE_NP_SendMessage(command, sizeof(command), -1);
				}
			}
		}
		if((c | 0x20) == 'h')
		{
			showPolls = !showPolls;
		}
		if((c | 0x20) == 'r')
		{
			printf("Attempting to restart the scanner\r\n");
			BLE_RestartScanner();
		}
		if((c | 0x20) == 'f')
		{
			useFilter = !useFilter;
		}
		if((c | 0x20) == 'p')
		{
			int rrr = BLE_ReleaseCOMPort();
			printf("Requested Release of COM Port : returned %d\r\n", rrr);
		}
		if((c | 0x20) == 's')
		{
			uint8 sBuffer[100];
			int bufferPos = 0;
			while(c != 10)
			{
				if(bufferPos >= 100)
				{
					break;
				}
				c = getchar();
				sBuffer[bufferPos] = c;
				bufferPos++;
			}

			BLE_NP_SendMessage(sBuffer, bufferPos, -1);

		}
		if((c | 0x20) == 'a')
		{
			BLE_Finalize();
		}
		if((c | 0x20) == 'e')
		{
			printf("Requesting Exclusive Mode - note this wont be released, shutdown the program to have the server release exclusive mode\r\n");
			BLE_NP_RequestExclusiveAccess();

		}
		while(c != 10)
		{
			c = getchar();
		}
	}

	
	printf("Exiting\r\n");
	BLE_Finalize();
	getchar();
	int x =  0;
}

