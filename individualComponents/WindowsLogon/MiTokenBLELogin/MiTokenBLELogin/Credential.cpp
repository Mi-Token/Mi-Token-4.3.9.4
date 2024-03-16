//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
//
#pragma warning ( disable : 4101 )
#pragma warning ( disable : 4189 )
#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )


#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif


#include "Credential.h"
#include "guid.h"
#include <assert.h>
#include <windows.h>
#include <winhttp.h>
#include <string.h>
#include <atlbase.h>
#include <malloc.h>
#include "oath.h"

#include <iostream>
#include <fstream>

#include <Sddl.h>

#pragma comment(lib, "netapi32.lib")
#include <lm.h>

#ifndef MAX_NAME
#define MAX_NAME 256
#endif
#pragma warning (push)
//#include <atlbase.h>
#include <tchar.h>
#include "freeradius-client.h"
#pragma warning (pop)
#include "OTPBypass.h"
#include "OTPCheck.h"


//#define POC TRUE

#define API 1

using namespace std;
// Credential ////////////////////////////////////////////////////////

Credential::Credential():
    _cRef(1),
    _pCredProvCredentialEvents(NULL)
{
	DEBUG_MESSAGE("Credential Constructor");
    DllAddRef();
	DEBUG_MESSAGE("Post DLL Add ref");

    ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));

	DEBUG_MESSAGE("Post Zero Memories");
	_lastDomain = NULL;
	_lastUsername = NULL;

	showingError = false;

	passMustChange = false;

	credType = KerbInteractiveLogon;

	HKEY subKey;
	this->_defaultDomain = NULL;
	this->_2FAEnabled = false;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon", 0, KEY_READ, &subKey);	
	if(result == ERROR_SUCCESS)
	{
		DWORD len = 0;
		
		int value;
		len = sizeof(value);
		result = RegQueryValueEx(subKey, "Enabled", NULL, NULL, (unsigned char*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			if(value == 1)
			{
				DEBUG_LOG("2FA Enabled");
				this->_2FAEnabled = true;
			}
		}
	}

	_requireStateChange = false;
	DEBUG_MESSAGE("Finished Contructor");
}

Credential::~Credential()
{
	DEBUG_MESSAGE("Credential Destructor");
	//Securely Zero any fields that may contain sensitive data (Currently the 3 password fields, and the OTP field)
    SecureZeroField(SFI_PASSWORD);
	SecureZeroField(SFI_OTP);
	SecureZeroField(SFI_NEWPASS_1);
	SecureZeroField(SFI_NEWPASS_2);

    for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
    {
        CoTaskMemFree(_rgFieldStrings[i]);
        CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
    }

    DllRelease();
}

void Credential::SecureZeroField(int FieldID)
{
	DEBUG_MESSAGE("SecureZeroField");
	if(_rgFieldStrings[FieldID])
	{
		size_t lenField;
		HRESULT hr = StringCchLengthW(_rgFieldStrings[FieldID], 1024, &(lenField));
		if(SUCCEEDED(hr))
		{
			SecureZeroMemory(_rgFieldStrings[FieldID], lenField * sizeof(*_rgFieldStrings[FieldID]));
		}
		else
		{
			//TODO: Finish TODOs // TODO: Determine how to handle count error here.
			//Would this ever be called? They would need to have more then 1K char in a string field :/
		}
	}
}

