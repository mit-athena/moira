/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v 1.2 1987-08-28 19:05:23 wesommer Exp $
 */

#ifndef lint
static char *rcsid_auth_001_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_001.c,v 1.2 1987-08-28 19:05:23 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include <strings.h>
#include "gdb.h"
#include "smsu_int.h"
#include "kludge.h"
#include <krb.h>
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
    char host[BUFSIZ];
    AUTH_DAT ad;
    char realm[REALM_SZ];
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
    code = rd_ap_req(&ticket_st, service,
		     PrincipalHostname(host), 0,
		     &ad, "/etc/srvtab");
    if (code) {
	code = krb_err_frob(code);
	strcpy(ad.pname, qmark);
	strcpy(ad.pinst, qmark);
	strcpy(ad.prealm, qmark);
	goto auth_failed;
    }
    if (get_krbrlm(realm,0))
	realm[0] = '\0';
    code = EPERM;
    if (strcmp(master, ad.pname))
	goto auth_failed;
    if (ad.pinst[0] != '\0')
	goto auth_failed;
    if (strcmp(realm, ad.prealm))
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
