/*
 * $Id: sendserver.c,v 1.25 2008/01/05 03:06:53 sobomax Exp $
 *
 * Copyright (C) 1995,1996,1997 Lars Fenneberg
 *
 * Copyright 1992 Livingston Enterprises, Inc.
 *
 * Copyright 1992,1993, 1994,1995 The Regents of the University of Michigan
 * and Merit Network, Inc. All Rights Reserved
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */
#include <Winsock2.h>
#include "..\event.provider\EventProvider.h"
#include "..\include\freeradius-client.h"
#include "..\include\pathnames.h"


#define	SA(p)	((struct sockaddr *)(p))

static void rc_random_vector (unsigned char *);
static int rc_check_reply (AUTH_HDR *, int, char *, unsigned char *, unsigned char);

/*
 * Function: rc_pack_list
 *
 * Purpose: Packs an attribute value pair list into a buffer.
 *
 * Returns: Number of octets packed.
 *
 */

static int rc_pack_list (VALUE_PAIR *vp, char *secret, AUTH_HDR *auth)
{
	int             length, i, pc, padded_length;
	int             total_length = 0;
	size_t			secretlen;
	uint32_t           lvalue, vendor;
	unsigned char   passbuf[MAX(AUTH_PASS_LEN, CHAP_VALUE_LENGTH)];
	unsigned char   md5buf[256];
	unsigned char   *buf, *vector, *vsa_length_ptr;

	buf = auth->data;

	while (vp != NULL)
	{
		vsa_length_ptr = NULL;
		if (VENDOR(vp->attribute) != 0) {
			*buf++ = PW_VENDOR_SPECIFIC;
			vsa_length_ptr = buf;
			*buf++ = 6;
			vendor = htonl(VENDOR(vp->attribute));
			memcpy(buf, &vendor, sizeof(uint32_t));
			buf += 4;
			total_length += 6;
		}
		*buf++ = (vp->attribute & 0xff);

		switch (vp->attribute)
		{
		 case PW_USER_PASSWORD:

		  /* Encrypt the password */

		  /* Chop off password at AUTH_PASS_LEN */
		  length = vp->lvalue;
		  if (length > AUTH_PASS_LEN)
			length = AUTH_PASS_LEN;

		  /* Calculate the padded length */
		  padded_length = (length+(AUTH_VECTOR_LEN-1)) & ~(AUTH_VECTOR_LEN-1);

		  /* Record the attribute length */
		  *buf++ = padded_length + 2;
		  if (vsa_length_ptr != NULL) *vsa_length_ptr += padded_length + 2;

		  /* Pad the password with zeros */
		  memset ((char *) passbuf, '\0', AUTH_PASS_LEN);
		  memcpy ((char *) passbuf, vp->strvalue, (size_t) length);

		  secretlen = strlen (secret);
		  vector = (unsigned char *)auth->vector;
		  for(i = 0; i < padded_length; i += AUTH_VECTOR_LEN)
		  {
		  	/* Calculate the MD5 digest*/
		  	strcpy_s ((char *) md5buf, MD5_BUFFER_SIZE, secret);
		  	memcpy ((char *) md5buf + secretlen, vector,
		  		  AUTH_VECTOR_LEN);
		  	rc_md5_calc (buf, md5buf, secretlen + AUTH_VECTOR_LEN);

		        /* Remeber the start of the digest */
		  	vector = buf;

			/* Xor the password into the MD5 digest */
			for (pc = i; pc < (i + AUTH_VECTOR_LEN); pc++)
		  	{
				*buf++ ^= passbuf[pc];
		  	}
		  }

		  total_length += padded_length + 2;

		  break;
#if 0
		 case PW_CHAP_PASSWORD:

		  *buf++ = CHAP_VALUE_LENGTH + 2;
		  if (vsa_length_ptr != NULL) *vsa_length_ptr += CHAP_VALUE_LENGTH + 2;

		  /* Encrypt the Password */
		  length = vp->lvalue;
		  if (length > CHAP_VALUE_LENGTH)
		  {
			length = CHAP_VALUE_LENGTH;
		  }
		  memset ((char *) passbuf, '\0', CHAP_VALUE_LENGTH);
		  memcpy ((char *) passbuf, vp->strvalue, (size_t) length);

		  /* Calculate the MD5 Digest */
		  secretlen = strlen (secret);
		  strcpy ((char *) md5buf, secret);
		  memcpy ((char *) md5buf + secretlen, (char *) auth->vector,
		  	  AUTH_VECTOR_LEN);
		  rc_md5_calc (buf, md5buf, secretlen + AUTH_VECTOR_LEN);

		  /* Xor the password into the MD5 digest */
		  for (i = 0; i < CHAP_VALUE_LENGTH; i++)
		  {
			*buf++ ^= passbuf[i];
		  }
		  total_length += CHAP_VALUE_LENGTH + 2;

		  break;
#endif
		 default:
		  switch (vp->type)
		  {
		    case PW_TYPE_STRING:
			length = vp->lvalue;
			*buf++ = length + 2;
			if (vsa_length_ptr != NULL) *vsa_length_ptr += length + 2;
			memcpy (buf, vp->strvalue, (size_t) length);
			buf += length;
			total_length += length + 2;
			break;

		    case PW_TYPE_INTEGER:
		    case PW_TYPE_IPADDR:
			*buf++ = sizeof (uint32_t) + 2;
			if (vsa_length_ptr != NULL) *vsa_length_ptr += sizeof(uint32_t) + 2;
			lvalue = htonl (vp->lvalue);
			memcpy (buf, (char *) &lvalue, sizeof (uint32_t));
			buf += sizeof (uint32_t);
			total_length += sizeof (uint32_t) + 2;
			break;

		    default:
			break;
		  }
		  break;
		}
		vp = vp->next;
	}
	return total_length;
}

