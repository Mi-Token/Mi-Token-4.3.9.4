/*
 * $Id: buildreq.c,v 1.15 2008/03/05 16:35:20 cparker Exp $
 *
 * Copyright (C) 1995,1997 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#include <windows.h>
#include <exception>
#include <sstream>
#include "..\include\freeradius-client.h"
#include "..\event.provider\EventProvider.h"

unsigned char rc_get_seqnbr(rc_handle *);

/*
 * Function: rc_buildreq
 *
 * Purpose: builds a skeleton RADIUS request using information from the
 * 	    config file.
 *
 */

void rc_buildreq(rc_handle *rh, SEND_DATA *data, int code, char *server, unsigned short port, 
		 char *secret, int timeout, int retries)
{
	std::string decoded = base64_decode(secret);
	char *decrypted_value = (char *)decrypt_server_secret((byte *)&decoded[0],decoded.size());

	//char *decrypted_value = (char *)decrypt_server_secret((byte *)&base64_decode(secret)[0],218);
	data->server = server;
	data->secret = decrypted_value;//"notsecure";//secret;
	data->svc_port = port;
	data->seq_nbr = rc_get_seqnbr(rh);
	data->timeout = timeout;
	data->retries = retries;
	data->code = code;
}



byte* decrypt_server_secret(byte* encrypted_data, int encrypted_data_length)
{
	byte *dec = NULL;
	DATA_BLOB DataOut;
	DATA_BLOB DataVerify;
	LPWSTR pDescrOut =  NULL;

	DataOut.pbData = (BYTE *)encrypted_data;
	DataOut.cbData = encrypted_data_length;
	DataVerify.pbData = NULL;
	DataVerify.cbData = 0;
	if (CryptUnprotectData(
			&DataOut,
			&pDescrOut,
			NULL,                 // Optional entropy
			NULL,                 // Reserved
			NULL,                 // Here, the optional 
								  // prompt structure is not
								  // used.
			0,
			&DataVerify))
	{
		dec = new byte[DataVerify.cbData + 1];
		memcpy(dec, DataVerify.pbData, DataVerify.cbData);
		dec[DataVerify.cbData] = '\0';

		SecureZeroMemory(DataVerify.pbData, DataVerify.cbData);
	}

	return dec;
}

 // static CONST wchar_t CHAR_63 = '*';
 // static CONST wchar_t CHAR_64 = '-';
 // static CONST wchar_t CHAR_PAD = '[';


 // // Decodes Base64 code to binary data
 // // Returns size of decoded data.
 // int Decode(CString inCode, int codeLength, byte* outData )
 // {
 // 
 // #ifndef _MANAGED
 // 
	//// used as temp 24-bits buffer
	//union
	//{
	//	unsigned char bytes[ 4 ];
	//	unsigned int block;
	//} buffer;
	//buffer.block = 0;
 // 
	//// number of decoded bytes
	//int j = 0;
 // 
	//for( int i = 0; i < codeLength; i++ )
	//{
	//	// position in temp buffer
	//	int m = i % 4;
 // 
	//	wchar_t x = inCode[ i ];
	//	int val = 0;
 // 
	//	// converts base64 character to six-bit value
	//	if( x >= 'A' && x <= 'Z' )
	//		val = x - 'A';
	//	else if( x >= 'a' && x <= 'z' )
	//		val = x - 'a' + 'Z' - 'A' + 1;
	//	else if( x >= '0' && x <= '9' )
	//		val = x - '0' + ( 'Z' - 'A' + 1 ) * 2;
	//	else if( x == CHAR_63 )
	//		val = 62;
	//	else if( x == CHAR_64 )
	//		val = 63;
 // 
	//	// padding chars are not decoded and written to output buffer
	//	if( x != CHAR_PAD )
	//		buffer.block |= val << ( 3 - m ) * 6;
	//	else
	//		m--;
 // 
	//	// temp buffer is full or end of code is reached
	//	// flushing temp buffer
	//	if( m == 3 || x == CHAR_PAD )
	//	{
	//		// writes byte from temp buffer (combined from two six-bit values) to output buffer
	//		outData[ j++ ] = buffer.bytes[ 2 ];
	//		// more data left?
	//		if( x != CHAR_PAD || m > 1 )
	//		{
	//			// writes byte from temp buffer (combined from two six-bit values) to output buffer
	//			outData[ j++ ] = buffer.bytes[ 1 ];
	//			// more data left?
	//			if( x != CHAR_PAD || m > 2 )
	//				// writes byte from temp buffer (combined from two six-bit values) to output buffer
	//				outData[ j++ ] = buffer.bytes[ 0 ];
	//		}
 // 
	//		// restarts temp buffer
	//		buffer.block = 0;
	//	}
 // 
	//	// when padding char is reached it is the end of code
	//	if( x == CHAR_PAD )
	//		break;
	//}
 // 
	//return j;
 // 
 // #else
 // 
	//// reverse changes of character before decoding data
	//if( CHAR_63 != '+' )
	//	inCode = inCode->Replace( CHAR_63, '+' );
	//if( CHAR_64 != '/' )
	//	inCode = inCode->Replace( CHAR_64, '/' );
	//if( CHAR_PAD != '=' )
	//	inCode = inCode->Replace( CHAR_PAD, '=' );
 // 
	//// decode
	//outData = System::Convert::FromBase64String( inCode );
	//return inCode->Length;
 // 
 // #endif
 // }


