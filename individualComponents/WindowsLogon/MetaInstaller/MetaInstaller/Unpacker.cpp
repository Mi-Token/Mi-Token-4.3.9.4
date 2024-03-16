#include "Unpacker.h"


long findMSILength(unsigned char* buffer, long startPosition)
{
	long t = 0;
	memcpy(&t, &(buffer[startPosition]), sizeof(t));
	return t;
}
long findMSIStart(unsigned char* buffer, long bufferLength)
{
	int pos = 0;
	for(int i = 0 ; i < bufferLength ; ++i)
	{
		if(buffer[i] == msiStartBuffer[pos])
		{
#if debugging
			printf("Found pos %d [%d] {%02lX-%02lX-%02lX}\n", pos, i, i > 0 ? buffer[i - 1] : 0, buffer[i], (i + 1) < bufferLength ? buffer[i+1] : 0);
#endif
			if(pos == 19) //we have found the entire buffer
			{
				long dat;
				memcpy(&dat, &(buffer[i+1]), sizeof(long));
				if(dat != i)
				{
#if debugging
					printf("Found MSI Buffer however dat != location {%d vs %d)", dat, i);
#endif
				}
				else
					return (i + 1 + sizeof(long)); //return the position after the header + 4 bytes location
			}
			pos++;
			
		}
		else
			pos = 0;
	}
	return -1;
}




bool unpackAll(PackedData *pData, BYTE *packedBuffer, long bufferLength, int& fileType)
{
	if(pData == NULL)
		return false;
	if(packedBuffer == NULL)
		return false;

	SAFE_FREE(pData->applicationData);
	SAFE_FREE(pData->autoConfigData);
	SAFE_FREE(pData->msi64Data);
	SAFE_FREE(pData->msi86Data);

	long MSIHeader = findMSIStart(packedBuffer, bufferLength);
	if(MSIHeader == -1)
	{
		pData->msi86Len = 0;
		pData->msi64Len = 0;
		pData->autoConfigLen = 0;
		pData->applicationData = (BYTE*)calloc(bufferLength, sizeof(BYTE));
		memcpy(pData->applicationData, packedBuffer, bufferLength);
		pData->applicationLen = bufferLength;
		if(fileType == EMBEDDED_FILE_DEFAULT)
		{
			fileType = EMBEDDED_FILE_MSI;
		}
		return true;
	}
	else
	{
		//there is actually a header
		headerBlockMSI header;
		memcpy(&header, &(packedBuffer[MSIHeader]), sizeof(headerBlockMSI));
		if(fileType == EMBEDDED_FILE_DEFAULT)
		{
			fileType = header.fileType;
		}
		BYTE* loc = &(packedBuffer[MSIHeader + sizeof(headerBlockMSI)]);
		
		pData->applicationLen = MSIHeader - sizeof(msiStartBuffer);
		pData->applicationData = (BYTE*)calloc(pData->applicationLen, sizeof(BYTE));
		memcpy(pData->applicationData, packedBuffer, pData->applicationLen);

		pData->autoConfigLen = header.autoConfigLen;
		pData->msi64Len = header.x64Len;
		pData->msi86Len = header.x86Len;

		pData->autoConfigData = (BYTE*)calloc(pData->autoConfigLen, sizeof(BYTE));
		pData->msi64Data = (BYTE*)calloc(pData->msi64Len, sizeof(BYTE));
		pData->msi86Data = (BYTE*)calloc(pData->msi86Len, sizeof(BYTE));

		memcpy(pData->autoConfigData, &(loc[header.autoConfigStart]), pData->autoConfigLen);
		memcpy(pData->msi64Data, &(loc[header.x64Start]), pData->msi64Len);
		memcpy(pData->msi86Data, &(loc[header.x86Start]), pData->msi86Len);
	
		return true;
	}
}
