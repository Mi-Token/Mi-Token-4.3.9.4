/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 BASED ON MICROSOFT Ginahook SDK Sample
 Module:   Ginahook.c
 Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.


-------------------------------------------------------------------------------*/

#define _WIN32_WINNT 0x0501

#include <windows.h>

#include <winwlx.h>
#include <assert.h>
#include <lm.h>

#include "UnlockGinaDlg.h"
#include "UnlockGinaHook.h"
#include "MiTokenAuthenticator.h"
#include "MiTokenWindowsLoginSettings.h"
#include "Debug.h"
#include "resource.h"
#include "OTPCheck.h"

HWND gHwndOTP = NULL;



//
// MSGINA dialog box IDs.
//  NOTE Windows 2000/XP seem to have different IDs than from before!
//

/*
#define IDD_WLXDIAPLAYSASNOTICE_DIALOG    1400
#define IDD_WLXLOGGEDOUTSAS_DIALOG        1450
#define IDD_CHANGE_PASSWORD_DIALOG        1550
#define IDD_WLXLOGGEDONSAS_DIALOG         1650
*/

#define IDD_WLXLOGGEDOUTSAS_DIALOG        1500
#define IDD_LOCKED_NOTICE_DIALOG	      1900
#define IDD_WLXWKSTALOCKEDSAS_DIALOG      1950

/*
#define IDC_WLXLOGGEDOUTSAS_USERNAME      1453
#define IDC_WLXLOGGEDOUTSAS_PASSWORD      1454
#define IDC_WLXLOGGEDOUTSAS_DOMAIN        1455
#define IDC_WLXWKSTALOCKEDSAS_DOMAIN      1856
*/

// MSGINA control IDs
#define IDC_WLXLOGGEDOUTSAS_USERNAME      1502
#define IDC_WLXLOGGEDOUTSAS_PASSWORD      1503
#define IDC_WLXLOGGEDOUTSAS_DOMAIN        1504
#define IDC_WLXLOGGEDOUTSAS_OK            1
//
// MSGINA control IDs
//
/*
#define IDC_WLXLOGGEDOUTSAS_USERNAME      1453
#define IDC_WLXLOGGEDOUTSAS_PASSWORD      1454
#define IDC_WLXLOGGEDOUTSAS_DOMAIN        1455
#define IDC_WLXWKSTALOCKEDSAS_DOMAIN      1856
*/

//
// Pointers to redirected functions.
//

static PWLX_DIALOG_BOX_PARAM pfOriginalWlxDialogBoxParam = NULL;

//
// Pointers to redirected dialog box.
//

static DLGPROC pfLockedNoticeDlgProc = NULL;
static DLGPROC pfWlxWkstaLockedSASDlgProc = NULL;
static DLGPROC pfLoggedOutDlgProc = NULL;


//
// Local functions.
//

int WINAPI MyWlxDialogBoxParam (HANDLE, HANDLE, LPWSTR, HWND, DLGPROC, LPARAM);

//
// Local variables.
//

static char g_szLocalMachineName[256] = "";

//LPWSTR username = NULL;

