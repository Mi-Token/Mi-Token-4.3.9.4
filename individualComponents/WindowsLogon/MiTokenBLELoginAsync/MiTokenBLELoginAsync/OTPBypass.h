#pragma once

//Used for CString
#include <atlstr.h>



bool checkOTPBypass(CString username, CString OTP, TCHAR * keyPath, CString domainAndUser);
void updateBypassCodesFromServer(void* APIData, int length);

