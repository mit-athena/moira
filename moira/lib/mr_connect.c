/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.2 1987-05-31 22:04:00 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	
 * 	This routine is part of the client library.  It handles
 * creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.2 1987-05-31 22:04:00 wesommer Exp $";
#endif lint

#include "sms_private.h"

/*
 * Open a connection to the sms server.
 */

int sms_connect()
{
	if (!sms_inited) sms_init();
	
	/* 
	 * should do a hesiod call to find the sms machine name & service
	 * number/name.
	 */

	_sms_conn = start_server_connection(SMS_GDB_SERV, "XXX"); 
	/* XXX gdb doesn't give real return codes. Can we trust errno?*/
	if (_sms_conn == NULL) {
		perror("gdb_connect"); /*XXX*/
		return SMS_CANT_CONNECT;
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
	struct sms_params *parms;
	struct sms_params *reply;
	

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
