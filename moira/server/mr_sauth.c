/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.6 1987-08-04 02:40:47 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.5  87/07/14  00:40:18  wesommer
 * Rearranged logging.
 * 
 * Revision 1.4  87/06/30  20:03:46  wesommer
 * Put parsed kerberos principal name into the per-client structure.
 * 
 * Revision 1.3  87/06/21  16:40:10  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.2  87/06/04  01:34:35  wesommer
 * Added logging of arguments for some perverse reason.
 * 
 * Revision 1.1  87/06/02  20:06:57  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_sauth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.6 1987-08-04 02:40:47 wesommer Exp $";
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
	
	auth.length = cl->args->sms_argl[0];

	bcopy(cl->args->sms_argv[0], (char *)auth.dat, auth.length);
	auth.mbz = 0;
	
	if ((status = rd_ap_req (&auth, "sms", "sms", cl->haddr.sin_addr,
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
	if (log_flags & LOG_RES) {
		com_err(whoami, 0, "Authenticated to %s", cl->clname);
	}
}
