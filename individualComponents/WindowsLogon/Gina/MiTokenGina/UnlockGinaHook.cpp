/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 BASED ON MICROSOFT Ginahook SDK Sample
 Module:   Ginahook.c
 Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.


-------------------------------------------------------------------------------*/

#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <winwlx.h>
#include <string>

#include "UnlockGinaHook.h"
#include "UnlockGinaDlg.h"
#include "resource.h"
#include "Debug.h"

#include <time.h>	
//
// Location of the real MSGINA.
//

#define REALGINA_PATH      TEXT("MSGINA.DLL")
#define GINASTUB_VERSION   (WLX_VERSION_1_4) // Highest version supported at
                                             // this point. Remember to modify
                                             // this as support for newer version
                                             // is added to this program.

#define SAS_TYPE_UNLOCK_CARD_PRESENTED (WLX_SAS_TYPE_MAX_MSFT_VALUE + 5)

// Our global variables
static HANDLE g_hModule = NULL;
static HANDLE g_hWlx = NULL;

//static volatile BOOL g_bReaderPresent = FALSE;
//static volatile BOOL g_bCardLoaded = FALSE;
//static volatile unsigned long g_cardSerialNumber = 0;
//static volatile BOOL g_bCardPresented = FALSE;

//static HANDLE g_ScreensaverThread = NULL;
//static volatile HANDLE g_ScreensaverThreadExitSignal = NULL;
//static const DWORD SCREENSAVER_THREAD_CARD_PRESENTED = 1;

//
// Winlogon function dispatch table.
//
static PVOID g_pWinlogon = NULL;
static DWORD g_dwVersion = WLX_VERSION_1_3;

//
// Pointers to the real MSGINA functions.
//
static PFWLXNEGOTIATE            pfWlxNegotiate;
static PFWLXINITIALIZE           pfWlxInitialize;
static PFWLXDISPLAYSASNOTICE     pfWlxDisplaySASNotice;
static PFWLXLOGGEDOUTSAS         pfWlxLoggedOutSAS;
static PFWLXACTIVATEUSERSHELL    pfWlxActivateUserShell;
static PFWLXLOGGEDONSAS          pfWlxLoggedOnSAS;
static PFWLXDISPLAYLOCKEDNOTICE  pfWlxDisplayLockedNotice;
static PFWLXWKSTALOCKEDSAS       pfWlxWkstaLockedSAS;
static PFWLXISLOCKOK             pfWlxIsLockOk;
static PFWLXISLOGOFFOK           pfWlxIsLogoffOk;
static PFWLXLOGOFF               pfWlxLogoff;
static PFWLXSHUTDOWN             pfWlxShutdown;

//
// New for version 1.1
//
static PFWLXSTARTAPPLICATION     pfWlxStartApplication  = NULL;
static PFWLXSCREENSAVERNOTIFY    pfWlxScreenSaverNotify = NULL;

//
// New for version 1.2 - No new GINA interface was added, except
//                       a new function in the dispatch table.
//

//
// New for version 1.3
//
static PFWLXNETWORKPROVIDERLOAD   pfWlxNetworkProviderLoad  = NULL;
static PFWLXDISPLAYSTATUSMESSAGE  pfWlxDisplayStatusMessage = NULL;
static PFWLXGETSTATUSMESSAGE      pfWlxGetStatusMessage     = NULL;
static PFWLXREMOVESTATUSMESSAGE   pfWlxRemoveStatusMessage  = NULL;

//
// New for version 1.4
//
static PFWLXGETCONSOLESWITCHCREDENTIALS		pfWlxGetConsoleSwitchCredentials	= NULL;
static PFWLXRECONNECTNOTIFY					pfWlxReconnectNotify				= NULL;
static PFWLXDISCONNECTNOTIFY				pfWlxDisconnectNotify				= NULL;

//
// Hook into the real MSGINA.
//

BOOL waitingForOTP = FALSE;

