/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.14 1990-02-15 15:32:14 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_sauth_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.14 1990-02-15 15:32:14 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <strings.h>
#include "sms_server.h"
#include <ctype.h>
#include <krb_et.h>

extern char buf1[];
extern char *whoami;
extern char *malloc();

char *kname_unparse();

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
	int status, ok;
	char buf[REALM_SZ+INST_SZ+ANAME_SZ], hostbuf[BUFSIZ], *host, *p;

	auth.length = cl->args->sms_argl[0];
	bcopy(cl->args->sms_argv[0], (char *)auth.dat, auth.length);
	auth.mbz = 0;
	if (gethostname(hostbuf, sizeof(hostbuf)) < 0)
	  com_err(whoami, errno, "Unable to get local hostname");
	host = canonicalize_hostname(strsave(hostbuf));
	for (p = host; *p && *p != '.'; p++)
	  if (isupper(*p))
	    *p = tolower(*p);
	*p = 0;

	if ((status = krb_rd_req (&auth, MOIRA_SNAME, host, cl->haddr.sin_addr,
				 &ad, "")) != 0) {
		status += ERROR_TABLE_BASE_krb;
		cl->reply.sms_status = status;
		if (log_flags & LOG_RES)
			com_err(whoami, status, "(authentication failed)");
		return;
	}
	free(host);

	bcopy(ad.pname, cl->kname.name, ANAME_SZ);
	bcopy(ad.pinst, cl->kname.inst, INST_SZ);
	bcopy(ad.prealm, cl->kname.realm, REALM_SZ);
	strcpy(cl->clname, kname_unparse(ad.pname, ad.pinst, ad.prealm));

	if (ad.pinst[0] == 0 && !strcmp(ad.prealm, krb_realm))
	  ok = 1;
	else
	  ok = 0;
	/* this is in a separate function because it accesses the database */
	set_krb_mapping(cl->clname, ad.pname, ok,
			&cl->client_id, &cl->users_id);

	if (cl->args->sms_version_no == SMS_VERSION_2) {
	    bcopy(cl->args->sms_argv[1], cl->entity, 8);
	    cl->entity[8] = 0;
	} else {
	    strcpy(cl->entity, "???");
	}
	bzero(&ad, sizeof(ad));	/* Clean up session key, etc. */

	if (log_flags & LOG_RES)
	    com_err(whoami, 0, "Auth to %s using %s, uid %d cid %d",
		    cl->clname, cl->entity, cl->users_id, cl->client_id);
	if (cl->users_id == 0)
	  cl->reply.sms_status = SMS_USER_AUTH;
}


/* Turn a principal, instance, realm triple into a single non-ambiguous 
 * string.  This is the inverse of kname_parse().  It returns a pointer
 * to a static buffer, or NULL on error.
 */

char *kname_unparse(p, i, r)
char *p;
char *i;
char *r;
{
    static char name[MAX_K_NAME_SZ];
    char *s;

    s = name;
    if (!p || strlen(p) > ANAME_SZ)
      return(NULL);
    while (*p) {
	switch (*p) {
	case '@':
	    *s++ = '\\';
	    *s++ = '@';
	    break;
	case '.':
	    *s++ = '\\';
	    *s++ = '.';
	    break;
	case '\\':
	    *s++ = '\\';
	    *s++ = '\\';
	    break;
	default:
	    *s++ = *p;
	}
	p++;
    }
    if (i && *i) {
	if (strlen(i) > INST_SZ)
	  return(NULL);
	*s++ = '.';
	while (*i) {
	    switch (*i) {
	    case '@':
		*s++ = '\\';
		*s++ = '@';
		break;
	    case '.':
		*s++ = '\\';
		*s++ = '.';
		break;
	    case '\\':
		*s++ = '\\';
		*s++ = '\\';
		break;
	    default:
		*s++ = *i;
	    }
	    i++;
	}
    }
    *s++ = '@';
    if (!r || strlen(r) > REALM_SZ)
      return(NULL);
    while (*r) {
	switch (*r) {
	case '@':
	    *s++ = '\\';
	    *s++ = '@';
	    break;
	case '\\':
	    *s++ = '\\';
	    *s++ = '\\';
	    break;
	default:
	    *s++ = *r;
	}
	r++;
    }
    *s = '\0';
    return(&name[0]);
}
