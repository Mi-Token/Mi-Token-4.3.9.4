#include "MultiString.h"

#include <string.h>
#include <stdlib.h>


#ifndef SAFE_FREE
#define SAFE_FREE(p) if(p) { free(p); p = NULL; }
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

MultiString::MultiString()
{
	_mCharStr = NULL;
	_mWCharStr = NULL;
	clen = 0;
}

MultiString::MultiString(MultiString& other)
{
	_mCharStr = NULL;
	_mWCharStr = NULL;
	clen = 0;

	_copyFromOther(other);
	
}

MultiString::~MultiString()
{
	_safeFree();
}

void MultiString::operator=(MultiString& other)
{
	_copyFromOther(other);
}

void MultiString::operator=(char* source)
{
	_safeFree();

	clen = strlen(source);
	_mCharStr = (char*)malloc((clen + 1) * sizeof(char));
	memcpy(_mCharStr, source, (clen + 1) * sizeof(char));

	_mWCharStr = (wchar_t*)calloc((clen + 1), sizeof(wchar_t));
	mbstowcs(_mWCharStr, _mCharStr, clen);
}

void MultiString::operator=(wchar_t* source)
{
	_safeFree();

	clen = wcslen(source);
	_mWCharStr = (wchar_t*)malloc((clen + 1) * sizeof(wchar_t));
	memcpy(_mWCharStr, source, (clen + 1) * sizeof(wchar_t));

	_mCharStr = (char*)calloc((clen + 1),  sizeof(char));
	wcstombs(_mCharStr, _mWCharStr, clen);
}

const char* MultiString::getChar()
{
	return _mCharStr;
}

const wchar_t* MultiString::getWChar()
{
	return _mWCharStr;
}

const TCHAR* MultiString::getTChar()
{
#ifdef UNICODE
	return getWChar();
#else
	return getChar();
#endif
}

int MultiString::length()
{
	return clen;
}

void MultiString::copyToChar(char* dest, int maxLen)
{
	if(_mCharStr == NULL)
	{
		return;
	}
	int bytesToCopy = MIN(clen + 1, maxLen);
	bytesToCopy *= sizeof(char);
	memcpy(dest, _mCharStr, bytesToCopy);
}

void MultiString::copyToWChar(wchar_t* dest, int maxLen)
{
	if(_mWCharStr == NULL)
	{
		return;
	}
	int bytesToCopy = MIN(clen + 1, maxLen);
	bytesToCopy *= sizeof(wchar_t);
	memcpy(dest, _mWCharStr, bytesToCopy);
}

char* MultiString::getCharCopy()
{
	if(_mCharStr == NULL)
	{
		return NULL;
	}
	char* output = (char*)malloc((clen + 1) * sizeof(char));
	memcpy(output, _mCharStr, (clen + 1) * sizeof(char));
	return output;
}

wchar_t* MultiString::getWCharCopy()
{
	if(_mWCharStr == NULL)
	{
		return NULL;
	}
	wchar_t* output = (wchar_t*)malloc((clen + 1) * sizeof(wchar_t));
	memcpy(output, _mWCharStr, (clen + 1) * sizeof(wchar_t));
	return output;
}

void MultiString::_safeFree()
{
	SAFE_FREE(_mCharStr);
	SAFE_FREE(_mWCharStr);
	clen = 0;
}

void MultiString::_copyFromOther(MultiString& other)
{
	_safeFree();

	if(other._mCharStr)
	{
		clen = other.clen;
		_mCharStr = (char*)calloc(clen + 1, sizeof(char));
		_mWCharStr = (wchar_t*)calloc(clen + 1, sizeof(wchar_t));
		memcpy(_mCharStr, other._mCharStr, (clen + 1) * sizeof(char));
		memcpy(_mWCharStr, other._mWCharStr, (clen + 1) * sizeof(wchar_t));
	}
}