/*
 * Function: rc_guess_seqnbr
 *
 * Purpose: return a random sequence number
 *
 */

static unsigned char rc_guess_seqnbr(void)
{
	/*srandom((unsigned int)(time(NULL)+getpid()));
	return (unsigned char)(random() & UCHAR_MAX);*/

	
    srand( (unsigned)time(NULL));
	return (unsigned char)(rand() & UCHAR_MAX);
}

/*
 * Function: rc_get_seqnbr
 *
 * Purpose: generate a sequence number
 *
 */

unsigned char rc_get_seqnbr(rc_handle *rh)
{
	FILE *sf;
	int tries = 1;
	int seq_nbr;
	char *seqfile = rc_conf_str(rh, "seqfile");
	char error_buffer[4096];

	fopen_s(&sf, seqfile, "a+");
	if (sf == NULL)
	{
		strerror_s(error_buffer, 4096, errno);
		error_buffer[4095] = '\0';
		rc_event_log(EVENTLOG_ERROR_TYPE, FREE_RADIUS_GENERAL_CATEGORY, FREE_RADIUS_MESSAGE, L"rc_get_seqnbr: couldn't open sequence file %s: %s", seqfile, error_buffer);

		/* well, so guess a sequence number */
		return rc_guess_seqnbr();
	}

	/*
	while (do_lock_exclusive(sf)!= 0)
	{
		if (errno != EWOULDBLOCK) {
			rc_event_log(EVENTLOG_ERROR_TYPE, FREE_RADIUS_GENERAL_CATEGORY, FREE_RADIUS_MESSAGE, L"rc_get_seqnbr: flock failure: %s: %s", seqfile, strerror(errno));
			fclose(sf);
			return rc_guess_seqnbr();
		}
		tries++;
		if (tries <= 10)
			rc_mdelay(500);
		else
			break;
	}
	*/

	if (tries > 10) 
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, FREE_RADIUS_GENERAL_CATEGORY, FREE_RADIUS_MESSAGE, L"rc_get_seqnbr: couldn't get lock after %d tries: %s", tries-1, seqfile);
 		fclose(sf);
		return rc_guess_seqnbr();
	}

	rewind(sf);
	if (fscanf_s(sf, "%d", &seq_nbr) != 1) 
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, FREE_RADIUS_GENERAL_CATEGORY, FREE_RADIUS_MESSAGE, L"rc_get_seqnbr: fscanf failure: %s", seqfile);
		seq_nbr = rc_guess_seqnbr();
	}

	rewind(sf);
	//ftruncate(fileno(sf),0);
	SetEndOfFile(sf);
	fprintf(sf,"%d\n", (seq_nbr+1) & UCHAR_MAX);

	fflush(sf); /* fflush because a process may read it between the do_unlock and fclose */

	/*if (do_unlock(sf) != 0)
		rc_event_log(EVENTLOG_ERROR_TYPE, FREE_RADIUS_GENERAL_CATEGORY, FREE_RADIUS_MESSAGE, L"rc_get_seqnbr: couldn't release lock on %s: %s", seqfile, strerror(errno));*/

	fclose(sf);

	return (unsigned char)seq_nbr;
}

