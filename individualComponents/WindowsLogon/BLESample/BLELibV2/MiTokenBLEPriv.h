#pragma once

#ifndef _MITOKENBLEPRIV_H_
#define _MITOEKNBLEPRIV_H_

#include "BLE_API.h"


#include "BLE_IO.h"
#include "BLE_Structs.h"


#ifndef MiTokenBLE_ClasslessMode
class MiTokenBLEPriv
{
public:
	MiTokenBLEPriv(MiTokenBLE* parent);
	IBLE_IO* BLE_IO;
	PollData* rootPoll;

protected:
	MiTokenBLE* _parent;
};
#endif
#endif