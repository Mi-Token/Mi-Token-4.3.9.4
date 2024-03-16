#include <windows.h>
#include <stdio.h> 
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <dbt.h>
#include <LMaccess.h>
#include <atlstr.h>
#include <lm.h> 

#include <DsGetDC.h>
#define GROUPS_KEY TEXT("Software\\Mi-Token\\Logon\\Groups\\")
#define KEY_NAME TEXT("User Group")
#define KEY_VALUE TEXT("domain Admins")
#define KEY_NAME_DEBUG TEXT("debug")

// This GUID is for all USB serial host PnP drivers, but you can replace it 
// with any valid device class guid.
GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 
                      0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };

// For informational messages and window titles
PWSTR g_pszAppName;

// Forward declarations
void OutputMessage(HWND hOutWnd, WPARAM wParam, LPARAM lParam);
void ErrorHandler(LPTSTR lpszFunction);
bool GetCurrentLogonUserName(/*[out]*/ LPTSTR& lpszAccountName, /*[out]*/ LPTSTR& lpszReferencedDomainName);
bool isDebugEnabled();
void debug(char *text);

typedef struct _USER_GROUP_BUFFER
{
	GROUP_USERS_INFO_0* buffer;
	DWORD count;
} USER_GROUP_BUFFER;

NET_API_STATUS getUserGroups(USER_GROUP_BUFFER* localBuffer, USER_GROUP_BUFFER* domainBuffer, CString username, CString domainAndUser);

void missingYubikey()
{
	if(isDebugEnabled()) debug("Yubikey was removed, locking user screen");
	LockWorkStation();
	if(isDebugEnabled()) debug("User workstation locked successfully");
}