BOOL MyInitialize (HINSTANCE hDll, DWORD dwWlxVersion)
{
   //
   // Get pointers to all of the WLX functions in the real MSGINA.
   //
	PrintDebug(L"-->MyInitialize");
   pfWlxInitialize = 
      (PFWLXINITIALIZE) GetProcAddress(hDll, "WlxInitialize");
   if (!pfWlxInitialize) 
   {
	  ERROR_WIN32("failed to load MSGINAs WlxInitialize");
      return FALSE;
   }

   pfWlxDisplaySASNotice =
      (PFWLXDISPLAYSASNOTICE) GetProcAddress(hDll, "WlxDisplaySASNotice");
   if (!pfWlxDisplaySASNotice) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxDisplaySASNotice");
      return FALSE;
   }

   pfWlxLoggedOutSAS = 
      (PFWLXLOGGEDOUTSAS) GetProcAddress(hDll, "WlxLoggedOutSAS");
   if (!pfWlxLoggedOutSAS) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxLoggedOutSAS");
      return FALSE;
   }

   pfWlxActivateUserShell =
      (PFWLXACTIVATEUSERSHELL) GetProcAddress(hDll, "WlxActivateUserShell");
   if (!pfWlxActivateUserShell) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxActivateUserShell");
      return FALSE;
   }

   pfWlxLoggedOnSAS =
      (PFWLXLOGGEDONSAS) GetProcAddress(hDll, "WlxLoggedOnSAS");
   if (!pfWlxLoggedOnSAS) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxLoggedOnSAS");
      return FALSE;
   }

   pfWlxDisplayLockedNotice =
      (PFWLXDISPLAYLOCKEDNOTICE) GetProcAddress(hDll, "WlxDisplayLockedNotice");
   if (!pfWlxDisplayLockedNotice) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxDisplayLockedNotice");
      return FALSE;
   }

   pfWlxIsLockOk = 
      (PFWLXISLOCKOK) GetProcAddress(hDll, "WlxIsLockOk");
   if (!pfWlxIsLockOk) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxIsLockOk");
      return FALSE;
   }

   pfWlxWkstaLockedSAS =
       (PFWLXWKSTALOCKEDSAS) GetProcAddress(hDll, "WlxWkstaLockedSAS");
   if (!pfWlxWkstaLockedSAS) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxWkstaLockedSAS");
      return FALSE;
   }

   pfWlxIsLogoffOk = 
      (PFWLXISLOGOFFOK) GetProcAddress(hDll, "WlxIsLogoffOk");
   if (!pfWlxIsLogoffOk) 
   {
   	  ERROR_WIN32("failed to load MSGINAs WlxIsLogoffOk");
      return FALSE;
   }

   pfWlxLogoff = 
      (PFWLXLOGOFF) GetProcAddress(hDll, "WlxLogoff");
   if (!pfWlxLogoff) 
   {
    ERROR_WIN32("failed to load MSGINAs WlxLogoff");
     return FALSE;
   }

   pfWlxShutdown = 
      (PFWLXSHUTDOWN) GetProcAddress(hDll, "WlxShutdown");
   if (!pfWlxShutdown) 
   {
  	 ERROR_WIN32("failed to load MSGINAs WlxShutdown");
     return FALSE;
   }

   //
   // Load functions for version 1.1 as necessary.
   //
   if (dwWlxVersion > WLX_VERSION_1_0)
   {
      pfWlxStartApplication = 
         (PFWLXSTARTAPPLICATION) GetProcAddress(hDll, "WlxStartApplication");
      if (!pfWlxStartApplication)
      {
     	  ERROR_WIN32("failed to load MSGINAs WlxStartApplication");
         return FALSE;
      }

      pfWlxScreenSaverNotify = 
         (PFWLXSCREENSAVERNOTIFY) GetProcAddress(hDll, "WlxScreenSaverNotify");
      if (!pfWlxScreenSaverNotify)
      {
         ERROR_WIN32("failed to load MSGINAs WlxScreenSaverNotify");
		 return FALSE;
      }
   }

   //
   // Load functions for version 1.3 as necessary.
   //
   if (dwWlxVersion > WLX_VERSION_1_2)
   {
      pfWlxNetworkProviderLoad = 
         (PFWLXNETWORKPROVIDERLOAD) 
            GetProcAddress(hDll, "WlxNetworkProviderLoad");
      if (!pfWlxNetworkProviderLoad)
      {
    	  ERROR_WIN32("failed to load MSGINAs WlxNetworkProviderLoad");
         return FALSE;
      }

      pfWlxDisplayStatusMessage = (PFWLXDISPLAYSTATUSMESSAGE) GetProcAddress(hDll, "WlxDisplayStatusMessage");
      if (!pfWlxDisplayStatusMessage)
      {
     	 ERROR_WIN32("failed to load MSGINAs WlxDisplayStatusMessage");
         return FALSE;
      }

      pfWlxGetStatusMessage = (PFWLXGETSTATUSMESSAGE) GetProcAddress(hDll, "WlxGetStatusMessage");
      if (!pfWlxGetStatusMessage)
      {
     	 ERROR_WIN32("failed to load MSGINAs WlxGetStatusMessage");
         return FALSE;
      }

      pfWlxRemoveStatusMessage = (PFWLXREMOVESTATUSMESSAGE) GetProcAddress(hDll, "WlxRemoveStatusMessage");
      if (!pfWlxRemoveStatusMessage)
      {
    	 ERROR_WIN32("failed to load MSGINAs WlxRemoveStatusMessage");
         return FALSE;
      }
   }

   //
   // Load functions for version 1.4 as necessary.
   //
   if (dwWlxVersion > WLX_VERSION_1_3)
   {
      pfWlxGetConsoleSwitchCredentials = (PFWLXGETCONSOLESWITCHCREDENTIALS) GetProcAddress(hDll, "WlxGetConsoleSwitchCredentials");
      if (!pfWlxGetConsoleSwitchCredentials)
      {
    	 ERROR_WIN32("failed to load MSGINAs WlxGetConsoleSwitchCredentials");
         return FALSE;
      }

      pfWlxReconnectNotify = (PFWLXRECONNECTNOTIFY) GetProcAddress(hDll, "WlxReconnectNotify");
      if (!pfWlxReconnectNotify)
      {
    	 ERROR_WIN32("failed to load MSGINAs WlxReconnectNotify");
         return FALSE;
      }

      pfWlxDisconnectNotify = (PFWLXDISCONNECTNOTIFY) GetProcAddress(hDll, "WlxDisconnectNotify");
      if (!pfWlxDisconnectNotify)
      {
    	 ERROR_WIN32("failed to load MSGINAs WlxDisconnectNotify");
         return FALSE;
      }
   }

   return TRUE;
}


