/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.5 1989-06-26 09:09:25 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_ticket_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.5 1989-06-26 09:09:25 mar Exp $";
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
static char master[] = "sms";
static char service[] = "rcmd";

extern char *tkt_string(), *PrincipalHostname();


static init()
{
    static int initialized = 0;

    if (!initialized) {
	get_krbrlm(realm, 1);
	initialize_krb_error_table();
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
       code += ERROR_TABLE_BASE_krb;
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
	return(code + ERROR_TABLE_BASE_krb);
}