void HookWlxDialogBoxParam (PVOID pWinlogonFunctions, DWORD dwWlxVersion)
{
   //
   // Hook WlxDialogBoxParam(). Note that we chould cheat here by always
   // casting to (PWLX_DISPATCH_VERSION_1_0) since WlxDialogBoxParam()
   // exists in all versions and is always in the same location of the
   // dispatch table. But, we will do it the hard way!
   //
   switch (dwWlxVersion)
   {
      case WLX_VERSION_1_0: 
      {
         pfOriginalWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_0) pWinlogonFunctions)->WlxDialogBoxParam;
         ((PWLX_DISPATCH_VERSION_1_0) pWinlogonFunctions)->WlxDialogBoxParam = MyWlxDialogBoxParam;
         break;
      }
  
      case WLX_VERSION_1_1:
      {
         pfOriginalWlxDialogBoxParam =  ((PWLX_DISPATCH_VERSION_1_1) pWinlogonFunctions)->WlxDialogBoxParam;
         ((PWLX_DISPATCH_VERSION_1_1) pWinlogonFunctions)->WlxDialogBoxParam = MyWlxDialogBoxParam;
         break;
      }
   
      case WLX_VERSION_1_2:
      {
         pfOriginalWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_2) pWinlogonFunctions)->WlxDialogBoxParam;
         ((PWLX_DISPATCH_VERSION_1_2) pWinlogonFunctions)->WlxDialogBoxParam = MyWlxDialogBoxParam;
         break;
      }
   
      case WLX_VERSION_1_3:
      {
         pfOriginalWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_3) pWinlogonFunctions)->WlxDialogBoxParam;
         ((PWLX_DISPATCH_VERSION_1_3) pWinlogonFunctions)->WlxDialogBoxParam = MyWlxDialogBoxParam;
         break;
      }

	  case WLX_VERSION_1_4:
      {
         pfOriginalWlxDialogBoxParam = ((PWLX_DISPATCH_VERSION_1_4) pWinlogonFunctions)->WlxDialogBoxParam;
         ((PWLX_DISPATCH_VERSION_1_4) pWinlogonFunctions)->WlxDialogBoxParam = MyWlxDialogBoxParam;
         break;
      }
   }
}


INT_PTR CALLBACK OneTimePasswordDlgProc 
								(
									HWND   hwndDlg,  // handle to dialog box
									UINT   uMsg,     // message  
									WPARAM wParam,   // first message parameter
									LPARAM lParam
								)   // second message parameter
{
	debugLog = 7;

	HWND hwndOTP = GetDlgItem(hwndDlg,ID_OTP);
	HWND hwndInvalidOTP = GetDlgItem(hwndDlg,ID_INVALID_OTP);
	INT otpLen;
	BOOL OTPValid;
	BOOL retVal = FALSE;
	LPWSTR otp;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetWindowText(hwndDlg, L"Enter the one time password");
			SetFocus(GetDlgItem(hwndDlg, ID_OTP)); 
			SetTimer(hwndDlg, 455 /* timer id */, 60000 /* 60sec interval */, NULL);
			{
				wchar_t dbuffer[80];
				swprintf_s(dbuffer, 80, L"Setting hwndOTP : %d", hwndDlg);
				PrintDebug(dbuffer);
			}
			gHwndOTP = hwndDlg;
			retVal = TRUE; /* set focus */
			break;

		case WM_COMMAND: 
			OTPValid = FALSE;
			SetWindowText(hwndInvalidOTP, L"");
			switch (LOWORD(wParam)) 
			{ 
				case ID_OK:
					EnableWindow(GetDlgItem(hwndDlg, ID_OK), FALSE);
					otpLen = GetWindowTextLength(hwndOTP); 
					otp = (LPWSTR)GlobalAlloc(GPTR, (otpLen + 1)* sizeof(TCHAR));
					
					GetWindowText(hwndOTP,otp,otpLen + 1);

					assert(cachedUserName != NULL);

					/*Stores the message.*/
					WCHAR message[4096];
					memset(message,'\0', 4096);
					//OTPValid = MiTokenAuthenticator::VerifyOTP(cachedUserName, otp, message, 4096);
					OTPValid = checkOTP(cachedUserName, otp, otp, message, 4096, cachedUserName);
					GlobalFree(otp);
					
					if(OTPValid)
					{

						EndDialog(hwndDlg, IDOK); 
					}
					else
					{
						/*The OTP is invalid. Warn the user.*/
						MessageBox(hwndDlg, message, (LPCWSTR)L"Mi-Token Gina", MB_ICONERROR | MB_OK);
						EnableWindow(GetDlgItem(hwndDlg, ID_OK), TRUE);
					}

					retVal = OTPValid; 
					break;

				default:
					break;
			} 
			break;

		case WM_TIMER:
		case WM_CLOSE:
			EndDialog(hwndDlg, IDCANCEL);
			break;

		default:
			break;
	}
	return retVal;
}


BOOL VerifyOTP(HANDLE hModule)
{
	 return DialogBoxParam((HINSTANCE) hModule, MAKEINTRESOURCE(IDD_INIT_CARD), 0, OneTimePasswordDlgProc, NULL);
}

