#include "common.h"
#include "packer.h"
#include "unpacker.h"
#include "Installer.h"
#include "SetupBypass.h"
#include <vector>
#include <assert.h>


//20 bytes of pseudo-random data to mark the start of where the MSI data is

/*
const unsigned char msiStartBuffer [] = { 'M', 'S', 'I', 'S', 0x6F, 0x49, 0x97, 0x51, 0x36, 0x2E, 0x45, 0x83, 0x84, 0xED, 0xB0, 0x41, 0xF6, 0xA3, 0x8E, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
typedef struct _headerBlock
{
	long x86Start;
	long x86Len;
	long x64Start;
	long x64Len;
	long autoConfigStart;
	long autoConfigLen;
} headerBlock;
*/

//Main Return codes
#define MAIN_RET_SUCCESS 0
#define MAIN_RET_INVALIDARG -1
#define MAIN_RET_INVALIDARGCOUNT -2
#define MAIN_RET_MISSINGARG -3
#define MAIN_RET_FAILEDTOOPENOUTPUT -4
#define MAIN_RET_INVALIDCONFIGDATA -5

const char* helpText = 
"This program can be used for both generating and using a packed MSI\n\n"
"INSTALLING Use:\n"
"\tSuply no arguments.\n\n"
"PACKING Use:\n"
"\t[-i path] [-x86 path|NULL] [-x64 path|NULL] [-ac path|NULL] -o path\n"
"\t\t-i : The input file that will be appended to.\n"
"\t\t\tIf omitted the currently running one is used.\n"
"\t\t-x86 : The path to the MSI to run on 32 bit systems.\n"
"\t\t\tIf omitted the MSI that is currently bound is kept.\n"
"\t\t\tUse DEL to remove the file.\n"
"\t\t-x64 : The path to the MSI to run on 64 bit systems.\n"
"\t\t\tIf omitted the MSI that is current bound is kept.\n"
"\t\t\tUse DEL to remove the file.\n"
"\t\t-ac : The path to the AutoConfigFile to use.\n"
"\t\t\tIf omitted the currently bound AutoConfigFile is kept.\n"
"\t\t\tUse DEL to remove the file.\n"
"\t\t-o : The path for the output file to be written to.\n"
"\t\t\tNOTE : Cannot be the same file as -i.\n";

#define ARG_INPUT_FILE 0
#define ARG_X86_FILE 1
#define ARG_X64_FILE 2
#define ARG_AC_FILE 3
#define ARG_OUT_FILE 4
#define ARG_TYPE_FILE 5
#define ARG_CMD_LINE_X86 6
#define ARG_CMD_LINE_X64 7
#define ARG_CMD_LINE_UX86 8
#define ARG_CMD_LINE_UX64 9
#define MAX_ARG_ID 10

#define ARG_GET_EMBEDDED_INFO 100
//not yet used
//#define ARG_EXTRACT_EMBEDDED_INFO 101 
#define ARG_TEST_EMBEDDED_INFO 102

#define ARG_UNINSTALL 200

#define ARG_ERR -1

#define EMBEDDED_X86 1
#define EMBEDDED_X64 2
#define EMBEDDED_CONFIG 4

int getArgumentType(const char* arg)
{
	if(strcmp(arg, "-i") == 0)
		return ARG_INPUT_FILE;
	if(strcmp(arg, "-x86") == 0)
		return ARG_X86_FILE;
	if(strcmp(arg, "-x64") == 0)
		return ARG_X64_FILE;
	if(strcmp(arg, "-ac") == 0)
		return ARG_AC_FILE;
	if(strcmp(arg, "-o") == 0)
		return ARG_OUT_FILE;
	if(strcmp(arg, "-GetInfo") == 0)
		return ARG_GET_EMBEDDED_INFO;
	if(strcmp(arg, "-TestAC") == 0)
		return ARG_TEST_EMBEDDED_INFO;
	if(strcmp(arg, "-U") == 0)
		return ARG_UNINSTALL;
	if(strcmp(arg, "-FileType") == 0)
		return ARG_TYPE_FILE;
	if(strcmp(arg, "-x86cmd") == 0)
		return ARG_CMD_LINE_X86;
	if(strcmp(arg, "-x64cmd") == 0)
		return ARG_CMD_LINE_X64;
	if(strcmp(arg, "-x64cmdu") == 0)
		return ARG_CMD_LINE_UX64;
	if(strcmp(arg, "-x86cmdu") == 0)
		return ARG_CMD_LINE_UX86;
	return ARG_ERR;
}

