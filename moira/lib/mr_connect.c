/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.9 1988-09-13 15:52:09 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *	
 * 	This routine is part of the client library.  It handles
 *	creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.9 1988-09-13 15:52:09 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"

/*
 * Open a connection to the sms server.
 */

int sms_connect()
{
    extern int errno;
	
    if (!sms_inited) sms_init();
    if (_sms_conn) return SMS_ALREADY_CONNECTED;
		
    /* 
     * XXX should do a hesiod call to find the sms machine name & service
     * number/name.
     */
    errno = 0;
    _sms_conn = start_server_connection(SMS_GDB_SERV, ""); 
    if (_sms_conn == NULL)
	return errno;
    if (connection_status(_sms_conn) == CON_STOPPED) {
	register status = connection_errno(_sms_conn);
	if (!status) status = SMS_CANT_CONNECT;
	sms_disconnect();
	return status;
    }
    return 0;
}
	
int sms_disconnect()
{
    CHECK_CONNECTED;
    _sms_conn = sever_connection(_sms_conn);
    return 0;
}

int sms_noop()
{
    int status;
    sms_params param_st;
    struct sms_params *params = NULL;
    struct sms_params *reply = NULL;

    CHECK_CONNECTED;
    params = &param_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_NOOP;
    params->sms_argc = 0;
    params->sms_argl = NULL;
    params->sms_argv = NULL;
	
    if ((status = sms_do_call(params, &reply)) == 0)
	status = reply->sms_status;
	
    sms_destroy_reply(reply);

    return status;
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