BOOL WINAPI WlxNegotiate (DWORD dwWinlogonVersion, DWORD* pdwDllVersion)
{
   HINSTANCE hDll;
   DWORD dwWlxVersion = GINASTUB_VERSION;

   //
   // Load MSGINA.DLL.
   //
   hDll = LoadLibrary(REALGINA_PATH);
   if (!hDll) 
   {
		ERROR_WIN32("failed to load" REALGINA_PATH);
		return FALSE;
   }

   //
   // Get pointers to WlxNegotiate function in the real MSGINA.
   //
   pfWlxNegotiate = (PFWLXNEGOTIATE) GetProcAddress(hDll, "WlxNegotiate");
   if (!pfWlxNegotiate) 
   {
	   ERROR_WIN32("failed to load MSGINA WlxNegotiate");
      return FALSE;
   }
 
   //
   // Handle older version of Winlogon.
   //
   if (dwWinlogonVersion < dwWlxVersion)
   {
      dwWlxVersion = dwWinlogonVersion;
   }

   //
   // Negotiate with MSGINA for version that we can support.
   //
   if (!pfWlxNegotiate(dwWlxVersion, &dwWlxVersion))
   {
  	  ERROR_WIN32("MSGINA WlxNegotiate failed");
      return FALSE;
   }
   
   //
   // Load the rest of the WLX functions from the real MSGINA.
   //
   if (!MyInitialize(hDll, dwWlxVersion))
   {
      return FALSE;
   }

   //
   // Inform Winlogon which version to use.
   //
   *pdwDllVersion = g_dwVersion = dwWlxVersion;
   
   return TRUE;
}