void Credential::setCredType(KERB_LOGON_SUBMIT_TYPE credType)
{
	DEBUG_MESSAGE("Set Cred Type");
	this->credType = credType;
}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_USERNAME field to pwzUsername.
HRESULT Credential::Initialize(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
    const FIELD_STATE_PAIR* rgfsp,
    PCWSTR pwzUsername,
	PCWSTR pwzDomain,
    PCWSTR pwzPassword
    )
{
	DEBUG_MESSAGE("Initialize");
    HRESULT hr = S_OK;

	UNREFERENCED_PARAMETER(pwzUsername);
	UNREFERENCED_PARAMETER(pwzDomain);

    // Copy the field descriptors for each field. This is useful if you want to vary the 
    // field descriptors based on what Usage scenario the credential was created for.
    for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
    {
        _rgFieldStatePairs[i] = rgfsp[i];
		if((i == SFI_USERNAME) && (_defaultDomain != NULL))
		{
			DEBUG_LOG("Setting CPFD to username\n");
			CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR tcpfd = { SFI_USERNAME, CPFT_EDIT_TEXT, L"Username" };
			hr = FieldDescriptorCopy(tcpfd, &_rgCredProvFieldDescriptors[i]);
		}
		else
		{
			hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
		}
    }
     		
	//Initialise all the strings as blank. (All string fields need to be initialised otherwise the credential will crash.
	for(DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		hr = SHStrDupW(L"", &_rgFieldStrings[i]);
	}

	usingDynamicPasswords = false;


    // Initialize the String values of all the fields that have default string values.
	if(SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Mi-Token", &_rgFieldStrings[SFI_HEADING]);
	}
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(pwzPassword ? pwzPassword : L"", &_rgFieldStrings[SFI_PASSWORD]);
		if(pwzPassword)
		{
			DEBUG_MESSAGE("pwzPassword exists");
		}
		else
		{
			DEBUG_MESSAGE("pwzPassword Doesn't Exist");
		}
    }
	if (SUCCEEDED(hr))
	{
		DEBUG_MESSAGE("Cred : Init : Writing DOM/User");
#ifdef ENABLE_LOGGING
		char buffer[100];
		if(debugLog)
		{
			sprintf_s(buffer, 100, "DOM = %s, User = %s", pwzDomain ? "[Data]" : "NULL", pwzUsername ? "[DATA]" : "NULL");
		}
#endif
		DEBUG_MESSAGE(buffer);
		WCHAR domainUser[100];
		swprintf_s(&domainUser[0], 100, L"%s%s%s", pwzDomain ? pwzDomain : L"", pwzDomain ? L"\\" : L"", pwzUsername ? pwzUsername : L"");
		DEBUG_WMESSAGE(domainUser);
		hr = SHStrDupW(&domainUser[0], &_rgFieldStrings[SFI_USERNAME]);
	}
    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
    }

	this->isRDPSession = false;

	if(SUCCEEDED(hr))
	{
		//pwzDomain is NOT required for an RDP session (if the remote PC is not on a domain)
		if((pwzUsername) && (pwzPassword))
		{
			this->isRDPSession = true;
			DEBUG_MESSAGE("RDP Session activated");
			//change the heading
			WCHAR newHeading[100];
			swprintf_s(&(newHeading[0]), 100, L"Mi-Token : %s", pwzUsername);
			hr = SHStrDupW(&(newHeading[0]), &_rgFieldStrings[SFI_HEADING]);
		}
	}

    return S_OK;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT Credential::Advise(
    ICredentialProviderCredentialEvents* pcpce
    )
{
	DEBUG_MESSAGE("Advise");
    if (_pCredProvCredentialEvents != NULL)
    {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = pcpce;
    _pCredProvCredentialEvents->AddRef();

	if(_requireStateChange)
	{
		SetFieldStates();
	}
    return S_OK;
}

// LogonUI calls this to tell us to release the callback.
HRESULT Credential::UnAdvise()
{
	DEBUG_MESSAGE("UnAdvise");
    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = NULL;
    return S_OK;
}



void Credential::SetFieldStates()
{
	if(_pCredProvCredentialEvents == NULL)
	{
		_requireStateChange = true;
		return; //cannot change the state if we don't have the events handle
	}
	_requireStateChange = false;

	DEBUG_LOG("SetFieldStates\n");

	switch(CredentialState)
	{
	case CredState_Normal:
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
		//_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_OTP);			

		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_PASSWORD, CPFIS_FOCUSED);
		break;
	case CredState_RDP:
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_HIDDEN);
		//_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_OTP);			

		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_OTP, CPFIS_FOCUSED);
		break;
	case CredState_StatusMessage:
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_HIDDEN);
		//_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_SUBMIT_RESULT);

		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_SUBMIT_BUTTON, CPFIS_FOCUSED);
		break;
	case CredState_ChangePass:
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
		//_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_OTP);			

		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_PASSWORD, CPFIS_FOCUSED);
		break;
	case CredState_DynamicPass:
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
		//_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_OTP);			

		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_PASSWORD, CPFIS_FOCUSED);
		break;
	}
}

// LogonUI calls this function when our tile is selected (zoomed).
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions.  But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT Credential::SetSelected(BOOL* pbAutoLogon)  
{
	DEBUG_MESSAGE("Set Selected");
	
	*pbAutoLogon = FALSE;  
	
	//State update - always set to either RDP or Normal mode
	if(isRDPSession)
	{
		CredentialState = CredState_RDP;
	}
	else
	{
		CredentialState = CredState_Normal;
	}

	if(passMustChange)
	{
		CredentialState = CredState_ChangePass;
	}

	showingError = false;
	

	//Update for Dynamic Passwords : If we are using them, hide the password field and change the Mi-Token OTP Field to say "Mi-Token OTP"
	HKEY configKey;
	DWORD result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token\\Logon\\Config", 0, KEY_READ, &configKey);
	if(result == ERROR_SUCCESS)
	{
		int value;
		DWORD len = sizeof(value);
		result = RegQueryValueEx(configKey, "DynamicPassword", NULL, NULL, (unsigned char*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			if(value == 1)
			{
				DEBUG_LOG("Dynamic Passwords Enabled - Setting usingDynamicPasswords to True\n");
				
				usingDynamicPasswords = true;
			}
		}

		len = sizeof(value);
		result = RegQueryValueEx(configKey, "EnableOTPCaching", NULL, NULL, (unsigned char*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			if(value == 1)
			{
				DEBUG_LOG("OTP Cache mode is enabled.\n");
				if(usingDynamicPasswords)
				{
					DEBUG_LOG("ERROR!!! Both OTP Cache Mode and Dynamic Password Mode are enabled. These modes are mutually exclusive. Turning off Dynamic Password Mode - Leaving OTP Cache Mode active\n");
					usingDynamicPasswords = false;
				}
			}
		}

		if(usingDynamicPasswords)
		{
			DEBUG_LOG("Dynamic Passwords are enabled - Setting CredMode to DP\n");
			CredentialState = CredState_DynamicPass;
			//We need to clear the password field if this was done via an RDP login, as it will contain the dynamic password, and they cannot be used to login.
			SecureZeroField(SFI_PASSWORD);
			if(SUCCEEDED(SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD])))
			{
				_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
			}
			
		}

		RegCloseKey(configKey);
	}

	SetFieldStates();
	
    return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT Credential::SetDeselected()
{
	DEBUG_MESSAGE("Set Deselected");
    HRESULT hr = S_OK;
	SecureZeroField(SFI_PASSWORD);
	SecureZeroField(SFI_OTP);
	SecureZeroField(SFI_NEWPASS_1);
	SecureZeroField(SFI_NEWPASS_2);
	SecureZeroField(SFI_USERNAME);
	this->isRDPSession = false; //no longer an RDP Session connection if the user deselects this tile
	DEBUG_MESSAGE("No longer RPD Session");
	CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
	CoTaskMemFree(_rgFieldStrings[SFI_OTP]);
	CoTaskMemFree(_rgFieldStrings[SFI_NEWPASS_1]);
	CoTaskMemFree(_rgFieldStrings[SFI_NEWPASS_2]);

	SHStrDupW(L"Mi-Token", &_rgFieldStrings[SFI_HEADING]);
	_pCredProvCredentialEvents->SetFieldString(this, SFI_USERNAME, _rgFieldStrings[SFI_USERNAME]);
	_pCredProvCredentialEvents->SetFieldString(this, SFI_HEADING, _rgFieldStrings[SFI_HEADING]);

	if(SUCCEEDED(SHStrDupW(L"", &_rgFieldStrings[SFI_PASSWORD])) && _pCredProvCredentialEvents)
		_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
	if(SUCCEEDED(SHStrDupW(L"", &_rgFieldStrings[SFI_OTP])) && _pCredProvCredentialEvents)
		_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, _rgFieldStrings[SFI_OTP]);
	if(SUCCEEDED(SHStrDupW(L"", &_rgFieldStrings[SFI_NEWPASS_1])) && _pCredProvCredentialEvents)
		_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_1, _rgFieldStrings[SFI_NEWPASS_1]);
	if(SUCCEEDED(SHStrDupW(L"", &_rgFieldStrings[SFI_NEWPASS_2])) && _pCredProvCredentialEvents)
		_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_2, _rgFieldStrings[SFI_NEWPASS_2]);

	passMustChange = false;

    return hr;
}

