/* $Id: mr_auth.c,v 1.22 1998-02-08 19:31:18 danw Exp $
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
#include <string.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_auth.c,v 1.22 1998-02-08 19:31:18 danw Exp $");

/* Authenticate this client with the Moira server.  prog is the name of the
 * client program, and will be recorded in the database.
 */

int mr_auth(char *prog)
{
  int status;
  mr_params params_st;
  char *args[2];
  int argl[2];
  char realm[REALM_SZ], host[BUFSIZ], *p;
  mr_params *params = &params_st;
  mr_params *reply = NULL;
  KTEXT_ST auth;

  CHECK_CONNECTED;

  /* Build a Kerberos authenticator. */

  memset(host, 0, sizeof(host));
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