int getYubiCount()
{
	bool IsDebug = isDebugEnabled();
	if(IsDebug) debug("Trying to get yubikey count");
	int cYubi = 0;
	// Get Number Of Devices
	UINT nDevices = 0;
	GetRawInputDeviceList( NULL, &nDevices, sizeof( RAWINPUTDEVICELIST ) );

	// Got Any?
	if( nDevices < 1 )
	{
		if(IsDebug) debug("Error! Looks like -ve USB devices attached");
		return 0;
	}
	
	// Allocate Memory For Device List
	PRAWINPUTDEVICELIST pRawInputDeviceList;
	pRawInputDeviceList = new RAWINPUTDEVICELIST[ sizeof( RAWINPUTDEVICELIST ) * nDevices ];

	// Got Memory?
	if( pRawInputDeviceList == NULL )
	{
		if(IsDebug) debug("Error! Unable to allocate memory for USB devices"); 
		return 0;
	}
	
	// Fill Device List Buffer
	int nResult;
	nResult = GetRawInputDeviceList( pRawInputDeviceList, &nDevices, sizeof( RAWINPUTDEVICELIST ) );
	if(IsDebug) debug("Attempted getting list of attached USB devices");

	// Got Device List?
	if( nResult < 0 )
	{
		if(IsDebug) debug("Error! -ve result received upon getting the list of attached USB devices.");
		// Clean Up
		delete [] pRawInputDeviceList;

		return 0;
	}

	// Loop Through Device List
	for( UINT i = 0; i < nDevices; i++ )
	{
		if(IsDebug) debug("Looping through device ");
		// Get Character Count For Device Name
		UINT nBufferSize = 0;
		nResult = GetRawInputDeviceInfo( pRawInputDeviceList[i].hDevice, // Device
										 RIDI_DEVICENAME,				 // Get Device Name
										 NULL,							 // NO Buff, Want Count!
										 &nBufferSize );				 // Char Count Here!
		if(IsDebug) debug("Attempted collecting device information");

		// Got Device Name?
		if( nResult < 0 )
		{
			// Next
			if(IsDebug) debug("Error! Unable to get device information, ignoring device...");
			continue;
		}

		// Allocate Memory For Device Name
		WCHAR* wcDeviceName = new WCHAR[ nBufferSize + 1 ];
		if(IsDebug) debug("Attempted memory allocation for device name");

		// Got Memory
		if( wcDeviceName == NULL )
		{
			if(IsDebug) debug("Failed to allocate memory for device");
			// Next
			continue;
		}

		// Get Name
		nResult = GetRawInputDeviceInfo( pRawInputDeviceList[i].hDevice, // Device
										 RIDI_DEVICENAME,				 // Get Device Name
										 wcDeviceName,					 // Get Name!
										 &nBufferSize );				 // Char Count
		if(IsDebug) debug("Attempted getting device name");

		// Got Device Name?
		if( nResult < 0 )
		{
			if(IsDebug) debug("Error! failed to get device name");
			// Clean Up
			delete [] wcDeviceName;

			// Next
			continue;
		}

		// Set Device Info & Buffer Size
		RID_DEVICE_INFO rdiDeviceInfo;
		rdiDeviceInfo.cbSize = sizeof( RID_DEVICE_INFO );
		nBufferSize = rdiDeviceInfo.cbSize;
		if(IsDebug) debug("Allocated buffer for getting device other information");

		// Get Device Info
		nResult = GetRawInputDeviceInfo( pRawInputDeviceList[i].hDevice,
										 RIDI_DEVICEINFO,
										 &rdiDeviceInfo,
										 &nBufferSize );
		if(IsDebug) debug("Attempted getting device other information");

		// Got All Buffer?
		if( nResult < 0 )
		{
			if(IsDebug) debug("Error! Failed to get device other information");
			// Next
			continue;
		}

		// Mouse
		if( rdiDeviceInfo.dwType == RIM_TYPEMOUSE )
		{
			if(IsDebug) debug("This device is categorized as a mouse");
			/*
			// Current Device
			cout << endl << "Displaying device " << i+1 << " information. (MOUSE)" << endl;
			wcout << L"Device Name: " << wcDeviceName << endl;
			cout << "Mouse ID: " << rdiDeviceInfo.mouse.dwId << endl;
			cout << "Mouse buttons: " << rdiDeviceInfo.mouse.dwNumberOfButtons << endl;
			cout << "Mouse sample rate (Data Points): " << rdiDeviceInfo.mouse.dwSampleRate << endl;
			if( rdiDeviceInfo.mouse.fHasHorizontalWheel )
			{
				cout << "Mouse has horizontal wheel" << endl;
			}
			else
			{
				cout << "Mouse does not have horizontal wheel" << endl;
			}
			*/
		}

		// Keyboard
		else if( rdiDeviceInfo.dwType == RIM_TYPEKEYBOARD )
		{
			if(IsDebug) debug("This device is categorized as a keyboard");

			wchar_t* yubiPrefix = L"\\\\?\\HID#VID_1050&PID_0010";
			int yubiLen = wcslen(yubiPrefix);
			if(IsDebug) debug("Checking wether this keyboard is a yubikey or not");
			if(memcmp(yubiPrefix, wcDeviceName, yubiLen * 2) == 0)
			{
				if(IsDebug) debug("Found Yubikey");
				cYubi++;
				// Current Device
				//cout << endl << "Displaying device " << i+1 << " information. (KEYBOARD)" << endl;
				//wcout << L"DN: " << wcDeviceName << endl;
				/*cout << "Keyboard mode: " << rdiDeviceInfo.keyboard.dwKeyboardMode << endl;
				cout << "Number of function keys: " << rdiDeviceInfo.keyboard.dwNumberOfFunctionKeys << endl;
				cout << "Number of indicators: " << rdiDeviceInfo.keyboard.dwNumberOfIndicators << endl;
				cout << "Number of keys total: " << rdiDeviceInfo.keyboard.dwNumberOfKeysTotal << endl;
				cout << "Type of the keyboard: " << rdiDeviceInfo.keyboard.dwType << endl;
				cout << "Subtype of the keyboard: " << rdiDeviceInfo.keyboard.dwSubType << endl;
				*/
			}
		}

		// Some HID
		/*
		else // (rdi.dwType == RIM_TYPEHID)
		{
			// Current Device
			cout << endl << "Displaying device " << i+1 << " information. (HID)" << endl;
			wcout << L"Device Name: " << wcDeviceName << endl;
			cout << "Vendor Id:" << rdiDeviceInfo.hid.dwVendorId << endl;
			cout << "Product Id:" << rdiDeviceInfo.hid.dwProductId << endl;
			cout << "Version No:" << rdiDeviceInfo.hid.dwVersionNumber << endl;
			cout << "Usage for the device: " << rdiDeviceInfo.hid.usUsage << endl;
			cout << "Usage Page for the device: " << rdiDeviceInfo.hid.usUsagePage << endl;
		}
		*/
		// Delete Name Memory!
		if(IsDebug) debug("Deleting allocated memory for device name");
		delete [] wcDeviceName;
	}

	// Clean Up - Free Memory
	if(IsDebug) debug("Clearing up the memory allocated for device list");
	delete [] pRawInputDeviceList;

	// Exit
	return cYubi;

}

