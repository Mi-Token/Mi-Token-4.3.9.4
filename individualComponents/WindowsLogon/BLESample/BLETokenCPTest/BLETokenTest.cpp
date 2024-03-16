#include <stdio.h>
#include <stdlib.h>

#include "BLEToken.h"

/*
class Provider
{
	void OnConnectStatusChanged()
	{};

};*/

int main()
{
	BLEToken* pToken = new BLEToken(nullptr);

	while(true)
	{
		Sleep(1000);
	}
}