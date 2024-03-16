/*
 * $Id: log.c,v 1.5 2007/06/21 18:07:23 cparker Exp $
 *
 * Copyright (C) 1995,1996,1997 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>
#include "..\event.provider\EventProvider.h"

#define PROVIDER_NAME L"MiToken Gina"

/*
 * Function: rc_openlog
 *
 * Purpose: open log
 *
 * Arguments: identification string
 *
 * Returns: nothing
 *
 */

extern "C" 
{
   void rc_log(int prio, const char *format, ...);
   void rc_openlog(char *ident);
   void rc_event_log(WORD errorId, WORD categoryId, DWORD messageId, LPWSTR message,...);
 }

void rc_openlog(char *ident)
{
	#ifndef _MSC_VER /* TODO: Fix me */
		openlog(ident, LOG_PID, RC_LOG_FACILITY);
	#else
		


	#endif
}

/*
 * Function: rc_log
 *
 * Purpose: log information
 *
 * Arguments: priority (just like syslog), rest like printf
 *
 * Returns: nothing
 *
 */

void rc_log(int prio, const char *format, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap,format);
    vsnprintf_s(buff,  sizeof(buff), sizeof(buff), format, ap);
    va_end(ap);

	#ifndef _MSC_VER /* TODO: Fix me */
		syslog(prio, "%s", buff);
	#endif
}

void rc_event_log(WORD errorId, WORD categoryId, DWORD messageId, LPWSTR message, ...)
{
	HANDLE hEventLog = NULL;
    LPWSTR pInsertStrings[1] = {NULL};
    WORD dwEventDataSize = 0;
	WORD dwCount;

	
	TCHAR buff[4096];
	memset(buff, 0, sizeof(buff));

	va_list args;
	va_start(args, message);
	_vsnwprintf(buff, sizeof(buff) - 1, message, args);
    va_end(args);

    // The source name (provider) must exist as a subkey of Application.
    hEventLog = RegisterEventSource(NULL, PROVIDER_NAME);
    if (NULL == hEventLog)
    {
        OutputDebugString(L"RegisterEventSource failed with" + GetLastError() );
        goto cleanup;
    }

    // This event uses insert strings.
    pInsertStrings[0] = buff;

	if(NULL == message)
	{
		dwCount = 0;
	}
	else
	{
		dwCount = 1;
	}

    if (!ReportEvent(hEventLog, errorId, categoryId, messageId, NULL, dwCount, 0, (LPCWSTR*)pInsertStrings, NULL))
    {
        OutputDebugString(L"ReportEvent failed for " +  errorId);
        goto cleanup;
    }

	cleanup:
    if (hEventLog)
        DeregisterEventSource(hEventLog);
}