BOOL WINAPI WlxInitialize (LPWSTR  lpWinsta,
						   HANDLE  hWlx,
						   PVOID   pvReserved,
						   PVOID   pWinlogonFunctions,
						   PVOID * pWlxContext)
{


   //
   // Save pointer to dispatch table.
   // 
   // Note that g_pWinlogon will need to be properly casted to the 
   // appropriate version when used to call function in the dispatch 
   // table.
   //
   // For example, assuming we are at WLX_VERSION_1_3, we would call
   // WlxSasNotify() as follows:
   //
   // ((PWLX_DISPATCH_VERSION_1_3) g_pWinlogon)->WlxSasNotify(hWlx, MY_SAS);
   //
   g_pWinlogon = pWinlogonFunctions;

   g_hWlx = hWlx;

   //
   // Now hook the WlxDialogBoxParam() dispatch function.
   //
   HookWlxDialogBoxParam(g_pWinlogon, g_dwVersion);

   return pfWlxInitialize(lpWinsta,
                          hWlx,
                          pvReserved,
                          pWinlogonFunctions,
                          pWlxContext);
}


static void SASNotify (DWORD dwSasType)
{
   DEBUG_PRINT(L"---> SASNotify");
   switch (g_dwVersion)
   {
      case WLX_VERSION_1_0: 
      {
         ((PWLX_DISPATCH_VERSION_1_0) g_pWinlogon)->WlxSasNotify(g_hWlx, dwSasType);
         break;
      }
  
      case WLX_VERSION_1_1:
      {
         ((PWLX_DISPATCH_VERSION_1_1) g_pWinlogon)->WlxSasNotify(g_hWlx, dwSasType);
         break;
      }
   
      case WLX_VERSION_1_2:
      {
         ((PWLX_DISPATCH_VERSION_1_2) g_pWinlogon)->WlxSasNotify(g_hWlx, dwSasType);
         break;
      }
   
      case WLX_VERSION_1_3:
      {
         ((PWLX_DISPATCH_VERSION_1_3) g_pWinlogon)->WlxSasNotify(g_hWlx, dwSasType);
         break;
      }

      case WLX_VERSION_1_4:
      {
         ((PWLX_DISPATCH_VERSION_1_4) g_pWinlogon)->WlxSasNotify(g_hWlx, dwSasType);
         break;
      }

	  default:
		  ERROR_WIN32("SASNotify: unsupported g_dwVersion!");

   }

   DEBUG_PRINT(L"<--- SASNotify");
}

VOID WINAPI WlxDisplaySASNotice (PVOID pWlxContext)
{
   DEBUG_PRINT(L"---> WlxDisplaySASNotice");
   pfWlxDisplaySASNotice(pWlxContext);
}




//Filters no CPs
#define FILTER_NONE 0
//Filters the Mi-Token CP
#define FILTER_MITOKEN 2

