/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.1 1987-05-20 03:11:18 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 *
 *	Handles the client side of the sending of authenticators to
 * the sms server. 	
 */

#ifndef lint
static char *rcsid_sms_auth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.1 1987-05-20 03:11:18 wesommer Exp $";
#endif lint

#include "sms_private.h"
#include <krb.h>

int sms_auth()
{
	int status;
	struct sms_params aparms;
	
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

	if (!sms_call_op)
		sms_call_op = create_operation();


		
	
	
	
}
