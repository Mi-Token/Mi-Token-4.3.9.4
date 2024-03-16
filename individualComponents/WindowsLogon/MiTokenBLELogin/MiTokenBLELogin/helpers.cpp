//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// Helper functions for copying parameters and packaging the buffer
// for GetSerialization.


#include "helpers.h"
#include <intsafe.h>

#include "DebugLogging.h"

// 
// Copies the field descriptor pointed to by rcpfd into a buffer allocated 
// using CoTaskMemAlloc. Returns that buffer in ppcpfd.
// 
HRESULT FieldDescriptorCoAllocCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{
    HRESULT hr;
    DWORD cbStruct = sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR);

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd = 
        (CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(cbStruct);

    if (pcpfd)
    {
        pcpfd->dwFieldID = rcpfd.dwFieldID;
        pcpfd->cpft = rcpfd.cpft;
        
        if (rcpfd.pszLabel)
        {
            hr = SHStrDupW(rcpfd.pszLabel, &pcpfd->pszLabel);
        }
        else
        {
            pcpfd->pszLabel = NULL;
            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
        *ppcpfd = pcpfd;
    }
    else
    {
        CoTaskMemFree(pcpfd);  
        *ppcpfd = NULL;
    }


    return hr;
}

//
// Coppies rcpfd into the buffer pointed to by pcpfd. The caller is responsible for
// allocating pcpfd. This function uses CoTaskMemAlloc to allocate memory for 
// pcpfd->pszLabel.
//
HRESULT FieldDescriptorCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd
    )
{
    HRESULT hr;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR cpfd;

    cpfd.dwFieldID = rcpfd.dwFieldID;
    cpfd.cpft = rcpfd.cpft;

    if (rcpfd.pszLabel)
    {
        hr = SHStrDupW(rcpfd.pszLabel, &cpfd.pszLabel);
    }
    else
    {
        cpfd.pszLabel = NULL;
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        *pcpfd = cpfd;
    }

    return hr;
}