int WINAPI WlxLoggedOutSAS (PVOID                pWlxContext,
							 DWORD                dwSasType,
							 PLUID                pAuthenticationId,
							 PSID                 pLogonSid,
							 PDWORD               pdwOptions,
							 PHANDLE              phToken,
							 PWLX_MPR_NOTIFY_INFO pMprNotifyInfo,
							 PVOID *              pProfile)
{
   DEBUG_PRINT(L"---> WlxLoggedOutSAS");

   clock_t startClock = clock();
   int iRet = pfWlxLoggedOutSAS(pWlxContext,
                            dwSasType,
                            pAuthenticationId,
                            pLogonSid,
                            pdwOptions,
                            phToken,
                            pMprNotifyInfo,
                            pProfile);


   clock_t endClock = clock();
   
   if(iRet == WLX_SAS_ACTION_LOGON)  
   {
	/*LPSTR resultstr = "Administrator";
	WideCharToMultiByte(CP_ACP,0,cachedUserName,-1,resultstr,20,NULL,NULL);
	*/
	  INT_PTR   result ;
	  PrintDebug(L"Received Valid User Creds [Logged out]");
	  {
		  wchar_t wbuffer[80];
		  swprintf(wbuffer, 80, L"User logon took %d clocks", (int)(endClock - startClock));
		  PrintDebug(wbuffer);
	  }
	if((int)(endClock - startClock) < 60) //less then 60ms - no way a human logged in
	{
		PrintDebug(L"Under 60ms to process the login request - not even checking connection mode");
		return iRet;
	}
	  
   /*
	if (strncmp ( "Administrator" , resultstr, 13 ) != 0){ */
   
   //if the user is a remote user, don't ask for OTP
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif
#ifndef SM_REMOTECONTROL
#define SM_REMOTECONTROL 0x2001
#endif
   int filterMode = (0 != GetSystemMetrics(SM_REMOTESESSION)) ? FILTER_NONE : FILTER_NONE;
   wchar_t* keyname = (0 != GetSystemMetrics(SM_REMOTESESSION)) ? L"filterRDP" : L"filterLocal";
   PrintDebug(L"Keyname [LoggedOut] : ");
   PrintDebug(keyname);
   //we are a remote user
   long value;
   DWORD len = sizeof(value);
   HKEY subKey;
   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon", 0, KEY_READ, &subKey) == ERROR_SUCCESS)
   {
	   if(ERROR_SUCCESS == RegQueryValueEx(subKey, keyname, NULL, NULL, (BYTE*)&value, &len))
			filterMode = value;
	   RegCloseKey(subKey);
   }

   //if we have MiToken Being filtered do not show the OTP Prompt
   if(filterMode == FILTER_MITOKEN)
   {
	   PrintDebug(L"Mi-Token filtered out");
	   result = IDOK;
   }
   else
   {
	   PrintDebug(L"Requesting Mi-Token OTP");

		result = VerifyOTP(g_hModule);
   }
	
	/*else {result = IDOK;}*/
	  

	  /*If the dialog is cancelled, allow the user to login with a diffegrent name.*/
	  if(result != IDOK)
	  {
		iRet = WLX_SAS_ACTION_NONE;
	  }  
   
	  return iRet;}
}



VOID WINAPI WlxDisplayLockedNotice (PVOID pWlxContext)
{
    DEBUG_PRINT(L"---> WlxDisplayLockedNotice");

	/* we will hook this dialog..  the hook can generate a SAS which will take us to WlxWkstaLockedSAS */
	pfWlxDisplayLockedNotice(pWlxContext);
}


BOOL WINAPI WlxIsLockOk (PVOID pWlxContext)
{
   DEBUG_PRINT(L"---> WlxIsLockOk");
   return pfWlxIsLockOk(pWlxContext);
}


