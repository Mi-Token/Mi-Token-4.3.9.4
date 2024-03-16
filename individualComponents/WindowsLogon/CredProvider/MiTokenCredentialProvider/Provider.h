//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#pragma once

#include <credentialprovider.h>
#include <windows.h>
#pragma warning (push)
#pragma warning (disable:4995)
#include <tchar.h>
#include <strsafe.h>
#pragma warning (pop)

#include "Credential.h"
#include "helpers.h"

#define MAX_CREDENTIALS 3

class Provider : public ICredentialProvider
{
  public:
    // IUnknown
    STDMETHOD_(ULONG, AddRef)()
    {
        return _cRef++;
    }
    
    STDMETHOD_(ULONG, Release)()
    {
        LONG cRef = _cRef--;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (IID_IUnknown == riid || 
            IID_ICredentialProvider == riid)
        {
			//MessageBox(NULL, "Provider->QueryInterface called","TRACE",MB_OK);
            *ppv = this;
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            hr = S_OK;
        }
        else
        {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

  public:
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
                                      __out DWORD* pdwDefault,
                                      __out BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, 
                                   __out ICredentialProviderCredential** ppcpc);

    friend HRESULT Provider_CreateInstance(REFIID riid, __deref_out void** ppv);

  protected:
    Provider();
    __override ~Provider();
    
  private:

	
    
    HRESULT _EnumerateOneCredential(__in DWORD dwCredentialIndex,
                                    __in PCWSTR pwzUsername
                                    );

    // Create/free enumerated credentials.
    HRESULT _CreateEnumeratedCredentials();
    void _ReleaseEnumeratedCredentials();
    
    HRESULT _EnumerateCredentials(bool bAlreadyHaveSetSerializationCred = false); //this enumerates the normal set of 2 creds
    HRESULT _EnumerateSetSerialization(); //this will enumerate one tile with the contents of _pkiulSetSerialization

private:
    LONG              _cRef;
    Credential *_rgpCredentials[MAX_CREDENTIALS]; // Pointers to the credentials which will be enumerated by 
                                                         // this Provider.
    KERB_INTERACTIVE_UNLOCK_LOGON *     _pkiulSetSerialization;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO  _cpus;
    DWORD                               _dwCredUIFlags;
    bool                                _bRecreateEnumeratedCredentials;
    bool                                _bAutoSubmitSetSerializationCred;
    bool                                _bDefaultToFirstCredential;
};