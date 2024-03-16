#include "base64.h"

#ifndef NULL
#define NULL (0)
#endif

const char* base64LookupTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";
int CharToBase64(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength)
{
	int outlen;
	unsigned char* pszOutput;
	int i;

	if(0 == pszData || 0 == ppszOutput)
		return -1;

	//Find the output length given the input length
	outlen = length * 4 / 3;
	while(outlen % 4 != 0)
	{
		outlen++;
	}

	if(outlen >= *outputLength)
		return -2;
	

	//if ppszOutput[0] is initialised, then delete it
	if(ppszOutput == NULL)
	{
		return -3;
	}
	else
	{
		pszOutput = ppszOutput;
	}
	
	//Make ppszOutput[0] a new char array of outlen
	
	
	for(i = 0 ; i < length; i += 3)
	{
		//get the 3 input bytes we are going to use
		unsigned char in[3];
		unsigned char out[4];
		int j;

		for(j = 0 ; j < 3; ++j)
		{
			in[j] = (i + j < length ? pszData[i + j] : 0);
		}

		//get the 4 output values we are going to use
		//Includes checks so data that is only from outside the range is a '='
		if(i >= length)
			out[0] = 64;
		else
			out[0] = (in[0] & 0xFC) >> 2;
		
		if(i >= length)
			out[1] = 64;
		else
			out[1] = ((in[0] & 0x03) << 4) + ((in[1] & 0xF0) >> 4);

		if(i + 1 >= length)
			out[2] = 64;
		else
			out[2] = ((in[1] & 0x0F) << 2) + ((in[2] & 0xC0) >> 6);

		if(i + 2 >= length)
			out[3] = 64;
		else
			out[3] = (in[2] & 0x3F);

		//now do the lookup
		for(j = 0 ; j < 4; ++j)
		{
			pszOutput[(i/3) * 4 + j] = base64LookupTable[out[j]];			
		}
	}
	pszOutput[outlen] = 0;
	outlen++;
	*outputLength = outlen;
	return 0;
}

int Base64ToChar(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength)
{
	return 0;
}

/*
int Base64ToChar(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength)
{
	int outlen,i;
	unsigned char* pszOutput;
	if(0 == pszData || 0 == ppszOutput)
		return RET_NULL_PARAMETER;

	length--;
	//Find the output length given the input length
	if(length % 4 != 0)
		return RET_MALFORMED; //not a valid base64 string

	outlen = length / 4 * 3;

	//if ppszOutput[0] is initialised, then delete it
	if(ppszOutput == NULL)
	{
		return RET_NULL_PARAMETER;
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
					return DATA_FIELD_INCORRECT_LENGTH;
				pszOutput[(i/4) * 3] = val[0] << 2;
				//outlen is actually i / 4 * 3 + 1
				outlen = i / 4 * 3 + 1;
				break;
			}

			if((i/4)*3 > *outputLength)
					return DATA_FIELD_INCORRECT_LENGTH;
			pszOutput[(i/4) * 3] = (val[0] << 2) + ((val[1] & 0x30) >> 4);

			if(val[2] == 0xFF)
			{
				if(((i/4)*3 + 1) >= *outputLength)
					return DATA_FIELD_INCORRECT_LENGTH;
				pszOutput[(i/4) * 3 + 1] = (val[1] & 0x0F) << 4;
				//outlen is actually i / 4 * 3 + 2
				outlen = i / 4 * 3 + 2;
				break;
			}
			
			if(((i/4)*3 + 1) >= *outputLength)
					return DATA_FIELD_INCORRECT_LENGTH;
			//val[3] must be -1
			pszOutput[(i/4) * 3 + 1] = ((val[1] & 0x0F) << 4) + ((val[2] & 0x3C) >> 2);
            if(((i/4)*3 + 2) >= *outputLength)
                return DATA_FIELD_INCORRECT_LENGTH;
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
				return DATA_FIELD_INCORRECT_LENGTH;

			pszOutput[(i/4) * 3 + j] = outData[j];
		}
	}

	*outputLength = outlen;
	return RET_SUCCESS;
}

*/