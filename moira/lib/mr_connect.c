/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.1 1987-05-20 03:11:46 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	
 * 	This routine is part of the client library.  It handles
 * creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.1 1987-05-20 03:11:46 wesommer Exp $";
#endif lint

#include <gdb.h>
#include "sms_private.h"

/*
 * Open a connection to the sms server.
 */

int sms_connect()
{
	gdb_init();		/* Harmless if called twice. */
	/* 
	 * should do a hesiod call to find the sms machine name & service
	 * number/name.
	 */

	_sms_conn = start_server_connection(SMS_GDB_SERV, "XXX"); 
	/* XXX gdb doesn't give real return codes. Can we trust errno?*/
	if (_sms_conn == NULL) {
		perror("gdb_connect");
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

	if (!_sms_conn) {
		return SMS_NOT_CONNECTED;
	}

	if (!sms_call_op)
		sms_call_op = create_operation();

	parms = (struct sms_params *) malloc(sizeof(*parms));
	
	parms->procno = SMS_NOOP;
	parms->argc = 0;

	gdb_inop(sms_call_op, sms_start_call, &parms, sms_abort_call);
	gdb_qop(_sms_conn, CON_OUTPUT, sms_call_op);

	gdb_inop(sms_recv_op, sms_start_recv, &reply, sms_abort_recv);
	gdb_qop(_sms_conn, CON_INPUT, sms_recv_op);

	/* Block until operation done. */
	gdb_cmpo(sms_call_op);
	gdb_cmpo(sms_recv_op);

	/* Look at results */
	if (OP_STATUS(sms_recv_op) != OP_COMPLETE) {
		return SMS_ABORTED;
	}
	/* should look at return code from server.. */
	return 0;
}

	
