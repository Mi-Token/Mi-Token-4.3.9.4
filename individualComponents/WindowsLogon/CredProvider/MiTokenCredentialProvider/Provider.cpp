#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4996 )


//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// Provider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we have decided to show two tiles, one for
// Administrator and one for Guest.  You will need to decide what
// tiles make sense for your situation.  Can you enumerate the
// users who will use your method to log on?  Or is it better
// to provide a tile where they can type in their username?
// Does the user need to interact with something other than the
// keyboard before you can recognize which user it is (such as insert 
// a smartcard)?  We call these "event driven" credential providers.  
// We suggest that such credential providers first provide one basic tile which
// tells the user what to do ("insert your smartcard").  Once the
// user performs the action, then you can callback into LogonUI to
// tell it that you have new tiles, and include a tile that is specific
// to the user that the user can then interact with if necessary.

#include <credentialprovider.h>
#include "OTPBypass.h"
#include "Provider.h"
#include "Credential.h"
#include "guid.h"
#include <wincred.h>
#include "CredentialFilter.h"
#include "SDDLs.h"
#include <Sddl.h>

// Provider ////////////////////////////////////////////////////////

Provider::Provider():
    _cRef(1),
    _pkiulSetSerialization(NULL),
    _dwCredUIFlags(0),
    _bRecreateEnumeratedCredentials(true),
    _bAutoSubmitSetSerializationCred(false),
    _bDefaultToFirstCredential(false)
{
	HKEY subKey = NULL;


    DllAddRef();
	
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Mi-Token", 0, KEY_READ, &subKey);
	if(result == ERROR_SUCCESS)
	{
		long value;
		DWORD len = sizeof(value);
		result = RegQueryValueEx(subKey, "CPDebugMode", NULL, NULL, (BYTE*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			debugLog = value;
			//if(value )
			//	debugLog = true;
		}
		RegCloseKey(subKey);
	}
	
#ifdef _DEBUG
	DEBUG_MESSAGE("---RUNNING DEBUG BUILD!---");
#endif
	DEBUG_MESSAGE("-------------Provider::Provider constructor()-------------------");
    ZeroMemory(_rgpCredentials, sizeof(_rgpCredentials));
}

Provider::~Provider()
{
	DEBUG_MESSAGE("Provider::~Provider() destructor");
    _ReleaseEnumeratedCredentials();
    DllRelease();
}



void Provider::_ReleaseEnumeratedCredentials()
{
	DEBUG_MESSAGE("Provider::Release Enumerated Credentials");
    for (int i = 0; i < ARRAYSIZE(_rgpCredentials); i++)
    {
        if (_rgpCredentials[i] != NULL)
        {
            _rgpCredentials[i]->Release();
        }
    }
}


// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
//
// This sample only handles the logon, unlock, and credui scenarios.
HRESULT Provider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags
    )
{
	DEBUG_MESSAGE("Provider: SetUsageScenario");
    HRESULT hr;

    _cpus = cpus;
    if (cpus == CPUS_CREDUI)
    {
		DEBUG_MESSAGE("Provider: SetUsageScenario CPUS_CREDUI");
        _dwCredUIFlags = dwFlags;  // currently the only flags ever passed in are only valid for the credui scenario
    }
    _bRecreateEnumeratedCredentials = true;

    // unlike SampleCredentialProvider, we're not going to enumerate here.  Instead, we'll store off the info
    // and then we'll wait for GetCredentialCount to enumerate.  That way we'll know at enumeration time
    // whether we have a SetSerialization cred to deal with.  That's a bit more important in the credUI case
    // than the logon case (although even in the logon case you could choose to only enumerate the SetSerialization
    // credential if there is one -- that's what the built-in password provider does).
    switch (cpus)
    {
    case CPUS_LOGON:
		DEBUG_MESSAGE("Provider: SetUsageScenario >LOGON");
    case CPUS_UNLOCK_WORKSTATION:
		DEBUG_MESSAGE("Provider: SetUsageScenario >UNLOCK");
		hr = S_OK;
        break;
    case CPUS_CREDUI:
		DEBUG_MESSAGE("Provider: SetUsageScenario >CPUS_CREDUI");
        hr = E_NOTIMPL;
		break;

    case CPUS_CHANGE_PASSWORD:
		DEBUG_MESSAGE("Provider: SetUsageScenario >CPUS_CHANGE_PASS");
        hr = E_NOTIMPL;
        break;

    default:
		DEBUG_MESSAGE("Provider: SetUsageScenario >DEFAULT");
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//


void printCPCS(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn)
{
	DEBUG_MESSAGE("Provider::printCPCS() to CPSerialized.log");
#ifdef ENABLE_LOGGING
	if(debugLog)
	{
		FILE* fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\CPSerialized.log", "a+");
		if(fp != NULL)
		{
			fprintf(fp, "printCPCS\r\n");
			fprintf(fp, "[%08lX] %08lX-%04lX-%04lX-", pcpcsIn->ulAuthenticationPackage, pcpcsIn->clsidCredentialProvider.Data1, pcpcsIn->clsidCredentialProvider.Data2, pcpcsIn->clsidCredentialProvider.Data3);
			for(unsigned int i = 0 ; i < 8;  ++i)
				fprintf(fp,"%02lX", pcpcsIn->clsidCredentialProvider.Data4[i]);
			fprintf(fp, "\r\n");
			fprintf(fp, "%d\t[%08lX]\r\n", pcpcsIn->cbSerialization, pcpcsIn->rgbSerialization);

			for(unsigned int i = 0 ; i < pcpcsIn->cbSerialization ; i++)
			{
				fprintf(fp, "%c", pcpcsIn->rgbSerialization[i]);
			}
			fprintf(fp,"\r\n\r\n");
			fclose(fp);
		}
	}
#endif
}
STDMETHODIMP Provider::SetSerialization(
    const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
    )
{
	DEBUG_MESSAGE("Provider::SetSerialization()");
    HRESULT hr = E_NOTIMPL;
	
    if ((CLSID_Provider == pcpcs->clsidCredentialProvider) || (CPUS_CREDUI == _cpus))
    {
        // Get the current AuthenticationPackageID that we are supporting
		DEBUG_MESSAGE("Provider::SetSerialization() - CLSID = pcpcs");
        ULONG ulAuthPackage;
        hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

        if (SUCCEEDED(hr))
        {
            //if ((CPUS_CREDUI == _cpus) && (_dwCredUIFlags & CREDUIWIN_AUTHPACKAGE_ONLY) && 
             //   (ulAuthPackage == pcpcs->ulAuthenticationPackage))
			if( (CPUS_CREDUI == _cpus) )
            {
				DEBUG_MESSAGE("Provider::SetSerialization() - CPUS_CREDUI = _cpus +more");
                // In the credui case, SetSerialization should only ever return S_OK if it is able to serialize the input cred.
                // Unfortunately, SetSerialization had to be overloaded to indicate whether or not it will be able to GetSerialization 
                // for the specific Auth Package that is being requested for CREDUIWIN_AUTHPACKAGE_ONLY to work, so when that flag is 
                // set, it should return S_FALSE unless it is ALSO able to serialize the input cred, then it can return S_OK.
                hr = S_FALSE;
				return hr;
            }
            if ((ulAuthPackage == pcpcs->ulAuthenticationPackage) &&
                (0 < pcpcs->cbSerialization && pcpcs->rgbSerialization))
            {
				DEBUG_MESSAGE("Provider::SetSerialization() - pkil stuff [v2]");
				printCPCS(pcpcs);


                KERB_INTERACTIVE_UNLOCK_LOGON* pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*) pcpcs->rgbSerialization;
                if (KerbInteractiveLogon == pkil->Logon.MessageType)
                {
					DEBUG_MESSAGE("Provider::SetSerialization() - KerbInteractiveLogon");
                    BYTE* rgbSerialization;
                    rgbSerialization = (BYTE*)HeapAlloc(GetProcessHeap(), 0, pcpcs->cbSerialization);
                    HRESULT hrCreateCred = rgbSerialization ? S_OK : E_OUTOFMEMORY;

                    if (SUCCEEDED(hrCreateCred))
                    {
						DEBUG_MESSAGE("Provider::SetSerialization() - hrCreateCred SUCCEED");
                        CopyMemory(rgbSerialization, pcpcs->rgbSerialization, pcpcs->cbSerialization);
                        KerbInteractiveLogonUnpackInPlace((KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization);

                        if (_pkiulSetSerialization)
                        {
                            HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
							DEBUG_MESSAGE("Provider::SetSerialization() -> Let it be free");
                        }
						DEBUG_MESSAGE("Provider::SetSerialization() - pre pkiulSetSerialization");
                        _pkiulSetSerialization = (KERB_INTERACTIVE_UNLOCK_LOGON*)rgbSerialization;
                        if (SUCCEEDED(hrCreateCred))
                        {
							DEBUG_MESSAGE("Provider::SetSerialization() - hrCreateCted");
                            // we allow success to override the S_FALSE for the CREDUIWIN_AUTHPACKAGE_ONLY, but
                            // failure to create the cred shouldn't override that we can still handle
                            // the auth package
#ifdef ENABLE_LOGGING
							if(debugLog)
							{
								FILE* fp = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\RDC.log", "a+");
								DEBUG_MESSAGE("Provider::SetSerialization() opening FP");
								if(fp != NULL)
								{
									char userstr[500];
									char passstr[500];
									char domainstr[500];
									memset(userstr, 0, 500);
									memset(passstr, 0, 500);
									memset(domainstr, 0, 500);

									DEBUG_MESSAGE("Provider::SetSerialization() FP not null");
									DEBUG_MESSAGE("Provider::SetSerialization() writing username");
									if((_pkiulSetSerialization->Logon.LogonDomainName.Length > 400) ||
										(_pkiulSetSerialization->Logon.Password.Length > 400) ||
										(_pkiulSetSerialization->Logon.UserName.Length > 400))
									{
										DEBUG_MESSAGE("Provider::SetSerialization() WARNING : Length is over 400");
									}

									memcpy(userstr, _pkiulSetSerialization->Logon.UserName.Buffer, _pkiulSetSerialization->Logon.UserName.Length);
									memcpy(passstr, _pkiulSetSerialization->Logon.Password.Buffer, _pkiulSetSerialization->Logon.Password.Length);
									memcpy(domainstr, _pkiulSetSerialization->Logon.LogonDomainName.Buffer, _pkiulSetSerialization->Logon.LogonDomainName.Length);
									fprintf(fp, "Username [%d] : %ls\n", _pkiulSetSerialization->Logon.UserName.Length, userstr);
									fprintf(fp, "Password [%d] : %ls\n", _pkiulSetSerialization->Logon.Password.Length, passstr);
									fprintf(fp, "Domain [%d] : %ls\n", _pkiulSetSerialization->Logon.LogonDomainName.Length, domainstr);

									fclose(fp);

									
								}
							}
#endif

							if(_pkiulSetSerialization->Logon.Password.Length == 0)
							{
								HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
								_pkiulSetSerialization = NULL;
								isRDP = false;
							}
							//DO NOT FREE THE DATA - WinLogon does that without telling us...
							//Thank you Microsoft :/
							//HeapFree(GetProcessHeap(), 0, pcpcs->rgbSerialization);
							hr = hrCreateCred;
                        }
                    }
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

	DEBUG_MESSAGE("Provider::SetSerialization() end SetSerialization");
    return S_OK; //currently stops LogonUI from crashing, should look into
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT Provider::Advise(
    ICredentialProviderEvents* pcpe,
    UINT_PTR upAdviseContext
    )
{
    DEBUG_MESSAGE("Provider::Advise()");
    UNREFERENCED_PARAMETER(pcpe);
    UNREFERENCED_PARAMETER(upAdviseContext);

    ::free(s_pCustomTitle);
    s_pCustomTitle = _wcsdup(Credential::GetCustomTitle());

    DEBUG_MESSAGE("End Provider::Advise");
    return E_NOTIMPL;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT Provider::UnAdvise()
{
	DEBUG_MESSAGE("Provider::UnAdvise");
    return E_NOTIMPL;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT Provider::GetFieldDescriptorCount(
    DWORD* pdwCount
    )
{
	DEBUG_MESSAGE("Provider::GetFieldDescriptorCount()");
    *pdwCount = SFI_NUM_FIELDS;

    return S_OK;
}

// Gets the field descriptor for a particular field
HRESULT Provider::GetFieldDescriptorAt(
    DWORD dwIndex, 
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
	DEBUG_MESSAGE("Provider::GetFieldDescriptorAt()");
    HRESULT hr;

    // Verify dwIndex is a valid field.
    if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
    {
        hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
    }
    else
    { 
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is the last cred prov
// get to specify the default tile.
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
HRESULT Provider::GetCredentialCount(
    DWORD* pdwCount,
    DWORD* pdwDefault,
    BOOL* pbAutoLogonWithDefault
    )
{
	DEBUG_MESSAGE("Provider::GetCredentialCount()");
    HRESULT hr = E_FAIL;
    if (_bRecreateEnumeratedCredentials)
    {
        _ReleaseEnumeratedCredentials();
        hr = _CreateEnumeratedCredentials();
        _bRecreateEnumeratedCredentials = false;
    }

    *pdwCount = 0;
    *pdwDefault = (_bDefaultToFirstCredential && _rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
    *pbAutoLogonWithDefault = FALSE;

    if (SUCCEEDED(hr))
    {
        // TODO: it would probably be nicer to keep a count of the number of creds
		DWORD dwNumCreds = _rgpCredentials[0] ? 1 : 0; //we have a max of 1 credential, whihc is in _rgbCredentials[0]
		/*
        for (int i = 0; i < MAX_CREDENTIALS; i++)
        {
            if (_rgpCredentials[i] != NULL)
            {
                dwNumCreds++;
            }
        }
		*/

        switch(_cpus)
        {
        case CPUS_LOGON:
            if (_bAutoSubmitSetSerializationCred)
            {
                *pdwCount = 1;
                *pbAutoLogonWithDefault = TRUE;
				*pdwDefault = (_rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
            }
            else
            {
                
                *pdwCount = dwNumCreds;
				*pdwDefault = (_rgpCredentials[0]) ? 0 : CREDENTIAL_PROVIDER_NO_DEFAULT;
                // since we have more than one tile and don't keep track of who logged on last, we don't really have a default in this case
            }
            hr = S_OK;
            break;

        case CPUS_UNLOCK_WORKSTATION:
            // in the unlock case, you likely would want to only enumerate tiles for the logged on user (that could be used to unlock)
            // but that's a bit complicated for a sample, so we'll just use our normal tiles
            // that we already set up in the logon case.  The default out params set up at the top work for this case.

			if(_rgpCredentials[0])
			{
				_rgpCredentials[0]->setCredType(KerbWorkstationUnlockLogon);
				*pdwDefault = 0;
			}
            *pdwCount = dwNumCreds;
            hr = S_OK;
            break;

        case CPUS_CREDUI:
            {
               *pdwCount = dwNumCreds;
                hr = S_OK;
            }
            break;

		default:
            hr = E_INVALIDARG;
            break;
        }
    }
    
    return hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT Provider::GetCredentialAt(
    DWORD dwIndex, 
    ICredentialProviderCredential** ppcpc
    )
{
	DEBUG_MESSAGE("Provider::GetCredentialAt()");
    HRESULT hr;

    // Validate parameters.
    if((dwIndex < ARRAYSIZE(_rgpCredentials)) && _rgpCredentials[dwIndex] != NULL && ppcpc)
    {
        hr = _rgpCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    }
    else
    {
        hr = E_INVALIDARG;
    }
        
    return hr;
}

bool hideLastLoggedInUser() 
{
	DEBUG_VERBOSE("Provider::hideLastLoggedInUser()\n");
	wchar_t* output = NULL;
	HKEY configKey;
	LONG keyResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ, &configKey);
	bool hideUsername = false;

	if(keyResult == ERROR_SUCCESS)
	{
		int value;
		DWORD len = sizeof(value);
		DWORD result = RegQueryValueEx(configKey, "HideLastLoggedInUsername", NULL, NULL, (unsigned char*)&value, &len);
		if(result == ERROR_SUCCESS)
		{
			if(value == 1)
			{
				hideUsername = true;
			}
		}
		RegCloseKey(configKey);
	}
	else
	{
		DEBUG_VERBOSE("Provider::hideLastLoggedInUser Failed reading Mi-Token Logon Config key HideLastLoggedInUsername : Could not open regkey\n");
	}
	DEBUG_VERBOSE("Provider::hideLastLoggedInUser hideLastLoggedInUser() returning [%s]\n", hideUsername ? "true" : "false");
	return hideUsername;

}

wchar_t* getDefaultUser()
{
	DEBUG_VERBOSE("Provider::getDefaultUser()\n");
	wchar_t* output = NULL;
	HKEY regKey;
	LONG keyResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Mi-Token\\Logon\\Config", 0, KEY_READ, &regKey);

	if(keyResult == ERROR_SUCCESS)
	{
		DWORD result;
		DWORD userlen = 0;
		DWORD valType;
		result = RegQueryValueEx(regKey, "LastUserID", NULL, &valType, NULL, &userlen);

		DEBUG_VERBOSE("Provider::getDefaultUser() Attempted to get reg value length, result = %d bytes with retcode %d\n", userlen, result);
		if(result == ERROR_FILE_NOT_FOUND)
		{
			DEBUG_VERBOSE("Provider::getDefaultUser() Warning : RegValue does not exist\n");
		}
		else
		{
			char* cstr = (char*)calloc(userlen + 1, sizeof(char));
			result = RegQueryValueEx(regKey, "LastUserID", NULL, &valType, (LPBYTE)cstr, &userlen);
			
			DEBUG_VERBOSE("Provider::getDefaultUser() Attempted to get reg value, result = %08lX\n", result);
			
			userlen = strlen(cstr);
			output = (wchar_t*)calloc(userlen + 1, sizeof(wchar_t));
			mbstowcs(output, cstr, userlen);

		}
		RegCloseKey(regKey);
	}
	else
	{
		DEBUG_VERBOSE("Povider::getDefaultUser() Failed to update username : Could not open regkey\n");
	}

	return output;
}

// Creates a Credential with the SFI_USERNAME field's value set to pwzUsername.
HRESULT Provider::_EnumerateOneCredential(
    DWORD dwCredentialIndex,
    PCWSTR pwzUsername
    )
{
	DEBUG_LOG("Provider::EnumerateOneCredential() dwCredentialIndex = [%d] pwzUsername [%ls]\n", dwCredentialIndex, pwzUsername);
    HRESULT hr;

    // Allocate memory for the new credential.
    Credential* ppc = new Credential();
    
    if (ppc)
    {
        // Set the Field State Pair and Field Descriptors for ppc's fields
        // to the defaults (s_rgCredProvFieldDescriptors, and s_rgFieldStatePairs) and the value of SFI_USERNAME
        // to pwzUsername.
		HKEY subKey;
		WCHAR* _defaultDomain = NULL;
		if(!isRDP && !hideLastLoggedInUser())
		{
			_defaultDomain = getDefaultUser();
			if(_defaultDomain == NULL)
			{
				LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI", 0, KEY_READ, &subKey);	
				if(result == ERROR_SUCCESS)
				{
					DWORD len = 0;
					result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, NULL, &len);
					if(result == ERROR_SUCCESS)
					{
						char* cbuffer = (char*)calloc(len, 1);
						DEBUG_LOG("Provider::EnumerateOneCredential() LastLoggedOnSAMUser Buffer len = %d\n", len);
						result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, (LPBYTE)cbuffer, &len);
						if(result == ERROR_SUCCESS)
						{
							_defaultDomain = (wchar_t*)calloc(len, sizeof(wchar_t));

							DEBUG_LOG("Provider::EnumerateOneCredential() LastLoggedOnSAMUser CBuffer [%s]\n", cbuffer);
							size_t charsConverted = mbstowcs(_defaultDomain, cbuffer, len);

							DEBUG_LOGW(L"Provider::EnumerateOneCredential() LastLoggedOnSAMUser [%s] chars converted [%d] len [%d]\n[Byte Dump] : ", _defaultDomain, charsConverted, len);
							DEBUG_BYTES((unsigned char*)_defaultDomain, len * 2);
							DEBUG_LOG("\n");
						}
						free(cbuffer);
					}
				}
			}
		}
		DEBUG_LOGW(L"Provider::EnumerateOneCredential() calling ppc->Initialize() with _defaultDomain[%ls]\n", _defaultDomain);
        hr = ppc->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, _defaultDomain, NULL, NULL, dwCredentialIndex);
        
        if (SUCCEEDED(hr))
        {
            _rgpCredentials[dwCredentialIndex] = ppc;
        }
        else
        {
            // Release the pointer to account for the local reference.
            ppc->Release();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

//
// Depending on whether SetSerialization has been called and what CPUS we're in
// creates the right set of credentials
//
HRESULT Provider::_CreateEnumeratedCredentials()
{
	DEBUG_LOG("Provider::_CreateEnumeratedCredentials() _cpus = [%d]\n", _cpus);
    HRESULT hr = E_INVALIDARG;
    switch(_cpus)
    {
    case CPUS_LOGON:
        if (_pkiulSetSerialization)
        {
			DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials Provider: CEC : (_pkiulSS)");
			DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPUS_LOGON calling _EnumerateSetSerialization()");
            hr = _EnumerateSetSerialization();
        }
        else
        {
			DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPUS_LOGON calling _EnumerateCredentials()");
            hr = _EnumerateCredentials();
        }
        break;

    case CPUS_CHANGE_PASSWORD:
        break;

    case CPUS_UNLOCK_WORKSTATION:
        // a more advanced implementation would only enumerate tiles that could gather creds for the logged on user
        // since those are the only creds that will work to unlock the session
        // but we're going with this for simplicity
		DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPIS_UNLOCK_WORKSTATION calling _EnumerateCredentials()");
        hr = _EnumerateCredentials();  
        break;

    case CPUS_CREDUI:
        _bDefaultToFirstCredential = true;

        if (_pkiulSetSerialization)
        {
			DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPUS_CREDUI calling _EnumerateSetSerialization()");
            hr = _EnumerateSetSerialization();
        }
        /*if (FALSE) //_dwCredUIFlags & CREDUIWIN_ADMINS_ONLY)
        {
            // this sample doesn't handle this particular case
            // You would want to handle this in order to participate
            // User Account Control elevations for a non-admin users (where all the
            // admins tiles are enumerated)
        }
		else */if (!(_dwCredUIFlags & CREDUIWIN_IN_CRED_ONLY))
        {
            // if we're here, then we're supposed to enumerate whatever we should enumerate for the normal case.  
            // In our case, that's our 2 tiles.  We may already have one tile, though
            if (_pkiulSetSerialization && SUCCEEDED(hr))
            {
				DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPUS_CREDUI calling _EnumerateCredentials(true)");
                hr = _EnumerateCredentials(true);
            }
            else
            {
				DEBUG_MESSAGE("Provider::_CreateEnumeratedCredentials case CPUS_CREDUI calling _EnumerateCredentials(false)");
                hr = _EnumerateCredentials(false);
            }
        }
        break;

    default:
        break;
    }
    return hr;
}


// Sets up the normal 2 tiles for this provider (Administrator and Guest)
HRESULT Provider::_EnumerateCredentials(bool bAlreadyHaveSetSerializationCred)
{
	DEBUG_LOG("Provider::_EnumerateCredentials(bAlreadyHaveSetSerializationCred) is %d\n", bAlreadyHaveSetSerializationCred);
    DWORD dwStart = bAlreadyHaveSetSerializationCred ? 1 : 0;

	HRESULT hr;
	/*
	if (hideLastLoggedInUser()) {
		DEBUG_LOG("Provider: EnumerateCredentials EnumerateOneCredential dwIndex 0 hideLastLoggedInUsername\n");
		hr = _EnumerateOneCredential(0, L"");
	}
	else
	*/
	{
		DEBUG_LOG("Provider::_EnumerateCredentials(..) EnumerateOneCredential dwIndex %d\n", dwStart + 1);
		hr = _EnumerateOneCredential(dwStart++, L"");
	}
    return hr;
}

// This enumerates a tile for the info in _pkiulSetSerialization.  See the SetSerialization function comment for
// more information.
HRESULT Provider::_EnumerateSetSerialization()
{
	DEBUG_MESSAGE("Provider::_EnumerateSetSerialization()");
    KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;

    _bAutoSubmitSetSerializationCred = false;
    _bDefaultToFirstCredential = false;

    // Since this provider only enumerates local users (not domain users) we are ignoring the domain passed in.
    // However, please note that if you receive a serialized cred of just a domain name, that domain name is meant 
    // to be the default domain for the tiles (or for the empty tile if you have one).  Also, depending on your scenario,
    // the presence of a domain other than what you're expecting might be a clue that you shouldn't handle
    // the SetSerialization.  For example, in this sample, we could choose to not accept a serialization for a cred
    // that had something other than the local machine name as the domain.

    // Use a "long" (MAX_PATH is arbitrary) buffer because it's hard to predict what will be
    // in the incoming values.  A DNS-format domain name, for instance, can be longer than DNLEN.
    WCHAR wszUsername[MAX_PATH] = {0};
    WCHAR wszPassword[MAX_PATH] = {0};
	WCHAR wszDomain[MAX_PATH] = {0};

    // since this sample assumes local users, we'll ignore domain.  If you wanted to handle the domain
    // case, you'd have to update Credential::Initialize to take a domain.
    HRESULT hr = StringCbCopyNW(wszUsername, sizeof(wszUsername), pkil->UserName.Buffer, pkil->UserName.Length);

    if (SUCCEEDED(hr))
    {
        hr = StringCbCopyNW(wszPassword, sizeof(wszPassword), pkil->Password.Buffer, pkil->Password.Length);

        if (SUCCEEDED(hr))
        {
			hr = StringCbCopyNW(wszDomain, sizeof(wszDomain), pkil->LogonDomainName.Buffer, pkil->LogonDomainName.Length);
			if(SUCCEEDED(hr))
			{
				DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() Domain: [%ls]\n", wszDomain);
				//DEBUG_WMESSAGE(wszDomain);
				DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() Username: [%ls]\n", wszUsername);
				//DEBUG_WMESSAGE(wszUsername);
				DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() Password: [%ls]\n", wszPassword);
				//DEBUG_WMESSAGE(wszPassword);

				Credential* pCred = new Credential();

				if (pCred)
				{
					HKEY subKey;
					WCHAR* domain;
					WCHAR* username;
					int flag_defaultDomain = 0;
					if(!isRDP && !hideLastLoggedInUser())
					{
						LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\LogonUI", 0, KEY_READ, &subKey);	
						if(result == ERROR_SUCCESS)
						{
							DWORD len = 0;
							result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, NULL, &len);
							if(result == ERROR_SUCCESS)
							{
								char* cbuffer = (char*)calloc(len, 1);
								DEBUG_LOG("Provider::_EnumerateSetSerialization() LastLoggedOnSAMUser Buffer len = %d\n", len);
								result = RegQueryValueEx(subKey, "LastLoggedOnSAMUser", NULL, NULL, (LPBYTE)cbuffer, &len);
								if(result == ERROR_SUCCESS)
								{
									WCHAR* _defaultDomain;
									_defaultDomain = (wchar_t*)calloc(len, sizeof(wchar_t));

									DEBUG_LOG("Provider::_EnumerateSetSerialization() LastLoggedOnSAMUser CBuffer [%s]\n", cbuffer);
									size_t charsConverted = mbstowcs(_defaultDomain, cbuffer, len);
									domain = wcstok(_defaultDomain, L"\\");
									username = wcstok(NULL, L"\\");

									DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() LastLoggedOnSAMUser : %ls Found [%d vs %d]\n[Byte Dump] : ", _defaultDomain, charsConverted, len);
									flag_defaultDomain = 1;
									DEBUG_BYTES((unsigned char*)_defaultDomain, len * 2);
									DEBUG_LOG("\n");
								}
								free(cbuffer);
							}
						}
						DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() flag_defaultDomain: %d, Domain: \'%ls\' and username: \'%ls\'\n", flag_defaultDomain, domain, username);
					}

					if(flag_defaultDomain == 1) {
						DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() calling initialize() with default domain username[%ls], domain[%ls], \nwszPassword[%ls]\n", username,	domain, wszPassword);
						hr = pCred->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, username,	domain, wszPassword, SFI_PASSWORD);
					}
					else {
						DEBUG_LOGW(L"Provider::_EnumerateSetSerialization() calling initialize() no default domain  wszUsername[%ls], wszDomain[%ls], \nwszPassword[%ls]\n",wszUsername, wszDomain, wszPassword);
						hr = pCred->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, wszUsername, wszDomain, wszPassword, SFI_USERNAME);
			
					}

					if (SUCCEEDED(hr))
					{
						// for the purposes of this sample, when we enumerate the SetSerialization cred, we only enumerate
						// that cred and no others, so we can assume it just goes in slot 0.
						_rgpCredentials[0] = pCred;

						//if we were able to create a cred, default to it
						_bDefaultToFirstCredential = true;  
					}
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}

				// If we were passed all the info we need (in this case username & password), we're going to automatically submit this credential.
				// (if we're in CPUS_LOGON that is.  In credUI we want the user to at least click the tile to choose to use those creds)
				if (SUCCEEDED(hr) && (0 < wcslen(wszPassword)))
				{
					//_bAutoSubmitSetSerializationCred = true;
				}
			}
        }
    }

    return hr;
}

// Boilerplate code to create our provider.
HRESULT Provider_CreateInstance(REFIID riid, void** ppv)
{
	DEBUG_MESSAGE("Provider::Provider_CreateInstance()");
    HRESULT hr;

    Provider* pProvider = new Provider();

    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}
