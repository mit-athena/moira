/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.3 1987-06-01 03:34:20 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	
 * 	This routine is part of the client library.  It handles
 * creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.3 1987-06-01 03:34:20 wesommer Exp $";
#endif lint

#include "sms_private.h"

/*
 * Open a connection to the sms server.
 */

int sms_connect()
{
	extern int errno;
	
	if (!sms_inited) sms_init();
	
	/* 
	 * should do a hesiod call to find the sms machine name & service
	 * number/name.
	 */
	/* XXX gdb doesn't give real return codes. Can we trust errno?*/
	errno = 0;
	_sms_conn = start_server_connection(SMS_GDB_SERV, ""); 
	if (_sms_conn == NULL) {
		return errno;
	}
	return 0;
}
	
int sms_disconnect()
{
	if (!_sms_conn) {
		return SMS_NOT_CONNECTED;
	}
	/* Is this guaranteed NOT to fail?? I don't believe it, but.. */
	_sms_conn = sever_connection(_sms_conn);
	return 0;
}

int sms_noop()
{
	int status;
	struct sms_params *parms = NULL;
	struct sms_params *reply = NULL;
	

	parms = (struct sms_params *) malloc(sizeof(*parms));
	
	parms->sms_procno = SMS_NOOP;
	parms->sms_argc = 0;

	if ((status = sms_do_call(parms, &reply)) || (status = reply->sms_status))
		goto punt;
	
	
punt:
	sms_destroy_reply(reply);
ok:
	free(parms);
	return status;
}

int sms_shutdown(why)
	char *why;
{
	int status;
	sms_params *parms=NULL;
	sms_params *reply=NULL;

	parms = (sms_params *) malloc(sizeof(*parms));
	
	parms->sms_procno = SMS_SHUTDOWN;
	parms->sms_argc = 1;
	parms->sms_argv = (char **)malloc(sizeof(char *) * 2);
	parms->sms_argv[0] = why;
	parms->sms_argv[1] = NULL;

	if ((status = sms_do_call(parms, &reply)) || (status = reply->sms_status))
		goto punt;
	
	
punt:
	sms_destroy_reply(reply);
ok:
	if(parms) {
		if(parms->sms_argv)
			free(parms->sms_argv);
		free(parms);
	}
	return status;
}
