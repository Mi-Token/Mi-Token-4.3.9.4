#include "DebugLogging.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>


int debugLog = 0;


//Generic File Writing function - These functions presume the debugging is allowed
#define DEBUG_PATH "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Logon\\Debug.log"
				 //"C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log"

#define INCLUDE_DEBUG_TIME
//fp = fopen(			  "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerialized.log", "a+");

void debug_time()
{
#ifdef INCLUDE_DEBUG_TIME
	FILE* fp;
	fopen_s(&fp, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log", "a+");
	if(fp)
	{
		char buf[80];
		time_t now = time(0);
		struct tm tstruct;
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		fprintf(fp, "[%s]", buf);
		fclose(fp);
	}
#endif
}

void debug_char(char* format, va_list argp)
{
	debug_time();
	FILE* fp;
	fopen_s(&fp, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log", "a+");
	if(fp)
	{
		vfprintf(fp, format, argp);
		fclose(fp);
	}

}

void debug_fchar(char* format, ...)
{
	debug_time();
	va_list argp;
	va_start(argp, format);
	debug_char(format, argp);
	va_end(argp);
}
void debug_wchar(wchar_t* wformat, va_list argp)
{
	debug_time();
	FILE* fp;
	fopen_s(&fp, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log", "a+");
	if(fp)
	{
		vfwprintf(fp, wformat, argp);
		fclose(fp);
	}
}

void debug_bytes(unsigned char* bytes, int length)
{
	debug_time();
	FILE* fp;
	fopen_s(&fp, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\Debug.log", "ab+");
	if(fp)
	{
		for(int i = 0 ; i < length ; ++i)
		{
			fprintf(fp, "%02lX ", (unsigned int)bytes[i]);
			fflush(fp);
		}
		fclose(fp);
	}
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
}

#endif