int WINAPI WlxWkstaLockedSAS (PVOID pWlxContext, DWORD dwSasType)
{
	
    DEBUG_PRINT(L"---> WlxWkstaLockedSAS");
	//Sleep(5000);
	PrintDebug(L"Calling pfWlxWkstaLockedSAS");
	int retVal = pfWlxWkstaLockedSAS(pWlxContext, dwSasType); //WLX_SAS_ACTION_NONE;
	if(retVal != WLX_SAS_ACTION_UNLOCK_WKSTA )
		return retVal;
	INT_PTR result;
	if(WLX_SAS_TYPE_CTRL_ALT_DEL == dwSasType)
	{
		//pfWlxDisplayLockedNotice(pWlxContext);
		PrintDebug(L"Received Valid User Creds [Locked]");
		//Sleep(5000);
		//return WLX_SAS_ACTION_FORCE_LOGOFF;
		//retVal= pfWlxWkstaLockedSAS(pWlxContext, dwSasType);
		
		wchar_t buffer[1024];
		swprintf_s(buffer, 1024, L"RetVal : %d", retVal);
		PrintDebug(buffer);
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif
#ifndef SM_REMOTECONTROL
#define SM_REMOTECONTROL 0x2001
#endif
	   int filterMode = (0 != GetSystemMetrics(SM_REMOTESESSION)) ? FILTER_NONE : FILTER_NONE;
	   wchar_t* keyname = (0 != GetSystemMetrics(SM_REMOTESESSION)) ? L"filterRDP" : L"filterLocal";
	   PrintDebug(L"KeyName [Locked] : ");
	   PrintDebug(keyname);
	   //we are a remote user
	   long value;
	   DWORD len = sizeof(value);
	   HKEY subKey;
	   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Mi-Token\\Logon", 0, KEY_READ, &subKey) == ERROR_SUCCESS)
	   {
		   if(ERROR_SUCCESS == RegQueryValueEx(subKey, keyname, NULL, NULL, (BYTE*)&value, &len))
				filterMode = value;
		   RegCloseKey(subKey);
	   }

	   //if we have MiToken Being filtered do not show the OTP Prompt
	   if(filterMode == FILTER_MITOKEN)
	   {
		   PrintDebug(L"Mi-Token Filtered out");
		   result = IDOK;
		   return retVal;
	   }
	   else
	   {
		   waitingForOTP = TRUE;
		   PrintDebug(L"Require Mi-Token OTP");
			result = VerifyOTP(g_hModule);
			PrintDebug(L"Setting hwndOTP back to NULL");
			gHwndOTP = NULL;
			waitingForOTP = FALSE;
	   }
		
		if(IDOK == result)
			retVal = WLX_SAS_ACTION_UNLOCK_WKSTA;
		else
			retVal = WLX_SAS_ACTION_NONE;
	}

	return retVal;
}


BOOL WINAPI WlxIsLogoffOk (PVOID pWlxContext)
{
   BOOL bSuccess;

   DEBUG_PRINT(L"---> WlxIsLogoffOk");
   bSuccess = pfWlxIsLogoffOk(pWlxContext);

   if(bSuccess) 
   {
	  //g_cardSerialNumber = 0;
	  //g_bCardLoaded = FALSE;
   }

   return bSuccess;
}


VOID WINAPI WlxLogoff (PVOID pWlxContext)
{
    DEBUG_PRINT(L"---> WlxLogoff");

	/*Destroy the cached username.*/
	if(cachedUserName != NULL)
	{
		GlobalFree(cachedUserName);
	}

	pfWlxLogoff(pWlxContext);
}


VOID WINAPI WlxShutdown(PVOID pWlxContext, DWORD ShutdownType)
{
    DEBUG_PRINT(L"---> WlxShutdown");

	/*Destroy the cached username.*/
	if(cachedUserName != NULL)
	{
		GlobalFree(cachedUserName);
	}

	pfWlxShutdown(pWlxContext, ShutdownType);
}


BOOL WINAPI WlxActivateUserShell (PVOID pWlxContext,
                      PWSTR pszDesktopName,
                      PWSTR pszMprLogonScript,
                      PVOID pEnvironment)
{
   DEBUG_PRINT(L"---> WlxActivateUserShell");

   return pfWlxActivateUserShell(pWlxContext,
                                 pszDesktopName,
                                 pszMprLogonScript,
                                 pEnvironment);
}