int getFileType(const char* arg)
{
	if(strcmp(arg, "MSI") == 0)
		return EMBEDDED_FILE_MSI;
	if(strcmp(arg, "EXE") == 0)
		return EMBEDDED_FILE_EXE;
	if(strcmp(arg, "PACK") == 0)
		return EMBEDDED_FILE_PACK;
	if(strcmp(arg, "BOOT") == 0)
		return EMBEDDED_FILE_BOOT;
	return EMBEDDED_FILE_ERROR;
}
#define DeleteInput(ArgId) ((input[ArgId] != NULL) && (strcmp(input[ArgId], "DEL") == 0))
const char* input[MAX_ARG_ID] = {0};


const char* setFilePath(const char * input)
{
	if(input == NULL)
		return input;

	if(input[0] != '\\')
		return input;
	//printf("Backslash detected\n");
	char* exePath = getExePath();
	int len = strlen(exePath) - 1;
	while(exePath[len] != '\\')
		len--;

	int olen = strlen(input) + len;
	char* buffer = (char*)calloc(olen + 1, sizeof(char));

	memcpy(buffer, exePath, len);
	memcpy(&(buffer[len]), input, strlen(input));

	return buffer;
}

//dont allow the files '.' or '..' through as valid files
bool validFolderFile(const char* fname)
{
	if(strcmp(".", fname) == 0)
		return false;
	if(strcmp("..", fname) == 0)
		return false;
	return true;
}

//memcpy a complete string into a newly alloced buffer
char* allocCopyStr(const char* src)
{
	int slen = strlen(src);
	char* r = (char*)malloc(slen + 1);
	memcpy(r, src, slen);
	r[slen] = 0;
	return r;
}

//structure that stores info for all the files returned by the search result
typedef struct _folderFileInfo
{
public:
	char* name;
	char* filename;
	DWORD sizeLow;
	DWORD sizeHigh;


	void release()
	{
		free(name);
		free(filename);
	}
	static _folderFileInfo* create(const char* name, DWORD sizelow, DWORD sizehigh, const char* filepath)
	{
		_folderFileInfo* fpme = (_folderFileInfo*)calloc(1, sizeof(_folderFileInfo));

		fpme->filename =allocCopyStr(name);
		fpme->sizeLow = sizelow;
		fpme->sizeHigh = sizehigh;
		
		int plen = strlen(filepath), nlen = strlen(name);
		fpme->name = (char*)calloc(plen + nlen, 1);  //filepath has an '*' at the end that we need to ignore (hence no +1)
		memcpy(fpme->name, filepath, plen - 1); //-1 to get rid of the extra '*'
		memcpy(fpme->name + plen - 1, name, nlen);
		return fpme;
	}
}FolderFileInfo;


//this acts like a memcpy but will increment the value pointed to by offset by the amount written
//Therefore when memcpying a lot of data from different sources to one buffer you do not need to change destStart.
//Instead just keep track of the value pointed to by offset and each write will start directly after the previous one
void memcpyoffset(void* destStart, int* offset, void* src, int len)
{

	memcpy((void*)((int)destStart + (*offset)), src, len);
	*offset = (*offset) + len;
}

