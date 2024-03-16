#pragma once

void PrintError(LPCWSTR function, LPCWSTR message);
void PrintDebug(LPCWSTR message);

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define ERROR_WIN32(msg) PrintError(__WFILE__, TEXT(msg))
#define DEBUG_PRINT(msg) PrintDebug(msg);