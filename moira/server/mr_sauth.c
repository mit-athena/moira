/* $Id: mr_sauth.c,v 1.24 1998-02-05 22:51:43 danw Exp $
 *
 * Handle server side of authentication
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_sauth.c,v 1.24 1998-02-05 22:51:43 danw Exp $");

extern char *whoami, *host;

typedef struct _replay_cache {
  KTEXT_ST auth;
  time_t expires;
  struct _replay_cache *next;
} replay_cache;

replay_cache *rcache = NULL;

/*
 * Handle a MOIRA_AUTH RPC request.
 *
 * argv[0] is a kerberos authenticator.  Decompose it, and if
 * successful, store the name the user authenticated to in
 * cl->cl_name.
 */

void do_auth(client *cl)
{
  KTEXT_ST auth;
  AUTH_DAT ad;
  int status, ok;
  replay_cache *rc, *rcnew;
  time_t now;

  auth.length = cl->args->mr_argl[0];
  memcpy(auth.dat, cl->args->mr_argv[0], auth.length);
  auth.mbz = 0;

  if ((status = krb_rd_req (&auth, MOIRA_SNAME, host,
			    cl->haddr.sin_addr.s_addr, &ad, "")))
    {
      status += ERROR_TABLE_BASE_krb;
      cl->reply.mr_status = status;
      if (log_flags & LOG_RES)
	com_err(whoami, status, " (authentication failed)");
      return;
    }

  if (!rcache)
    {
      rcache = malloc(sizeof(replay_cache));
      memset(rcache, 0, sizeof(replay_cache));
    }

  /* scan replay cache */
  for (rc = rcache->next; rc; rc = rc->next)
    {
      if (auth.length == rc->auth.length &&
	  !memcmp(&(auth.dat), &(rc->auth.dat), auth.length))
	{
	  com_err(whoami, 0,
		  "Authenticator replay from %s using authenticator for %s",
		  inet_ntoa(cl->haddr.sin_addr),
		  kname_unparse(ad.pname, ad.pinst, ad.prealm));
	  com_err(whoami, KE_RD_AP_REPEAT, " (authentication failed)");
	  cl->reply.mr_status = KE_RD_AP_REPEAT;
	  return;
	}
    }

  /* add new entry */
  time(&now);
  rcnew = malloc(sizeof(replay_cache));
  memcpy(&(rcnew->auth), &auth, sizeof(KTEXT_ST));
  rcnew->expires = now + 2 * CLOCK_SKEW;
  rcnew->next = rcache->next;
  rcache->next = rcnew;

  /* clean cache */
  for (rc = rcnew; rc->next; )
    {
      if (rc->next->expires < now)
	{
	  rcnew = rc->next;
	  rc->next = rc->next->next;
	  free(rcnew);
	}
      else
	rc = rc->next;
    }

  memcpy(cl->kname.name, ad.pname, ANAME_SZ);
  memcpy(cl->kname.inst, ad.pinst, INST_SZ);
  memcpy(cl->kname.realm, ad.prealm, REALM_SZ);
  strcpy(cl->clname, kname_unparse(ad.pname, ad.pinst, ad.prealm));

  if (ad.pinst[0] == 0 && !strcmp(ad.prealm, krb_realm))
    ok = 1;
  else
    ok = 0;
  /* this is in a separate function because it accesses the database */
  status = set_krb_mapping(cl->clname, ad.pname, ok,
			   &cl->client_id, &cl->users_id);

  if (cl->args->mr_version_no == MR_VERSION_2)
    {
      strncpy(cl->entity, cl->args->mr_argv[1], 8);
      cl->entity[8] = 0;
    }
  else
    strcpy(cl->entity, "???");
  memset(&ad, 0, sizeof(ad));	/* Clean up session key, etc. */

  if (log_flags & LOG_RES)
    {
      com_err(whoami, 0, "Auth to %s using %s, uid %d cid %d",
	      cl->clname, cl->entity, cl->users_id, cl->client_id);
    }
  if (status != MR_SUCCESS)
    cl->reply.mr_status = status;
  else if (cl->users_id == 0)
    cl->reply.mr_status = MR_USER_AUTH;
}
