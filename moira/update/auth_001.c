/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v 1.7 1992-08-25 14:40:17 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_auth_001_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v 1.7 1992-08-25 14:40:17 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <gdb.h>
#include <krb.h>
#include <krb_et.h>
#include <netinet/in.h>
#include <errno.h>

extern char buf[BUFSIZ];
extern int have_authorization;
extern struct sockaddr_in *client_address();
extern CONNECTION conn;
int code;
extern char *PrincipalHostname();
static char service[] = "rcmd";
static char master[] = "sms";
static char qmark[] = "???";

/*
 * authentication request auth_001:
 *
 * >>> (STRING) "auth_001"
 * <<< (int) 0
 * >>> (STRING) ticket
 * <<< (int) code
 *
 */

int
auth_001(str)
     char *str;
{
    STRING data;
    char host[BUFSIZ], realm[REALM_SZ];
    char aname[ANAME_SZ], ainst[INST_SZ], arealm[REALM_SZ];
    AUTH_DAT ad;
    char *p, *first, *config_lookup();
    KTEXT_ST ticket_st;

    if (send_ok())
	lose("sending okay for authorization (auth_001)");
    code = receive_object(conn, (char *)&data, STRING_T);
    if (code) {
	code = connection_errno(conn);
	lose("awaiting Kerberos authenticators");
    }
    gethostname(host, BUFSIZ);
    ticket_st.mbz = 0;
    ticket_st.length = MAX_STRING_SIZE(data);
    bcopy(STRING_DATA(data), ticket_st.dat, MAX_STRING_SIZE(data));
    code = krb_rd_req(&ticket_st, service,
		      krb_get_phost(host), 0,
		      &ad, KEYFILE);
    if (code) {
	code += ERROR_TABLE_BASE_krb;
	strcpy(ad.pname, qmark);
	strcpy(ad.pinst, qmark);
	strcpy(ad.prealm, qmark);
	goto auth_failed;
    }

    /* If there is an auth record in the config file matching the
     * authenticator we received, then accept it.  If there's no
     * auth record, assume [master]@[local realm].
     */
    if (first = p = config_lookup("auth")) {
	do {
	    kname_parse(aname, ainst, arealm, p);
	    if (strcmp(aname, ad.pname) ||
		strcmp(ainst, ad.pinst) ||
		strcmp(arealm, ad.prealm))
	      p = config_lookup("auth");
	    else
	      p = first;
	} while (p != first);
    } else {
	strcpy(aname, master);
	strcpy(ainst, "");
	if (krb_get_lrealm(arealm,1))
	  strcpy(arealm, KRB_REALM);
    }
    code = EPERM;
    if (strcmp(aname, ad.pname) ||
	strcmp(ainst, ad.pinst) ||
	strcmp(arealm, ad.prealm))
      goto auth_failed;
    if (send_ok())
	lose("sending approval of authorization");
    have_authorization = 1;
    return(0);
auth_failed:
    sprintf(buf, "auth for %s.%s@%s failed: %s",
	    ad.pname, ad.pinst, ad.prealm, error_message(code));
    {
	register int rc;
	rc = send_object(conn, (char *)&code, INTEGER_T);
	code = rc;
    }
    if (code)
	lose("sending rejection of authenticator");
    return(EPERM);
}
