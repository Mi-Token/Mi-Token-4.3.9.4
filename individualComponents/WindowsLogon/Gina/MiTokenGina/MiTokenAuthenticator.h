//Used as a hard switch on logging. Logging requires both this and the registry key to be set
#define ENABLE_LOGGING

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define ERROR_HSESSION_NULL	1
#define ERROR_REGKEY_EXISTENTIAL_CRISIS	2
#define ERROR_REGVALUE_NOT_RETRIEVED 3
#define ERROR_NO_SERVERS_FOUND 4
#define ERROR_SERVER_NAME_INCORRECT 5
#include "..\..\Common\RegistryHelper.h"




#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define ERROR_HSESSION_NULL	1
#define ERROR_REGKEY_EXISTENTIAL_CRISIS	2
#define ERROR_REGVALUE_NOT_RETRIEVED 3
#define ERROR_NO_SERVERS_FOUND 4
#define ERROR_SERVER_NAME_INCORRECT 5
#define ERROR_WINHTTP_FAILAT_HCONNECT 6
#define ERROR_WINHTTP_FAILAT_HREQUEST 7
#define ERROR_WINHTTP_FAILAT_BRESULT_SEND 8
#define ERROR_WINHTTP_FAILAT_BRESULT_RECIEVERESPONSE 9
#define ERROR_WINHTTP_FAILAT_BRESULT_QHEADERS 10
#define ERROR_WINHTTP_BAD_RESPONSE_CODE 11 //Will return as bad token.
#define ERROR_CRED_WINHTTP_TIMEOUT 12
#define ERROR_WINHTTP_FAILAT_HSESSION 13

#define ERROR_MAJOR_SUCCESS 42

#ifdef ENABLE_LOGGING
#define DEBUG_DEF(string, message) \
	{ \
		if(debugLog) \
		{ \
			FILE* fpspam = fopen("C:\\Program Files\\Mi-Token\\Mi-Token Desktop Login\\General.log", "a+"); \
			if(fpspam != NULL) \
			{ \
				fprintf(fpspam, string, message); \
				fclose(fpspam); \
			} \
		} \
	} 

#define DEBUG_MESSAGE(message) DEBUG_DEF("%s\n", message)
//	{ FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%s\n", message); fclose(fpspam); } }
#define DEBUG_MESSAGEd(message) DEBUG_DEF("%d\n", message)
// { FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%d\n", message); fclose(fpspam); } }
#define DEBUG_WMESSAGE(wmessage) DEBUG_DEF("%ls\n", wmessage)
//{ FILE* fpspam = fopen("C:\\Test\\log2.txt", "a+"); if(fpspam != NULL) { fprintf(fpspam, "%ls\n", wmessage); fclose(fpspam); } }
#else
#define DEBUG_MESSAGE(message)
#define DEBUG_WMESSAGE(wmessage)
#define DEBUG_MESSAGEd(message)
#endif

extern int debugLog;

class MiTokenAuthenticator
{
	public:
		static BOOL VerifyOTP(LPWSTR username, LPWSTR otp, LPWSTR wMessage, int wMessageLength);
};