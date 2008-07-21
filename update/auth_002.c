/* $Id: auth_002.c,v 1.9 1998-10-21 19:27:29 danw Exp $
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

#include <krb.h>

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_002.c,v 1.9 1998-10-21 19:27:29 danw Exp $");

static char service[] = "rcmd";
static char master[] = "sms";
static char qmark[] = "???";
extern des_cblock session;

/*
 * authentication request auth_002:
 *
 * >>> (STRING) "auth_002"
 * <<< (int) 0
 * >>> (STRING) ticket
 * <<< (int) code
 * <<< (STRING) nonce
 * >>> (STRING) encrypted nonce
 * <<< (int) code
 *
 */

void auth_002(int conn, char *str)
{
  char aname[ANAME_SZ], ainst[INST_SZ], arealm[REALM_SZ];
  AUTH_DAT ad;
  char *p, *first, *data;
  size_t size;
  KTEXT_ST ticket_st;
  des_key_schedule sched;
  des_cblock nonce, nonce2;
  long code;

  send_ok(conn);
  
  recv_string(conn, &data, &size);
  if (size > sizeof(ticket_st.dat))
    {
      code = KE_RD_AP_UNDEC;
      com_err(whoami, code, ": authenticator too large");
      send_int(conn, code);
      return;
    }
  memcpy(ticket_st.dat, data, size);
  free(data);
  ticket_st.mbz = 0;
  ticket_st.length = size;
  code = krb_rd_req(&ticket_st, service, krb_get_phost(hostname), 0,
		    &ad, KEYFILE);
  if (code)
    {
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
  if ((first = p = config_lookup("auth")))
    {
      do
	{
	  kname_parse(aname, ainst, arealm, p);
	  if (strcmp(aname, ad.pname) ||
	      strcmp(ainst, ad.pinst) ||
	      strcmp(arealm, ad.prealm))
	    p = config_lookup("auth");
	  else
	    p = first;
	}
      while (p != first);
    }
  else
    {
      strcpy(aname, master);
      strcpy(ainst, "");
      if (krb_get_lrealm(arealm, 1))
	strcpy(arealm, KRB_REALM);
    }
  code = EPERM;
  if (strcmp(aname, ad.pname) ||
      strcmp(ainst, ad.pinst) ||
      strcmp(arealm, ad.prealm))
    goto auth_failed;

  send_ok(conn);

  /* replay protection */
  des_random_key(&nonce);
  send_string(conn, (char *)nonce, sizeof(nonce));
  recv_string(conn, &data, &size);
  des_key_sched(ad.session, sched);
  des_ecb_encrypt(data, nonce2, sched, 0);
  free(data);
  if (memcmp(nonce, nonce2, sizeof(nonce)))
    goto auth_failed;
  send_ok(conn);

  have_authorization = 1;
  /* Stash away session key */
  memcpy(session, ad.session, sizeof(session));
  return;

auth_failed:
  com_err(whoami, code, "auth for %s.%s@%s failed",
	  ad.pname, ad.pinst, ad.prealm);
  send_int(conn, code);
}
