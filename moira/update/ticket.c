/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.12 1993-01-27 11:23:26 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_ticket_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.12 1993-01-27 11:23:26 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <krb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <update.h>
#include <com_err.h>
#include <krb_et.h>

/* too bad we can't set the pathname easily */
static char *srvtab = KEYFILE; /* default == /etc/srvtab */
static char realm[REALM_SZ];
static char master[INST_SZ] = "sms";
static char service[ANAME_SZ] = "rcmd";
C_Block session;


static init()
{
    static int initialized = 0;

    if (!initialized) {
	if (krb_get_lrealm(realm, 1))
	    strcpy(realm, KRB_REALM);
	initialize_krb_error_table();
	initialized=1;
    }
}


int
get_mr_update_ticket(host, ticket)
     char *host;
     KTEXT ticket;
{
     register int code;
     register int pass;
     char phost[BUFSIZ];
     CREDENTIALS cr;

     pass = 1;
     init();
     strcpy(phost, krb_get_phost(host));
 try_it:
     code = krb_mk_req(ticket, service, phost, realm, (long)0);
     if (code) {
	 if (pass == 1) {
	     /* maybe we're taking too long? */
	     if ((code = get_mr_tgt()) != 0) {
		 com_err(whoami, code, " can't get Kerberos TGT");
		 return(code);
	     }
	     pass++;
	     goto try_it;
	 }
	 com_err(whoami, code, "in krb_mk_req");
     } else {
	 code = krb_get_cred(service, phost, realm, &cr);
	 bcopy(cr.session, session, sizeof(session));
     }
     return(code);
}

int
get_mr_tgt()
{
    register int code;
    char linst[INST_SZ], kinst[INST_SZ];

    init();
    linst[0] = '\0';
    strcpy(kinst, "krbtgt");
    code = krb_get_svc_in_tkt(master, linst, realm, kinst, realm,
			      DEFAULT_TKT_LIFE, srvtab);
    if (!code)
	return(0);
    else
	return(code + ERROR_TABLE_BASE_krb);
}
