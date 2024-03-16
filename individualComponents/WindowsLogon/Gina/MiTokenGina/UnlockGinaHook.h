/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 BASED ON MICROSOFT Ginahook SDK Sample
 Module:   Ginahook.c
 Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.

------------------------------------------------------------------------------*/

#pragma once

static LPWSTR cachedUserName = NULL;


//
// Function prototypes for the GINA interface.
//

typedef BOOL (WINAPI * PFWLXNEGOTIATE)  (DWORD, DWORD *);
typedef BOOL (WINAPI * PFWLXINITIALIZE) (LPWSTR, HANDLE, PVOID, PVOID, PVOID *);
typedef VOID (WINAPI * PFWLXDISPLAYSASNOTICE) (PVOID);
typedef int  (WINAPI * PFWLXLOGGEDOUTSAS) (PVOID, DWORD, PLUID, PSID, PDWORD,
                                           PHANDLE, PWLX_MPR_NOTIFY_INFO, 
                                           PVOID *);
typedef BOOL (WINAPI * PFWLXACTIVATEUSERSHELL) (PVOID, PWSTR, PWSTR, PVOID);
typedef int  (WINAPI * PFWLXLOGGEDONSAS) (PVOID, DWORD, PVOID);
typedef VOID (WINAPI * PFWLXDISPLAYLOCKEDNOTICE) (PVOID);
typedef int  (WINAPI * PFWLXWKSTALOCKEDSAS) (PVOID, DWORD);
typedef BOOL (WINAPI * PFWLXISLOCKOK) (PVOID);
typedef BOOL (WINAPI * PFWLXISLOGOFFOK) (PVOID);
typedef VOID (WINAPI * PFWLXLOGOFF) (PVOID);
typedef VOID (WINAPI * PFWLXSHUTDOWN) (PVOID, DWORD);

//
// New for version 1.1
//

typedef BOOL (WINAPI * PFWLXSCREENSAVERNOTIFY) (PVOID, BOOL *);
typedef BOOL (WINAPI * PFWLXSTARTAPPLICATION) (PVOID, PWSTR, PVOID, PWSTR);

//
// New for version 1.3
//

typedef BOOL (WINAPI * PFWLXNETWORKPROVIDERLOAD) (PVOID, PWLX_MPR_NOTIFY_INFO);
typedef BOOL (WINAPI * PFWLXDISPLAYSTATUSMESSAGE) (PVOID, HDESK, DWORD, PWSTR, PWSTR);
typedef BOOL (WINAPI * PFWLXGETSTATUSMESSAGE) (PVOID, DWORD *, PWSTR, DWORD);
typedef BOOL (WINAPI * PFWLXREMOVESTATUSMESSAGE) (PVOID);

//
// New for version 1.4
//

typedef BOOL (WINAPI * PFWLXGETCONSOLESWITCHCREDENTIALS) (PVOID, PVOID);
typedef VOID (WINAPI * PFWLXRECONNECTNOTIFY) (PVOID);
typedef VOID (WINAPI * PFWLXDISCONNECTNOTIFY) (PVOID);