void fixCommandLine(char* cmdLine)
{
	//change any 's to "s (as you cannot use " in the input command arg, but it may be needed in the output command arg when installing/uninstalling
	int i = 0;
	while(cmdLine[i])
	{
		if(cmdLine[i] == '\'')
			cmdLine[i] = '\"';
		i++;
	}
}
bool openInputFolder(pFileData fileData, int argNumber, const char* commandLine, const char* commandLineUninstall)
{
	if(argNumber > 4)
		return false;
	if((input[argNumber] == NULL) || (DeleteInput(argNumber)))
	{
		fileData->data = NULL;
		fileData->length = 0;
		fileData->fp = NULL;
		return true;
	}
	//now we shall read all the files in the directory and put them all in 1 fileData structure
	//So therefore the "file" that fileData will point to will be a block of a header for # of files + their individual names + sizes +
	// the fileData appended to the end of the header.
	int slen = strlen(input[argNumber]);
	if(input[argNumber][slen - 1] != '*')
	{
		//last character should be an "*"
		return false;
	}
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind =  FindFirstFile(input[argNumber], &FindFileData);
	std::vector<FolderFileInfo*> filenames;
	//dsize is the size of all non-filedata info (ie. Headers + Filenames + Command Lines)
	int dsize = 0;
	//fsize is the size of all included files (just their raw data)
	int fsize = 0;

	if(hFind == INVALID_HANDLE_VALUE)
		return false;

	//read through every file that matches the search filter creating a list of their names and sizes
	if(validFolderFile(FindFileData.cFileName))
	{
		printf("Found file : %s\n", FindFileData.cFileName);
		FolderFileInfo* pFFI = FolderFileInfo::create(FindFileData.cFileName, FindFileData.nFileSizeLow, FindFileData.nFileSizeHigh, input[argNumber]);
		filenames.push_back(pFFI);
		dsize += strlen(pFFI->filename);
		fsize += FindFileData.nFileSizeLow;
		assert(FindFileData.nFileSizeHigh == 0);
	}
	while(FindNextFile(hFind, &FindFileData) != 0)
	{
		if(validFolderFile(FindFileData.cFileName))
		{
			printf("Found file : %s\n", FindFileData.cFileName);
			FolderFileInfo* pFFI = FolderFileInfo::create(FindFileData.cFileName, FindFileData.nFileSizeLow, FindFileData.nFileSizeHigh, input[argNumber]);
			filenames.push_back(pFFI);
			dsize += strlen(pFFI->filename);
			fsize += FindFileData.nFileSizeLow;
			assert(FindFileData.nFileSizeHigh == 0);
		}
	}

	//fix the command lines (change ' to ")
	fixCommandLine(const_cast<char*>(commandLine));
	fixCommandLine(const_cast<char*>(commandLineUninstall));
	
	dsize += strlen(commandLine) + strlen(commandLineUninstall);
	dsize += sizeof(_headerBlockFiles); //we require 1 headerBlockFiles struct
	dsize += (sizeof(_headerBlockFileData) * filenames.size()); //we require 1 headerBlockFileData struct for each file
	void* vp = calloc(dsize + fsize, 1); //allocate all the data

	//packing all the data
	void* vpc;

	//variables that will be used for packing these files
	_headerBlockFiles* pHBF;
	_headerBlockFileData* pHBFD;
	//where the current block being written to the 'data' section should be placed (offset from void* vp)
	int dataStartOffset = sizeof(_headerBlockFiles) + (sizeof(_headerBlockFileData) * filenames.size());
	//where the current block being written to the 'file' section should be placed (offset from void* vp)
	int fileDataStart = /*dataStartOffset + */ dsize;

	//Create the main header
	pHBF = (_headerBlockFiles*)vp;
	pHBF->headerSize = dataStartOffset;
	pHBF->fileCount = filenames.size();

	//Setting the commandline args
	pHBF->commandLineLength = strlen(commandLine);
	pHBF->commandLineStart = dataStartOffset;
	memcpyoffset(vp, &dataStartOffset, (void*)commandLine, pHBF->commandLineLength);
	pHBF->commandLineULength = strlen(commandLineUninstall);
	pHBF->commandLineUStart = dataStartOffset;
	memcpyoffset(vp, &dataStartOffset, (void*)commandLineUninstall, pHBF->commandLineULength);

	//vpc will point to the location where our current headerBlockFileData header will be stored
	vpc = (void*)((int)vp + sizeof(_headerBlockFiles));
	
	for(std::vector<FolderFileInfo*>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)
	{
		pHBFD = (_headerBlockFileData*)vpc;
		//set filename data
		pHBFD->filenameStart = dataStartOffset;
		pHBFD->filenameLen = strlen((*it)->filename);
		//copy filename data
		memcpyoffset(vp, &dataStartOffset, (*it)->filename, pHBFD->filenameLen);
		//set file data
		pHBFD->start = fileDataStart;
		pHBFD->len = (*it)->sizeLow;
		//load the file data
		void* fdata;
		FILE* fp;
		fp = fopen((*it)->name, "rb");
		printf("Packing file %s...", (*it)->filename);
		if(fp != NULL)
		{
			fdata = calloc(pHBFD->len, 1);
			int bread = 0, btot = pHBFD->len;
			while(bread != btot)
			{
				int tread = fread((char*)fdata + bread, 1, btot - bread, fp);
				if(tread == 0)
					break;

				bread += tread;
			}
			printf("Done\n");
			fclose(fp);
			//place the filedata in the current fileData block
			memcpyoffset(vp, &fileDataStart, fdata, bread);
			free(fdata);
		}
		else
		{
			assert(0);
		}
		//move vpc to the location of the next HeaderBlockFileData header
		vpc = (void*)((int)vpc + sizeof(_headerBlockFileData));
	}

	//free allocated memory for the found files
	for(std::vector<FolderFileInfo*>::iterator it = filenames.begin() ; it != filenames.end() ; ++it)
	{
		(*it)->release();
	}
	filenames.clear();

	//setup file data. FP is null as this is just a virtual file.
	fileData->fp = NULL;
	fileData->data = (BYTE*)vp;
	fileData->length = dsize + fsize;
	//
	return false;

}