//
// DoRegisterDeviceInterfaceToHwnd
//
BOOL DoRegisterDeviceInterfaceToHwnd( 
    IN GUID InterfaceClassGuid, 
    IN HWND hWnd,
    OUT HDEVNOTIFY *hDeviceNotify 
)
// Routine Description:
//     Registers an HWND for notification of changes in the device interfaces
//     for the specified interface class GUID. 

// Parameters:
//     InterfaceClassGuid - The interface class GUID for the device 
//         interfaces. 

//     hWnd - Window handle to receive notifications.

//     hDeviceNotify - Receives the device notification handle. On failure, 
//         this value is NULL.

// Return Value:
//     If the function succeeds, the return value is TRUE.
//     If the function fails, the return value is FALSE.

// Note:
//     RegisterDeviceNotification also allows a service handle be used,
//     so a similar wrapper function to this one supporting that scenario
//     could be made from this template.
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDeviceNotify = RegisterDeviceNotification( 
        hWnd,                       // events recipient
        &NotificationFilter,        // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
        );

    if ( NULL == *hDeviceNotify ) 
    {
        ErrorHandler(TEXT("RegisterDeviceNotification"));
        return FALSE;
    }

    return TRUE;
}

//
// MessagePump
//
void MessagePump(
    HWND hWnd
)
// Routine Description:
//     Simple main thread message pump.
//

// Parameters:
//     hWnd - handle to the window whose messages are being dispatched

