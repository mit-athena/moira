/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.16 1997-01-29 23:24:12 danw Exp $
 *
 *	Copyright (C) 1987, 1990 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Handles the client side of the sending of authenticators to
 * the mr server. 	
 */

#ifndef lint
static char *rcsid_sms_auth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.16 1997-01-29 23:24:12 danw Exp $";
#endif

#include <mit-copyright.h>
#include "mr_private.h"
#include <ctype.h>
#include <krb.h>
#include <krb_et.h>
#include <string.h>

/* Authenticate this client with the MR server.  prog is the name of the
 * client program, and will be recorded in the database.
 */

int mr_auth(prog)
char *prog;
{
    register int status;
    mr_params params_st;
    char *args[2];
    int argl[2];
    char realm[REALM_SZ], host[BUFSIZ], *p;

    register mr_params *params = &params_st;
    mr_params *reply = NULL;
    KTEXT_ST auth;

    CHECK_CONNECTED;
	
    /* Build a Kerberos authenticator. */
	
    memset(host, 0, sizeof(host));
    if (status = mr_host(host, sizeof(host) - 1))
	return status;

    strcpy(realm, (char *)krb_realmofhost(host));
    for (p = host; *p && *p != '.'; p++)
      if (isupper(*p))
	*p = tolower(*p);
    *p = 0;

    status = krb_mk_req(&auth, MOIRA_SNAME, host, realm, 0);
    if (status != KSUCCESS) {
	status += ERROR_TABLE_BASE_krb;
	return status;
    } 
    params->mr_version_no = sending_version_no;
    params->mr_procno = MR_AUTH;
    params->mr_argc = 2;
    params->mr_argv = args;
    params->mr_argl = argl;
    params->mr_argv[0] = (char *)auth.dat;
    params->mr_argl[0] = auth.length;
    params->mr_argv[1] = prog;
    params->mr_argl[1] = strlen(prog) + 1;
	
    if (sending_version_no == MR_VERSION_1)
	params->mr_argc = 1;

    if ((status = mr_do_call(params, &reply)) == 0)
	status = reply->mr_status;

    mr_destroy_reply(reply);

    return status;
}