//
// This function copies the length of pwz and the pointer pwz into the UNICODE_STRING structure
// This function is intended for serializing a credential in GetSerialization only.
// Note that this function just makes a copy of the string pointer. It DOES NOT ALLOCATE storage!
// Be very, very sure that this is what you want, because it probably isn't outside of the
// exact GetSerialization call where the sample uses it.
//
HRESULT UnicodeStringInitWithString(
    PWSTR pwz, 
    UNICODE_STRING* pus
    )
{
    HRESULT hr;
    if (pwz)
    {
        size_t lenString;
        hr = StringCchLengthW(pwz, USHORT_MAX, &(lenString));
        if (SUCCEEDED(hr))
        {
            USHORT usCharCount;
            hr = SizeTToUShort(lenString, &usCharCount);
            if (SUCCEEDED(hr))
            {
                USHORT usSize;
                hr = SizeTToUShort(sizeof(WCHAR), &usSize);
                if (SUCCEEDED(hr))
                {
                    hr = UShortMult(usCharCount, usSize, &(pus->Length)); // Explicitly NOT including NULL terminator
                    if (SUCCEEDED(hr))
                    {
                        pus->MaximumLength = pus->Length;
                        pus->Buffer = pwz;
                        hr = S_OK;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
                    }
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

//
// The following function is intended to be used ONLY with the Kerb*Pack functions.  It does
// no bounds-checking because its callers have precise requirements and are written to respect 
// its limitations.
// You can read more about the UNICODE_STRING type at:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/unicode_string.asp
//
static void _UnicodeStringPackedUnicodeStringCopy(
    const UNICODE_STRING& rus,
    PWSTR pwzBuffer,
    UNICODE_STRING* pus
    )
{
    pus->Length = rus.Length;
    pus->MaximumLength = rus.Length;
    pus->Buffer = pwzBuffer;

    CopyMemory(pus->Buffer, rus.Buffer, pus->Length);
}


HRESULT KerbInteractiveChangePassPack(
	const KERB_CHANGEPASSWORD_REQUEST& rkiulIn,
	BYTE** prgb,
	DWORD* pcb
	)
{
	HRESULT hr;

	DWORD cb = sizeof(rkiulIn) +
		rkiulIn.DomainName.Length +
		rkiulIn.AccountName.Length +
		rkiulIn.OldPassword.Length +
		rkiulIn.NewPassword.Length;

	KERB_CHANGEPASSWORD_REQUEST* pkiulOut = (KERB_CHANGEPASSWORD_REQUEST*)CoTaskMemAlloc(cb);

	if(pkiulOut)
	{
		ZeroMemory(pkiulOut, cb);

		///
		/// point pbBuffer at the beginning of the extra space
		///
		BYTE* pbBuffer = (BYTE*)pkiulOut + sizeof(*pkiulOut);

		pkiulOut->MessageType = rkiulIn.MessageType;

		///
		/// Copy each string
		/// fix up appropriate buffer pointer to the offset
		/// advance buffer pointer over copied character space
		///
 
		_UnicodeStringPackedUnicodeStringCopy(rkiulIn.DomainName, (PWSTR)pbBuffer, &pkiulOut->DomainName);
		pkiulOut->DomainName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkiulOut->DomainName.Length;

		_UnicodeStringPackedUnicodeStringCopy(rkiulIn.AccountName, (PWSTR)pbBuffer, &pkiulOut->AccountName);
		pkiulOut->AccountName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkiulOut->AccountName.Length;


		_UnicodeStringPackedUnicodeStringCopy(rkiulIn.OldPassword, (PWSTR)pbBuffer, &pkiulOut->OldPassword);
		pkiulOut->OldPassword.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkiulOut->OldPassword.Length;

		_UnicodeStringPackedUnicodeStringCopy(rkiulIn.NewPassword, (PWSTR)pbBuffer, &pkiulOut->NewPassword);
		pkiulOut->NewPassword.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
		pbBuffer += pkiulOut->NewPassword.Length;

		*prgb = (BYTE*)pkiulOut;
		*pcb = cb;

		hr = S_OK;
	}
	else
	{
		DEBUG_LOG("KERB INTERACTIVE CHANGE PASS PACK : OUT OF MEMORY [%d]", cb);
		hr = E_OUTOFMEMORY;
	}

	return hr;
}


BYTE* UnicodeBufferCopy(const UNICODE_STRING source, BYTE* buffer, UNICODE_STRING* outBuffer, void* bufferStart)
{
	_UnicodeStringPackedUnicodeStringCopy(source, (PWSTR)buffer, outBuffer);
	outBuffer->Buffer = (PWSTR)(buffer - (BYTE*)bufferStart);
	return (buffer + outBuffer->Length);
}

HRESULT MiTokenInteractiveLogonPack(
	const KERB_INTERACTIVE_UNLOCK_LOGON& rkiulIn,
	BYTE** prgb,
	DWORD* pcb,
	PWSTR otp,
	int MiTokenAPID)
{
	UNICODE_STRING ucOTP;

	DEBUG_SENSITIVEW(true, L"MTILP : OTP = %s\n", otp);
	UnicodeStringInitWithString(otp, &ucOTP);

	DEBUG_SENSITIVE(true, "Converted OTP : [%d]", ucOTP.Length);
	DEBUG_SENSITIVEBYTES(true, (unsigned char*) ucOTP.Buffer, ucOTP.Length);
	DEBUG_SENSITIVE(true, "\n");

	HRESULT hr;

	const KERB_INTERACTIVE_LOGON* pkilIn = &rkiulIn.Logon;
	DWORD cb = sizeof(CREDS_WITH_2FA) +
		pkilIn->LogonDomainName.Length +
		pkilIn->UserName.Length +
		pkilIn->Password.Length +
		ucOTP.Length;

	CREDS_WITH_2FA* pkiulOut = (CREDS_WITH_2FA*)CoTaskMemAlloc(cb);

	if(pkiulOut)
	{
		ZeroMemory(&pkiulOut->LogonId, sizeof(LUID));

		BYTE* pbBuffer = (BYTE*)pkiulOut + sizeof(*pkiulOut);

		KERB_INTERACTIVE_LOGON* pkilOut = &pkiulOut->Logon;

		//The passType attribute is what type the inner pkilOut structure contains
		pkiulOut->passType = pkilIn->MessageType;
		//The MessageType is MESSAGE_WITH2FA which is a Mi-Token AP Custom Message ID.
		pkilOut->MessageType = (KERB_LOGON_SUBMIT_TYPE)MESSAGE_WITH2FA;
		
		pbBuffer = UnicodeBufferCopy(pkilIn->LogonDomainName, pbBuffer, &pkilOut->LogonDomainName, pkiulOut);
		pbBuffer = UnicodeBufferCopy(pkilIn->UserName, pbBuffer, &pkilOut->UserName, pkiulOut);
		pbBuffer = UnicodeBufferCopy(pkilIn->Password, pbBuffer, &pkilOut->Password, pkiulOut);
		pbBuffer = UnicodeBufferCopy(ucOTP, pbBuffer, &pkiulOut->OTP, pkiulOut);

		*prgb = (BYTE*)pkiulOut;
		*pcb = cb;

		
		hr = S_OK;
		
	}

	else
	{
		DEBUG_LOG("MiTokenAP Interactive Logon Pack : Out of Memory [%d]", cb);
		hr = E_OUTOFMEMORY;
	}
	return hr;


}


//
// WinLogon and LSA consume "packed" KERB_INTERACTIVE_LOGONs.  In these, the PWSTR members of each
// UNICODE_STRING are not actually pointers but byte offsets into the overall buffer represented
// by the packed KERB_INTERACTIVE_LOGON.  For example:
// 
// kil.LogonDomainName.Length = 14                             -> Length is in bytes, not characters
// kil.LogonDomainName.Buffer = sizeof(KERB_INTERACTIVE_LOGON) -> LogonDomainName begins immediately
//                                                                after the KERB_... struct in the buffer
// kil.UserName.Length = 10
// kil.UserName.Buffer = sizeof(KERB_INTERACTIVE_LOGON) + 14   -> UNICODE_STRINGS are NOT null-terminated
//
// kil.Password.Length = 16
// kil.Password.Buffer = sizeof(KERB_INTERACTIVE_LOGON) + 14 + 10
// 
// THere's more information on this at:
// http://msdn.microsoft.com/msdnmag/issues/05/06/SecurityBriefs/#void
//

HRESULT KerbInteractiveLogonPack(
    const KERB_INTERACTIVE_UNLOCK_LOGON& rkiulIn,
    BYTE** prgb,
    DWORD* pcb
    )
{
    HRESULT hr;

	const KERB_INTERACTIVE_LOGON* pkilIn = &rkiulIn.Logon;
    // alloc space for struct plus extra for the three strings
    DWORD cb = sizeof(rkiulIn) +
        pkilIn->LogonDomainName.Length +
        pkilIn->UserName.Length +
        pkilIn->Password.Length;

	KERB_INTERACTIVE_UNLOCK_LOGON* pkiulOut = (KERB_INTERACTIVE_UNLOCK_LOGON*)CoTaskMemAlloc(cb);
    
    if (pkiulOut)
    {
		ZeroMemory(&pkiulOut->LogonId, sizeof(LUID));

		//
		// point pbBuffer at the beginning of the extra space
		//
		BYTE* pbBuffer = (BYTE*)pkiulOut + sizeof(*pkiulOut);

		//
		// set up the Logon structure within the KERB_INTERACTIVE_UNLOCK_LOGON
		//
		KERB_INTERACTIVE_LOGON* pkilOut = &pkiulOut->Logon;

		pkilOut->MessageType = pkilIn->MessageType;

		//
		// copy each string
		// fix up appropriate buffer pointer to be offset,
		// advance buffer pointer over copied characters in extra space
		//
        _UnicodeStringPackedUnicodeStringCopy(pkilIn->LogonDomainName, (PWSTR)pbBuffer, &pkilOut->LogonDomainName);
        pkilOut->LogonDomainName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
        pbBuffer += pkilOut->LogonDomainName.Length;

        _UnicodeStringPackedUnicodeStringCopy(pkilIn->UserName, (PWSTR)pbBuffer, &pkilOut->UserName);
        pkilOut->UserName.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);
        pbBuffer += pkilOut->UserName.Length;

        _UnicodeStringPackedUnicodeStringCopy(pkilIn->Password, (PWSTR)pbBuffer, &pkilOut->Password);
        pkilOut->Password.Buffer = (PWSTR)(pbBuffer - (BYTE*)pkiulOut);

        *prgb = (BYTE*)pkiulOut;
        *pcb = cb;

        hr = S_OK;
    }
    else
    {
		DEBUG_LOG("KERB INTERACTIVE LOGON PACK : OUT OF MEMORY [%d]", cb);
		hr = E_OUTOFMEMORY;
    }

    return hr;
}

// 
// Unpack a KERB_INTERACTIVE_UNLOCK_LOGON *in place*.  That is, reset the Buffers from being offsets to
// being real pointers.  This means, of course, that passing the resultant struct across any sort of 
// memory space boundary is not going to work -- repack it if necessary!
//
//
// Unpack a KERB_INTERACTIVE_UNLOCK_LOGON *in place*.  That is, reset the Buffers from being offsets to
// being real pointers.  This means, of course, that passing the resultant struct across any sort of 
// memory space boundary is not going to work -- repack it if necessary!
//
void KerbInteractiveLogonUnpackInPlace(
    __inout_bcount(cb) KERB_INTERACTIVE_UNLOCK_LOGON* pkiul
    )
{
    KERB_INTERACTIVE_LOGON* pkil = &pkiul->Logon;

    pkil->LogonDomainName.Buffer = pkil->LogonDomainName.Buffer
        ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->LogonDomainName.Buffer)
        : NULL;

    pkil->UserName.Buffer = pkil->UserName.Buffer
        ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->UserName.Buffer)
        : NULL;

    pkil->Password.Buffer = pkil->Password.Buffer 
        ? (PWSTR)((BYTE*)pkiul + (ULONG_PTR)pkil->Password.Buffer)
        : NULL;
}

// 
// This function packs the string pszSourceString in pszDestinationString
// for use with LSA functions including LsaLookupAuthenticationPackage.
//
HRESULT LsaInitString(PSTRING pszDestinationString, PCSTR pszSourceString)
{
    size_t usLength;
    HRESULT hr = StringCchLength(/*(STRSAFE_LPCWSTR)*/pszSourceString, USHORT_MAX, (size_t *) &usLength);
    if (SUCCEEDED(hr))
    {
        pszDestinationString->Buffer = (PCHAR)pszSourceString;
        pszDestinationString->Length = (USHORT)usLength;
        pszDestinationString->MaximumLength = pszDestinationString->Length+1;
        hr = S_OK;
    }
    return hr;
}



//Same as RetrieveNegotiateAuthPackage but gets the value for the MiTokenAP Package instead of Kerberos
HRESULT RetrieveMiTokenAPPackage(ULONG* pulAuthPackage)
{
    HRESULT hr;
    HANDLE hLsa;

    NTSTATUS status = LsaConnectUntrusted(&hLsa);
    if (SUCCEEDED(HRESULT_FROM_NT(status)))
    {
        
        ULONG ulAuthPackage;
        LSA_STRING lsaszKerberosName;
        LsaInitString(&lsaszKerberosName, "MiTokenAP");

        status = LsaLookupAuthenticationPackage(hLsa, &lsaszKerberosName, &ulAuthPackage);
        if (SUCCEEDED(HRESULT_FROM_NT(status)))
        {
            *pulAuthPackage = ulAuthPackage;
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_NT(status);
        }
        LsaDeregisterLogonProcess(hLsa);
    }
    else
    {
        hr= HRESULT_FROM_NT(status);
    }

    return hr;
}

//
// Retrieves the 'negotiate' AuthPackage from the LSA. In this case, Kerberos
// For more information on auth packages see this msdn page:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/secauthn/security/msv1_0_lm20_logon.asp
//
HRESULT RetrieveNegotiateAuthPackage(ULONG * pulAuthPackage)
{
    HRESULT hr;
    HANDLE hLsa;

    NTSTATUS status = LsaConnectUntrusted(&hLsa);
    if (SUCCEEDED(HRESULT_FROM_NT(status)))
    {
        
        ULONG ulAuthPackage;
        LSA_STRING lsaszKerberosName;
        LsaInitString(&lsaszKerberosName, NEGOSSP_NAME);

        status = LsaLookupAuthenticationPackage(hLsa, &lsaszKerberosName, &ulAuthPackage);
        if (SUCCEEDED(HRESULT_FROM_NT(status)))
        {
            *pulAuthPackage = ulAuthPackage;
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_NT(status);
        }
        LsaDeregisterLogonProcess(hLsa);
    }
    else
    {
        hr= HRESULT_FROM_NT(status);
    }

    return hr;
}