/*
 * Function: rc_aaa
 *
 * Purpose: Builds an authentication/accounting request for port id client_port
 *	    with the value_pairs send and submits it to a server
 *
 * Returns: received value_pairs in received, messages from the server in msg
 *	    and 0 on success, negative on failure as return value
 *
 */

int rc_aaa(rc_handle *rh, uint32_t client_port, VALUE_PAIR *send, VALUE_PAIR **received,
    char *msg, int add_nas_port, int request_type)
{
	SEND_DATA       data;
	VALUE_PAIR	*adt_vp;
	int		result;
	int		i, skip_count;
	SERVER		*aaaserver;
	int		timeout = rc_conf_int(rh, "radius_timeout");
	int		retries = rc_conf_int(rh, "radius_retries");
	int		radius_deadtime = rc_conf_int(rh, "radius_deadtime");
	double		start_time;
	time_t		dtime;

	if (request_type != PW_ACCOUNTING_REQUEST) 
	{
		aaaserver = rc_conf_srv(rh, "authserver");
	} 
	else 
	{
		aaaserver = rc_conf_srv(rh, "acctserver");
	}

	if (aaaserver == NULL)
		return ERROR_AUTH_SERVER_NOT_FOUND;

	data.send_pairs = send;
	data.receive_pairs = NULL;

	// Jegan
	//if (add_nas_port != 0) {
	//	/*
	//	 * Fill in NAS-Port
	//	 */
	//	if (rc_avpair_add(rh, &(data.send_pairs), PW_NAS_PORT,
	//	    &client_port, 0, 0) == NULL)
	//		return ERROR_RC;
	//}

	if (request_type == PW_ACCOUNTING_REQUEST) {
		/*
		 * Fill in Acct-Delay-Time
		 */
		dtime = 0;
		if ((adt_vp = rc_avpair_add(rh, &(data.send_pairs),
		    PW_ACCT_DELAY_TIME, &dtime, 0, 0)) == NULL)
			return ERROR_RC;
	}

	start_time = rc_getctime();
	skip_count = 0;
	result = ERROR_RC;
	for (i=0; (i < aaaserver->max) && (result != OK_RC) && (result != BADRESP_RC)
	    ; i++)
	{
		if (aaaserver->deadtime_ends[i] != -1 &&
		    aaaserver->deadtime_ends[i] > start_time) {
			skip_count++;
			continue;
		}
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(rh, &data, request_type, aaaserver->name[i],
		    aaaserver->port[i], aaaserver->secret[i], timeout, retries);

		if (request_type == PW_ACCOUNTING_REQUEST) {
			dtime = rc_getctime() - start_time;
			rc_avpair_assign(adt_vp, &dtime, 0);
		}

		result = rc_send_server (rh, &data, msg);
		if (result == TIMEOUT_RC && radius_deadtime > 0)
			aaaserver->deadtime_ends[i] = start_time + (double)radius_deadtime;
	}
	if (result == OK_RC || result == BADRESP_RC || skip_count == 0)
		goto exit;

	result = ERROR_RC;
	for (i=0; (i < aaaserver->max) && (result != OK_RC) && (result != BADRESP_RC)
	    ; i++)
	{
		if (aaaserver->deadtime_ends[i] == -1 ||
		    aaaserver->deadtime_ends[i] <= start_time) {
			continue;
		}
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(rh, &data, request_type, aaaserver->name[i],
		    aaaserver->port[i], aaaserver->secret[i], timeout, retries);

		if (request_type == PW_ACCOUNTING_REQUEST) {
			dtime = rc_getctime() - start_time;
			rc_avpair_assign(adt_vp, &dtime, 0);
		}

		result = rc_send_server (rh, &data, msg);
		if (result != TIMEOUT_RC)
			aaaserver->deadtime_ends[i] = -1;
	}

exit:
	if (request_type != PW_ACCOUNTING_REQUEST) {
		*received = data.receive_pairs;
	} else {
		rc_avpair_free(data.receive_pairs);
	}

	return result;
}

