#include "Installer.h"
#include "Unpacker.h"
#include "SetupBypass.h"


char* createCommandLine(const char* msiPath, const char* configPath, const long fileType)
{
	printf("File Type is %d\r\n", fileType);
	//buffer = msiexec.exe /i "msiPath" (%configPath% == NULL ? "" : APICONFIG=%configPath%);
	if(fileType == EMBEDDED_FILE_MSI)
	{
		printf("Creating MSI based commandline\n");
		const char* msiexec = "msiexec.exe /i ";
		const char* silentInstall = configPath == NULL ? "" : "/passive ";
		const char* configID = configPath == NULL ? "" : "USEAC=1";
		size_t execLen = strlen(msiexec), msiLen = strlen(msiPath), configLen = strlen(configID);
		size_t tLen = execLen + msiLen + configLen + 5 + strlen(silentInstall);
		char* buffer = (char*)calloc(tLen,sizeof(char));
		sprintf_s(buffer, tLen, "%s\"%s\" %s %s", msiexec, msiPath, silentInstall, configID);
		return buffer;
	}
	if(fileType == EMBEDDED_FILE_EXE)
	{
		printf("Creating EXE based commandline\n");
		//buffer = "Start "msiPath"
		size_t tLen = strlen(msiPath) + 10;
		char* buffer = (char*)calloc(tLen, sizeof(char));
		sprintf_s(buffer, tLen, "Start \"%s\"", msiPath);
		return buffer;
	}
	if(fileType == EMBEDDED_FILE_PACK)
	{
		size_t tlen = strlen(msiPath);
		char* buffer = (char*)calloc(tlen + 1, sizeof(char));
		memcpy(buffer, msiPath, tlen);
		return buffer;
	}
	if(fileType == EMBEDDED_FILE_BOOT)
	{
		printf("Creating Bootstapper based commandline\n");
		const char* silentInstall = configPath == NULL ? "" : "/passive ";
		const char* configID = configPath == NULL ? "" : "USEAC=1";
		size_t msiLen = strlen(msiPath), configLen = strlen(configID);
		size_t tLen = msiLen + configLen + 5 + strlen(silentInstall);
		char* buffer = (char*)calloc(tLen,sizeof(char));
		printf("Path : %s\r\nSilent : %s\r\nConfig : %s\r\n", msiPath, silentInstall, configID);
		sprintf_s(buffer, tLen, "\"%s\" %s %s", msiPath, silentInstall, configID);
		return buffer;
	}
}

char* createRollbackCommandLine(const char* msiPath, int fileDataType)
{
	switch(fileDataType)
	{
	case EMBEDDED_FILE_MSI:
		{
			const char* msiexec = "msiexec.exe /uninstall ";
			const char* silentInstall = "/passive ";
			size_t execLen = strlen(msiexec), msiLen = strlen(msiPath);
			size_t tLen = execLen + msiLen + strlen(silentInstall) + 5;
			char* buffer = (char*)calloc(tLen, sizeof(char));;
			sprintf_s(buffer, tLen, "%s\"%s\" %s", msiexec, msiPath, silentInstall);
			return buffer;
		}
	case EMBEDDED_FILE_EXE:
		{
			//no current commandline type for exe files
		}
	case EMBEDDED_FILE_PACK:
		{
			//Packs store the uninstall data inside the MSI path
			//MSIPath is [Install Cmdline]\0[Uninstall CmdLine]\0\0"
			int offset = strlen(msiPath);
			const char* uPath = &msiPath[offset + 1];
			int slen = strlen(uPath);
			char* buffer = (char*)calloc(slen + 1, 1);
			memcpy(buffer, uPath, slen);
			return buffer;
		}
	}
}




