/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.2 1988-04-19 12:39:23 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	
 * 	This routine is part of the client library.  It handles
 * creating a connection to the sms server.
 */

#ifndef lint
static char *rcsid_sms_do_update_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.2 1988-04-19 12:39:23 mar Exp $";
#endif lint

#include "sms_private.h"

int sms_do_update()
{
    int status;
    sms_params param_st;
    struct sms_params *params = NULL;
    struct sms_params *reply = NULL;

    CHECK_CONNECTED;
    params = &param_st;
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_DO_UPDATE;
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
