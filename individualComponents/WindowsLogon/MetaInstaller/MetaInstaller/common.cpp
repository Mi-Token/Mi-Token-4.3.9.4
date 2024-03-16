#include "common.h"

bool wow64;
extern bool isRDP; //defined in CredFilter

char * getExePath()
{
	char * wresult = (char *)calloc(MAX_PATH, sizeof(char));
	GetModuleFileName( NULL, wresult, MAX_PATH ) ;
	return wresult;
}

int openFile(pFileData fileData, const char* path, const char* mode)
{
	if(fileData == NULL)
		return 0;
	fileData->fp = fopen(path, mode);
	fileData->data = NULL;
	fileData->length = 0;

	return (int)fileData->fp;
}

int closeFile(pFileData fileData, bool freeBuffers)
{
	if(fileData == NULL)
		return 0;

	fclose(fileData->fp);
	if(freeBuffers)
	{
		SAFE_FREE(fileData->data);
		fileData->length = 0;
	}
	return 1;
}

int readFile(pFileData fileData)
{
	if(fileData == NULL)
		return 0;

	if(fileData->fp == NULL)
		return 0;

	SAFE_FREE(fileData->data);
	fseek(fileData->fp, 0, SEEK_END);
	fileData->length = ftell(fileData->fp);
	fseek(fileData->fp, 0, SEEK_SET);
	fileData->data = (BYTE*)calloc(fileData->length, sizeof(BYTE));
	if(fileData->data == NULL)
		return 0;

	fread(fileData->data, sizeof(BYTE), fileData->length, fileData->fp);

	return (int)fileData->data;
}

int writeFile(pFileData fileData)
{
	if(fileData == NULL)
		return -1;
	if(fileData->fp == NULL)
		return -2;
	if(fileData->data == NULL)
		return -3;

	return (int)fwrite(fileData->data, sizeof(BYTE), fileData->length, fileData->fp);
}


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;


BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;
	//IsWow64Process isn't supported on all systems
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if(NULL != fnIsWow64Process)
	{
		if(!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//there is an error :(
		}
	}
	return bIsWow64;
}



int Base64ToChar(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength)
{
	int outlen,i;
	unsigned char* pszOutput;
	if(0 == pszData || 0 == ppszOutput)
		return -1;

	//length--;
	//Find the output length given the input length
	if(length % 4 != 0)
		return -2; //not a valid base64 string

	outlen = length / 4 * 3;

	//if ppszOutput[0] is initialised, then delete it
	if(ppszOutput == NULL)
	{
		return -3;
	}
	else
	{
		pszOutput = ppszOutput;
	}

	for(i = 0 ; i < length; i += 4)
	{
		unsigned char val[4];
		unsigned char outData[3];
		int j;
		//get the binary values of the 4 numbers (0->63)
		for(j = 0  ; j < 4; ++j)
		{
			unsigned char data = pszData[i + j];
			if('A' <= data && data <= 'Z')
				val[j] = data - 'A';
			else if('a' <= data && data <= 'z')
				val[j] = data - 'a' + 26;
			else if('0' <= data && data <= '9')
				val[j] = data - '0' + 52;
			else if('+' == data)
				val[j] = 62;
			else if('/' == data)
				val[j] = 63;
			else if('=' == data)
				val[j] = 0xFF;
		}
		if(val[3] == 0xFF)
		{
			//we are going to have to clip things short
			if(val[0] == 0xFF)
			{
				//interesting, didn't expect that.
				//Outlen is actually i / 4 * 3
				outlen = i / 4 * 3;
				break;
			}
			else if(val[1] == 0xFF)
			{
				if((i/4)*3 >= *outputLength)
					return -4;
				pszOutput[(i/4) * 3] = val[0] << 2;
				//outlen is actually i / 4 * 3 + 1
				outlen = i / 4 * 3 + 1;
				break;
			}

			if((i/4)*3 > *outputLength)
					return -5;
			pszOutput[(i/4) * 3] = (val[0] << 2) + ((val[1] & 0x30) >> 4);

			if(val[2] == 0xFF)
			{
				if(((i/4)*3 + 1) >= *outputLength)
					return -6;
				pszOutput[(i/4) * 3 + 1] = (val[1] & 0x0F) << 4;
				//outlen is actually i / 4 * 3 + 2
				outlen = i / 4 * 3 + 2;
				break;
			}
			
			if(((i/4)*3 + 1) >= *outputLength)
					return -7;
			//val[3] must be -1
			pszOutput[(i/4) * 3 + 1] = ((val[1] & 0x0F) << 4) + ((val[2] & 0x3C) >> 2);
            if(((i/4)*3 + 2) >= *outputLength)
                return -8;
            pszOutput[(i/4) * 3 + 2] = (val[2] & 0x03) << 6;
			//outlen is i / 4 * 3 + 3
			outlen = i / 4 * 3 + 3;
			break;
		}
		else
		{
			outData[0] = (val[0] << 2) + ((val[1] & 0x30) >> 4);
			outData[1] = ((val[1] & 0x0F) << 4) + ((val[2] & 0x3C) >> 2);
			outData[2] = ((val[2] & 0x03) << 6) + val[3];
		}
		for(j = 0 ; j < 3 ; ++j)
		{
			if(((i/4) * 3 + j) > outlen)
				continue;
			if(((i/4) * 3 + j) >= *outputLength)	
			{
				printf("%d vs %d [%d : %d]\n", (i/4) * 3 + j, *outputLength, i, j);
				return -9;
			}

			pszOutput[(i/4) * 3 + j] = outData[j];
		}
	}

	*outputLength = outlen;
	return 1;
}