bool openInputFile(pFileData fileData, int argNumber)
{
	if(argNumber > 4)
		return false;
	if((input[argNumber] == NULL) || (DeleteInput(argNumber)))
	{
		//we dont want any data so fill the struct with blank data
		fileData->data = NULL;
		fileData->length = 0;
		fileData->fp = NULL;
		return true;
	}
	if(openFile(fileData, input[argNumber], "rb") == 0)
	{
		printf("Failed to open file\n");
		fileData->data = NULL;
		fileData->length = 0;
		fileData->fp = NULL;
		return true;
	}
	readFile(fileData);
	closeFile(fileData, false);

	return true;
}

void updatePackedData(pFileData fileData, BYTE** packedData, long* packedLength)
{
	if(fileData->data == NULL)
		return;

	if(packedData == NULL)
		return;

	SAFE_FREE(*packedData);
	*packedData = fileData->data;
	*packedLength = fileData->length;
}

int ProcessEmbeddedInfo()
{
	printf("Returning Embedded Info\n");
	char* path = getExePath();
	FileData fd = {0};
	if(openFile(&fd, path, "rb") == 0)
		printf("failed to open the file\n");
	readFile(&fd);
	PackedData pData = {0};
	int embeddedFileType = EMBEDDED_FILE_DEFAULT;
	unpackAll(&pData, fd.data, fd.length, embeddedFileType);
	closeFile(&fd);
	/*
	printf("%d/%d\n%d/%d\n%d/%d\n%d/%d\n", 
		pData.applicationData, pData.applicationLen,
		pData.autoConfigData, pData.autoConfigLen,
		pData.msi64Data, pData.msi64Len,
		pData.msi86Data, pData.msi86Len);
	*/
	int val = ((pData.autoConfigLen == 0 ? 0 : EMBEDDED_CONFIG) |
			(pData.msi64Len == 0 ? 0 : EMBEDDED_X64) |
			(pData.msi86Len == 0 ? 0 : EMBEDDED_X86));
	//printf("val = %d\n", val);
	//getchar();
	printf("This installer contains the following\n32 Bit Installer : %s\n64 Bit Installer : %s\nAuto Config File : %s\n", ((val & EMBEDDED_X86) == 0) ? "no" : "yes", ((val & EMBEDDED_X64) == 0) ? "no" : "yes", ((val & EMBEDDED_CONFIG) == 0) ? "no" : "yes");
	return val;
}

