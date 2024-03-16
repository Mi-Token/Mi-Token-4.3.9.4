#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "BLE_API.h"
#include "DLLWrapper.h" //used to make certain DLL calls easier
#include "DebugLogging.h"


#ifdef BLEDLLV2_EXPORTS
#define BLEDLLV2_API __declspec(dllexport)
#else
#define BLEDLLV2_API __declspec(dllimport)
#endif

#define DLL_VERSION_MAG (0x0002)
#define DLL_VERSION_MIN (0x02)
#define DLL_VERSION_REV (0x20)

#define MIN_WRAPPER_VERSION_MAG (0x0002)
#define MIN_WRAPPER_VERSION_MIN (0x02)
#define MIN_WRAPPER_VERSION_REV (0x03)

