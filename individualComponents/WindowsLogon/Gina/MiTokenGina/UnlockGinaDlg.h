/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 BASED ON MICROSOFT Ginahook SDK Sample
 Module:   Ginahook.c
 Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.


------------------------------------------------------------------------------*/

#pragma once

//
// Function prototypes.
//
void HookWlxDialogBoxParam (PVOID pWinlogonFunctions, DWORD dwWlxVersion);
BOOL VerifyOTP (HANDLE hModule);
INT_PTR CALLBACK LockedNoticeHookDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HWND gHwndOTP;