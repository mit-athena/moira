/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_002.c,v 1.6 1998-01-06 20:40:19 danw Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_auth_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/auth_002.c,v 1.6 1998-01-06 20:40:19 danw Exp $";
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include <gdb.h>
#include <krb.h>
#include <krb_et.h>
#include <netinet/in.h>
#include <errno.h>
#ifdef POSIX
#include <sys/utsname.h>
#endif

extern char buf[BUFSIZ];
extern int have_authorization;
extern struct sockaddr_in *client_address();
extern CONNECTION conn;
extern int code;
extern char *PrincipalHostname();
static char service[] = "rcmd";
static char master[] = "sms";
static char qmark[] = "???";
extern C_Block session;

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

int auth_002(char *str)
{
  STRING data;
  char realm[REALM_SZ];
  char aname[ANAME_SZ], ainst[INST_SZ], arealm[REALM_SZ];
  AUTH_DAT ad;
  char *p, *first, *config_lookup();
  KTEXT_ST ticket_st;
  struct utsname name;
  des_key_schedule sched;
  C_Block nonce, nonce2;

  if (send_ok())
    lose("sending okay for authorization (auth_002)");
  code = receive_object(conn, (char *)&data, STRING_T);
  if (code)
    {
      code = connection_errno(conn);
      lose("awaiting Kerberos authenticators");
    }
  uname(&name);
  ticket_st.mbz = 0;
  ticket_st.length = MAX_STRING_SIZE(data);
  memcpy(ticket_st.dat, STRING_DATA(data), MAX_STRING_SIZE(data));
  code = krb_rd_req(&ticket_st, service, krb_get_phost(name.nodename), 0,
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
  if (first = p = config_lookup("auth"))
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

  if (send_ok())
    lose("sending preliminary approval of authorization");

  /* replay protection */
  des_random_key(&nonce);
  STRING_DATA(data) = (char *)nonce;
  MAX_STRING_SIZE(data) = 8;
  if (send_object(conn, (char *)&data, STRING_T))
    lose("sending nonce");
  code = receive_object(conn, (char *)&data, STRING_T);
  if (code)
    {
      code = connection_errno(conn);
      goto auth_failed;
    }
  des_key_sched(ad.session, sched);
  des_ecb_encrypt(STRING_DATA(data), nonce2, sched, 0);
  if (memcmp(nonce, nonce2, sizeof(nonce)))
    goto auth_failed;

  if (send_ok())
    lose("sending approval of authorization");
  have_authorization = 1;
  /* Stash away session key */
  memcpy(session, ad.session, sizeof(session));
  return 0;
auth_failed:
  sprintf(buf, "auth for %s.%s@%s failed: %s",
	  ad.pname, ad.pinst, ad.prealm, error_message(code));
  {
    int rc;
    rc = send_object(conn, (char *)&code, INTEGER_T);
    code = rc;
  }
  if (code)
    lose("sending rejection of authenticator");
  return EPERM;
}
