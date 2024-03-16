#include "BLETokenLogin.h"
#include <stdio.h>
#include <string>



char flushInput(char c)
{
	while(c != 10)
	{
		c = getchar();
	}
	return c;
}

class TestClass
{
public:
	void ProcessMessage(unsigned char* username, int usernameLen, unsigned char* password, int passwordLen, bool needPassword);
protected:
};


void TestClass::ProcessMessage(unsigned char* username, int usernameLen, unsigned char* password, int passwordLen, bool needPassword)
{
	wchar_t *wuser = nullptr, *wpass = nullptr;
	BLETokenLogin* login = BLETokenLogin::getInstance();

	int userlen = 0, passlen = 0;
	
	if(login->hasUsername(&wuser))
	{
		userlen = wcslen(wuser);
	}
	if(needPassword = !login->hasPassword(&wpass), !needPassword)
	{
		passlen = wcslen(wpass);
	}

	

	printf("TestClass : \r\n\tGot Username of Length : %d\r\n\t[", userlen);
	for(int i = 0 ;  i < userlen ;++i)
	{
		wprintf(L"%c", wuser[i]);
	}
	printf("]\r\n\tGot Password of Length : %d\r\n\t[", passlen);
	for(int i = 0 ; i < passlen ; ++i)
	{
		printf("%c", wpass[i]);
	}
	printf("]\r\n");

	if(!needPassword)
	{
		login->SendUserPassMessage(wuser, wpass);
	}
	else
	{
		printf("Password for user : ");
		//char c = getchar();
		login->SendUserPassMessage(wuser, L"P@55word");
		printf("\r\nPassword sent\r\n");
	}
}

void StaticGotUserAndPass (unsigned char* username, int userLen, unsigned char* password, int passLen, bool needPassword, void* LPVOID)
{
	((TestClass*)LPVOID)->ProcessMessage(username, userLen, password, passLen, needPassword);
}


int main()
{
	TestClass* TC = new TestClass();

	BLETokenLogin* tokenLogin = BLETokenLogin::getInstance();
	tokenLogin->SetGotUserPassCallback(StaticGotUserAndPass, (void*)TC);

	bool running = true;
	while(running)
	{
		char c = getchar();
		if((c >= '0') && (c <= '9'))
		{
		}
		else
		{
			c |= 0x20; //make c lowercase
			switch(c)
			{
			case 'o':
				printf("Sending CP Online message");
				tokenLogin->SendOnlineMessage();
				break;
			case 'd':
				printf("Sending CP Done Message");
				tokenLogin->SendLoggedInMessage();
				break;
			case 'u':
				c = flushInput(c);
				{
					printf("Input username : ");
					char username[128] = {0};
					char password[128] = {0};
					int userLen = 0, passLen = 0;
					while(c = getchar(), (c != 10) && (userLen != 128))
					{
						username[userLen] = c;
						userLen++;
					}
					username[userLen] = 0;
					c = flushInput(c);
					printf("Input password : ");
					while(c = getchar(), (c != 10) && (passLen != 128))
					{
						password[passLen] = c;
						passLen++;
					}
					password[passLen] = 0;
					c = flushInput(c);
					tokenLogin->SendUserPassMessage(username, userLen, password, passLen);
				}

			}

		}
		c = flushInput(c);
	}
}