int ProcessTestEmbeddedInfo()
{
	printf("Testing Embedded Info\n");
	char* path = getExePath();
	FileData fd = {0};
	if(openFile(&fd, path, "rb") == 0)
		printf("failed to open the file\n");
	readFile(&fd);
	PackedData pData = {0};
	int embeddedFileType = EMBEDDED_FILE_DEFAULT;
	unpackAll(&pData, fd.data, fd.length, embeddedFileType);
	closeFile(&fd);
	const char* errorMessage = NULL;
	bool rollback;
	if(!WriteBypassSettings(pData.autoConfigData, pData.autoConfigLen, &errorMessage, &rollback))
	{
		printf("Embedded Config Failed : %s\n", errorMessage);
		getchar();
		return MAIN_RET_INVALIDCONFIGDATA;
	}
	
	if(errorMessage != NULL)
		printf("Embedded Config May has failed : %s\n", errorMessage);
	else
		printf("Done.\n");

	getchar();
	return MAIN_RET_SUCCESS;	
}

int ReadCommandLineArgs( int argc, const char* argv[] )
{
	for(int arg = 1 ; arg < argc ; arg += 2)
	{
		int type = getArgumentType(argv[arg]);
		if(type == ARG_ERR)
		{
			printf("Invalid argument [%s]\n", argv[arg]);
			printf("%s", helpText);
			getchar();
			return MAIN_RET_INVALIDARG;
		}
		else
		{
			input[type] = argv[arg+1];
		}
	}
	return MAIN_RET_SUCCESS;
}

void FixFilePaths()
{
	//printf("Fixing file paths\n");
	for(int i = 0 ; i < 5 ; ++i)
		input[i] = setFilePath(input[i]);
}

bool CheckForSameFilePaths()
{
	if(!((input[ARG_X64_FILE] == NULL) || (input[ARG_X86_FILE] == NULL)))
	{
		if(strcmp(input[ARG_X64_FILE], input[ARG_X86_FILE]) == 0)
			return true;
	}
	return false;
}

void ReadInputFiles(FileData& fdInput, FileData& fdAC, FileData& fdX64, FileData& fdX86, int& embeddedFileType)
{
	//printf("%s\n", input[ARG_INPUT_FILE]);
	openInputFile(&fdInput, ARG_INPUT_FILE);
	//printf("%s\n", input[ARG_AC_FILE]);
	openInputFile(&fdAC, ARG_AC_FILE);

	if(embeddedFileType == EMBEDDED_FILE_PACK)
	{
		//Seperate packing for EMBEDDED_FILE_PACK types
		openInputFolder(&fdX64, ARG_X64_FILE, input[ARG_CMD_LINE_X64], input[ARG_CMD_LINE_UX64]);
		openInputFolder(&fdX86, ARG_X86_FILE, input[ARG_CMD_LINE_X86], input[ARG_CMD_LINE_UX86]);
	}
	else
	{
		//printf("%s\n", input[ARG_X86_FILE]);
		openInputFile(&fdX86, ARG_X86_FILE);
		//printf("%s\n", input[ARG_X64_FILE]);
		openInputFile(&fdX64, ARG_X64_FILE);
	}
}

void CheckAndDeleteSection(const int sectionID, BYTE*& data, long& length)
{
	if(DeleteInput(sectionID))
	{
		//printf("MSI x86\n");
		SAFE_FREE(data);
		length = 0;
	}
}

