// FreeRadius.cpp : Defines the entry point for the console application.
//

#include "..\freeradius-client\include\freeradius-client.h"
#include "..\\freeradius-client\include\pathnames.h"
#include "..\Common\RegistryHelper.h"

#define USER_NAME_LENGTH 128



int get_radius_reply(char *username, char *passwd)
{
	VALUE_PAIR 	*send, *received;
	uint32_t	service;
	char 		msg[4096];
	char		*default_realm;
	rc_handle	*rh;

	/*Retrieve the server settings from the registry.*/
	if ((rh = rc_read_registry()) == NULL)
		return ERROR_REGISTRY;

	/*Add other attributes like user name, password etc.*/
	if (rc_initialize_dictionary(rh) < 0)
		return ERROR_DICTIONARY;

	default_realm = rc_conf_str(rh, "default_realm");

	send = NULL;

	/*Add the user name.*/
	if (rc_avpair_add(rh, &send, PW_USER_NAME, username, -1, 0) == NULL)
		return ERROR_DICTIONARY;

	/* Fill in User-Password */
	if (rc_avpair_add(rh, &send, PW_USER_PASSWORD, passwd, -1, 0) == NULL)
		return ERROR_DICTIONARY;

	/* Fill in Service-Type */
	service = PW_AUTHENTICATE_ONLY;
	if (rc_avpair_add(rh, &send, PW_SERVICE_TYPE, &service, -1, 0) == NULL)
		return ERROR_DICTIONARY;

	return rc_auth(rh, 0, send, &received, msg);
}

int decode_message(char* buffer, int buffer_length, int error_code)
{

	/*
		ERROR_SOCKET_BIND	-7
		ERROR_INVALID_SOCKET	-6
		ERROR_AUTH_SERVER_NOT_FOUND -5
		ERROR_DICTIONARY	-4
		ERROR_REGISTRY	-3
		BADRESP_RC	-2
		ERROR_RC	-1
		OK_RC		0
		TIMEOUT_RC	1
		REJECT_RC	2
		ERROR_INTERNAL 3
	*/

	int retval = 0;
	switch(error_code)
	{
		case 0:
			retval = strcpy_s(buffer,buffer_length,"Authentication successful.");
			break;

		case 1:
			retval = strcpy_s(buffer,buffer_length,"Request timeout.");
			break;

		case 2:
			retval = strcpy_s(buffer,buffer_length,"Authentication unsuccessful.");
			break;

		case 3:
			retval = strcpy_s(buffer,buffer_length,"Internal error. Conversion failed.");
			break;

		case -2:
			retval = strcpy_s(buffer,buffer_length,"Radius reply is invalid.");
			break;

		case -3:
			retval = strcpy_s(buffer,buffer_length,"Error initializing radius servers from registry.");
			break;

		case -4:
			retval = strcpy_s(buffer,buffer_length,"Error initializing username or password.");
			break;

		case -5:
			retval = strcpy_s(buffer,buffer_length,"Failed to retrieve initialized auth server.");
			break;

		case -1:
			retval = strcpy_s(buffer,buffer_length,"Internal error. Blah Blah (jegan told me to say that)");
			break;

		default:
			retval = strcpy_s(buffer,buffer_length,"Unknown error.");
			break;
	}
	return retval;
}


int _tmain(int argc, _TCHAR* argv[])
{

	char	username[900];
	char    passwd[900];

	//Plug me in
	strncpy_s(username, 900, "administrator", sizeof(username));
	strncpy_s (passwd, 900, "cccccccbhncrdhchjgfgltieguebkucgcdthjkvcvnft", sizeof(passwd));
	int r = get_radius_reply(username, passwd);

	char message[4096];
	memset(message,4096, '\0');
	decode_message(message, 4096, r);
	WCHAR X[4096];

	memset(message,4096 * sizeof(WCHAR), '\0');

	MultiByteToWideChar(CP_ACP, 0, message, -1, X, 4096);
	//rc_openlog(passwd);
	return 0;
	

	//TCHAR buff[10];
	//rc_event_log(EVENTLOG_ERROR_TYPE,_T("THIS IS LOGGED!!!!") );
}