// Return Value:
//     None.
{
    MSG msg; 
    int retVal;

    // Get all messages for any window that belongs to this thread,
    // without any filtering. Potential optimization could be
    // obtained via use of filter values if desired.

    while( (retVal = GetMessage(&msg, NULL, 0, 0)) != 0 ) 
    { 
        if ( retVal == -1 )
        {
            ErrorHandler(TEXT("GetMessage"));
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } 
}


void yubikeyRemoved()
{
	bool IsDebug = isDebugEnabled();
	if(IsDebug)	debug("WinProcCallback(): Yubikey was removed");
	//FetchSID(username.GetBuffer(), NULL, &sid, &sidDomain);
	//username.ReleaseBuffer();
	LPTSTR lpszAccountName = NULL;
	LPTSTR lpszReferencedDomainName = NULL;

	if(GetCurrentLogonUserName(lpszAccountName, lpszReferencedDomainName))
	{
		if(IsDebug)	debug("WinProcCallback(): Got currently logged on user domain and name");
		CString username = lpszAccountName;
		CString temp = lpszReferencedDomainName;
		CString domainAndUser = temp + "\\" + username;

		//delete user account and clear internal info
		if (lpszAccountName)
		{
			if(IsDebug)	debug("WinProcCallback(): Clearing allocated memory for username");
			delete [] lpszAccountName;
			lpszAccountName = NULL;
		}

		//delete user domain and clear internal info
		if (lpszReferencedDomainName)
		{
			if(IsDebug)	debug("WinProcCallback(): Clearing allocated memory for domain");
			delete [] lpszReferencedDomainName;
			lpszReferencedDomainName = NULL;
		}
	}

	wchar_t user[256];  
	DWORD size = sizeof(user)/sizeof(user[0]); 
	GetUserNameW(user, &size); 
	LPBYTE buffer;  
	DWORD entries, total_entries; 
	PDOMAIN_CONTROLLER_INFO pdci;

	if(IsDebug)	debug("WinProcCallback(): Trying to get domain controller name");
	DWORD dwRet = DsGetDcName(NULL, NULL, NULL, NULL, DS_ONLY_LDAP_NEEDED, &pdci);
	if(IsDebug)	debug("WinProcCallback(): Attempted getting domain controller name");

	if(dwRet == NO_ERROR)
	{
		if(IsDebug)	debug("WinProcCallback(): Trying to get domain user groups for currently logged in user");
		DWORD rc = NetUserGetGroups(pdci->DomainControllerName, user, 0, &buffer, MAX_PREFERRED_LENGTH, &entries, &total_entries); 
		if(IsDebug)	debug("WinProcCallback(): Attempted getting user groups");
		GROUP_USERS_INFO_0 *ggroups = (GROUP_USERS_INFO_0*)buffer;
					
		//get registy key value
		LPWSTR dwValue = new WCHAR[255];
		//MultiByteToWideChar(932, 0, "Domain Users", -1, dwValue, 100);
		HKEY hKey = NULL;
		LONG lResult;
		DWORD dwType = REG_SZ;
		BYTE szBuff[255] = {0};
		DWORD dwSize = sizeof(szBuff);
		LPTSTR lpValueName = KEY_NAME;
		LPTSTR lpValueValue = KEY_VALUE;

		// First, check for a policy.
		if(IsDebug)	debug("WinProcCallback(): Trying to open registry key");
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GROUPS_KEY, 0, KEY_QUERY_VALUE, &hKey);

		if(lResult == ERROR_SUCCESS)
		{
			if(IsDebug)	debug("WinProcCallback(): Successfully read registry key and now trying to read value");
			lResult = RegQueryValueEx(hKey, lpValueName, 0, &dwType, (LPBYTE)dwValue, &dwSize);
			RegCloseKey(hKey);
			if(lResult == ERROR_SUCCESS && IsDebug)
				debug("WinProcCallback(): Successfully read key value for user group");
		}
		else
		if(lResult == ERROR_FILE_NOT_FOUND)
		{
			if(IsDebug)	debug("WinProcCallback(): Registry key was not found. Now trying to create one");
			lResult = RegCreateKeyEx( 
				HKEY_LOCAL_MACHINE, 
				GROUPS_KEY, 
				0, 
				NULL, 
				REG_OPTION_NON_VOLATILE, 
				KEY_WRITE, 
				NULL, 
				&hKey, 
				NULL);
			if(IsDebug)	debug("WinProcCallback(): Attempted creating a new registry key");

			if(lResult == ERROR_SUCCESS)
			{
				if(IsDebug)	debug("WinProcCallback(): Successfully created a registry key. Now trying write new key value");
				lResult = RegSetValueEx (hKey, lpValueName, 0, REG_SZ, (LPBYTE)lpValueValue, _tcslen(lpValueValue)*2 + 1);
				if(IsDebug)	debug("WinProcCallback(): Attempted setting a new key value");
				if(lResult == ERROR_SUCCESS)
				{
					if(IsDebug)	debug("WinProcCallback(): Successfully set a new key value");
					dwValue = lpValueValue + '\0';
				}
			}
		}

		for (int i=0; i<entries; i++)
		{
			if(IsDebug)	debug("WinProcCallback(): Trying to compare a group");
			if(_wcsicmp(dwValue, ggroups[i].grui0_name) == 0)
			{
				if(IsDebug)	debug("WinProcCallback(): User group matched! Going to lock the screen");
				missingYubikey();
				break;
			}
		}

		if(buffer != NULL)
		{
			if(IsDebug)	debug("WinProcCallback(): Clearing the buffer");
			NetApiBufferFree(buffer);
		}

		if(pdci != NULL)
		{
			if(IsDebug)	debug("WinProcCallback(): Clearing pdci");
			NetApiBufferFree(pdci);
		}
	}
}
//
// WinProcCallback
//
INT_PTR WINAPI WinProcCallback(
                              HWND hWnd,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam
                              )
// Routine Description:
//     Simple Windows callback for handling messages.
//     This is where all the work is done because the example
//     is using a window to process messages. This logic would be handled 
//     differently if registering a service instead of a window.

// Parameters:
//     hWnd - the window handle being registered for events.

//     message - the message being interpreted.

//     wParam and lParam - extended information provided to this
//          callback by the message sender.