/*
 * Function: rc_auth
 *
 * Purpose: Builds an authentication request for port id client_port
 *          with the value_pairs send and submits it to a server
 *
 * Returns: received value_pairs in received, messages from the server in msg
 *          and 0 on success, negative on failure as return value
 *
 */

int rc_auth(rc_handle *rh, uint32_t client_port, VALUE_PAIR *send, VALUE_PAIR **received,
    char *msg)
{

 	return rc_aaa(rh, client_port, send, received, msg, 1, PW_ACCESS_REQUEST);
}

/*
 * Function: rc_auth_proxy
 *
 * Purpose: Builds an authentication request
 *	    with the value_pairs send and submits it to a server.
 *	    Works for a proxy; does not add IP address, and does
 *	    does not rely on config file.
 *
 * Returns: received value_pairs in received, messages from the server in msg
 *	    and 0 on success, negative on failure as return value
 *
 */

int rc_auth_proxy(rc_handle *rh, VALUE_PAIR *send, VALUE_PAIR **received, char *msg)
{

	return rc_aaa(rh, 0, send, received, msg, 0, PW_ACCESS_REQUEST);
}


/*
 * Function: rc_acct
 *
 * Purpose: Builds an accounting request for port id client_port
 *	    with the value_pairs send
 *
 * Remarks: NAS-IP-Address, NAS-Port and Acct-Delay-Time get filled
 *	    in by this function, the rest has to be supplied.
 */

int rc_acct(rc_handle *rh, uint32_t client_port, VALUE_PAIR *send)
{
	char		msg[4096];

	return rc_aaa(rh, client_port, send, NULL, msg, 1, PW_ACCOUNTING_REQUEST);
}

/*
 * Function: rc_acct_proxy
 *
 * Purpose: Builds an accounting request with the value_pairs send
 *
 */

int rc_acct_proxy(rc_handle *rh, VALUE_PAIR *send)
{
	char		msg[4096];

	return rc_aaa(rh, 0, send, NULL, msg, 0, PW_ACCOUNTING_REQUEST);
}

/*
 * Function: rc_check
 *
 * Purpose: ask the server hostname on the specified port for a
 *	    status message
 *
 */

int rc_check(rc_handle *rh, char *host, char *secret, unsigned short port, char *msg)
{
	SEND_DATA       data;
	int		result;
	uint32_t		service_type;
	int		timeout = rc_conf_int(rh, "radius_timeout");
	int		retries = rc_conf_int(rh, "radius_retries");

	data.send_pairs = data.receive_pairs = NULL;

	/*
	 * Fill in Service-Type
	 */

	service_type = PW_ADMINISTRATIVE;
	rc_avpair_add(rh, &(data.send_pairs), PW_SERVICE_TYPE, &service_type, 0, 0);

	rc_buildreq(rh, &data, PW_STATUS_SERVER, host, port, secret, timeout, retries);
	result = rc_send_server (rh, &data, msg);

	rc_avpair_free(data.receive_pairs);

	return result;
}


//char base64_digit (n) unsigned n; 
//{
//  if (n < 10) return n - '0';
//  else if (n < 10 + 26) return n - 'a';
//  else if (n < 10 + 26 + 26) return n - 'A';
//  else assert(0);
//  return 0;
//}

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string encoded_string) 
{
	  int in_len = encoded_string.size();
	  int i = 0;
	  int j = 0;
	  int in_ = 0;
	  unsigned char char_array_4[4], char_array_3[3];
	  std::string ret;

	  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	  {
		  char_array_4[i++] = encoded_string[in_]; in_++;
		  if (i ==4) 
		  {
			for (i = 0; i <4; i++)
			  char_array_4[i] = base64_chars.find(char_array_4[i]);

			  char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			  char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			  char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];

			i = 0;
		 }
	  }

	  if (i) {
		for (j = i; j <4; j++)
		  char_array_4[j] = 0;

		for (j = 0; j <4; j++)
		  char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	  }

     return ret;
}

