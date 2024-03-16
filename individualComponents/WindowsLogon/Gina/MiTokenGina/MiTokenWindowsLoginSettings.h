#include <atlstr.h>
#define MAX_SERVERS 10
struct ServerDetails
{
	CString name;
	CString secret;
	INT port;
};

struct RegistryValues
{
	INT TryServersinOrder;
	struct ServerDetails servers[MAX_SERVERS];
};



//
//void LoadMiTokenGINARegistryValues();

class MiTokenWindowsLoginSettings
{
	MiTokenWindowsLoginSettings();
	void LoadMiTokenGINARegistryValues();
	void LoadRegistryValues();
	/*{
		RegOpenKeyEx (HKEY_LOCAL_MACHINE,key,NULL,KEY_READ,&hKey);
	}*/

};