/*
 * Function: rc_send_server
 *
 * Purpose: send a request to a RADIUS server and wait for the reply
 *
 */

int rc_send_server (rc_handle *rh, SEND_DATA *data, char *msg)
{
	int             sockfd;
	struct sockaddr_in sinlocal;
	struct sockaddr_in sinremote;
	struct timeval  authtime;
	fd_set          readfds;
	AUTH_HDR       *auth, *recv_auth;
	uint32_t           auth_ipaddr;//, nas_ipaddr;
	char           *server_name;	/* Name of server to query */
	int/*socklen_t*/       salen;
	int             result;
	int             total_length;
	int             length;
	int             retry_max;
	size_t			secretlen;
	char            secret[MAX_SECRET_LENGTH + 1];
	unsigned char   vector[AUTH_VECTOR_LEN];
	char            recv_buffer[BUFFER_LEN];
	char            send_buffer[BUFFER_LEN];
	int		retries;
	VALUE_PAIR 	*vp;
	char error_buffer[4096];

	server_name = data->server;
	if (server_name == NULL || server_name[0] == '\0')
		return ERROR_RC;

	if ((vp = rc_avpair_get(data->send_pairs, PW_SERVICE_TYPE, 0)) && \
	    (vp->lvalue == PW_ADMINISTRATIVE))
	{
		strcpy_s(secret, MAX_SECRET_LENGTH_WITH_NULL_TERMINATOR, MGMT_POLL_SECRET);
		if ((auth_ipaddr = rc_get_ipaddr(server_name)) == 0)
			return ERROR_RC;
	}
	else
	{
		if(data->secret != NULL)
		{
			strncpy_s(secret, MAX_SECRET_LENGTH_WITH_NULL_TERMINATOR, data->secret, MAX_SECRET_LENGTH);
		}
		
		if (rc_find_server (rh, server_name, &auth_ipaddr, secret) != 0)
		{
			rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: unable to find server: %s", server_name);
			return ERROR_RC;
		}
	}

    WSADATA wsaData = {0};
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (sockfd < 0)
	{
		memset (secret, '\0', sizeof (secret));
		strerror_s(error_buffer, 4096, errno);
		error_buffer[4095] = '\0';

		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: socket: %s", error_buffer);
		return ERROR_INVALID_SOCKET;
	}

	memset((char *)&sinlocal, '\0', sizeof(sinlocal));
	sinlocal.sin_family = AF_INET;
	sinlocal.sin_addr.s_addr = htonl(rc_own_bind_ipaddress(rh));
	sinlocal.sin_port = htons((unsigned short) 0);
	if (bind(sockfd, SA(&sinlocal), sizeof(sinlocal)) < 0)
	{
		closesocket (sockfd);
		
		/*Clear the secret*/
		memset (secret, '\0', sizeof (secret));

		strerror_s(error_buffer, 4096, errno);
		error_buffer[4095] = '\0';

		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: bind: %s: %s", server_name, error_buffer);
		return ERROR_SOCKET_BIND;
	}

	retry_max = data->retries;	/* Max. numbers to try for reply */
	retries = 0;			/* Init retry cnt for blocking call */

	memset ((char *)&sinremote, '\0', sizeof(sinremote));
	sinremote.sin_family = AF_INET;
	sinremote.sin_addr.s_addr = htonl (auth_ipaddr);
	sinremote.sin_port = htons ((unsigned short) /*1645*/data->svc_port);

	/*
	 * Fill in NAS-IP-Address
	 */
	if (sinlocal.sin_addr.s_addr == htonl(INADDR_ANY)) {
		if (rc_get_srcaddr(SA(&sinlocal), SA(&sinremote)) != 0) 
		{
			closesocket (sockfd);
			memset (secret, '\0', sizeof (secret));
			return ERROR_RC;
		}
	}
	//nas_ipaddr = ntohl(sinlocal.sin_addr.s_addr);
	//rc_avpair_add(rh, &(data->send_pairs), PW_NAS_IP_ADDRESS,   &nas_ipaddr, 0, 0);

	/* Build a request */
	auth = (AUTH_HDR *) send_buffer;
	auth->code = data->code;
	auth->id = data->seq_nbr;

	if (data->code == PW_ACCOUNTING_REQUEST)
	{
		total_length = rc_pack_list(data->send_pairs, secret, auth) + AUTH_HDR_LEN;

		auth->length = htons ((unsigned short) total_length);

		memset((char *) auth->vector, 0, AUTH_VECTOR_LEN);
		secretlen = strlen (secret);
		memcpy ((char *) auth + total_length, secret, secretlen);
		//rc_md5_calc (vector, (unsigned char *) auth, total_length + secretlen);
		memcpy ((char *) auth->vector, (char *) vector, AUTH_VECTOR_LEN);
	}
	else
	{
		rc_random_vector (vector);
		memcpy ((char *) auth->vector, (char *) vector, AUTH_VECTOR_LEN);

		total_length = rc_pack_list(data->send_pairs, secret, auth) + AUTH_HDR_LEN;

		auth->length = htons ((unsigned short) total_length);
	}

	for (;;)
	{
		sendto (sockfd, (char *) auth, (unsigned int) total_length, (int) 0,
			SA(&sinremote), sizeof (struct sockaddr_in));

		authtime.tv_usec = 0L;
		authtime.tv_sec = (long) data->timeout;
		FD_ZERO (&readfds);
		FD_SET (sockfd, &readfds);
		if (select (sockfd + 1, &readfds, NULL, NULL, &authtime) < 0)
		{
			if (errno == WSAEINTR)
				continue;

			strerror_s(error_buffer, 4096, errno);
			rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: select: %s", error_buffer);
			memset (secret, '\0', sizeof (secret));
			closesocket (sockfd);
			return ERROR_RC;
		}
		if (FD_ISSET (sockfd, &readfds))
			break;

		/*
		 * Timed out waiting for response.  Retry "retry_max" times
		 * before giving up.  If retry_max = 0, don't retry at all.
		 */
		if (++retries >= retry_max)
		{
			rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: no reply from RADIUS server %s:%u, %s", rc_ip_hostname (auth_ipaddr), data->svc_port, inet_ntoa(sinremote.sin_addr));
			closesocket (sockfd);
			memset (secret, '\0', sizeof (secret));
			return TIMEOUT_RC;
		}
	}
	salen = sizeof(sinremote);
	length = recvfrom (sockfd, (char *) recv_buffer,
			   (int) sizeof (recv_buffer),
			   (int) 0, SA(&sinremote), &salen);

	if (length <= 0)
	{
		strerror_s(error_buffer, 4096, errno);
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: recvfrom: %s:%d: %s", server_name, data->svc_port, error_buffer);
		closesocket (sockfd);
		memset (secret, '\0', sizeof (secret));
		return ERROR_RC;
	}

	recv_auth = (AUTH_HDR *)recv_buffer;

	if (length < AUTH_HDR_LEN || length < ntohs(recv_auth->length)) 
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_send_server: recvfrom: %s:%d: reply is too short", server_name, data->svc_port);
		closesocket(sockfd);
		memset(secret, '\0', sizeof(secret));
		return ERROR_RC;
	}

	result = rc_check_reply (recv_auth, BUFFER_LEN, secret, vector, data->seq_nbr);

	length = ntohs(recv_auth->length)  - AUTH_HDR_LEN;
	if (length > 0) 
	{
		data->receive_pairs = rc_avpair_gen(rh, NULL, recv_auth->data, length, 0);
	} 
	else 
	{
		data->receive_pairs = NULL;
	}

	closesocket (sockfd);
	memset (secret, '\0', sizeof (secret));

	if (result != OK_RC) return result;

	*msg = '\0';
	vp = data->receive_pairs;
	while (vp)
	{
		if ((vp = rc_avpair_get(vp, PW_REPLY_MESSAGE, 0)))
		{
			strcat_s(msg,4096, vp->strvalue);
			strcat_s(msg,4096, "\n");
			vp = vp->next;
		}
	}

	if ((recv_auth->code == PW_ACCESS_ACCEPT) || (recv_auth->code == PW_PASSWORD_ACK) || (recv_auth->code == PW_ACCOUNTING_RESPONSE))
	{
		result = OK_RC;
	}
	else if ((recv_auth->code == PW_ACCESS_REJECT) || (recv_auth->code == PW_PASSWORD_REJECT))
	{
		result = REJECT_RC;
	}
	else
	{
		result = BADRESP_RC;
	}

	return result;
}

