/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.9 1988-07-23 18:48:52 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef lint
static char *rcsid_sms_sauth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.9 1988-07-23 18:48:52 mar Exp $";
#endif lint

extern int krb_err_base;
#include <strings.h>
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
	static char *unknown = "???";
	
	if (cl->clname) {
		free(cl->clname);
		cl->clname = 0;
		cl->users_id = 0;
		bzero(&cl->kname, sizeof(cl->kname));
	}
	if (cl->entity && cl->entity != unknown) {
		free(cl->entity);
		cl->entity = 0;
	}
	
	auth.length = cl->args->sms_argl[0];
	bcopy(cl->args->sms_argv[0], (char *)auth.dat, auth.length);
	auth.mbz = 0;
	
	if ((status = krb_rd_req (&auth, "sms", "sms", cl->haddr.sin_addr,
				 &ad, "")) != KSUCCESS) {
		status += krb_err_base;
		cl->reply.sms_status = status;
		if (log_flags & LOG_RES)
			com_err(whoami, status, "(authentication failed)");
		return;
	}
	bcopy(ad.pname, cl->kname.name, ANAME_SZ);
	bcopy(ad.pinst, cl->kname.inst, INST_SZ);
	bcopy(ad.prealm, cl->kname.realm, REALM_SZ);
	
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
	bzero(&ad, sizeof(ad));	/* Clean up session key, etc. */

	cl->users_id = get_users_id(cl->kname.name);

	if (cl->args->sms_version_no == SMS_VERSION_2) {
	    unsigned len = strlen(cl->args->sms_argv[1]) + 1;

	    cl->entity = (char *)malloc(len);
	    bcopy(cl->args->sms_argv[1], cl->entity, len+1);
	} else {
	    cl->entity = unknown;
	}

	if (log_flags & LOG_RES)
	    com_err(whoami, 0, "Authenticated to %s using %s, id %d",
		    cl->clname, cl->entity, cl->users_id);
	if (cl->users_id == 0)
	  cl->reply.sms_status = SMS_USER_AUTH;
}
