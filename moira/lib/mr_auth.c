/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.6 1988-06-01 15:31:53 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.5  88/04/19  12:40:29  mar
 * pass version number on queries
 * 
 * Revision 1.4  87/10/20  15:09:22  mar
 * replace hardcoded ATHENA realm with call to get_krbrlm();
 * 
 * Revision 1.3  87/06/16  17:47:20  wesommer
 * Clean up memory allocation, indenting.
 * 
 * Revision 1.2  87/05/31  22:03:23  wesommer
 * It's working better.
 * 
 * Revision 1.1  87/05/20  03:11:18  wesommer
 * Initial revision
 * 
 *
 *	Handles the client side of the sending of authenticators to
 * the sms server. 	
 */

#ifndef lint
static char *rcsid_sms_auth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.6 1988-06-01 15:31:53 mar Exp $";
#endif lint

#include "sms_private.h"
#include <krb.h>

extern int krb_err_base;

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
    status = mk_ap_req(&auth, "sms", "sms", realm, 0);
    if (status != KSUCCESS) {
	status += krb_err_base;
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
