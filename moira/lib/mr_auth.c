/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.9 1989-06-26 12:38:26 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Handles the client side of the sending of authenticators to
 * the sms server. 	
 */

#ifndef lint
static char *rcsid_sms_auth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.9 1989-06-26 12:38:26 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "sms_private.h"
#include <krb.h>
#include <krb_et.h>


/* Authenticate this client with the SMS server.  prog is the name of the
 * client program, and will be recorded in the database.
 */

int sms_auth(prog)
char *prog;
{
    register int status;
    sms_params params_st;
    char *args[2];
    int argl[2];
    char realm[REALM_SZ];

    register sms_params *params = &params_st;
    sms_params *reply = NULL;
    KTEXT_ST auth;

    CHECK_CONNECTED;
	
    /*
     * Build a Kerberos authenticator.
     * The "service" and "instance" should not be hardcoded here.
     */
	
    if ((status = get_krbrlm(realm, 1)) != KSUCCESS) {
	return status;
    }
    status = krb_mk_req(&auth, "sms", "sms", realm, 0);
    if (status != KSUCCESS) {
	status += ERROR_TABLE_BASE_krb;
	return status;
    } 
    params->sms_version_no = sending_version_no;
    params->sms_procno = SMS_AUTH;
    params->sms_argc = 2;
    params->sms_argv = args;
    params->sms_argl = argl;
    params->sms_argv[0] = (char *)auth.dat;
    params->sms_argl[0] = auth.length;
    params->sms_argv[1] = prog;
    params->sms_argl[1] = strlen(prog) + 1;
	
    if (sending_version_no == SMS_VERSION_1)
	params->sms_argc = 1;

    if ((status = sms_do_call(params, &reply)) == 0)
	status = reply->sms_status;

    sms_destroy_reply(reply);

    return status;
}
