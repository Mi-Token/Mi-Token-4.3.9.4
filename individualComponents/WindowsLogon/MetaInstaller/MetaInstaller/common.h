#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <atlbase.h>
#include <atlConv.h>

const unsigned char msiStartBuffer [] = { 'M', 'S', 'I', 'S', 0x6F, 0x49, 0x97, 0x51, 0x36, 0x2E, 0x45, 0x83, 0x84, 0xED, 0xB0, 0x41, 0xF6, 0xA3, 0x8E, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };

//Default header block - depending on fileType the child headerblock to use may change
typedef struct _headerBlock
{
	long headerSize;
	long fileType; //fileType will not only determine if it is an MSI / EXE but also if it is a set of files
} headerBlock;

//header for all current fileTypes
typedef struct _headerBlockMSI : _headerBlock
{
	long x86Start;
	long x86Len;
	long x64Start;
	long x64Len;
	long autoConfigStart;
	long autoConfigLen;
} headerBlockMSI;


//header used for a single file name
typedef struct _headerBlockFileData
{
	long start; //start location of file data
	long len; //length of the stored file
	int filenameStart; //start location of the files name
	int filenameLen; //length of the stored files name
} headerBlockFileData;

//header at the start of multi-file blocks (Virtual files created when fileType = PACK)
typedef struct _headerBlockFiles
{
	long headerSize;
	int fileCount; //how many stored files
	int commandLineStart; //location of the commandline to use when installing
	int commandLineLength; //length of the commandline to use when installing
	int commandLineUStart; //location of the commandline to use when uninstalling
	int commandLineULength; //length of the commandline to use when uninstalling
} headerBlockFiles;

typedef struct _PackedData
{
	BYTE *applicationData;
	BYTE *msi86Data;
	BYTE *msi64Data;
	BYTE *autoConfigData;
	long applicationLen;
	long msi86Len;
	long msi64Len;
	long autoConfigLen;
} PackedData;

char* getExePath();

typedef struct _FileData
{
	FILE* fp;
	BYTE* data;
	long length;
} FileData, *pFileData;

int openFile(pFileData fileData, const char* path, const char* mode);
int closeFile(pFileData fileData, bool freeBuffers = true);
int readFile(pFileData fileData);
int writeFile(pFileData fileData);

#ifndef SAFE_FREE(p)
#define SAFE_FREE(p) { if((p) != NULL) { free(p); (p) = NULL; } }
#endif

#ifndef SAFE_SET(p, v)
#define SAFE_SET(p, v) { if((p) != NULL) { free(p); } (p) = (v); }
#endif

extern bool wow64;

BOOL IsWow64();

int Base64ToChar(const unsigned char* pszData, int length, unsigned char* ppszOutput, unsigned short *outputLength);

#define EMBEDDED_FILE_ERROR (-1)
#define EMBEDDED_FILE_MSI 0
#define EMBEDDED_FILE_EXE 1
#define EMBEDDED_FILE_PACK 2
#define EMBEDDED_FILE_BOOT 3
#define EMBEDDED_FILE_DEFAULT (-2)