// Gets info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT Credential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
    )
{
	DEBUG_MESSAGE("GetFieldState");
    HRESULT hr;

    // Validate paramters.
    if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)) && pcpfs && pcpfis)
    {
        *pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
        *pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;

        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID.
HRESULT Credential::GetStringValue(
    DWORD dwFieldID, 
    PWSTR* ppwsz
    )
{
	DEBUG_MESSAGE("GetStringValue");
    HRESULT hr;

    // Check to make sure dwFieldID is a legitimate index.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && ppwsz) 
    {
        // Make a copy of the string and return that. The caller
        // is responsible for freeing it.
        hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
		DEBUG_MESSAGEd(dwFieldID);
		DEBUG_WMESSAGE(*ppwsz);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Gets the image to show in the user tile.
HRESULT Credential::GetBitmapValue(
    DWORD dwFieldID, 
    HBITMAP* phbmp
    )
{
	DEBUG_MESSAGE("GetBitmapValue");
    HRESULT hr;
    if ((SFI_TILEIMAGE == dwFieldID) && phbmp)
    {
        HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_BITMAP2));
        if (hbmp != NULL)
        {
            hr = S_OK;
            *phbmp = hbmp;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be 
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT Credential::GetSubmitButtonValue(
    DWORD dwFieldID,
    DWORD* pdwAdjacentTo
    )
{
	DEBUG_MESSAGE("GetSubmitButtonValue");
    HRESULT hr;

    // Validate parameters.
    if ((SFI_SUBMIT_BUTTON == dwFieldID) && pdwAdjacentTo)
    {
        // pdwAdjacentTo is a pointer to the fieldID you want the submit button to appear next to.
        *pdwAdjacentTo = SFI_OTP;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field.
HRESULT Credential::SetStringValue(
    DWORD dwFieldID, 
    PCWSTR pwz      
    )
{
	DEBUG_VERBOSE("Set String Value : [%d] ", dwFieldID);
	DEBUG_SENSITIVEW(true, L"%s\n", pwz);

	HRESULT hr;
    // Validate parameters.
    if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
       (CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft || 
        CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft)) 
    {
		if((dwFieldID == SFI_PASSWORD) &&
			(isRDPSession) &&
			(wcscmp(pwz, _rgFieldStrings[dwFieldID]) != 0) &&
			(!usingDynamicPasswords))
		{
			DEBUG_MESSAGE("Trying to change password string - aborting change");
			_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[dwFieldID]);
			hr = S_OK;
		}
		else
		{
			PWSTR* ppwszStored = &_rgFieldStrings[dwFieldID];
			CoTaskMemFree(*ppwszStored);
			hr = SHStrDupW(pwz, ppwszStored);
			//DEBUG_WMESSAGE(pwz);
		}
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

//------------- 
// The following methods are for logonUI to get the values of various UI elements and then communicate
// to the credential about what the user did in that field.  However, these methods are not implemented
// because our tile doesn't contain these types of UI elements
HRESULT Credential::GetCheckboxValue(
    DWORD dwFieldID, 
    BOOL* pbChecked,
    PWSTR* ppwszLabel
    )
{
	DEBUG_MESSAGE("GetCheckboxValue");
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(pbChecked);
    UNREFERENCED_PARAMETER(ppwszLabel);

    return E_NOTIMPL;
}

HRESULT Credential::GetComboBoxValueCount(
    DWORD dwFieldID, 
    DWORD* pcItems, 
    DWORD* pdwSelectedItem
    )
{
	DEBUG_MESSAGE("GetComboBoxValueCount");
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(pcItems);
    UNREFERENCED_PARAMETER(pdwSelectedItem);
	//pdwSelectedItem = 
    return E_NOTIMPL;
}

HRESULT Credential::GetComboBoxValueAt(
    DWORD dwFieldID, 
    DWORD dwItem,
    PWSTR* ppwszItem
    )
{
	DEBUG_MESSAGE("GetComboBoxValueAt");
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(dwItem);
    UNREFERENCED_PARAMETER(ppwszItem);
    return E_NOTIMPL;
}

HRESULT Credential::SetCheckboxValue(
    DWORD dwFieldID, 
    BOOL bChecked
    )
{
	DEBUG_MESSAGE("Set Checkbox Value");
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(bChecked);

    return E_NOTIMPL;
}

HRESULT Credential::SetComboBoxSelectedValue(
    DWORD dwFieldId,
    DWORD dwSelectedItem
    )
{
	DEBUG_MESSAGE("SetComboBoxSelectedValue");
    UNREFERENCED_PARAMETER(dwFieldId);
    UNREFERENCED_PARAMETER(dwSelectedItem);
    return E_NOTIMPL;
}

HRESULT Credential::CommandLinkClicked(DWORD dwFieldID)
{
	DEBUG_MESSAGE("CommandLinkClicked");
    UNREFERENCED_PARAMETER(dwFieldID);
    return E_NOTIMPL;
}

HRESULT Credential::SetPassword(KERB_INTERACTIVE_UNLOCK_LOGON &kil)
{
	HRESULT hr;

	if(_dynamicPassword)
	{
		DEBUG_VERBOSE("Adding a Dynamic Password\n");
		int slen = strlen((char*)_dynamicPassword);
		wchar_t * wDynamicPassword = (wchar_t*)calloc(slen + 1, sizeof(wchar_t));
		mbstowcs(wDynamicPassword, (char*)_dynamicPassword, slen);
		hr = UnicodeStringInitWithString(wDynamicPassword, &kil.Logon.Password);

		//BUG FIX : Make sure you clear dynamic password for cases where the user logged on with an account that wasn't allowed to login.
		if(_dynamicPassword)
		{
			free(_dynamicPassword);
			_dynamicPassword = NULL;
		}
	}
	else
	{
		//if we are usingDynamicPasswords and the DynamicPassword hasn't been sent, the user is using an account without a token - so they use the OTP Field as their password.
		//Note : Due to how the CP now works both the OTP and PASSWORD field are shown at all times, therefore we will always grab from the password field. - otherwise they may get confused.
		if(usingDynamicPasswords)
		{
			//hr = UnicodeStringInitWithString(_rgFieldStrings[SFI_OTP], &kil.Logon.Password);
			hr = UnicodeStringInitWithString(_rgFieldStrings[SFI_PASSWORD], &kil.Logon.Password);
		}
		else
		{
			hr = UnicodeStringInitWithString(_rgFieldStrings[SFI_PASSWORD], &kil.Logon.Password);
		}
	}

	return hr;
}

PWSTR Credential::GetOTP()
{
	DEBUG_LOG("Credential::GetOTP\r\n");
	PWSTR retValue;
	if(!usingDynamicPasswords)
	{
		DEBUG_LOG("Normal Password mode\r\n");
		int passLength = wcslen(_rgFieldStrings[SFI_OTP]);
		retValue = (PWSTR)calloc(passLength + 1, sizeof(wchar_t));
		memcpy(retValue, _rgFieldStrings[SFI_OTP], passLength * sizeof(wchar_t));
	}
	else
	{
		DEBUG_LOG("Dynamic Password mode\r\n");
		int otpLength, passLength;
		otpLength = wcslen(_rgFieldStrings[SFI_OTP]);
		passLength = wcslen(_rgFieldStrings[SFI_PASSWORD]);
		retValue = (PWSTR)calloc(otpLength + passLength + 1, sizeof(wchar_t));
		memcpy(retValue, _rgFieldStrings[SFI_PASSWORD], passLength * sizeof(wchar_t));
		memcpy(retValue + passLength, _rgFieldStrings[SFI_OTP], otpLength * sizeof(wchar_t));
	}

	DEBUG_VERBOSE("Credential::GetOTP Returned with : ");
	DEBUG_SENSITIVEW(true, L"%s\r\n", retValue);
	return retValue;
}


PWSTR Credential::GetBypass()
{
	//almost identical to GetOTP, however the Bypasscode is only ever written into the OTP Field
	DEBUG_LOG("Credential::GetBypass\r\n");
	PWSTR retValue;

	int passLength = wcslen(_rgFieldStrings[SFI_OTP]);
	retValue = (PWSTR)calloc(passLength + 1, sizeof(wchar_t));
	memcpy(retValue, _rgFieldStrings[SFI_OTP], passLength * sizeof(wchar_t));

	DEBUG_VERBOSE("Credential::GetBypass Returned with : ");
	DEBUG_SENSITIVEW(true, L"%s\r\n", retValue);
	return retValue;
}


void Credential::SplitDomainAndUsername(int splitLocation)
{
	_lastDomain = (WCHAR*)::CoTaskMemAlloc(sizeof(WCHAR) * (splitLocation+1));
	//Username is actually domain\username
	for(int j = 0 ; j < splitLocation ; ++j)
	{
		_lastDomain[j] = _rgFieldStrings[SFI_USERNAME][j];
	}

	_lastDomain[splitLocation] = L'\0';

	int j = splitLocation+1;
	while(_rgFieldStrings[SFI_USERNAME][j] != L'\0')
	{
		j++;
	}
	
	_lastUsername = &(_rgFieldStrings[SFI_USERNAME][splitLocation+1]);	

	DEBUG_LOGW(L"Split Domain and Username:\n\tDomain : %s\n\tUsername : %s\n", _lastDomain, _lastUsername);
}

HRESULT Credential::SetDomainToLocalDomain()
{
	HRESULT hr = S_OK;
	DWORD cch = MAX_COMPUTERNAME_LENGTH  + 1;
	_lastDomain = (WCHAR*)::CoTaskMemAlloc(sizeof(WCHAR) * cch);
	if(!GetComputerNameW(_lastDomain, &cch))
	{
		::CoTaskMemFree(_lastDomain);
		DWORD dwErr = GetLastError();
		hr = HRESULT_FROM_WIN32(dwErr);
	}
	return hr;
}

HRESULT Credential::GetDomainAndUsername()
{
	HRESULT hr = S_OK;
	int i = 0;
	while(_rgFieldStrings[SFI_USERNAME][i] != L'\0' && _rgFieldStrings[SFI_USERNAME][i] != L'\\')
	{
		i++;
	}

	if(_rgFieldStrings[SFI_USERNAME][i] == '\\')
	{
		SplitDomainAndUsername(i);
	}
	else
	{
		if(_defaultDomain == NULL)
		{
			hr = SetDomainToLocalDomain();
			DEBUG_LOGW(L"No Default Domain found - using Local Domain [%s]", _lastDomain);
		}
		else
		{
			//copy _defaultDomain into _lastDomain
			_lastDomain = (WCHAR*)::CoTaskMemAlloc(sizeof(WCHAR) * (1 + wcslen(_defaultDomain)));
			memcpy(_lastDomain, _defaultDomain, sizeof(WCHAR) * (1 + wcslen(_defaultDomain)));
			DEBUG_LOGW(L"Using Default Domain [%s]", _defaultDomain);
		}
		_lastUsername = _rgFieldStrings[SFI_USERNAME];
	}
	return hr;
}

HRESULT Credential::CreateKerbLogonPacket(KERB_INTERACTIVE_UNLOCK_LOGON& kil)
{
	
	//KERB_INTERACTIVE_LOGON kil;
	ZeroMemory(&(kil.Logon), sizeof(KERB_INTERACTIVE_LOGON));
	ZeroMemory(&(kil.LogonId), sizeof(LUID));

	// Initialize the UNICODE_STRINGS to share our username and password strings.
	//hr = UnicodeStringInitWithString(L"CHRISDR2", &kil.LogonDomainName);
	DEBUG_VERBOSEW(L"Adding Domain Name to Ticket : %s\n", _lastDomain);
	HRESULT hr = UnicodeStringInitWithString(_lastDomain, &kil.Logon.LogonDomainName);


	if (SUCCEEDED(hr))
	{
		DEBUG_VERBOSEW(L"Adding Username To Ticket : %s\n", _lastUsername);
		hr = UnicodeStringInitWithString(_lastUsername, &kil.Logon.UserName);

		if (SUCCEEDED(hr))
		{
			DEBUG_VERBOSEW(L"Adding Password to Ticket :");
			DEBUG_SENSITIVEW(true, L"%s\n", _rgFieldStrings[SFI_PASSWORD]);


			hr = SetPassword(kil);

			if (SUCCEEDED(hr))
			{
				// Allocate copies of, and package, the strings in a binary blob.
				kil.Logon.MessageType = credType;
				return hr;
			}
				
		}
	}
}

HRESULT Credential::SetAuthPackage(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
	ULONG ulAuthPackage;
	DEBUG_VERBOSE("Pre RetrieveNegotiateAuthPackage\n");
	HRESULT hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
	if(SUCCEEDED(hr))
	{
		pcpcs->ulAuthenticationPackage = ulAuthPackage;
		pcpcs->clsidCredentialProvider = CLSID_Provider;
		*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
		DEBUG_VERBOSE("SetAuthPackage completed\n");
	}
	return hr;
}


HRESULT Credential::SerializeDataForMiTokenAP(const KERB_INTERACTIVE_UNLOCK_LOGON kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR otp, int MiTokenAPID)
{
	DEBUG_MESSAGE("SerializeDataFromMiTokenAP\n");
	HRESULT hr = MiTokenInteractiveLogonPack(kil, &pcpcs->rgbSerialization, &pcpcs->cbSerialization, otp, MiTokenAPID);
	
	DEBUG_MESSAGE("PMTP\n");
	if(SUCCEEDED(hr))
	{
		pcpcs->ulAuthenticationPackage = MiTokenAPID;
		pcpcs->clsidCredentialProvider = CLSID_Provider;
		*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;

		DEBUG_VERBOSE("Verberos Mi-Token AP Ticket Info : %d Bytes\n", pcpcs->cbSerialization);
		DEBUG_SENSITIVEBYTES(true, pcpcs->rgbSerialization, pcpcs->cbSerialization);

	}

	return hr;
}

HRESULT Credential::SerializeData(const KERB_INTERACTIVE_UNLOCK_LOGON kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR otp)
{

	DEBUG_MESSAGE("SerializeData\n");
	//Update to support Mi-Token AP based authentication.
	ULONG MiTokenAPID;
	if(SUCCEEDED(RetrieveMiTokenAPPackage(&MiTokenAPID)))
	{
		return SerializeDataForMiTokenAP(kil, pcpgsr, pcpcs, otp, MiTokenAPID);
	}

	DEBUG_MESSAGE("SerializeData P2\n");

	HRESULT hr = KerbInteractiveLogonPack(kil, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
	DEBUG_MESSAGE("PKP\n");
	if (SUCCEEDED(hr))
	{
		DEBUG_VERBOSE("Verberos Kerberos Ticket Info : %d Bytes\n", pcpcs->cbSerialization);
		DEBUG_SENSITIVEBYTES(true, pcpcs->rgbSerialization, pcpcs->cbSerialization);
	
		hr = SetAuthPackage(pcpgsr, pcpcs);
	}

	return hr;
}

HRESULT Credential::CreateChangePasswordPacket(KERB_CHANGEPASSWORD_REQUEST& kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
	HRESULT hr;
	ZeroMemory(&(kil), sizeof(KERB_CHANGEPASSWORD_REQUEST));

	DEBUG_MESSAGE("About to package kerberos ticket");
	DEBUG_VERBOSEW(L"Adding Domain Name : %s\n", _lastDomain);
	hr = UnicodeStringInitWithString(_lastDomain, &kil.DomainName);

	if(SUCCEEDED(hr))
	{
		DEBUG_VERBOSEW(L"Adding User Name : %s\n", _lastUsername);
		hr = UnicodeStringInitWithString(_lastUsername, &kil.AccountName);

		if(SUCCEEDED(hr))
		{
			DEBUG_VERBOSEW(L"Adding Old Password");
			DEBUG_SENSITIVEW(true, L"%s\n", _rgFieldStrings[SFI_PASSWORD]);

			hr = UnicodeStringInitWithString(_rgFieldStrings[SFI_PASSWORD], &kil.OldPassword);

			if(SUCCEEDED(hr))
			{
				DEBUG_VERBOSEW(L"Adding New Password");
				DEBUG_SENSITIVEW(true, L"%s\n", _rgFieldStrings[SFI_NEWPASS_1]);

				hr = UnicodeStringInitWithString(_rgFieldStrings[SFI_NEWPASS_1], &kil.NewPassword);
				if(SUCCEEDED(hr))
				{
					DEBUG_VERBOSE("Setting Extra Kerb Info\n");
					kil.MessageType = KerbChangePasswordMessage;
					hr = KerbInteractiveChangePassPack(kil, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
					DEBUG_MESSAGE("Kerberos Ticket Packed\n");

					DEBUG_LOG("Ticket Info : Username %d bytes. Old Pass %d bytes. New Pass %d bytes.\n", kil.AccountName.Length, kil.OldPassword.Length, kil.NewPassword.Length);

					DEBUG_VERBOSE("Verbose Kerberos Ticket info : %d Bytes\n", pcpcs->cbSerialization);
					DEBUG_SENSITIVEBYTES(true, pcpcs->rgbSerialization, pcpcs->cbSerialization);

					if(SUCCEEDED(hr))
					{
						hr = SetAuthPackage(pcpgsr, pcpcs);
					}
				}
			}
		}
	}
	return hr;
}


void updateDefaultUsername(wchar_t* username)
{	
	DEBUG_VERBOSE("Updating Default Username\n");
	HKEY regKey;
	LONG keyResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ | KEY_WRITE, &regKey);

	if(keyResult == ERROR_SUCCESS)
	{
		int userlen = wcslen(username);
		char* cstr = (char*)calloc(userlen + 1, sizeof(char));
		wcstombs(cstr, username, userlen);
		DWORD result = RegSetValueEx(regKey, "LastUserID", NULL, REG_SZ, (BYTE*)cstr, userlen + 1);
		DEBUG_VERBOSE("Attempted to set reg value, result = %d\n", result);
		RegCloseKey(regKey);
	}
	else
	{
		DEBUG_VERBOSE("Failed to update username : Could not open regkey\n");
	}
}

bool checkBLE()
{
	srand(time(NULL));

	if(rand() % 2 == 0)
	{
		return false;
	}
	else
		return true;
}

//------ end of methods for controls we don't have in our tile ----//

// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
HRESULT Credential::GetSerialization(
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
    PWSTR* ppwszOptionalStatusText, 
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
    )
{
	DEBUG_MESSAGE("GetSerialization");
    UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);


	HRESULT hr = S_OK;

	WCHAR message[4096];

	DEBUG_MESSAGE("WRITE TO LOG");
	DEBUG_WMESSAGE(_rgFieldStrings[SFI_OTP]);

	if(!showingError)
	{
		
		if(_lastDomain)
		{
			::CoTaskMemFree(_lastDomain);
			_lastDomain = NULL;
		}
		if(_lastUsername)
		{
			//Do not free _lastUsername as it will point to an index of _rgFieldStrings[SFI_USERNAME] - NOT to alloc'd memory
			_lastUsername = NULL;
		}

		hr = GetDomainAndUsername();
		if(FAILED(hr))
		{
			return hr;
		}
		DEBUG_MESSAGE("PDLU\n");

		
		PWSTR otpValue = GetOTP();
		PWSTR bypassValue = GetBypass();
		
		//allowed if 2FAEnabled is false or the OTP is correct
		ULONG MiTokenAPID;
		bool CheckOTPInAP = false;
		DEBUG_VERBOSE("Checking for Mi-Token AP\n");
		if(SUCCEEDED(RetrieveMiTokenAPPackage(&MiTokenAPID)))
		{
			DEBUG_VERBOSE("MiToken AP Detected : Using AP for OTP Verification\n");
			CheckOTPInAP = true;
		}
		/*
			There are 2 cases where we don't need to check the OTP
				1) Mi-Token 2FA is not enabled - this should only be true during the installation of the CP
				2) The Mi-Token AP is installed - in this case the AP will take care of validating the OTP
		*/

		wchar_t getUserString[UNLEN + 1] = {0};
		DWORD getUserStringLen = UNLEN + 1;
		if(GetUserNameW(getUserString, &getUserStringLen))
		{
			DEBUG_VERBOSEW(L"Get Username Returned: %s\n", getUserString);
		}
		else
		{
			DEBUG_VERBOSE("Get Username Failed\n");
		}

		if( (!this->_2FAEnabled) || (CheckOTPInAP) || (checkBLE()))
		{
			
			free(bypassValue);
			DEBUG_MESSAGE("PCOTP\n");
			if(CredentialState != CredState_ChangePass)
			{
				//Update the stored username value
				updateDefaultUsername(_rgFieldStrings[SFI_USERNAME]);

				DEBUG_VERBOSE("%s", "Creating Kerb Unlock Logon Pack\n");
				KERB_INTERACTIVE_UNLOCK_LOGON kil;
				hr = CreateKerbLogonPacket(kil);
				if(SUCCEEDED(hr))
				{
					hr = SerializeData(kil, pcpgsr, pcpcs, otpValue);
					free(otpValue);
				}
			}
			else /*CredState_ChangePass state*/
			{
				free(otpValue);
				DEBUG_MESSAGE("In Password Must Change");
				DEBUG_VERBOSE("Comparing Newpass1 vs Newpass2\n");
				if(::wcscmp(_rgFieldStrings[SFI_NEWPASS_1], _rgFieldStrings[SFI_NEWPASS_2]) == 0) /*New Passwords are the same*/
				{
					DEBUG_VERBOSE("Password Compare Check Passed\nCreating Kerberos Change Password Ticket\n");
					KERB_CHANGEPASSWORD_REQUEST kil;
					
					hr = CreateChangePasswordPacket(kil, pcpgsr, pcpcs);
				}
				else /*New Passwords are different*/
				{
					DEBUG_VERBOSE("New Passwords did not Match : ");
					DEBUG_SENSITIVEW(true, L"5s vs %s\n", _rgFieldStrings[SFI_NEWPASS_1], _rgFieldStrings[SFI_NEWPASS_2]);
					CredentialState = CredState_StatusMessage;
					
					_pCredProvCredentialEvents->SetFieldState(this, SFI_SUBMIT_RESULT, CPFS_DISPLAY_IN_SELECTED_TILE);
					_pCredProvCredentialEvents->SetFieldString(this, SFI_SUBMIT_RESULT, L"New Passwords did not match");					
					_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_1, L"");
					_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_2, L"");
					_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, L"");
					SetFieldStates();
				}
			}
		}
		else /*INVALID OTP*/
		{
			DEBUG_VERBOSE("Bluetooth Device Not Detected\n");
			free(otpValue);
			free(bypassValue);
			CredentialState = CredState_StatusMessage;
			
			if(!this->isRDPSession)
			{
				//only clear the password when we are not in an RPD session.
				_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
			}

			_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, L"");
			_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_1, L"");
			_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_2, L"");
			

			_pCredProvCredentialEvents->SetFieldString(this, SFI_SUBMIT_RESULT, L"Bluetooth Device Not Detected");
			showingError = true;
			SetFieldStates();
			/*
			_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_HIDDEN);
			_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_HIDDEN);
			_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_HIDDEN);
			
			_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, L"");

			_pCredProvCredentialEvents->SetFieldString(this, SFI_SUBMIT_RESULT, message);
			_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_SUBMIT_RESULT);
			_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_SUBMIT_BUTTON, CPFIS_FOCUSED);
			
			

			if(passMustChange)
			{
				_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_HIDDEN);
				_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_HIDDEN);
				_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_1, L"");
				_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_2, L"");
			}
			*/
			return 0;
		}
	}
	else /* Showing error */
	{
		DEBUG_VERBOSE("Showing Error\n");
		if(this->passMustChange)
		{
			CredentialState = CredState_ChangePass;
		}
		else if(this->usingDynamicPasswords)
		{
			CredentialState = CredState_DynamicPass;
		}
		else if(this->isRDPSession)
		{
			CredentialState = CredState_RDP;
		}
		else
		{
			CredentialState = CredState_Normal;
		}
		/*
		if(!this->isRDPSession)
		{
			//only show username/password when not in an RPD Session
			//_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
			//_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
			CredentialState = CredState_Normal;
		}
		else
		{
			CredentialState = CredState_RDP;
		}
		/*
		_pCredProvCredentialEvents->SetFieldState(this, SFI_OTP, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldInteractiveState(this, SFI_USERNAME, CPFIS_FOCUSED);

		_pCredProvCredentialEvents->SetFieldString(this, SFI_SUBMIT_RESULT,L"");
		_pCredProvCredentialEvents->SetFieldSubmitButton(this, SFI_SUBMIT_BUTTON, SFI_OTP);

		*//*
		if(usingDynamicPasswords)
		{
			//_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_HIDDEN);	
		}

		if(passMustChange)
		{
			_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_1, CPFS_DISPLAY_IN_SELECTED_TILE);
			_pCredProvCredentialEvents->SetFieldState(this, SFI_NEWPASS_2, CPFS_DISPLAY_IN_SELECTED_TILE);
		}
		*/
		SetFieldStates();
		showingError = false;
		hr = S_OK;
	}
	return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
    NTSTATUS ntsStatus;
    NTSTATUS ntsSubstatus;
    PWSTR     pwzMessage;
    CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

//static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
//{
//    { STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
//    { STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
//};


// Report result is used to check if the users logon was successfull, and if not, for what reason did it fail
HRESULT Credential::ReportResult(
    NTSTATUS ntsStatus, 
    NTSTATUS ntsSubstatus,
    PWSTR* ppwszOptionalStatusText, 
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
    )
{
	DEBUG_MESSAGE("Report Result");

	if(debugLog)
	{
		char buffer[150];
		sprintf_s(buffer, "Status = %08lX, SubStatus = %08lX\n", ntsStatus, ntsSubstatus);
		DEBUG_MESSAGE(buffer);
	}
	
    *ppwszOptionalStatusText = NULL;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

 
	DEBUG_VERBOSE("Last domain = 0x%08lX, Last username = 0x%08lX", _lastDomain, _lastUsername);
	if(passMustChange && ntsStatus == STATUS_NO_MEMORY && ntsSubstatus == STATUS_UNSUCCESSFUL)
	{
		DEBUG_VERBOSE("Password change failed, trying NetUserChangePass\n");
		if(NetUserChangePassword(_lastDomain, _lastUsername, _rgFieldStrings[SFI_PASSWORD], _rgFieldStrings[SFI_NEWPASS_1]) == NERR_Success)
		{
			DEBUG_VERBOSE("Pass change via NetUserChangePassword worked\n");
			*ppwszOptionalStatusText = L"Password was changed successfully.";
		}
	}
	if(passMustChange && ntsStatus == STATUS_SUCCESS && ntsSubstatus == STATUS_SUCCESS)
	{
		DEBUG_VERBOSE("Password changed successfully\n");
		//Password needed to be changed, and has been changed successfully
		passMustChange = false;
		_pCredProvCredentialEvents->SetFieldState(this,SFI_NEWPASS_1, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this,SFI_NEWPASS_2, CPFS_HIDDEN);
		_pCredProvCredentialEvents->SetFieldState(this,SFI_SUBMIT_RESULT, CPFS_HIDDEN);
		if(SUCCEEDED(SHStrDupW(L"Password Change Successful. Please re-login with your new password.",ppwszOptionalStatusText)))
		{
			*pcpsiOptionalStatusIcon = CPSI_SUCCESS;
		}
	}
	else if(passMustChange)
	{
		DEBUG_VERBOSE("Password did not change successfully\n");
		//Password needed to be changed, and has not been changed successfully
		_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_1, L"");
		_pCredProvCredentialEvents->SetFieldString(this, SFI_NEWPASS_2, L"");
	}
	else if(ntsStatus == STATUS_PASSWORD_MUST_CHANGE && ntsSubstatus == STATUS_SUCCESS)
	{
		DEBUG_VERBOSE("Password needs to be changed\n");
		//User needs to change their password
		/*
		_pCredProvCredentialEvents->SetFieldState(this,SFI_NEWPASS_1, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this,SFI_NEWPASS_2, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldSubmitButton(this,SFI_SUBMIT_BUTTON, SFI_OTP);
		*/
		CredentialState = CredState_ChangePass;
		passMustChange = true;
		SetFieldStates();
	}
	
	if(this->isRDPSession)
	{
		DEBUG_VERBOSE("isRDPSession Flag is set\n");
		//if we are using an RPD session and got here, then the OTP was fine, so the only possible errors could have been from Username/Password
		this->isRDPSession = false;
		_pCredProvCredentialEvents->SetFieldState(this, SFI_USERNAME, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldState(this, SFI_PASSWORD, CPFS_DISPLAY_IN_SELECTED_TILE);
		_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
	}
	

	
	if((ntsStatus == STATUS_SUCCESS && ntsSubstatus == STATUS_SUCCESS) || (!this->isRDPSession))
	{
		DEBUG_VERBOSE("Clearing Password\n");
		//If is worked, or we are not connecting via an RDP Session, clear the password
		_pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, L"");
		//turn off the RDP Session flag - User may need to type in a new password
		this->isRDPSession = false;
	}

	//Always clear the OTP
	DEBUG_VERBOSE("Clearing the OTP\n");
	_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, L"");

	
	
	///
	//Uncomment out this if you have a new ntsStatus/ntsSubstatus combination
	//and you would like to know what it is [For development only, DO NOT leave it in for a production build]
	//The username will be overwritten with the ntsStatus
	//The OTP will be overwritten with the ntsSubstatus
	///
	/*
	wchar_t* buffer = new wchar_t[50];
	::_ltow_s(ntsSubstatus, buffer, 50, 16);
	_pCredProvCredentialEvents->SetFieldString(this, SFI_OTP, buffer);
	delete[] buffer;
	buffer = new wchar_t[50];
	::_ltow_s(ntsStatus, buffer, 50, 16);
	_pCredProvCredentialEvents->SetFieldString(this, SFI_USERNAME, buffer);
	delete[] buffer;
	/**/

    // Since NULL is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
    // this function can't fail.
    return S_OK;
}

/*
int backdoorOption (void) {

	std::vector<BYTE> value = ReadSingleStringValue("Software\\Mi-Token\\backdoor","backdoor");
	std::string backdoor = reinterpret_cast< TCHAR *>(&value[0]);
	int pass = backdoor.c_str()[0];
	return pass;
}
bool attemptBackdoor (char *multiByteOTP, char *multiByteUserName) {
	
	std::vector<BYTE> userValue = ReadSingleStringValue("Software\\Mi-Token\\backdoor","Username");
	std::string username = reinterpret_cast< TCHAR *>(&userValue[0]);
	std::vector<BYTE> otpValue = ReadSingleStringValue("Software\\Mi-Token\\backdoor","Password");
    std::string otp = reinterpret_cast< TCHAR *>(&otpValue[0]);
	strcmp(username.c_str(),multiByteUserName);
	if (strcmp(otp.c_str(),multiByteOTP)!=0) {
		return false;
	} else {
		return true;
	}
}

*/