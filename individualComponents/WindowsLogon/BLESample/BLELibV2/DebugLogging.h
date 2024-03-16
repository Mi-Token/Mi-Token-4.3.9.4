//Enables the logging functions - note they are also controlled by a registry key
#define ENABLE_LOGGING

//Enables the VERBOSE Logging Functions - these are also controlled by a registry key
//Note VERBOSE logging does a load of logging
#define ENABLE_VERBOSE_LOGGING

#include <stdarg.h>

extern char* filepath;

#define LOG_INFO 1
#define LOG_VERBOSE 2
#define LOG_SENSITIVE 4

extern int debugLog;
#include <Windows.h>

extern HANDLE debugMutex;

#define MASS_DEBUG

#ifdef MASS_DEBUG
#define LockDebugMutex (WaitForSingleObject((debugMutex == INVALID_HANDLE_VALUE ? (debugMutex = CreateMutex(NULL, FALSE, NULL), debugMutex) : debugMutex), INFINITE))
#define UnlockDebugMutex (ReleaseMutex(debugMutex))

#define DEBUG_THREADID LockDebugMutex, DEBUG_VERBOSE("[TID] : [%d]", GetCurrentThreadId()), UnlockDebugMutex
#define DEBUG_ENTER_FUNCTION LockDebugMutex, DEBUG_THREADID, DEBUG_VERBOSE("+" __FUNCTION__ "\r\n"), UnlockDebugMutex
#define DEBUG_FUNCTION_SECTION LockDebugMutex, DEBUG_THREADID, DEBUG_VERBOSE(">" __FUNCTION__ "   %d\r\n", __LINE__), UnlockDebugMutex
#define DEBUG_END_FUNCTION LockDebugMutex, DEBUG_THREADID, DEBUG_VERBOSE("-" __FUNCTION__ "\r\n"), UnlockDebugMutex
#define DEBUG_RETURN return DEBUG_END_FUNCTION , 
#else
#define LockDebugMutex 
#define UnlockDebugMutex 

#define DEBUG_THREADID 
#define DEBUG_ENTER_FUNCTION 
#define DEBUG_FUNCTION_SECTION 
#define DEBUG_END_FUNCTION 
#define DEBUG_RETURN return 

#endif
//Info Logs
void DEBUG_LOG(char* format, ...);
void DEBUG_LOGW(wchar_t* wformat, ...);
void DEBUG_LOGBYTES(unsigned char* bytes, int length);

//Verbose Logs
void DEBUG_VERBOSE(char* format, ...);
void DEBUG_VERBOSEW(wchar_t* wformat, ...);
void DEBUG_VERBOSEBYTES(unsigned char* bytes, int length);

//Sensitive Logs
void DEBUG_SENSITIVE(bool requireVerbose, char* format, ...);
void DEBUG_SENSITIVEW(bool requireVerbose, wchar_t* wformat, ...);
void DEBUG_SENSITIVEBYTES(bool requireVerbose, unsigned char* bytes, int length);

//To be removed
//void DEBUG_MESSAGE(char message);
//void DEBUG_WMESSAGE(wchar_t message);
#define DEBUG_MESSAGE(message) DEBUG_LOG("%s\n", message)
#define DEBUG_WMESSAGE(message) DEBUG_LOGW(L"%s\n", message)
#define DEBUG_MESSAGEd(number) DEBUG_LOG("%d\n", number)
#define DEBUG_BYTES(b,l) DEBUG_LOGBYTES(b,l)

#ifdef UNICODE
#define DEBUG_TMESSAGE(tmessage) DEBUG_WMESSAGE(tmessage)
#else
#define DEBUG_TMESSAGE(tmessage) DEBUG_MESSAGE(tmessage)
#endif


void DEBUG_LOGLIST(char* format, va_list argp);
/*
#ifdef ENABLE_LOGGING
#define DEBUG_WDEF(wformat, ...) \
	{ \
		if(debugLog) \
		{ \
			FILE* fpspam; \
			fopen_s(&fpspam, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\General.log", "a+"); \
			if(fpspam != NULL) \
			{ \
				fwprintf(fpspam, wformat, __VA_ARGS__); \
				fclose(fpspam); \
			} \
		} \
	} 

#define DEBUG_DEF(format, ...) \
	{ \
		if(debugLog) \
		{ \
			FILE* fpspam; \
			fopen_s(&fpspam, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\General.log", "a+"); \
			if(fpspam != NULL) \
			{ \
				fprintf(fpspam, format, __VA_ARGS__); \
				fclose(fpspam); \
			} \
		} \
	} 

#define DEBUG_MESSAGE(message) DEBUG_DEF("%s\n", message)
//	{ FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%s\n", message); fclose(fpspam); } }
#define DEBUG_MESSAGEd(message) DEBUG_DEF("%d\n", message)
// { FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%d\n", message); fclose(fpspam); } }
#define DEBUG_WMESSAGE(wmessage) DEBUG_DEF("%ls\n", wmessage)

#define DEBUG_BYTES(bytes, count) \
	{ \
		if(debugLog) \
		{ \
			FILE* fpspam; \
			fopen_s(&fpspam, "C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\General.log", "a+"); \
			if(fpspam != NULL) \
			{ \
				fwrite(bytes, sizeof(char), count, fpspam); \
				fclose(fpspam); \
			} \
		} \
	} 

#ifdef UNICODE
#define DEBUG_TMESSAGE(tmessage) DEBUG_DEF("%ls\n", tmessage)
#else
#define DEBUG_TMESSAGE(tmessage) DEBUG_DEF("%s\n", tmessage)
#endif
//{ FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%ls\n", wmessage); fclose(fpspam); } }
#else
#define DEBUG_MESSAGE(message)
#define DEBUG_WMESSAGE(wmessage)
#define DEBUG_MESSAGEd(message)
#define DEBUG_BYTES(bytes, count)
#endif

#ifdef ENABLE_VERBOSE_LOGGING
#define DEBUG_VERBOSE(format, ...) \ 
	{ \
	if(debugLog & LOG_VERBOSE) \
		{ \
			DEBUG_DEF(format, __VA_ARGS__ );
		} \
	} 

#define DEBUG_VERBOSEW(wformat, ...) \
	{ \
		if(debugLog & LOG_VERBOSE) \
		{ \
			DEBUG_WDEF(wformat, __VA_ARGS__ ); \
		} \
	} 

#else
#define DEBUG_VERBOSE(format, ...)
#define DEBUG_VERBOSEW(wformat, ...)
#endif
*/