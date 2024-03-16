#pragma once


#include <Windows.h>

bool GetMTC_SDDL_String(char** stringData, bool allowAdmins);

#define MTC_SDDL_STRING_LS_ONLY "O:SYG:SYD:(A;OICI;GA;;;SY)" //Owner/Group - Local System, No DACL, Allow (Object Inherit, Container Inherit, General Allow for Local System)
#define MTC_SDDL_STRING_LS_ADMIN_ONLY "O:SYG:SYD:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)" //Owner/Group - Local System, No DACL, Allow (Object Inherit, Container Inherit, General Allow for Local System)

bool SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);



