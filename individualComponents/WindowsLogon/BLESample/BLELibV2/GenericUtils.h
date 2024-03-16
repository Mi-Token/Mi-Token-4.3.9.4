#pragma once
#include <Windows.h>

void NameThread(LPCSTR name);

HANDLE CreateNamedThread(__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,__in SIZE_T dwStackSize, __in LPTHREAD_START_ROUTINE lpStartAddress, __in_opt __deref __drv_aliasesMem LPVOID lpParameter, __in DWORD dwCreationFlags, __out_opt LPDWORD lpThreadId, __in LPCSTR ThreadName);