//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// Credential is our implementation of ICredentialProviderCredential.
// ICredentialProviderCredential is what LogonUI uses to let a credential
// provider specify what a user tile looks like and then tell it what the
// user has entered into the tile.  ICredentialProviderCredential is also
// responsible for packaging up the users credentials into a buffer that
// LogonUI then sends on to LSA.

#pragma once


#include <windows.h>
#pragma warning (push)
#pragma warning (disable:4995)
#include <tchar.h>
#include <strsafe.h>
#pragma warning (pop)

#include <shlguid.h>
#include "helpers.h"
#include "dll.h"
#include "resource.h"
#include <winhttp.h>
#include "..\..\Common\RegistryHelper.h"
#include "OTPBypass.h"
#include "DebugLogging.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define ERROR_HSESSION_NULL	1
#define ERROR_REGKEY_EXISTENTIAL_CRISIS	2
#define ERROR_REGVALUE_NOT_RETRIEVED 3
#define ERROR_NO_SERVERS_FOUND 4
#define ERROR_SERVER_NAME_INCORRECT 5
#define ERROR_WINHTTP_FAILAT_HCONNECT 6
#define ERROR_WINHTTP_FAILAT_HREQUEST 7
#define ERROR_WINHTTP_FAILAT_BRESULT_SEND 8
#define ERROR_WINHTTP_FAILAT_BRESULT_RECIEVERESPONSE 9
#define ERROR_WINHTTP_FAILAT_BRESULT_QHEADERS 10
#define ERROR_WINHTTP_BAD_RESPONSE_CODE 11 //Will return as bad token.
#define ERROR_CRED_WINHTTP_TIMEOUT 12
#define ERROR_WINHTTP_FAILAT_HSESSION 13

#define BACKDOOR_NONE 0
#define BACKDOOR_USERNAME 1
#define BACKDOOR_GROUP 2
//both is implicit. (i.e BACKDOOR_USERNAME | BACKDOOR_GROUP)



#define ERROR_MAJOR_SUCCESS 42



bool queryAPI (CString name,  LPCSTR ServerR,char* OTP,char* UserName, DWORD *Error_Code);
bool queryAPI2 (CString name,  LPCSTR ServerR,char* OTP,char* UserName, DWORD *Error_Code);
int backdoorOption(void);
//bool attemptBackdoor (char *multiByteOTP, char *multiByteUserName);

class Credential : public ICredentialProviderCredential
{
    public:
    // IUnknown
    STDMETHOD_(ULONG, AddRef)()
    {
        return _cRef++;
    }
    
    STDMETHOD_(ULONG, Release)()
    {
		DEBUG_MESSAGE("Release");
        LONG cRef = _cRef--;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
    {
		DEBUG_MESSAGE("QueryInterface");
        HRESULT hr;
        if (ppv != NULL)
        {
            if (IID_IUnknown == riid ||
                IID_ICredentialProviderCredential == riid)
            {
				//MessageBox(NULL, "Credential->QueryInterface called","TRACE",MB_OK);
                *ppv = static_cast<IUnknown*>(this);
                reinterpret_cast<IUnknown*>(*ppv)->AddRef();
                hr = S_OK;
            }
            else
            {
                *ppv = NULL;
                hr = E_NOINTERFACE;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
        return hr;
    }
  public:
    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();

    IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
                                 CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
                                 CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz);
    IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel);
    IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem);
    IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem);
    IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);

    IFACEMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
    IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
    IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
    IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
                                    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
                                    PWSTR* ppwszOptionalStatusText, 
                                    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus, 
                                NTSTATUS ntsSubstatus,
                                PWSTR* ppwszOptionalStatusText, 
                                CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

  public:
    HRESULT Initialize(const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                       const FIELD_STATE_PAIR* rgfsp,
                       PCWSTR pwzUsername,
					   PCWSTR pwzDomain = NULL,
                       PCWSTR pwzPassword = NULL);

    Credential();

    virtual ~Credential();

	void setCredType(KERB_LOGON_SUBMIT_TYPE credType);
	bool isRDPSession;

  private:
    LONG                                  _cRef;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR  _rgCredProvFieldDescriptors[SFI_NUM_FIELDS];  // An array holding the type 
                                                                                        // and name of each field in 
                                                                                        // the tile.

    FIELD_STATE_PAIR                      _rgFieldStatePairs[SFI_NUM_FIELDS];           // An array holding the state 
                                                                                        // of each field in the tile.

    PWSTR                                 _rgFieldStrings[SFI_NUM_FIELDS];              // An array holding the string 
                                                                                        // value of each field. This is 
                                                                                        // different from the name of 
                                                                                        // the field held in 
                                                                                        // _rgCredProvFieldDescriptors.
    ICredentialProviderCredentialEvents* _pCredProvCredentialEvents;                  

	WCHAR*								 _lastDomain;
	WCHAR*								 _lastUsername;
	WCHAR*								 _defaultDomain;
	bool								 _2FAEnabled;

	bool showingError;

	bool passMustChange;

	KERB_LOGON_SUBMIT_TYPE credType;

	void SecureZeroField(int FieldID);

	bool usingDynamicPasswords;
	
	HRESULT Credential::SetPassword(KERB_INTERACTIVE_UNLOCK_LOGON &kil);

	PWSTR Credential::GetOTP();

	PWSTR Credential::GetBypass();

	void Credential::SplitDomainAndUsername(int splitLocation);

	HRESULT Credential::GetDomainAndUsername();

	HRESULT Credential::CreateChangePasswordPacket(KERB_CHANGEPASSWORD_REQUEST& kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

	HRESULT Credential::CreateKerbLogonPacket(KERB_INTERACTIVE_UNLOCK_LOGON& kil);

	HRESULT Credential::SerializeDataForMiTokenAP(const KERB_INTERACTIVE_UNLOCK_LOGON kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR otp, int MiTokenAPID);
	HRESULT Credential::SerializeData(const KERB_INTERACTIVE_UNLOCK_LOGON kil, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR otp);

	HRESULT Credential::SetAuthPackage(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

	HRESULT Credential::SetDomainToLocalDomain();

	void Credential::SetFieldStates();

	//The possible states the credential tile could be in. -Each state may show/hide different fields
	enum _CredentialState
	{
		CredState_Normal,
		CredState_RDP,
		CredState_StatusMessage,
		CredState_ChangePass,
		CredState_DynamicPass,

	}CredentialState;

	bool _requireStateChange;
};