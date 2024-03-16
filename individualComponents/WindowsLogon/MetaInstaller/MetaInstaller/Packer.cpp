#include "Packer.h"

void setupHeaderData(headerBlockMSI& header, const PackedData packData, const long FileType, const bool sameFilesForBoth)
{
	memset(&header, 0, sizeof(headerBlockMSI));
	header.headerSize = sizeof(header);
	header.fileType = FileType;
	header.x86Start = 0;
	header.x86Len = packData.msi86Len;
	if(sameFilesForBoth)
	{
		header.x64Start = header.x86Start;
		header.x64Len = header.x86Len;
	}
	else
	{
		header.x64Start = header.x86Len + header.x86Start;
		header.x64Len = packData.msi64Len;
	}
	header.autoConfigStart = header.x64Len + header.x64Start;
	header.autoConfigLen = packData.autoConfigLen;
}

bool packData(PackedData packData, BYTE** packedData, long* packedDataLength, long FileType, bool sameFilesForBoth)
{
	if(packedData == NULL)
		return false;

	if(*packedData != NULL)
		free(*packedData);

	long len = 0;
	if(sameFilesForBoth)
		len = packData.applicationLen + packData.autoConfigLen + packData.msi64Len + sizeof(msiStartBuffer) + sizeof(headerBlockMSI);
	else
		len = packData.applicationLen + packData.autoConfigLen + packData.msi64Len + packData.msi86Len + sizeof(msiStartBuffer) + sizeof(headerBlockMSI);

	*packedDataLength = len;
	*packedData= (BYTE*)calloc(len, sizeof(BYTE));

	
	//Pre-Memcpy operations : 
	//the 4 byte pos reference
	long pos = packData.applicationLen + sizeof(msiStartBuffer) - sizeof(long) - 1;
	//Setup the header
	headerBlockMSI header;
	setupHeaderData(header, packData, FileType, sameFilesForBoth);

	BYTE* loc = *packedData;

	//now for many memcpys
	memcpy(loc, packData.applicationData, packData.applicationLen);
	loc += packData.applicationLen;
	memcpy(loc, msiStartBuffer, sizeof(msiStartBuffer));
	loc += sizeof(msiStartBuffer) - sizeof(long);
	memcpy(loc, &pos, sizeof(long));
	loc += sizeof(long);
	memcpy(loc, &header, sizeof(headerBlockMSI));
	loc += sizeof(headerBlockMSI);

	//loc now directly maps with the values in header block
	memcpy(&(loc[header.x86Start]), packData.msi86Data, header.x86Len);

	if(!sameFilesForBoth) //don't copy if we are using the same files for both
		memcpy(&(loc[header.x64Start]), packData.msi64Data, header.x64Len);
	
	memcpy(&(loc[header.autoConfigStart]), packData.autoConfigData, header.autoConfigLen);

	return true;
}