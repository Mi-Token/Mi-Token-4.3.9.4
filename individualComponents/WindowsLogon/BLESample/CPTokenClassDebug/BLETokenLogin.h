#include "BLE_API.h"

class BLETokenLogin
{
public:
	static BLETokenLogin* getInstance();

	void SendOnlineMessage();
	void SendLoggedInMessage();
	void SendUserPassMessage(char* username, int userLen, char* password, int passLen);
	void SetGotUserPassCallback(void (*callback) (unsigned char* username, int userLen, unsigned char* password, int passLen, bool needPassword, void* LPVOID), void* LPVOID);

	bool isSameUsername(char* username);
	bool isSamePassword(char* password);

	const char* getUsername();
	const char* getPassword();

protected:
	static bool isSameString(char* str1, char* str2);
	void _setUsernameAndPassword(unsigned char* username, int usernameLen, unsigned char* password, int passwordLen);
	char* _username, *_password;
	void* _LPVOID;
	BLETokenLogin();
	IMiTokenBLEV2_2* _instance;
	void gotMessage(unsigned char* data, int length);
	void (*gotUserPassCallback) (unsigned char* username, int userLen, unsigned char* password, int passLen, bool needPassword, void* LPVOID);
	static void Pipe_Callback(IMiTokenBLE* sender, unsigned char* data, int length, int connectionID);
	static BLETokenLogin* MyInstance;


};