//     For more information regarding these parameters and return value,
//     see the documentation for WNDCLASSEX and CreateWindowEx.
{
    LRESULT lRet = 1;
    static HDEVNOTIFY hDeviceNotify;
    static HWND hEditWnd;
    static ULONGLONG msgCount = 0;

    switch (message)
    {
	case WM_QUIT:
		break;
	case WM_CREATE:
        //
        // This is the actual registration., In this example, registration 
        // should happen only once, at application startup when the window
        // is created.
        //
        // If you were using a service, you would put this in your main code 
        // path as part of your service initialization.
        //
        if ( ! DoRegisterDeviceInterfaceToHwnd(
                        WceusbshGUID, 
                        hWnd,
                        &hDeviceNotify) )
        {
            // Terminate on failure.
            ErrorHandler(TEXT("DoRegisterDeviceInterfaceToHwnd"));
            ExitProcess(1);
        }


        //
        // Make the child window for output.
        //
        hEditWnd = CreateWindow(TEXT("EDIT"),// predefined class 
                                NULL,        // no window title 
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
                                ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
                                0, 0, 0, 0,  // set size in WM_SIZE message 
                                hWnd,        // parent window 
                                (HMENU)1,    // edit control ID 
                                (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 
                                NULL);       // pointer not needed 

        if ( hEditWnd == NULL )
        {
            // Terminate on failure.
            ErrorHandler(TEXT("CreateWindow: Edit Control"));
            ExitProcess(1);
        }
        // Add text to the window. 
        SendMessage(hEditWnd, WM_SETTEXT, 0, 
            (LPARAM)TEXT("Registered for USB device notification...\n")); 

        break;

    case WM_SETFOCUS: 
        SetFocus(hEditWnd); 

        break;

    case WM_SIZE: 
        // Make the edit control the size of the window's client area. 
        MoveWindow(hEditWnd, 
                   0, 0,                  // starting x- and y-coordinates 
                   LOWORD(lParam),        // width of client area 
                   HIWORD(lParam),        // height of client area 
                   TRUE);                 // repaint window 

        break;

    case WM_DEVICECHANGE:
    {
        //
        // This is the actual message from the interface via Windows messaging.
        // This code includes some additional decoding for this particular device type
        // and some common validation checks.
        //
        // Note that not all devices utilize these optional parameters in the same
        // way. Refer to the extended information for your particular device type 
        // specified by your GUID.
        //
        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
        TCHAR strBuff[256];

        // Output some messages to the window.
        switch (wParam)
        {
		case DBT_DEVICEARRIVAL:
			msgCount++;
			StringCchPrintf(
				strBuff, 256, 
				TEXT("Message %d: DBT_DEVICEARRIVAL\n"), msgCount);
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			msgCount++;
			StringCchPrintf(
				strBuff, 256, 
				TEXT("Message %d: DBT_DEVICEREMOVECOMPLETE\n"), msgCount);
			break;
		case DBT_DEVNODES_CHANGED:
			{
			msgCount++;
			bool IsDebug = isDebugEnabled();
			if(IsDebug)	debug("WinProcCallback(): Device count is changed.");
			static int oldyc = 0;
			int yc = getYubiCount();
			if(IsDebug)	debug("WinProcCallback(): Received yubikey count");
			if(oldyc > yc)
			{
				yubikeyRemoved();
			}

			oldyc = yc;
			StringCchPrintf(
				strBuff, 256, 
				TEXT("Message %I64d: DBT_DEVNODES_CHANGED [%d]\n"), msgCount, yc);
			break;
			}
        default:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: WM_DEVICECHANGE message received, value %d unhandled.\n"), 
                msgCount, wParam);
            break;
        }
        OutputMessage(hEditWnd, wParam, (LPARAM)strBuff);
    }
            break;
    case WM_CLOSE:
        if ( ! UnregisterDeviceNotification(hDeviceNotify) )
        {
           ErrorHandler(TEXT("UnregisterDeviceNotification")); 
        }
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
		PostQuitMessage(0);
        break;

    default:
        // Send all other messages on to the default windows handler.
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}

bool GetCurrentLogonUserName(/*[out]*/ LPTSTR& lpszAccountName, /*[out]*/ LPTSTR& lpszReferencedDomainName)
{
    HANDLE           hToken = NULL;
    LPBYTE lpBytes = NULL;
    PTOKEN_USER  lpTokenUser = NULL; 
    DWORD            dwSize = 0;
    BOOL             bSuccess;

    // get token handle
    if ( !OpenProcessToken ( GetCurrentProcess ( ), TOKEN_QUERY, &hToken ) )
    {
        return false;
    }
   
    GetTokenInformation 
    (
      hToken, 
      TokenUser, 
      (LPVOID)NULL, 
      0, 
      &dwSize
    );

    lpBytes = new BYTE[dwSize];
    lpTokenUser = (PTOKEN_USER)lpBytes;

    bSuccess = GetTokenInformation 
    (
       hToken, 
       TokenUser, 
       (LPVOID)lpTokenUser, 
       dwSize, 
       &dwSize
   );

   CloseHandle (hToken);
   hToken = NULL;
   if (!bSuccess)
   {
      if (lpBytes)
      {
        delete [] lpBytes;
        lpBytes = NULL;
      }
      lpTokenUser = NULL;
      return false;
   }

   DWORD   dwSize01 = 0;
   DWORD   dwSize02 = 0;
   SID_NAME_USE sid_name_use;

   LookupAccountSid
   (
        NULL,
        (PSID)((lpTokenUser -> User).Sid),
        (LPTSTR)lpszAccountName,
        (LPDWORD)&dwSize01,
        (LPTSTR)lpszReferencedDomainName,
        (LPDWORD)&dwSize02,
        (PSID_NAME_USE)&sid_name_use
    );

    lpszAccountName    = new TCHAR[dwSize01];
    lpszReferencedDomainName = new TCHAR[dwSize02];

    bSuccess = LookupAccountSid
    (
        NULL,
        (PSID)((lpTokenUser -> User).Sid),
        (LPTSTR)lpszAccountName,
        (LPDWORD)&dwSize01,
        (LPTSTR)lpszReferencedDomainName,
        (LPDWORD)&dwSize02,
        (PSID_NAME_USE)&sid_name_use
    );

    delete [] lpBytes;
    lpBytes = NULL;
    lpTokenUser = NULL;

    return (bSuccess == TRUE);
}

bool isDebugEnabled()
{
	HKEY hKey = NULL;
	LONG lResult;
	DWORD dwType = REG_SZ;
	BYTE szBuff[255] = {0};
	LPWSTR dwValue = new WCHAR[5];
	//LPWSTR compVal = (LPWSTR)"1";
	DWORD dwSize = sizeof(szBuff);
	LPTSTR lpValueName = KEY_NAME_DEBUG;

	//debug("isDebugEnabled(): Trying to open registry key");
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GROUPS_KEY, 0, KEY_QUERY_VALUE, &hKey);

	if(lResult == ERROR_SUCCESS)
	{
		//debug("isDebugEnabled(): Successfully read registry key and now trying to read value");
		lResult = RegQueryValueEx(hKey, lpValueName, 0, &dwType, (LPBYTE)dwValue, &dwSize);
		RegCloseKey(hKey);
		if(lResult == ERROR_SUCCESS)
		{
			//debug("isDebugEnabled(): Successfully read key value for debug. Now trying to check if debug is enabled.");
			if(_wcsicmp(dwValue, (LPWSTR)"1") == 0)
				return true;
		}
	}
	
	return false;
}

