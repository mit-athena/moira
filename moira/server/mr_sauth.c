/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.2 1987-06-04 01:34:35 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/06/02  20:06:57  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_sauth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.2 1987-06-04 01:34:35 wesommer Exp $";
#endif lint

extern int krb_err_base;
#include <krb.h>
#include <strings.h>
#include "sms_private.h"
#include "sms_server.h"

extern char buf1[];
extern char *whoami;
extern char *malloc();

/*
 * Handle a SMS_AUTH RPC request.
 *
 * argv[0] is a kerberos authenticator.  Decompose it, and if
 * successful, store the name the user authenticated to in 
 * cl->cl_name.
 */

void
do_auth(cl)
	client *cl;
{
	KTEXT_ST auth;
	AUTH_DAT ad;
	int status;
	char buf[REALM_SZ+INST_SZ+ANAME_SZ];
	extern int krb_err_base;
	
	com_err(whoami, 0, "Processing auth: ");
	log_args(cl->args->sms_argc, cl->args->sms_argv);

	auth.length = cl->args->sms_argl[0];

	bcopy(cl->args->sms_argv[0], (char *)auth.dat, auth.length);
	auth.mbz = 0;
	
	if ((status = rd_ap_req (&auth, "sms", "sms", cl->haddr.sin_addr,
				 &ad, "")) != KSUCCESS) {
		status += krb_err_base;
		cl->reply.sms_status = status;
		com_err(whoami, status, "(authentication failed)");
		return;
	}
	(void) strcpy(buf, ad.pname);
	if(ad.pinst[0]) {
		(void) strcat(buf, ".");
		(void) strcat(buf, ad.pinst);
	}
	(void) strcat(buf, "@");
	(void) strcat(buf, ad.prealm);
	if (cl->clname) free((char *)cl->clname);
	
	cl->clname = (char *)malloc((unsigned)(strlen(buf)+1));
	(void) strcpy(cl->clname, buf);
	(void) sprintf(buf1, "Authenticated to %s", cl->clname);
	com_err(whoami, 0, buf1);
}
