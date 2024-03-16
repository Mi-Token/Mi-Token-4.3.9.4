#include "SDDLs.h"
#include "DebugLogging.h"

#pragma comment(lib, "advapi32.lib")

bool GetMTC_SDDL_String(char** stringData, bool allowAdmins)
{
#ifdef _DEBUG
	allowAdmins = true;
#endif

	const char* source = (allowAdmins ? MTC_SDDL_STRING_LS_ADMIN_ONLY : MTC_SDDL_STRING_LS_ONLY);
	//just copy over MTC_SDDL_STRING, it contains everything we need
	*stringData = (char*)malloc(strlen(source) + 1);
	memcpy(*stringData, source, strlen(source) + 1);
	return true;
}

bool SetPrivilege(HANDLE hToken, LPCSTR lpszPrivilege, BOOL enablePrivilege)
{
	//Note : SetPrivilege is not being run, we don't need it.
	return true;

	TOKEN_PRIVILEGES tp;
	LUID luid;

	if(!LookupPrivilegeValueA(NULL, lpszPrivilege, &luid))
	{
		DEBUG_VERBOSE("LookupPrivilegeValue Error : %u\n",GetLastError());
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(enablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)nullptr, (PDWORD)nullptr))
	{
		DEBUG_VERBOSE("Adjust Token privileges failed : GLE = %u\r\n", GetLastError());
		return false;
	}
	else if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		DEBUG_VERBOSE("Error : Token Does not have the specified privilege. \r\n");
		return false;
	}

	DEBUG_VERBOSE("Privilege Changed\r\n");
	return true;
}