void debug(char *text)
{
	static int i = 0;
	FILE * pFile= fopen ("yubi-debug.txt","a");
	fprintf (pFile, "[%d]\t %s\n",i++, text);
	fclose (pFile);
}

wchar_t * getDomainString(CString domainAndUser)
{
	CT2CW wstr(domainAndUser.GetBuffer());
	domainAndUser.ReleaseBuffer();

	int count = 0;
	size_t strlen = wcslen(wstr);
	bool found = false;
	while(count < strlen)
	{
		if(wstr[count] == '\\')
		{
			found = true;
			break;
		}
		count++;
	}
	if(found)
	{
		wchar_t * retbuffer = (wchar_t*)calloc(count + 1, sizeof(wchar_t));
		memcpy(retbuffer, wstr, count * sizeof(wchar_t));
		return retbuffer;
	}
	return NULL;
}

wchar_t* convertToWchar(TCHAR* buffer)
{
	wchar_t* ret;
	int length;
#ifdef UNICODE
	length = wcslen(buffer);
	ret = (wchar_t*)calloc((length + 1), sizeof(wchar_t));
	memcpy(ret, buffer, length * sizeof(wchar_t));
#else
	length = strlen(buffer);
	ret = (wchar_t*)calloc((length + 1), sizeof(wchar_t));
	mbstowcs(ret, buffer, length);
#endif
	return ret;
}


