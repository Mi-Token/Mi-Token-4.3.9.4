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

#pragma once
#include "common.h"
#include <windows.h>
#pragma warning (push)
#pragma warning (disable:4995)
#include <tchar.h>
#include <strsafe.h>
#pragma warning (pop)

#pragma warning(push)
#pragma warning(disable : 4995)
#include <shlwapi.h>
#pragma warning(pop)


#define MESSAGE_WITH2FA 0x2000

typedef struct _CREDS_WITH_2FA : public KERB_INTERACTIVE_UNLOCK_LOGON
{
	UNICODE_STRING OTP;
	KERB_LOGON_SUBMIT_TYPE passType;
}CREDS_WITH_2FA;


//makes a copy of a field descriptor using CoTaskMemAlloc
HRESULT FieldDescriptorCoAllocCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    );

//makes a copy of a field descriptor on the normal heap
HRESULT FieldDescriptorCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd
    );

//creates a UNICODE_STRING from a normal string
HRESULT UnicodeStringInitWithString(
    PWSTR pwz, 
    UNICODE_STRING* pus
    );

HRESULT KerbInteractiveChangePassPack(
	const KERB_CHANGEPASSWORD_REQUEST& rkiulIn,
	BYTE** prgb,
	DWORD* pcb
	);

HRESULT MiTokenInteractiveLogonPack(
	const KERB_INTERACTIVE_UNLOCK_LOGON& rkiulIn,
	BYTE** prgb,
	DWORD* pcb,
	PWSTR otp,
	int MiTokenAPID);

//packages the credentials into the buffer that the system expects
HRESULT KerbInteractiveLogonPack(
    const KERB_INTERACTIVE_UNLOCK_LOGON& rkil,
    BYTE** prgb,
    DWORD* pcb
    );

//unpackages the "packed" version of the creds in-place into the "unpacked" version
void KerbInteractiveLogonUnpackInPlace(
    KERB_INTERACTIVE_UNLOCK_LOGON* pkiul
    );

HRESULT RetrieveMiTokenAPPackage(ULONG* pulAuthPackage);

//get the authentication package that will be used for our logon attempt
HRESULT RetrieveNegotiateAuthPackage(
    ULONG * pulAuthPackage
    );