int WINAPI WlxLoggedOnSAS (PVOID pWlxContext,
                DWORD dwSasType,
                PVOID pReserved)
{
   DEBUG_PRINT(L"---> WlxLoggedOnSAS");

   if(waitingForOTP)
   {
	   PrintDebug(L"[Waiting For OTP]");
	   {
		   wchar_t dbuffer[80];
		   swprintf_s(dbuffer, 80, L"hwndOTP = %d", gHwndOTP);
		   PrintDebug(dbuffer);
	   }
	   if(gHwndOTP)
	   {
		   PrintDebug(L"[hwndOTP is Not Null... Sending EndDialog Message]");
		   //EndDialog(hwndOTP, IDCANCEL);
		   SendMessage(gHwndOTP, WM_CLOSE, 0, 0);
		   PrintDebug(L"[Message Sent]");
	   }
	   PrintDebug(L"[Locking Workstation]");
	   return WLX_SAS_ACTION_LOCK_WKSTA;
   }
   return pfWlxLoggedOnSAS(pWlxContext, 
                           dwSasType, 
                           pReserved);
}


//
// New for version 1.1
//

BOOL WINAPI WlxScreenSaverNotify (PVOID  pWlxContext,
						          BOOL * pSecure)
{
	/*DWORD ret = 0;
	pWlxContext;*/


    DEBUG_PRINT(L"---> WlxScreenSaverNotify");

	/* tell MSGINA about it - but we don't care what it says */
	return  pfWlxScreenSaverNotify(pWlxContext, pSecure);
}

BOOL WINAPI WlxStartApplication (PVOID pWlxContext,
								 PWSTR pszDesktopName,
								 PVOID pEnvironment,
								 PWSTR pszCmdLine)
{
   return pfWlxStartApplication(pWlxContext,
                                pszDesktopName,
                                pEnvironment,
                                pszCmdLine);
}


//
// New for version 1.3
//

BOOL WINAPI WlxNetworkProviderLoad (PVOID                pWlxContext,
									 PWLX_MPR_NOTIFY_INFO pNprNotifyInfo)
{
   return pfWlxNetworkProviderLoad(pWlxContext, pNprNotifyInfo);
}


BOOL WINAPI WlxDisplayStatusMessage (PVOID pWlxContext,
                         HDESK hDesktop,
                         DWORD dwOptions,
                         PWSTR pTitle,
                         PWSTR pMessage)
{
   return pfWlxDisplayStatusMessage(pWlxContext,
                                    hDesktop,
                                    dwOptions,
                                    pTitle,
                                    pMessage);
}


BOOL WINAPI WlxGetStatusMessage (PVOID   pWlxContext,
                     DWORD * pdwOptions,
                     PWSTR   pMessage,
                     DWORD   dwBufferSize)
{
   return pfWlxGetStatusMessage(pWlxContext,
                                pdwOptions,
                                pMessage,
                                dwBufferSize);
}


BOOL WINAPI WlxRemoveStatusMessage (PVOID pWlxContext)
{
   return pfWlxRemoveStatusMessage(pWlxContext);
}


//
// New for 1.4
//
BOOL WINAPI WlxGetConsoleSwitchCredentials (PVOID pWlxContext, PVOID pCredInfo)
{
	return pfWlxGetConsoleSwitchCredentials(pWlxContext, pCredInfo);
}

VOID WINAPI WlxReconnectNotify (PVOID pWlxContext)
{
	pfWlxReconnectNotify(pWlxContext);
}

VOID WINAPI WlxDisconnectNotify (PVOID  pWlxContext)
{
	pfWlxDisconnectNotify(pWlxContext);
}

//
// DLL Main
//

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  reason, LPVOID lpReserved)
{
    if (DLL_PROCESS_ATTACH == reason) 
	{
        g_hModule = hModule;
		lpReserved;
    }
    return TRUE;
}
