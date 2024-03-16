#pragma once

#ifndef _SAFE_OBJECT_H_
#define _SAFE_OBJECT_H_

#include <Windows.h>

class BaseSafeObject
{
public:
	BaseSafeObject();

	void AddRef();
	void RemRef();

protected:

	int _refCount;
	HANDLE _refMutex;
	HANDLE _safeToDel;
	void waitUntilSafe();

protected:
	~BaseSafeObject();
	BaseSafeObject(BaseSafeObject& ref);
	void operator=(BaseSafeObject& ref);
};

template <class T>
class SafeObject
{
public:
	SafeObject()
	{
		_root = nullptr;
	}
	SafeObject(T* parent)
	{
		_root = parent;
		SafeAddRef();
	}
	SafeObject(SafeObject<T>& ref)
	{
		_root = ref._root;
		SafeAddRef();
	}
	void operator=(T* ref)
	{
		SafeRemRef();
		_root = ref;
		SafeAddRef();
	}
	void operator=(SafeObject<T>& ref)
	{
		SafeRemRef();
		_root = ref._root;
		SafeAddRef();
	}
	~SafeObject()
	{
		SafeRemRef();
		_root = nullptr;
	}

	T * GetRoot()
	{
		return _root;
	}
protected:
	void SafeAddRef()
	{
		if (_root != nullptr)
		{
			dynamic_cast<BaseSafeObject*>(_root)->AddRef();
		}
	}
	void SafeRemRef()
	{
		if (_root != nullptr)
		{
			dynamic_cast<BaseSafeObject*>(_root)->RemRef();
		}
	}

private:
	T* _root;
};

#endif