char* unpackItem(char* path, char* extention, unsigned char *buffer, long start, long length)
{
	size_t extraLen = strlen(extention), pathLen = strlen(path) - 4, tlen = extraLen + pathLen + 1;
	char* pPath = (char*)calloc(tlen, sizeof(char));
	memcpy(pPath, path, pathLen);
	memcpy(&(pPath[pathLen]),  extention, extraLen);
	FILE* fp = fopen(pPath, "wb");
	printf("Extracting Item %s [%d bytes]\n", path, length);
	if(fp != NULL)
	{
		fwrite(&(buffer[start]), sizeof(unsigned char), length, fp);
		fclose(fp);
		return pPath;
	}
	free(pPath);
	return NULL;
}

char* unpackItemPack(char* path, unsigned char* buffer, long start, long length)
{
	headerBlockFiles* pHBF;
	pHBF = (headerBlockFiles*)buffer;
	char* cmdline = (char*)calloc(pHBF->commandLineLength + pHBF->commandLineULength + 3, 1);
	memcpy(cmdline, buffer + pHBF->commandLineStart, pHBF->commandLineLength);
	memcpy(cmdline + pHBF->commandLineLength + 1, buffer + pHBF->commandLineUStart, pHBF->commandLineULength);
	int fcount = pHBF->fileCount;
	//we don't really care for header size.
	void * vpc = (void*)((int)buffer + sizeof(headerBlockFiles));

	for(int file = 0 ; file < fcount ; ++file)
	{
		headerBlockFileData* pHBFD = (headerBlockFileData*)vpc;
		char* fname = (char*)calloc(pHBFD->filenameLen + 1, 1);
		memcpy(fname, buffer + pHBFD->filenameStart, pHBFD->filenameLen);
		FILE* fp;
		fp = fopen(fname, "wb");
		printf("Extracting Item %s [%d bytes]...[00%]", fname, pHBFD->len);
		if(fp)
		{
			int fileOffset, //start location of next write
				fileLeftLength, //number of bytes left to write
				fileWriteLength,  //number of bytes next write will do (Min(fileLeftLength, 1Meg))
				lastDataWritten; //how many bytes were written with the last fwrite

			fileOffset = pHBFD->start;
			fileLeftLength = pHBFD->len;

			while(fileLeftLength)
			{
				fileWriteLength = min(fileLeftLength, 1048576); //1MByte sections max
				lastDataWritten = fwrite(buffer + fileOffset, 1, fileWriteLength, fp);
				fileLeftLength -= lastDataWritten;
				fileOffset += lastDataWritten;

				int percent = (int)(100 * ((float)(fileOffset - pHBFD->start) / (float)pHBFD->len));
				printf("\b\b\b\b\b[%02ld%%]", percent);
			}
			fclose(fp);

		}
		printf("\n");
		free(fname);
		vpc = (void*)((int)vpc + sizeof(headerBlockFileData));
	}

	return cmdline;
}

bool removePacked(unsigned char* buffer)
{
	headerBlockFiles* pHBF = (headerBlockFiles*)buffer;
	int fcount = pHBF->fileCount;
	void* vpc = (void*)((int)buffer + sizeof(headerBlockFiles));
	bool removedAll = true;
	for(int i = 0 ; i < fcount ; ++i)
	{
		headerBlockFileData* pHBFD = (headerBlockFileData*)vpc;
		char* fname = (char*)calloc(pHBFD->filenameLen + 1, 1);
		memcpy(fname, buffer + pHBFD->filenameStart, pHBFD->filenameLen);
		if(remove(fname))
			removedAll = false;
		free(fname);
		vpc = (void*)((int)vpc + sizeof(headerBlockFileData));
	}

	return removedAll;
}

char* unpackMSI(char* path, unsigned char*buffer, headerBlockMSI headerData, bool x64)
{
	long start = x64 ? headerData.x64Start : headerData.x86Start;
	long length = x64 ? headerData.x64Len : headerData.x86Len;
	switch(headerData.fileType)
	{
	case EMBEDDED_FILE_MSI:
		return unpackItem(path, "temp.msi", buffer, start, length);
	case EMBEDDED_FILE_EXE:	
		return unpackItem(path, "temp.exe", buffer, start, length);
	case EMBEDDED_FILE_PACK:
		return unpackItemPack(path, &buffer[start], start, length);
	case EMBEDDED_FILE_BOOT:
		return unpackItem(path, "temp.exe", buffer, start, length);
	default:
		return nullptr;

	}
}

