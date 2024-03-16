
#include "..\ConsoleAP\DebugMessages.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int debugLog = 7; //just do verbose logging ATM

#define LOG_BUFFER_SIZE 4096
//#define DEBUG_DLOGGER
FILE* fp = NULL;
FILE* fpDebug = NULL;
char logBuffer[LOG_BUFFER_SIZE];
int logBufferPos = 0;
char logBuffer2[LOG_BUFFER_SIZE];
char logBufferWorkingID = 0;

//Generic File Writing function - These functions presume the debugging is allowed
#define DEBUG_PATH "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Logon\\CAPDebug.log"
				 //"C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log"

//fp = fopen(			  "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerialized.log", "a+");


void DEBUG_INIT()
{
	if(fp == NULL)
		fopen_s(&fp, "C:\\Debug\\CAPDebugV2.log", "a+");
	
}

void DEBUG_FORCE_FLUSH()
{
	if(fp)
	{
		void* logFrom = (logBufferWorkingID ? &logBuffer[0] : &logBuffer2[0]);
		int len = logBufferPos;
		fwrite(logFrom, 1, len, fp);
		fflush(fp);
		logBufferPos = 0;
	}
}
void writeToLogBuffer(char* data, int length)
{

#ifdef DEBUG_DLOGGER
	if(fpDebug == NULL)
	{
		fpDebug = fopen("C:\\Debug\\CAPDebugD.log", "a+");
	}
	if(fpDebug)
	{
		fprintf(fpDebug, "Debug is trying to log %d bytes\n", length);
		fflush(fpDebug);
	}
#endif

	if(LOG_BUFFER_SIZE - logBufferPos < length)
	{
		int wl, rl; //writeLength / rewriteLength
		wl = LOG_BUFFER_SIZE - logBufferPos; //we want to write the exact number of bytes to fill up this buffer
		rl = length - wl; //we need to write this many bytes to the next log
#ifdef DEBUG_DLOGGER
		if(fpDebug)
		{
			fprintf(fpDebug, "Debug couldn't fit (%d vs %d). Splitting to %d and %d\n", length, LOG_BUFFER_SIZE - logBufferPos, wl, rl);
			fflush(fpDebug);
		}
#endif
		writeToLogBuffer(data, wl); //write to fill the current log buffer
		writeToLogBuffer(data + wl, rl); //write into the next log buffer (can be >1024 because the length is checked again)
	}
	else
	{
		//we can write all these bytes to the log
		char* logBufferW = (logBufferWorkingID ? &logBuffer[0] : &logBuffer2[0]);
		memcpy(logBufferW + logBufferPos, data, length);
		logBufferPos += length;
#ifdef DEBUG_DLOGGER
		if(fpDebug)
		{
			fprintf(fpDebug, "Debug Write %d bytes to logger %d. New pos %d\n", length, logBufferWorkingID, logBufferPos);
			fflush(fp);
		}
#endif
		if(logBufferPos >= LOG_BUFFER_SIZE)
		{
#ifdef DEBUG_DLOGGER
			if(fpDebug)
			{
				fprintf(fpDebug, "Debug %d >= %d, wrote to file.\n", logBufferPos, LOG_BUFFER_SIZE);
				fflush(fp);
			}
#endif
			//time to print down
			//first flip the bufferID
			logBufferWorkingID = 1 - logBufferWorkingID;
			//set logBufferPos to 0
			logBufferPos = 0;
			//now write the log out
			fwrite(logBufferW, 1, LOG_BUFFER_SIZE, fp);
			//flush the file
			fflush(fp);
			//and done :D
#ifdef DEBUG_DLOGGER
			if(fpDebug)
			{
				fprintf(fpDebug, "New BufferID %d, Pos %d\n", logBufferWorkingID, logBufferPos);
				fflush(fp);
			}
#endif

		}
		else if(logBufferPos > LOG_BUFFER_SIZE)
		{
			//this is very bad.
			throw "LOGBUFFERPOS > LOG_BUFFER_SIZE";
		}
	}
	DEBUG_FORCE_FLUSH();
}

void debug_char(char* format, va_list argp)
{
	if(fp == NULL)
		fopen_s(&fp, "C:\\Debug\\CAPDebugV2.log", "a+");
	if(fp)
	{
		char buffer[1024]; //we will allow a 1024 char buffer
		int bytesWritten = vsprintf_s(buffer, 1024, format, argp);
		writeToLogBuffer(buffer, bytesWritten);
//		vfprintf(fp, format, argp);
//		fflush(fp);
		DEBUG_FORCE_FLUSH();
		fclose(fp);
		fp = NULL;
	}

}

void debug_fchar(char* format, ...)
{
	va_list argp;
	va_start(argp, format);
	debug_char(format, argp);
	va_end(argp);
}
void debug_wchar(wchar_t* wformat, va_list argp)
{
	if(fp == NULL)
		fopen_s(&fp, "C:\\Debug\\CAPDebugV2.log", "a+");

	if(fp)
	{
		wchar_t wbuffer[512];
		int bytesWritten = vswprintf_s(wbuffer, 512, wformat, argp) * 2; //*2 as wchar_t are 2 bytes
		writeToLogBuffer((char*)wbuffer, bytesWritten);
		DEBUG_FORCE_FLUSH();
		fclose(fp);
		fp = NULL;
		//vfwprintf(fp, wformat, argp);
		//fflush(fp); //we wont close FP anymore - we will just flush it.
	}
}

