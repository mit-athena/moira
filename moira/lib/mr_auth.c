/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.2 1987-05-31 22:03:23 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/05/20  03:11:18  wesommer
 * Initial revision
 * 
 *
 *	Handles the client side of the sending of authenticators to
 * the sms server. 	
 */

#ifndef lint
static char *rcsid_sms_auth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.2 1987-05-31 22:03:23 wesommer Exp $";
#endif lint

#include "sms_private.h"
#include <krb.h>

extern int krb_err_base;

int sms_auth()
{
	int status;
	sms_params *params = NULL, *reply = NULL;
	KTEXT_ST auth;
	
	if (!_sms_conn) {
		return SMS_NOT_CONNECTED;
	}
	
	/*
	 * Build a Kerberos authenticator.
	 */
	
	status = mk_ap_req(&auth, "sms", "sms", "ATHENA.MIT.EDU", 0);
	if (status != KSUCCESS) {
		status += krb_err_base;
		goto punt;
	}
	params = (struct sms_params *) malloc(sizeof(*params));
	params->sms_procno = SMS_AUTH;
	params->sms_argc = 1;

	params->sms_argv = (char **)malloc(sizeof(char *) * 2);
	params->sms_argl = (int *)malloc(sizeof(int) * 2);
	params->sms_argv[0] = (char *)auth.dat;
	params->sms_argl[0] = auth.length;
	params->sms_argv[1] = NULL;
	params->sms_argl[1] = 0;
	
	if ((status = sms_do_call(params, &reply)) || (status = reply->sms_status))
		goto punt;
punt:
	sms_destroy_reply(reply);
ok:
	if(params) {
		if(params->sms_argv)
			free(params->sms_argv);
		if(params->sms_argl)
			free(params->sms_argl);
		free(params);
	}
	return status;
}
