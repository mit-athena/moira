/* $Id$
 *
 * Kerberos routines for registration server
 *
 * Copyright (C) 1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>
#include "reg_svr.h"

#if !defined(KRB4) && !defined(KRB5)
#define KRB5
#endif

#include <errno.h>
#include <string.h>

#include <com_err.h>

#define KRB5_DEPRECATED 1
#define KRB5_PRIVATE 1

#ifdef KRB4
#include <des.h>
#include <kadm.h>
#include <kadm_err.h>
#include <krb.h>
#endif

#ifdef KRB5
#include <kadm5/admin.h>
#include <krb5.h>

krb5_context context;
#endif

RCSID("$HeadURL$ $Id$");

extern char *hostname, *shorthostname;

#ifdef KRB5
long init_kerberos(void)
{
  krb5_error_code code;

  /* Initialize Kerberos stuff. */
  code = krb5_init_context(&context);
  if (code)
    return code;
  krb_set_tkt_string("/tmp/tkt_ureg");
  return 0;
}

/* Check the kerberos database to see if a principal exists */
long check_kerberos(char *username)
{
  void *kadm_server_handle = NULL;
  kadm5_ret_t status;
  krb5_principal princ;
  kadm5_principal_ent_rec dprinc;
  kadm5_config_params realm_params;
  char admin_princ[256];
#ifdef KERBEROS_TEST_REALM
  char ubuf[256];

  sprintf(ubuf, "%s@%s", username, KERBEROS_TEST_REALM);
  username = ubuf;
#else
  strcpy(admin_princ, REG_SVR_PRINCIPAL);
  realm_params.mask = 0;
#endif

  memset(&princ, 0, sizeof(princ));
  memset(&dprinc, 0, sizeof(dprinc));

  status = krb5_parse_name(context, username, &princ);
  if (status)
    return status;

  status = kadm5_init_with_skey(admin_princ, NULL, KADM5_ADMIN_SERVICE,
                                &realm_params, KADM5_STRUCT_VERSION,
                                KADM5_API_VERSION_2, NULL, &kadm_server_handle);
  if (status)
    goto cleanup;

  status =  kadm5_get_principal(kadm_server_handle, princ, &dprinc, KADM5_PRINCIPAL_NORMAL_MASK);

 cleanup:
  krb5_free_principal(context, princ);
  if (kadm_server_handle)
    kadm5_destroy(kadm_server_handle);

  if (status == KADM5_OK)
    return MR_IN_USE;
  else if (status == KADM5_UNK_PRINC)
    return MR_SUCCESS;
  else
    return MR_INTERNAL;
}

/* Create a new principal in Kerberos */
long register_kerberos(char *username, char *password)
{
  void *kadm_server_handle = NULL;
  kadm5_ret_t status;
  kadm5_principal_ent_rec princ;
  kadm5_policy_ent_rec defpol;
  kadm5_config_params realm_params;
  char admin_princ[256];
  long mask = 0;
#ifdef KERBEROS_TEST_REALM
  char ubuf[256];

  sprintf(admin_princ, "moira/%s@%s", hostname, KERBEROS_TEST_REALM);
  sprintf(ubuf, "%s@%s", username, KERBEROS_TEST_REALM);
  username = ubuf;
  realm_params.realm = KERBEROS_TEST_REALM;
  realm_params.mask = KADM5_CONFIG_REALM;
#else
  strcpy(admin_princ, REG_SVR_PRINCIPAL);
  realm_params.mask = 0;
#endif

  memset(&princ, 0, sizeof(princ));

  status = krb5_parse_name(context, username, &(princ.principal));
  if (status)
    return status;


  status = kadm5_init_with_skey(admin_princ, NULL, KADM5_ADMIN_SERVICE,
				&realm_params, KADM5_STRUCT_VERSION,
				KADM5_API_VERSION_2, NULL, &kadm_server_handle);
  if (status)
    goto cleanup;

  /* Assign "default" policy if it exists. */
  if (!kadm5_get_policy(kadm_server_handle, "default", &defpol))
    {
      princ.policy = "default";
      mask |= KADM5_POLICY;
      (void) kadm5_free_policy_ent(kadm_server_handle, &defpol);
    } 

  mask |= KADM5_PRINCIPAL | KADM5_ATTRIBUTES;
  princ.attributes |= KRB5_KDB_REQUIRES_PRE_AUTH | KRB5_KDB_DISALLOW_SVR;
  status = kadm5_create_principal(kadm_server_handle, &princ, mask, password);

cleanup:
  krb5_free_principal(context, princ.principal);
  if (kadm_server_handle)
    kadm5_destroy(kadm_server_handle);

  if (status == KADM5_DUP)
    return MR_IN_USE;
  else if (status == KADM5_PASS_Q_TOOSHORT || 
	   status == KADM5_PASS_Q_CLASS ||
	   status == KADM5_PASS_Q_DICT)
    return MR_QUALITY;
  else return status;
}
#endif

#ifdef KRB4
char realm[REALM_SZ];

long init_kerberos(void)
{
  return krb_get_lrealm(realm, 1);
}

long check_kerberos(char *username)
{
  long status;

  status = krb_get_pw_in_tkt(username, "", realm, "krbtgt", realm, 1, "");
  if (status == KDC_PR_UNKNOWN)
    return MR_SUCCESS;
  else
    return MR_IN_USE;
}

long register_kerberos(char *username, char *password)
{
  long status;
  Kadm_vals new;
  des_cblock key;
  unsigned long *lkey = (unsigned long *)key;

  if ((status = krb_get_svc_in_tkt(MOIRA_SNAME, shorthostname, realm,
				   PWSERV_NAME, KADM_SINST, 3, KEYFILE)))
    return status;

  if ((status = kadm_init_link(PWSERV_NAME, KADM_SINST, realm)) !=
      KADM_SUCCESS)
    return status;

  memset(&new, 0, sizeof(new));
  SET_FIELD(KADM_DESKEY, new.fields);
  SET_FIELD(KADM_NAME, new.fields);

  des_string_to_key(password, key);
  new.key_low = htonl(lkey[0]);
  new.key_high = htonl(lkey[1]);
  strcpy(new.name, username);

  status = kadm_add(&new);
  memset(&new, 0, sizeof(new));
  dest_tkt();

  if (status == KADM_INUSE)
    return MR_IN_USE;
  else
    return status;
}
#endif