void debug_bytes(unsigned char* bytes, int length)
{
	if(fp == NULL)
		fopen_s(&fp, "C:\\Debug\\CAPDebugV2.log", "ab+");
	if(fp)
	{
		writeToLogBuffer((char*)bytes, length);
		DEBUG_FORCE_FLUSH();
		fclose(fp);
		fp = NULL;
		//fwrite(bytes, sizeof(char), length, fp);
		//fflush(fp);
	}
}

void debug_hexbytes(void* mem, int length)
{
	debug_fchar("Error : DEBUG HexBytes currently not supported\n");
	/*
	if(fp == NULL)
		fopen_s(&fp, "C:\\Debug\\CAPDebugV2.log", "ab+");
	if(fp)
	{
		for(int i = 0 ; i < length ; ++i)
		{
			unsigned int c = (unsigned int)*(unsigned char*)mem;
			fprintf(fp, "%02lX-", c);
			mem = (void*)((char*)mem + 1);
		}
		fflush(fp);
	}
	*/
}

#ifndef ENABLE_LOGGING
void DEBUG_LOG(char* format, ...) {}
void DEBUG_LOGW(wchar_t* format, ...) {}
void DEBUG_LOGBYTES(unsigned char* bytes, int length) {}
#else
void DEBUG_LOG(char* format, ...)
{
	if((debugLog & LOG_INFO) != 0)
	{
		va_list argp;
		va_start(argp, format);
		debug_char(format, argp);
		va_end(argp);
	}
}

void DEBUG_LOGW(wchar_t* wformat, ...)
{
	if(debugLog & LOG_INFO)
	{
		va_list argp;
		va_start(argp, wformat);
		debug_wchar(wformat, argp);
		va_end(argp);
	}
}

void DEBUG_LOGLIST(char* format, va_list argp)
{
	if((debugLog & LOG_INFO) != 0)
	{
		debug_char(format, argp);
	}
}

void DEBUG_LOGBYTES(unsigned char* bytes, int length)
{
	if(debugLog & LOG_INFO)
		debug_bytes(bytes, length);
}

void DEBUG_LOGHEXBYTES(void* mem, int length)
{
	if(debugLog & LOG_INFO)
		debug_hexbytes(mem, length);
}
#endif

#ifndef ENABLE_VERBOSE_LOGGING
void DEBUG_VERBOSE(char* format, ...){}
void DEBUG_VERBOSEW(wchar_t* format, ...) {}
void DEBUG_VERBOSEBYTES(unsigned char* bytes, int length) {}
#else
void DEBUG_VERBOSE(char* format, ...)
{
	if(debugLog & LOG_VERBOSE)
	{
		va_list argp;
		va_start(argp, format);
		debug_char(format, argp);
		va_end(argp);
	}
}
void DEBUG_VERBOSEW(wchar_t* wformat, ...) 
{
	if(debugLog & LOG_VERBOSE)
	{
		va_list argp;
		va_start(argp, wformat);
		debug_wchar(wformat, argp);
		va_end(argp);
	}
}
void DEBUG_VERBOSEBYTES(unsigned char* bytes, int length) 
{
	if(debugLog & LOG_VERBOSE)
		debug_bytes(bytes, length);
}
#endif

#ifndef ENABLE_LOGGING
void DEBUG_SENSITIVE(bool requireVerbose, char* format, ...) {}
void DEBUG_SENSITIVEW(bool requireVerbose, wchar_t* wformat, ...) {}
void DEBUG_SENSITIVEBYTES(bool requireVerbose, unsigned char* bytes, int length) {}
#else
void DEBUG_SENSITIVE(bool requireVerbose, char* format, ...) 
{
	if(debugLog & LOG_SENSITIVE)
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			if(debugLog & LOG_VERBOSE)
			{
				va_list argp;
				va_start(argp, format);
				debug_char(format, argp);
				va_end(argp);
			}
#endif
		}
		else
		{
			va_list argp;
			va_start(argp, format);
			debug_char(format, argp);
			va_end(argp);
		}
	}
	else
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]");
#endif
		}
		else
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]");
	}
}
void DEBUG_SENSITIVEW(bool requireVerbose, wchar_t* wformat, ...) 
{
	if(debugLog & LOG_SENSITIVE)
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			if(debugLog & LOG_VERBOSE)
			{
				va_list argp;
				va_start(argp, wformat);
				debug_wchar(wformat, argp);
				va_end(argp);
			}
#endif
		}
		else
		{
			va_list argp;
			va_start(argp, wformat);
			debug_wchar(wformat, argp);
			va_end(argp);
		}
	}
	else
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]\n");
#endif
		}
		else
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]\n");
	}
}
void DEBUG_SENSITIVEBYTES(bool requireVerbose, unsigned char* bytes, int length) 
{
	if(debugLog & LOG_SENSITIVE)
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			if(debugLog & LOG_VERBOSE)
			{
				debug_bytes(bytes, length);
			}
#endif
		}
		else
		{
			debug_bytes(bytes, length);
		}
	}
	else
	{
		if(requireVerbose)
		{
#ifdef ENABLE_VERBOSE_LOGGING
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]");
#endif
		}
		else
			debug_fchar("%s", "[FILTERED OUT DUE TO FILTER SETTINGS]");
	}
}

#endif