char* unpackAutoConfig(char* path, unsigned char* buffer, headerBlockMSI headerData)
{
	if(headerData.autoConfigLen == 0)
		return NULL;
	return unpackItem(path, "config.xml", buffer, headerData.autoConfigStart, headerData.autoConfigLen);
}

void MultiFileInstall(char* path, unsigned char* buffer, headerBlock* pHeaderData, bool x64, bool install)
{

}

void RunProcessPackedFile(char* commandLine, unsigned char* buffer)
{
	char* cCommand = commandLine;
	char* tempCmd;
	while(*cCommand)
	{
		//search for any instances of '\\n' in cCommand
		int index = 0;

		//cCommand[index + 1] to check if we are next to a null, otherwise check we aren't at a "\n"
		while((cCommand[index + 1])  && (cCommand[index] != '\\' || cCommand[index + 1] != 'n'))
			index++;

		//move forward one when the next character is a \0
		if(cCommand[index+1] == 0)
			index++;

		//this works for both \0 and \\n strings
		tempCmd = (char*)calloc(index + 1, 1);
		memcpy(tempCmd, cCommand, index);
		cCommand = &(cCommand[index]);
		//if we are not on a \0 string, we must be on a \\n string, so move forward another one
		if(*cCommand)
			cCommand += 2;

		STARTUPINFO startInfo;
		ZeroMemory(&startInfo, sizeof(STARTUPINFO));
		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.dwFlags = STARTF_FORCEONFEEDBACK;
		PROCESS_INFORMATION procInfo;
		printf("Running command : %s\n", tempCmd);
		if(CreateProcess(NULL, tempCmd, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo) == 0)
		{
			printf("Create Process Failed %d\n", GetLastError());
		}
		else
		{
			printf("Waiting until the process is finished\n");
			WaitForSingleObject(procInfo.hProcess, INFINITE);
			DWORD exitCode;
			if(GetExitCodeProcess(procInfo.hProcess, &exitCode))
				printf("Process terminated with exit code %d\n", exitCode);
		}

		free(tempCmd);
	}

	printf("Installation completed\nRemoving Temp Files... ");
	if(removePacked(buffer))
		printf("SUCCESS\nTemp Files Delete\n");
	else
		printf("FAILED\nOne or more Temp files could not be deleted\n");

	return;
}


/*
if(pHeaderData->fileType == EMBEDDED_FILE_PACK)
			{
				if(removePacked(buffer))
					printf("Temp Files Deleted\n");
				else
					printf("One or more Temp Files were not deleted\n");
			}
			else
			{
			*/
