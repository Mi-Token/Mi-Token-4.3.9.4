#pragma once

#ifndef _MultiString_H_
#define _MultiString_H_


#include <TCHAR.h>

class MultiString
{
public:
	MultiString();
	MultiString(MultiString& other);
	~MultiString();
	void operator = (MultiString& other);
	void operator = (char* source);
	void operator = (wchar_t* source);

	//Returns the internal * value - DO NOT FREE
	const char* getChar();
	const wchar_t* getWChar();
	const TCHAR* getTChar();

	int length();

	void copyToChar(char* dest, int maxLen);
	void copyToWChar(wchar_t* dest, int maxLen);

	//Allocates memory for a copy of the internal * value - you must free
	char* getCharCopy();
	wchar_t* getWCharCopy();

private:
	char* _mCharStr;
	wchar_t* _mWCharStr;
	int clen;

	void _safeFree();
	void _copyFromOther(MultiString& other);

};

#endif