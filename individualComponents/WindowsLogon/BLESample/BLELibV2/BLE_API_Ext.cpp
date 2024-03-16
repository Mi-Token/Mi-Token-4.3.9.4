#include "BLE_API_Ext.h"

#ifdef BLEAPI_USEEX

#include <string>

#define COMMAND_BYTE_STORE ((uint8)0x01)
#define COMMAND_BYTE_ERASE ((uint8)0x02)
#define COMMAND_BYTE_READ ((uint8)0x03)

#define EXT_ERR_SUCCESS 0
#define EXT_ERR_MORE_DATA -1
#define EXT_ERR_INVALID_FORMAT -2
#define EXT_ERR_FAILED -3

bool EncryptData(const int inputLength, const uint8* inputData, const int keyLength, const uint8* keyData, const int maxOutputLength, int& outputLength, uint8* outputData, uint8* inIvec = NULL)
{
	AES_KEY key;
	AES_set_encrypt_key(keyData, keyLength * 8, &key);
	bool newInput = false;
	const uint8* cinput = inputData;
	int newSize = inputLength;

	if((inputLength % 0x10) != 0)
	{
		int padID = (0x10 - (inputLength % 0x10));
		newInput = true;
		newSize = (inputLength + padID);
		uint8* cinput2 = new uint8[newSize];
		
		memcpy(cinput2, inputData, inputLength);
		memset(cinput2 + inputLength, padID, padID);
		cinput = cinput2;
	}

	uint8 ivec[0x10] = {0};
	memset(ivec, 0xCD, 0x10);
	uint8* vector = (inIvec == NULL ? ivec : inIvec);

	AES_cbc_encrypt(cinput, outputData, newSize, &key, vector, 1);

	return true;
}

bool DecryptData(const int inputLength, const uint8* inputData, const int keyLength, const uint8* keyData, const int maxOutputLength, int& outputLength, uint8* outputData, uint8* ivec)
{
	AES_KEY key;
	AES_set_decrypt_key(keyData, keyLength * 8, &key);
	if(maxOutputLength < inputLength)
	{
		outputLength = inputLength;
		return false;
	}

	AES_cbc_encrypt(inputData, outputData, inputLength, &key, ivec, 0);

	return true;
}

const uint8 key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };

int ConvertByteStreamToCommandStream(const int inputLength, const uint8* inputData, const uint8 commandByte, uint8* ivector, const int maxOutputLength, int& outputLength, uint8* outputData)
{
	int encStreamLength = inputLength + 1; //[command Byte] + [input]
	encStreamLength += (0x10 - (encStreamLength % 0x10)); //add padding bytes to make encStreamLength a multiple of 16
	
	int commandStreamLength = encStreamLength + 1 + 16; //[ivector] + [encStream block count] + [encStream]

	outputLength = commandStreamLength;
	if((commandStreamLength > maxOutputLength) || (outputData == NULL))
	{
		return -1;
	}

	memcpy(outputData + 0, ivector, 16);
	outputData[16] = (encStreamLength / 0x10);
	uint8* dataToEncrypt = new uint8[encStreamLength];
	dataToEncrypt[0] = commandByte;
	memcpy(dataToEncrypt + 1, inputData, inputLength);
	//padding
	int padID = encStreamLength - inputLength - 1;
	memset(dataToEncrypt + 1 + inputLength, padID, padID);
	int EncDataLength;
	if(!EncryptData(encStreamLength, dataToEncrypt, sizeof(key), key, encStreamLength, EncDataLength, outputData + 17, ivector))
	{
		free(dataToEncrypt);
		return -2;
	}
	free(dataToEncrypt);

	return 0;
}

int GetDataFromCommandStream(const int inputLength, const uint8* inputData, int& outLength , uint8*& outputData)
{
	if(inputLength < 17)
	{
		//not enough data : need at least [16 byte IV] + [1 byte block count]
		return -2;
	}
	uint8 blockCount = inputData[16];
	int expectedlength = 17 + (blockCount * 16);

	if(inputLength != expectedlength)
	{
		outLength = expectedlength;
		return -3;
	}

	int expectedOutputLength = (blockCount * 16);
	if((outLength < expectedOutputLength) || (outputData == NULL))
	{
		outLength = expectedOutputLength;
		return -1;
	}

	uint8 iv[16];
	memcpy(iv, inputData, 16);
	const uint8* encDataStream = inputData + 17;

	if(!DecryptData((blockCount * 16), encDataStream, sizeof(key), key, outLength, outLength, outputData, iv))
	{
		return -4;
	}

	return 0;
}


int EXT_MakeStoreBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	uint8 ivec[16];
	for(int i = 0; i < 16 ; ++i)
	{
		ivec[i] = rand() & 0xFF;
	}

	int requiredLength = 0;
	int retCode;
	retCode = ConvertByteStreamToCommandStream(length, data, COMMAND_BYTE_STORE, ivec, outlength, requiredLength, outdata);
	outlength = requiredLength;
	return retCode;	
}

int EXT_MakeReadBlock(int length, const uint8* data,int& outlength, uint8*& outdata)
{
	uint8 ivec[16];
	for(int i = 0 ; i < 16 ; ++i)
	{
		ivec[i] = rand() & 0xFF;
	}

	int requiredLength = 0;
	int retCode;
	retCode =  ConvertByteStreamToCommandStream(length, data, COMMAND_BYTE_READ, ivec, outlength, requiredLength, outdata);
	outlength = requiredLength;
	return retCode;
}




int EXT_DecryptReadBlock(int length, const uint8* data, int& outlength, uint8*& outdata)
{
	int retCode, requiredLength;
	retCode = GetDataFromCommandStream(length, data, outlength, outdata);
	//outlength = requiredLength;
	return retCode;
}

#endif