void DumpAndRunProcess(char * path, unsigned char* buffer, headerBlock* pHeaderData, bool x64, bool install)
{
	char *msiPath, *configPath;
	char * commandLine ;
//	if((pHeaderData->fileType == EMBEDDED_FILE_EXE) || (pHeaderData->fileType == EMBEDDED_FILE_MSI))
//	{
		headerBlockMSI headerData;
		memcpy(&headerData, pHeaderData, sizeof(headerBlockMSI));
		msiPath = unpackMSI(path, buffer, headerData, x64);
		if(msiPath == NULL)
		{
			printf("Failed to extract MSI from installer\n");
			return;
		}
		//configPath = unpackAutoConfig(path, buffer, headerData);
		configPath = headerData.autoConfigLen == 0 ? NULL : "";
		//char * commandLine ;
		if(install)
			commandLine = createCommandLine(msiPath, configPath, headerData.fileType);
		else
			commandLine = createRollbackCommandLine(msiPath, headerData.fileType);

		if(pHeaderData->fileType == EMBEDDED_FILE_PACK)
		{
			return RunProcessPackedFile(commandLine, buffer);
		}
		
	#if debugging
		printf("Commandline : %s\n", commandLine);
	#endif
		STARTUPINFO startInfo;
		ZeroMemory(&startInfo, sizeof(STARTUPINFO));
		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.dwFlags = STARTF_FORCEONFEEDBACK;
		PROCESS_INFORMATION procInfo;
		printf("Starting installer (%s)\n", commandLine);
		if(CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo) == 0)
		{
			printf("Create Process Failed %d\n", GetLastError());
		}
		else
		{
			printf("Waiting until the installation process is finished\n");
			WaitForSingleObject(procInfo.hProcess, INFINITE);
			DWORD exitCode;
			if(GetExitCodeProcess(procInfo.hProcess, &exitCode))
				printf("Process terminated with exit code %d\n", exitCode);

			bool rollback = false;
			
			if((install) && (headerData.autoConfigLen != 0))
			{
				printf("Running Auto-Config Process\n");
				const char* errorMessage = NULL;
				if(!WriteBypassSettings(&(buffer[headerData.autoConfigStart]), headerData.autoConfigLen, &errorMessage, &rollback))
				{
					printf("Auto-Config Settings Failed\n\tError : %s\n", errorMessage == NULL ? "(NULL)" : errorMessage);
				}
				else
				{
					printf("Auto-Config settings successfully added to the regsitry\n");
				}
			}

			if(rollback)
			{
				//ok time to rollback everything... YAY :D
				//fortunently it is as simple as telling the MSI to uninstall
				char* rollbackCmdLine = createRollbackCommandLine(msiPath, pHeaderData->fileType);
				ZeroMemory(&startInfo, sizeof(STARTUPINFO));
				startInfo.cb = sizeof(STARTUPINFO);
				startInfo.dwFlags = STARTF_FORCEONFEEDBACK;
				PROCESS_INFORMATION procInfo;
				printf("Starting uninstaller\n");
				if(CreateProcess(NULL, rollbackCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo) == 0)
				{
					printf("Create Process Failed %d\n", GetLastError());
				}
				else
				{
					printf("Waiting until the uninstallation process is finished\n");
					WaitForSingleObject(procInfo.hProcess, INFINITE);
					DWORD exitCode;
					if(GetExitCodeProcess(procInfo.hProcess, &exitCode))
						printf("Process terminated with exit code %d\n", exitCode);

				}
			}
			printf("Process terminated... Cleaning up\n");
		
			int ret;
			long delay = 100L;
			for (int i = 1; i <= 5; i++, delay *= 2)
			{
				if (ret = remove(msiPath) == 0)
					break;
				Sleep(delay);
			}

			if(ret == 0)
				printf("Temp file deleted\n");
			else
				printf("Temp file was not deleted,  error: %d", errno);
		}
//	}	

	/*
	else
	{
		MultiFileInstall(path, buffer, pHeaderData, x64, install);
	}
	*/
}

bool installMSI(bool install)
{
	wow64 = IsWow64();
	if(wow64)
		printf("64 Bit system\n");
	else
		printf("32 Bit System\n");

	char * exepath = getExePath();
#if debugging
	printf("Exepath : %s\n", exepath);
#endif

	FILE* fp;
	fp = fopen(exepath, _T("rb"));
	if(fp == NULL)
	{
		printf("failed to open file\n");
	}
	else
	{
		fseek(fp, 0L, SEEK_END);
		long sz = ftell(fp);
#if debugging
		printf("file is %ld bytes long\n", sz);
#endif
		fseek(fp, 0L, SEEK_SET);
		unsigned char* data = (unsigned char*)calloc(sz, sizeof(unsigned char));
		fread(data, sizeof(unsigned char), sz, fp);
#if debugging	
		printf("read data from file\n");
#endif

		long msiStart = findMSIStart(data, sz);
#if debugging
		printf("msi data found at location %ld\n", msiStart);
#endif

		if(msiStart != -1)
		{
			headerBlock* header;
			header = (headerBlock*)&data[msiStart];
			//memcpy(&header, &(data[msiStart]), sizeof(headerBlock));

			DumpAndRunProcess(exepath, &(data[msiStart + header->headerSize]), header, wow64, install);
		}
		fclose(fp);
	}
	return true;
}