/*
 * Function: rc_check_reply
 *
 * Purpose: verify items in returned packet.
 *
 * Returns:	OK_RC       -- upon success,
 *		BADRESP_RC  -- if anything looks funny.
 *
 */

static int rc_check_reply (AUTH_HDR *auth, int bufferlen, char *secret, unsigned char *vector, uint8_t seq_nbr)
{
	int             secretlen;
	int             totallen;
	unsigned char   calc_digest[AUTH_VECTOR_LEN];
	unsigned char   reply_digest[AUTH_VECTOR_LEN];
#ifdef DIGEST_DEBUG
	uint8_t		*ptr;
#endif

	totallen = ntohs (auth->length);
	secretlen = (int)strlen (secret);

	/* Do sanity checks on packet length */
	if ((totallen < 20) || (totallen > 4096))
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_check_reply: received RADIUS server response with invalid length. The length is %d", totallen );
		return BADRESP_RC;
	}

	/* Verify buffer space, should never trigger with current buffer size and check above */
	if ((totallen + secretlen) > bufferlen)
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_check_reply: not enough buffer space to verify RADIUS server response. The buffer length is %d and total length is %d",bufferlen, (totallen + secretlen));
		return BADRESP_RC;
	}

	/* Verify that id (seq. number) matches what we sent */
	if (auth->id != seq_nbr)
	{
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_check_reply: received non-matching id in RADIUS server response.");
		return BADRESP_RC;
	}

	/* Verify the reply digest */
	memcpy ((char *) reply_digest, (char *) auth->vector, AUTH_VECTOR_LEN);
	memcpy ((char *) auth->vector, (char *) vector, AUTH_VECTOR_LEN);
	memcpy ((char *) auth + totallen, secret, secretlen);
