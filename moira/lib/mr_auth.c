/* $Id: mr_auth.c,v 1.24 1999-07-17 21:39:38 danw Exp $
 *
 * Handles the client side of the sending of authenticators to the moira server
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.24 1999-07-17 21:39:38 danw Exp $");

/* Authenticate this client with the Moira server.  prog is the name of the
 * client program, and will be recorded in the database.
 */

int mr_auth(char *prog)
{
  int status;
  mr_params params, reply;
  char *args[2];
  int argl[2];
  char realm[REALM_SZ], host[BUFSIZ], *p;
  KTEXT_ST auth;

  CHECK_CONNECTED;

  if ((status = mr_host(host, sizeof(host) - 1)))
    return status;

  strcpy(realm, krb_realmofhost(host));
  for (p = host; *p && *p != '.'; p++)
    {
      if (isupper(*p))
	*p = tolower(*p);
    }
  *p = '\0';

  status = krb_mk_req(&auth, MOIRA_SNAME, host, realm, 0);
  if (status != KSUCCESS)
    {
      status += ERROR_TABLE_BASE_krb;
      return status;
    }
  params.u.mr_procno = MR_AUTH;
  params.mr_argc = 2;
  params.mr_argv = args;
  params.mr_argl = argl;
  params.mr_argv[0] = (char *)auth.dat;
  params.mr_argl[0] = auth.length;
  params.mr_argv[1] = prog;
  params.mr_argl[1] = strlen(prog) + 1;

  if ((status = mr_do_call(&params, &reply)) == MR_SUCCESS)
    status = reply.u.mr_status;

  mr_destroy_reply(reply);

  return status;
}

int mr_proxy(char *principal, char *orig_authtype)
{
  int status;
  mr_params params, reply;
  char *args[2];

  params.u.mr_procno = MR_PROXY;
  params.mr_argc = 2;
  params.mr_argv = args;
  params.mr_argv[0] = principal;
  params.mr_argv[1] = orig_authtype;
  params.mr_argl = NULL;

  if ((status = mr_do_call(&params, &reply)) == MR_SUCCESS)
    status = reply.u.mr_status;

  mr_destroy_reply(reply);

  return status;
}