NET_API_STATUS getUserGroups(USER_GROUP_BUFFER* localBuffer, USER_GROUP_BUFFER* domainBuffer, CString username, CString domainAndUser)
{
	
	//DEBUG_LOG("Getting User Groups:\n");
	if((localBuffer == NULL) || (domainBuffer == NULL))
		return (DWORD)-1;

	wchar_t* wDomainAndUser = convertToWchar(domainAndUser.GetBuffer());
	//DEBUG_LOGW(L"\nDomain and User = %s\n", wDomainAndUser);
	
	DWORD totalEntries;
	//time_t stime = time(NULL);
	NET_API_STATUS status = NetUserGetLocalGroups(NULL, wDomainAndUser, 0, LG_INCLUDE_INDIRECT, (LPBYTE*)&(localBuffer->buffer), MAX_PREFERRED_LENGTH, &localBuffer->count, &totalEntries);
	//DEBUG_LOG("Query Took %I64d Seconds\n", time(NULL) - stime);
	domainAndUser.ReleaseBuffer();
	free(wDomainAndUser);

	//DEBUG_LOG("Results : Count = %d, Total Entries = %d [Status = %d]\n", localBuffer->count, totalEntries, status);

	wchar_t* wDomain = getDomainString(domainAndUser);
	wchar_t* wUsername = convertToWchar(username.GetBuffer());
	username.ReleaseBuffer();
	//DEBUG_LOGW(L"Domain / Username = %s / %s", wDomain, wUsername);
	//stime = time(NULL);
	NET_API_STATUS status2 = NetUserGetGroups(wDomain, wUsername, 0, (LPBYTE*)&(domainBuffer->buffer), MAX_PREFERRED_LENGTH, &domainBuffer->count, &totalEntries);
	//DEBUG_LOG("Query Took %I64d Seconds\n", time(NULL) - stime);
	//DEBUG_LOG("Results : Count = %d, Total Entries = %d [Status = %d]\n", domainBuffer->count, totalEntries, status2);
	free(wUsername);
	free(wDomain);

	if(status != 0)
		return status;
	return status2;
}

#define WND_CLASS_NAME TEXT("SampleAppWindowClass")

//
// InitWindowClass
//
BOOL InitWindowClass()
// Routine Description:
//      Simple wrapper to initialize and register a window class.

// Parameters:
//     None

// Return Value:
//     TRUE on success, FALSE on failure.

// Note: 
//     wndClass.lpfnWndProc and wndClass.lpszClassName are the
//     important unique values used with CreateWindowEx and the
//     Windows message pump.
{
    WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
    wndClass.lpfnWndProc = reinterpret_cast<WNDPROC>(WinProcCallback);
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground = CreateSolidBrush(RGB(192,192,192));
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.lpszClassName = WND_CLASS_NAME;
    wndClass.lpszMenuName = NULL;
    wndClass.hIconSm = wndClass.hIcon;


    if ( ! RegisterClassEx(&wndClass) )
    {
        ErrorHandler(TEXT("RegisterClassEx"));
        return FALSE;
    }
    return TRUE;
}