#ifdef DIGEST_DEBUG
        rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"Calculating digest on:");
        for (ptr = (u_char *)auth; ptr < ((u_char *)auth) + totallen + secretlen; ptr += 32) {
                char buf[65];
                int i;

                buf[0] = '\0';
                for (i = 0; i < 32; i++) {
                        if (ptr + i >= ((u_char *)auth) + totallen + secretlen)
                                break;
                        sprintf(buf + i * 2, "%.2X", ptr[i]);
                }
                rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"  %s", buf);
        }
#endif
	rc_md5_calc (calc_digest, (unsigned char *) auth, totallen + secretlen);
#ifdef DIGEST_DEBUG
	rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"LOG_ERR, "Calculated digest is:");
        for (ptr = (u_char *)calc_digest; ptr < ((u_char *)calc_digest) + 16; ptr += 32) {
                char buf[65];
                int i;

                buf[0] = '\0';
                for (i = 0; i < 32; i++) {
                        if (ptr + i >= ((u_char *)calc_digest) + 16)
                                break;
                        sprintf(buf + i * 2, "%.2X", ptr[i]);
                }
                rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"  %s", buf);
        }
	rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"Reply digest is:");
        for (ptr = (u_char *)reply_digest; ptr < ((u_char *)reply_digest) + 16; ptr += 32) {
                char buf[65];
                int i;

                buf[0] = '\0';
                for (i = 0; i < 32; i++) {
                        if (ptr + i >= ((u_char *)reply_digest) + 16)
                                break;
                        sprintf(buf + i * 2, "%.2X", ptr[i]);
                }
               rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"  %s", buf);
        }