int ProcessCommandLineArgs( int argc, const char* argv[] )
{
	if(getArgumentType(argv[1]) == ARG_GET_EMBEDDED_INFO)
	{
		return ProcessEmbeddedInfo();
	}

	if(getArgumentType(argv[1]) == ARG_TEST_EMBEDDED_INFO)
	{
		return ProcessTestEmbeddedInfo();
	}

	if(getArgumentType(argv[1]) == ARG_UNINSTALL)
	{
		installMSI(false);
		return 0;
	}

	if(argc % 2 == 0)
	{
		printf("Invalid number of arguments received\n");
		printf("%s", helpText);
		return MAIN_RET_INVALIDARGCOUNT;
	}

	int ret = ReadCommandLineArgs(argc, argv);
	if(ret != MAIN_RET_SUCCESS)
	{
		return ret;
	}
	
	FixFilePaths();

	//printf("Printing Found Arguments:\n");
	/*
	printf("Input File : %s\nX86 MSI : %s\nX64 MSI : %s\nAC File : %s\nOut File : %s\n", 
		input[0] == NULL ? "NULL" : input[0],
		input[1] == NULL ? "NULL" : input[1],
		input[2] == NULL ? "NULL" : input[2],
		input[3] == NULL ? "NULL" : input[3],
		input[4] == NULL ? "NULL" : input[4]);
	/**/

	if(input[ARG_OUT_FILE] == NULL)
	{
		printf("Missing required argument [-o Path]\n%s", helpText);
		getchar();
		return MAIN_RET_MISSINGARG;
	}

	if(input[ARG_INPUT_FILE] == NULL)
		input[ARG_INPUT_FILE] = getExePath();
	
	int embeddedFileType = EMBEDDED_FILE_DEFAULT; //default to MSI
	if(input[ARG_TYPE_FILE] != NULL)
	{
		embeddedFileType = getFileType(input[ARG_TYPE_FILE]);
		if(embeddedFileType == EMBEDDED_FILE_ERROR)
		{
			printf("Invalid Embedded FileType Value of %s\n", input[ARG_TYPE_FILE]);
			return MAIN_RET_INVALIDARG;
		}
		printf("Set File Type : %d\n", embeddedFileType);
		
	}

	//open the input file + others
	FileData fdInput = {0};
	FileData fdX86 = {0};
	FileData fdX64 = {0};
	FileData fdAC = {0};
	FileData fdOutput = {0};

	bool sameFilePaths = CheckForSameFilePaths();

	printf("Reading Files\n");
	ReadInputFiles(fdInput, fdAC, fdX64, fdX86, embeddedFileType);
	
	//unpack the input file
	printf("Unpacking data\n");
	PackedData packedData = {0};
	unpackAll(&packedData, fdInput.data, fdInput.length, embeddedFileType);

	//delete sections
	printf("Deleting sections\n");
	CheckAndDeleteSection(ARG_X86_FILE, packedData.msi86Data, packedData.msi86Len);
	CheckAndDeleteSection(ARG_X64_FILE, packedData.msi64Data, packedData.msi64Len);
	CheckAndDeleteSection(ARG_AC_FILE, packedData.autoConfigData, packedData.autoConfigLen);

	//add new input
	printf("Adding new input\n");
	//Should never need to write application data
	//updatePackedData(&fdInput, &packedData.applicationData, &packedData.applicationLen);
	updatePackedData(&fdX86, &packedData.msi86Data, &packedData.msi86Len);
	updatePackedData(&fdX64, &packedData.msi64Data, &packedData.msi64Len);
	updatePackedData(&fdAC, &packedData.autoConfigData, &packedData.autoConfigLen);

	//pack the new file
	printf("Packing file\n");
	BYTE* newBuffer = NULL;
	long newBufferLen;
	packData(packedData, &newBuffer, &newBufferLen, embeddedFileType, sameFilePaths);
	//printf("%08lX / %d\n", newBuffer, newBufferLen);
	//save the new output file
	FileData outFile;
	if(openFile(&outFile, input[ARG_OUT_FILE], "wb+") == 0)
	{
		printf("Failed to open output file [%s]\nAborting.\n", input[ARG_OUT_FILE]);
		getchar();
		return MAIN_RET_FAILEDTOOPENOUTPUT;
	}

	printf("Writing file\n");
	outFile.data = newBuffer;
	outFile.length = newBufferLen;
	//printf("%08lX / %d\n", outFile.data, outFile.length);
	writeFile(&outFile);
	closeFile(&outFile, false);
	printf("Done");


	return 0;
}

int main( int argc, const char* argv[] )
{
#ifdef _DEBUG
	printf("Press enter to continue");
	getchar();
#endif
	if(argc < 2) //no supplied arguments - install mode
	{
		installMSI(true);
	}
	else
	{
		int ret = ProcessCommandLineArgs(argc, argv);
		if(ret != 0)
			return ret;
		//getchar();
	}
#ifdef _DEBUG
	getchar();
#endif
	return MAIN_RET_SUCCESS;
}