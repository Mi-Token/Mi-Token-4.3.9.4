#include "MiTokenBLE.h"
#include "MiTokenBLEChild.h"
/*
MiTokenBLEChild::MiTokenBLEChild(MiTokenBLE* parent)
{
	_parent = parent;
	_parent->RegisterNewChild();
}

MiTokenBLEChild::MiTokenBLEChild(MiTokenBLEChild& ref)
{
	_parent = ref._parent;
	_parent->RegisterNewChild();
}

void MiTokenBLEChild::operator=(MiTokenBLEChild& ref)
{
	if (_parent != nullptr)
	{
		_parent->UnregisterChild();
		_parent = nullptr;
	}

	_parent = ref._parent;
	_parent->RegisterNewChild();
}

MiTokenBLEChild::~MiTokenBLEChild()
{
	if (_parent != nullptr)
	{
		_parent->UnregisterChild();
		_parent = nullptr;
	}
}

MiTokenBLE* MiTokenBLEChild::GetRoot()
{
	return _parent;
}*/