#endif

	if (memcmp ((char *) reply_digest, (char *) calc_digest,
		    AUTH_VECTOR_LEN) != 0)
	{
#ifdef RADIUS_116
		/* the original Livingston radiusd v1.16 seems to have
		   a bug in digest calculation with accounting requests,
		   authentication request are ok. i looked at the code
		   but couldn't find any bugs. any help to get this
		   kludge out are welcome. preferably i want to
		   reproduce the calculation bug here to be compatible
		   to stock Livingston radiusd v1.16.	-lf, 03/14/96
		 */
		if (auth->code == PW_ACCOUNTING_RESPONSE)
			return OK_RC;
#endif
		rc_event_log(EVENTLOG_ERROR_TYPE, RADIUS_CATEGORY, RADIUS_MESSAGE, L"rc_check_reply: received invalid reply digest from RADIUS server");
		return BADRESP_RC;
	}

	return OK_RC;

}

/*
 * Function: rc_random_vector
 *
 * Purpose: generates a random vector of AUTH_VECTOR_LEN octets.
 *
 * Returns: the vector (call by reference)
 *
 */

static void rc_random_vector (unsigned char *vector)
{
	int             randno;
	int             i;
#if defined(HAVE_DEV_URANDOM)
	int		fd;

/* well, I added this to increase the security for user passwords.
   we use /dev/urandom here, as /dev/random might block and we don't
   need that much randomness. BTW, great idea, Ted!     -lf, 03/18/95	*/

	if ((fd = open(_PATH_DEV_URANDOM, O_RDONLY)) >= 0)
	{
		unsigned char *pos;
		int readcount;

		i = AUTH_VECTOR_LEN;
		pos = vector;
		while (i > 0)
		{
			readcount = read(fd, (char *)pos, i);
			pos += readcount;
			i -= readcount;
		}

		close(fd);
		return;
	} /* else fall through */
#endif
	srand ((unsigned)time (0) + 1/* getppid()*/ + 1);//getpid()); /* random enough :) */
	for (i = 0; i < AUTH_VECTOR_LEN;)
	{
		randno = rand ();
		memcpy ((char *) vector, (char *) &randno, sizeof (int));
		vector += sizeof (int);
		i += sizeof (int);
	}

	return;
}
