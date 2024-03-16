#pragma once

void MemLeak_SetLocationID(int locationID);

void MemLeak_CheckMemChanges(void);

#ifdef UNICODE
void MemLeak_Init(wchar_t* pipeName);
#else
void MemLeak_Init(char* pipeName);
#endif


