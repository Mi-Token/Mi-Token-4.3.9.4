#pragma once

#ifndef _MiTokenBLEChild_H_
#define _MiTokenBLEChild_H_

class MiTokenBLE;
#include "SafeObject.h"

#define MiTokenBLEChild SafeObject<MiTokenBLE>
//Allows for better control over who gets access to the MiTokenBLE object
/*
class MiTokenBLEChild : public SafeObject<MiTokenBLE>
{
public:
	/*
	MiTokenBLEChild(MiTokenBLE* parent);
	MiTokenBLEChild(MiTokenBLEChild& ref);
	void operator = (MiTokenBLEChild& ref);
	~MiTokenBLEChild();
	MiTokenBLE* GetRoot();

protected:
	MiTokenBLE* _parent;
	*//*
};
/**/
#endif