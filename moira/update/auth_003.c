/* $Id$
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/utsname.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <mr_krb.h>
#include <krb5.h>

RCSID("$HeadURL$ $Id$");

static char service[] = "host";
static char master[] = "sms";
static char qmark[] = "???";

/*
 * authentication request auth_003:
 *
 * >>> (STRING) "auth_003"
 * <<< (int) 0
 * >>> (STRING) ticket
 * <<< (int) code
 *
 */

void auth_003(int conn, char *str)
{
  krb5_context context = NULL;
  krb5_auth_context auth_con = NULL;
  krb5_data auth;
  krb5_principal server = NULL, client = NULL, allowed = NULL;
  krb5_ticket *ticket;
  char *p, *first, *data, *pname = NULL, *mname = NULL, *lrealm = NULL;
  char name[ANAME_SZ], inst[INST_SZ], realm[REALM_SZ];
  char aname[ANAME_SZ], ainst[INST_SZ], arealm[REALM_SZ];
  size_t size;
  long code;
  struct utsname uts;

  ticket = NULL;

  send_ok(conn);

  recv_string(conn, &data, &size);
  auth.data = malloc(size);
  if (!auth.data)
    goto out;
  memcpy(auth.data, data, size);
  free(data);
  auth.length = size;

  code = krb5_init_context(&context);
  if (code)
    {
      com_err(whoami, code, "Initializing context");
      send_int(conn, code);
      goto out;
    }

  code = krb5_auth_con_init(context, &auth_con);
  if (code)
    {
      com_err(whoami, code, "Initializing auth context");
      send_int(conn, code);
      goto out;
    }

  if (uname(&uts) < 0)
    {
      com_err(whoami, errno, "Unable to get local hostname");
      send_int(conn, errno);
      goto out;
    }

  code = krb5_sname_to_principal(context, uts.nodename, service,
				 KRB5_NT_SRV_HST, &server);

  if (code)
    {
      com_err(whoami, code, "(krb5_sname_to_principal failed)");
      send_int(conn, code);
      goto out;
    }

  code = krb5_rd_req(context, &auth_con, &auth, server, NULL, NULL, &ticket);

  if (code)
    {
      strcpy(name, qmark);
      strcpy(inst, qmark);
      strcpy(realm, qmark);
      com_err(whoami, code, "auth for %s.%s@%s failed", name, inst, realm);
      send_int(conn, code);
      goto out;
    }

  code = krb5_copy_principal(context, ticket->enc_part2->client, &client);
  if (code)
    {
      com_err(whoami, code, "(krb5_copy_principal failed)");
      send_int(conn, code);
      goto out;
    }

  /* If there is an auth record in the config file matching the
   * authenticator we received, then accept it.  If there's no
   * auth record, assume [master]@[local realm].
   */
  if ((first = p = config_lookup("auth")))
    {
      do
	{
	  code = krb5_parse_name(context, p, &allowed);
	  if (code)
	    {
	      com_err(whoami, code, "(krb5_parse_name failed)");
	      send_int(conn, code);
	      goto out;
	    }
	  if (!krb5_principal_compare(context, client, allowed))
	    {
	      krb5_free_principal(context, allowed);
	      p = config_lookup("auth");
	    }
	  else
	    p = first;
	}
      while (p != first);
    }
  else 
    {
      mname = malloc(strlen(master) + 1);
      if (!mname)
	goto out;

      strcpy(mname, master);

      if (!krb5_get_default_realm(context, &lrealm))
	{
	  mname = realloc(mname, strlen(master) + strlen(lrealm) + 2);
	  if (!mname)
	    goto out;
	  strncat(mname, "@", 1);
	  strncat(mname, lrealm, strlen(lrealm));
	}
      else
	{
	  mname = realloc(mname, strlen(master) + strlen(KRB_REALM) + 2);
	  if (!mname)
	    goto out;
	  strncat(mname, "@", 1);
	  strncat(mname, KRB_REALM, strlen(KRB_REALM));
	}

      code = krb5_parse_name(context, mname, &allowed);
      free(mname);

      if (code)
	{
	  com_err(whoami, code, "(krb5_parse_name failed)");
	  send_int(conn, code);
	  goto out;
	}
    }

  code = EPERM;
  if (!krb5_principal_compare(context, client, allowed))
    {
      krb5_unparse_name(context, client, &pname);
      com_err(whoami, code, "auth for %s failed", pname);
      send_int(conn, code);
      goto out;
    }
  send_ok(conn);
  have_authorization = 1;
  
 out:
  if (pname)
    krb5_free_unparsed_name(context, pname);
  if (lrealm)
    free(lrealm);
  if (allowed)
    krb5_free_principal(context, allowed);
  if (client)
    krb5_free_principal(context, client);
  if (server)
    krb5_free_principal(context, server);
  if (ticket)
    krb5_free_ticket(context, ticket);
  krb5_free_data_contents(context, &auth);
  if (auth_con)
    krb5_auth_con_free(context, auth_con);
  return;
}