INT_PTR CALLBACK LockedNoticeHookDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   BOOL bResult;
   assert(pfLockedNoticeDlgProc != NULL);
   
   /* Pass on to MSGINA first */
    bResult = pfLockedNoticeDlgProc(hwndDlg, uMsg, wParam, lParam);

	switch (uMsg)
	{
		case WM_INITDIALOG:
			/* change the caption.. */
			SetWindowText(hwndDlg, L"Welcome to Windows (with one time password support)");
			break;

		default:
			break;		
    }
    return bResult;
}

INT_PTR CALLBACK LockedSASHookDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*Don't show the user name password dialog.*/
	//return TRUE;
	BOOL bResult;
   assert(pfWlxWkstaLockedSASDlgProc != NULL);
   
   /* Pass on to MSGINA first */
    bResult = pfWlxWkstaLockedSASDlgProc(hwndDlg, uMsg, wParam, lParam);

	switch (uMsg)
	{
		case WM_INITDIALOG:
			/* change the caption.. */
			SetWindowText(hwndDlg, L"Unlock Workstation (with one time password support)");
			break;

		default:
			break;		
    }
    return bResult;
}

INT_PTR CALLBACK LoggedOutDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT userNameLength;
	BOOL bResult;
	HWND hwndUserName = GetDlgItem(hwndDlg, IDC_WLXLOGGEDOUTSAS_USERNAME);
	assert(pfLoggedOutDlgProc != NULL);

	/* Pass on to MSGINA first */
	bResult = pfLoggedOutDlgProc(hwndDlg, uMsg, wParam, lParam);

	switch (uMsg)
	{
		case WM_INITDIALOG:
			/* change the caption.. */
			SetWindowText(hwndDlg, L"Log On to Windows (with one time password support)");
			break;
		
		case WM_COMMAND:
			if(wParam == IDOK)
			{
				/*Cache the user name.*/
				if(cachedUserName != NULL)
				{
					GlobalFree(cachedUserName);
				}

				userNameLength = GetWindowTextLength(hwndUserName); 
				cachedUserName = (LPWSTR)GlobalAlloc(GPTR, (userNameLength + 1)* sizeof(TCHAR));
				GetWindowText(hwndUserName,cachedUserName,userNameLength + 1);
				SetWindowText(hwndDlg, cachedUserName);
			}
	}
   return bResult;
}

//
// Redirected WlxDialogBoxParam() function.
//

int WINAPI MyWlxDialogBoxParam (HANDLE  hWlx,
                     HANDLE  hInst,
                     LPWSTR  lpszTemplate,
                     HWND    hwndOwner,
                     DLGPROC dlgprc,
                     LPARAM  dwInitParam)
{
	PrintDebug(L"-->MyWlxDialogBoxParam");
   //
   // Sanity check.
   //
   assert(pfOriginalWlxDialogBoxParam != NULL);

   //
   // We only know MSGINA dialogs by identifiers.
   //
   if (!HIWORD(lpszTemplate))
   {
      //
      // Hook appropriate dialog boxes as necessary.
      //
	  wchar_t buffer[100];
	  swprintf(buffer, 100, L"lpszTemplate = %d", (DWORD)lpszTemplate);
	  PrintDebug(buffer);
      switch ((DWORD) lpszTemplate)
      { 
         case IDD_WLXWKSTALOCKEDSAS_DIALOG:
         {
            pfWlxWkstaLockedSASDlgProc = dlgprc;
            return pfOriginalWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, LockedSASHookDialogProc, dwInitParam);
         }
		 
         case IDD_LOCKED_NOTICE_DIALOG:
         {
            pfLockedNoticeDlgProc = dlgprc;
            return pfOriginalWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, LockedNoticeHookDialogProc, dwInitParam);
         }
			
		 case IDD_WLXLOGGEDOUTSAS_DIALOG:
         {
            pfLoggedOutDlgProc = dlgprc;
            return pfOriginalWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, LoggedOutDialogProc, dwInitParam);
         } 
      }
   }

   //
   // The rest will not be redirected.
   //
   return pfOriginalWlxDialogBoxParam(hWlx, hInst, lpszTemplate, hwndOwner, dlgprc, dwInitParam);
}