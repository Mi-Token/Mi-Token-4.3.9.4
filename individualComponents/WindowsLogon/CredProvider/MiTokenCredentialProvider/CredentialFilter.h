
#pragma once 
#include "credentialprovider.h" 
#include "dll.h" 

extern bool isRDP;


//This class implements ICredentialProviderFilter, which is responsible for filtering out other credential providers. 
//The LMS Credential Provider uses this to mask out the default Windows provider. 
class CredentialFilter : public ICredentialProviderFilter 
{ 
public: 
	//This section contains some COM boilerplate code 

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
		if (IID_IUnknown == riid || IID_ICredentialProviderFilter == riid) 
		{ 
			//MessageBox(NULL, "FILTER->QueryInterface called","TRACE",MB_OK);
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
	friend HRESULT CredentialFilter_CreateInstance(REFIID riid, __deref_out void** ppv); 

	//Implementation of ICredentialProviderFilter 
	IFACEMETHODIMP Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags, GUID* rgclsidProviders, BOOL* rgbAllow, DWORD cProviders); 
	IFACEMETHODIMP UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcsOut); 

protected: 
	CredentialFilter(); 
	__override ~CredentialFilter(); 
	void restrictRegkeyAccess();

private: 
	LONG _cRef; 
}; 
