#include "BLE_API.h"


enum StageEnums
{
	Initializing,
	LookingForToken,
	AuthenticatingToken,
	WaitingForUsername,
};

class BLETokenLogin
{
public:
	static BLETokenLogin* getInstance();

	void SendOnlineMessage();
	void SendLoggedInMessage();
	void SendUserPassMessage(char* username, int userLen, char* password, int passLen);
	void SendUserPassMessage(wchar_t* username, int usernameLen, wchar_t* passsword, int passwordLen);
	void SendUserPassMessage(wchar_t* username, wchar_t* passsword);
	void SetGotUserPassCallback(void (*callback) (unsigned char* username, int userLen, unsigned char* password, int passLen, bool needPassword, void* LPVOID), void* LPVOID);

	bool isSameUsername(char* username);
	bool isSameUsername(wchar_t* wusername);
	bool isSamePassword(char* password);

	const char* getUsername();
	const char* getPassword();

	bool canAutoLogin();
	bool hasUsername(wchar_t* pUsername);
	bool hasPassword(wchar_t* pPassword);
	bool hasDevice();
	bool authingDevice();
	void clearUsernamePassword();

	void passwordMustChange();
	void passwordHasBeenChanged();

	StageEnums getCurrentStage();

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

	StageEnums _myStage;

	wchar_t *_wusername, *_wpassword;
	
};