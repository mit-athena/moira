/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.4 1988-09-14 12:16:39 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_ticket_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.4 1988-09-14 12:16:39 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <krb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <update.h>
#include <com_err.h>

/* too bad we can't set the pathname easily */
static char *srvtab = KEYFILE; /* default == /etc/srvtab */
static char realm[REALM_SZ];
static char master[] = "sms";
static char service[] = "rcmd";

extern char *tkt_string(), *PrincipalHostname();
extern int krb_err_base;

static init()
{
    static int initialized = 0;

    if (!initialized) {
	get_krbrlm(realm, 1);
	init_krb_err_tbl();
	initialized=1;
    }
}


int
get_sms_update_ticket(host, ticket)
     char *host;
     KTEXT ticket;
{
     register int code;
     register int pass;
     char phost[BUFSIZ];

     pass = 1;
     init();
     strcpy(phost, PrincipalHostname(host));
 try_it:
     code = krb_mk_req(ticket, service, phost, realm, (long)0);
     if (code)
       code += krb_err_base;
     if (pass == 1) {
	 /* maybe we're taking too long? */
	 if ((code = get_sms_tgt()) != 0) {
	     /* don't need phost buffer any more */
	     com_err(whoami, code, " can't get Kerberos TGT");
	     return(code);
	 }
	 pass++;
	 goto try_it;
     }
     return(code);
}

int
get_sms_tgt()
{
    register int code;
    init();
    code = get_svc_in_tkt(master, "", realm, "krbtgt", realm, 1, srvtab);
    if (!code)
	return(0);
    else
	return(code + krb_err_base);
}