//
// main
//
//int _declspec(dllexport) _MainFunc()
//int __declspec(dllexport) _ttWinMain(
//#define DllExport __declspec(dllexport)
//DllExport int __stdcall _ttWinMain(
int _declspec(dllexport) _MainFunc()
//int __declspec(dllexport) _ttWinMain(
//#define DllExport __declspec(dllexport)
//DllExport int __stdcall _ttWinMain(
//int __stdcall _tWinMain(
//                      HINSTANCE hInstanceExe, 
//                      HINSTANCE, // should not reference this parameter
//                      PTSTR lpstrCmdLine, 
//                      int nCmdShow
//                      )
{
//
// To enable a console project to compile this code, set
// Project->Properties->Linker->System->Subsystem: Windows.
//

    //int nArgC = 0;
    //PWSTR* ppArgV = CommandLineToArgvW(lpstrCmdLine, &nArgC);
    //g_pszAppName = ppArgV[0];
	//g_pszAppName = "test";


	if(getYubiCount() == 0)
	{
		//there are no yubikeys - which may be required depending on the user - act as if a yubikey was removed
		yubikeyRemoved();
	}

    if ( ! InitWindowClass() )
    {
        // InitWindowClass displays any errors
        return -1;
    }

    // Main app window

    HWND hWnd = CreateWindowEx(
                    WS_EX_CLIENTEDGE | WS_EX_APPWINDOW,
                    WND_CLASS_NAME,
                    "BLELocker",
                    WS_OVERLAPPEDWINDOW, // style
                    CW_USEDEFAULT, 0, 
                    640, 480,
                    NULL, NULL, 
//                    hInstanceExe,
NULL,
                    NULL);
    
    if ( hWnd == NULL )
    {
        ErrorHandler(TEXT("CreateWindowEx: main appwindow hWnd"));
        return -1;
    }

    // Actually draw the window.

    //ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    // The message pump loops until the window is destroyed.
	WinProcCallback(hWnd, WM_DEVICECHANGE, DBT_DEVNODES_CHANGED, 0);
    MessagePump(hWnd);

    return 1;
}

//
// OutputMessage
//
void OutputMessage(
    HWND hOutWnd, 
    WPARAM wParam, 
    LPARAM lParam
)
// Routine Description:
//     Support routine.
//     Send text to the output window, scrolling if necessary.

// Parameters:
//     hOutWnd - Handle to the output window.
//     wParam  - Standard windows message code, not used.
//     lParam  - String message to send to the window.

// Return Value:
//     None

// Note:
//     This routine assumes the output window is an edit control
//     with vertical scrolling enabled.

//     This routine has no error checking.
{
    LRESULT   lResult;
    LONG      bufferLen;
    LONG      numLines;
    LONG      firstVis;
  
    // Make writable and turn off redraw.
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, FALSE, 0L);
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, FALSE, 0L);

    // Obtain current text length in the window.
    bufferLen = SendMessage (hOutWnd, WM_GETTEXTLENGTH, 0, 0L);
    numLines = SendMessage (hOutWnd, EM_GETLINECOUNT, 0, 0L);
    firstVis = SendMessage (hOutWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
    lResult = SendMessage (hOutWnd, EM_SETSEL, bufferLen, bufferLen);

    // Write the new text.
    lResult = SendMessage (hOutWnd, EM_REPLACESEL, 0, lParam);

    // See whether scrolling is necessary.
    if (numLines > (firstVis + 1))
    {
        int        lineLen = 0;
        int        lineCount = 0;
        int        charPos;

        // Find the last nonblank line.
        numLines--;
        while(!lineLen)
        {
            charPos = SendMessage(
                hOutWnd, EM_LINEINDEX, (WPARAM)numLines, 0L);
            lineLen = SendMessage(
                hOutWnd, EM_LINELENGTH, charPos, 0L);
            if(!lineLen)
                numLines--;
        }
        // Prevent negative value finding min.
        lineCount = numLines - firstVis;
        lineCount = (lineCount >= 0) ? lineCount : 0;
        
        // Scroll the window.
        lResult = SendMessage(
            hOutWnd, EM_LINESCROLL, 0, (LPARAM)lineCount);
    }

    // Done, make read-only and allow redraw.
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, TRUE, 0L);
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, TRUE, 0L);
}  

//
// ErrorHandler
//
void ErrorHandler(
   LPTSTR lpszFunction
) 
// Routine Description:
//     Support routine.
//     Retrieve the system error message for the last-error code
//     and pop a modal alert box with usable info.

// Parameters:
//     lpszFunction - String containing the function name where 
//     the error occurred plus any other relevant data you'd 
//     like to appear in the output. 

// Return Value:
//     None

// Note:
//     This routine is independent of the other windowing routines
//     in this application and can be used in a regular console
//     application without modification.
{ 

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)
                  + lstrlen((LPCTSTR)lpszFunction)+40)
                  * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, g_pszAppName, MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}