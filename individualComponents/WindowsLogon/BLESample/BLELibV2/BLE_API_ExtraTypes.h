#pragma once

#ifdef __GNUC__

#define PACKSTRUCT( decl ) decl __attribute__((__packed__))
#define ALIGNED __attribute__((aligned(0x4)))

#else //msvc

#define PACKSTRUCT( decl ) __pragma( pack(push, 1) ) decl __pragma( pack(pop) )
#define ALIGNED

#endif


#ifndef _BLE_API_EXTRATYPES_H_
#define _BLE_API_EXTRATYPES_H_

typedef mac_address hwaddr;

#ifndef UINT8ARR
typedef struct
{
	uint8 len;
	uint8 data[];
}uint8array;
#define UINT8ARR
#endif

typedef struct
{
	uint8 len;
	int8 data[];
}string;



#endif