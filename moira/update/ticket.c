/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.1 1987-08-22 17:55:21 wesommer Exp $
 */

#ifndef lint
static char *rcsid_ticket_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/ticket.c,v 1.1 1987-08-22 17:55:21 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include <krb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include "sms_update_int.h"
#include "com_err.h"
#include "kludge.h"

/* too bad we can't set the pathname easily */
/*static char tkt_pathname[] = "/tmp/tkt:sms";*/
static char *srvtab = SRVTAB; /* default == /etc/srvtab */
static char realm[REALM_SZ];
static char sms[] = "sms";

extern char *tkt_string(), *PrincipalHostname();
static int initialized = 0;

#define init() { if (!initialized) { get_krbrlm(realm,0); initialized=1; }}

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
     code = mk_ap_req(ticket, sms, phost, realm, (long)0);
     if (pass == 1) {
	 /* maybe we're taking too long? */
	 if ((code = get_sms_tgt()) != 0) {
	     /* don't need phost buffer any more */
	     sprintf(phost, "%s: can't get Kerberos TGT",
		     error_message(code));
	     sms_log_error(phost);
	     return(code);
	 }
	 pass++;
	 goto try_it;
     }
     return(krb_err_frob(code));
}

int
get_sms_tgt()
{
    register int code;
    init();
    code = get_svc_in_tkt(sms, "", realm, "krbtgt", realm, 1, srvtab);
    if (!code)
	return(0);
    else
	return(krb_err_frob(code));
}
