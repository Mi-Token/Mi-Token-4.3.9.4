#pragma once

#ifndef _BLE_API_RETURNS_H_
#define _BLE_API_RETURNS_H_

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned long uint32;
typedef signed char int8;

typedef struct mac_address_t
{
	uint8 addr[6];
}mac_address;

//#define for compatibility - will be removed when all references to bd_addr_t are gone
//#define bd_addr_t mac_address_t
//#define bd_addr mac_address

enum BLE_API_RET : int
{
	BLE_API_SUCCESS = 0,
	BLE_API_MORE_DATA = -1,
	BLE_API_ERR_NULL_POINTER = -2,
	BLE_API_ERR_NO_SUCH_DEVICE = -3,
	BLE_API_ERR_NO_SUCH_META_FLAG = -4,
	BLE_API_ERR_FAILED_GETTING_META_DATA = -5,

	BLE_API_ERR_FAILED_TO_OPEN_COM_PORT = -6,


	BLE_API_ERR_NULL_INTERFACE = -7,
	BLE_API_ERR_NULL_INTERNAL_INTERFACE = -8,

	BLE_API_ERR_INVALID_DEVICE_ID = -9,

	BLE_API_ERR_SEARCH_FINISHED = -10,

	BLE_API_ERR_INTERFACE_TO_BE_DELETED = -11,

	BLE_API_ERR_CLIENT_STILL_HAS_COM = -12,

	BLE_API_ERR_UNKNOWN = -255,
	BLE_API_ERR_FUNCTION_NOT_IMPLEMENTED = -256,
	BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED = -257,
};

enum BLE_CONN_RET : int
{
	BLE_CONN_NEW_REQUEST = 0,
	BLE_CONN_SUCCESS = 1,
	BLE_CONN_BAD_REQUEST = 0,
	BLE_CONN_ERR_NOT_CONNECTED = -1,
	BLE_CONN_ERR_NO_SUCH_CONNECTION = -2,
	BLE_CONN_ERR_NO_SUCH_SERVICE = -3,
	BLE_CONN_ERR_SERVICES_NOT_SCANNED = -4,
	BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS = -5,
	BLE_CONN_ERR_NO_SUCH_ATTRIBUTE = -6,
	BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS = -7,
	BLE_CONN_ERR_ATTRIBUTE_BEING_READ = -8,
	BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA = -9,
	BLE_CONN_ERR_CONNECTION_TIMED_OUT = -10,	
	//new error codes as of Version 2.2

	BLE_CONN_ERR_NOT_IMPLEMENTED = -11,
	BLE_CONN_ERR_SYNC_TIMEOUT = -12,
	BLE_CONN_ERR_INTERNAL_ERROR = -13,
};

enum BLE_API_NP_RETURNS : int
{
	BLE_API_NP_SUCCESS = BLE_CONN_SUCCESS,
	
	BLE_API_NP_ERR_UNKNOWN = BLE_API_ERR_UNKNOWN,
	BLE_API_NP_ERR_NULL_INTERNAL_INTERFACE = BLE_API_ERR_NULL_INTERNAL_INTERFACE,

	BLE_API_EXCLUSIVE_ALLOWED = 1,
	BLE_API_EXCLUSIVE_ASKED = -20,
	BLE_API_EXCLUSIVE_REJECTED = -21,
	BLE_API_NP_ERR_NOT_CLIENT = -22,
	BLE_API_EXCLUSIVE_DISABLED = -23,
	BLE_API_NP_INTERNAL_ERROR = -24,
	BLE_API_NP_BAD_EXCLUSIVE_STATE = -25,
	BLE_API_NP_COM_FALLBACK = -26,
	BLE_API_NP_ERR_NOT_CONNECTED = -27,
};

enum BLE_BOND_RET: int
{
    BOND_RET_SUCCESS = 0,
    BOND_RET_FAILED,
    BOND_RET_ERR_TIMEOUT,
    BOND_RET_ERR_PREVBOND_MAXBOND,
};

#pragma pack(push)
#pragma pack(1)
struct DeviceInfo
{
	unsigned char address[6];
	char RSSI;
	unsigned int lastSeen;
};
#pragma pack(pop)

struct DeviceData
{
	unsigned char flag;
	unsigned char length;
	unsigned char* data;
};

typedef void *FilteredSearchObject;
typedef void* MessageBufferObject;


typedef int REQUEST_ID;
typedef uint16 ATTRIBUTE_HANDLE;
typedef uint16 CHARACTERISTIC_HANDLE;
typedef uint16 SERVICE_HANDLE;
typedef uint16 SERVICE_GUID_HANDLE;


#ifdef USE_APITYPES_HEADER
#include "BLE_API_ExtraTypes.h"
#endif

#endif