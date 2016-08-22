/* $Id: afs.c 3973 2010-02-02 19:15:44Z zacheiss $
 *
 * Do Kerberos incremental updates
 */

#include <moira.h>
#include <moira_site.h>
#include <moira_schema.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com_err.h>
#include <krb5.h>
#include <kadm5/admin.h>

#define STOP_FILE "/moira/krb/nokrb"
#define file_exists(file) (access((file), F_OK) == 0)

#define MOIRA_SVR_PRINCIPAL "sms"
#define MAX_TRIES 3

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/incremental/afs/afs.c $ $Id: afs.c 3973 2010-02-02 19:15:44Z zacheiss $");

void check_krb(void);
long modify_kerberos(char *username, int activate);

char *whoami;
static char tbl_buf[1024];

int main(int argc, char **argv)
{
  int beforec, afterc, i, astate = 0, bstate = 0, tries = 0, activate;
  char *table, **before, **after;
  long status;

  whoami = ((whoami = strrchr(argv[0], '/')) ? whoami+1 : argv[0]);

  if (argc < 4)
    {
      com_err(whoami, 0, "Unable to process %s", "argc < 4");
      exit(1);
    }
  
  if (argc < (4 + atoi(argv[2]) + atoi(argv[3])))
    {
      com_err(whoami, 0, "Unable to process %s", "argc < (4 + beforec + afterc)");
      exit(1);
    }

  table = argv[1];
  beforec = atoi(argv[2]);
  before = &argv[4];
  afterc = atoi(argv[3]);
  after = &argv[4 + beforec];

  strcpy(tbl_buf, table);
  strcat(tbl_buf, " (");
  for (i = 0; i < beforec; i++)
    {
      if (i > 0)
	strcat(tbl_buf, ",");
      strcat(tbl_buf, before[i]);
    }
  strcat(tbl_buf, ")->(");
  for (i = 0; i < afterc; i++)
    {
      if (i > 0)
	strcat(tbl_buf, ",");
      strcat(tbl_buf, after[i]);
    }
  strcat(tbl_buf, ")");

  if (afterc > U_STATE)
    astate = atoi(after[U_STATE]);
  if (beforec > U_STATE)
    bstate = atoi(before[U_STATE]);

  check_krb();

  /* Reactivating a principal */
  if ((astate == 1) || (astate == 6) || (astate == 9))
    activate = 1;
  /* Deactivating a principal */
  else if ((astate == 3) || (astate == 10))
    activate = 0;
  /* Forcing password change */
  else if ((astate == 11) || (astate == 12))
    activate = 2;
  /* Can ignore other changes */
  else
    exit(0);

  while (status = modify_kerberos(after[U_NAME], activate))
    {
      if (++tries > MAX_TRIES)
	break;

      sleep(30);
    }

  if (status)
    {
      com_err(whoami, status, "while modifying Kerberos principal for user %s", after[U_NAME]);
      if (activate == 2)
	critical_alert(whoami, "incremental", "Couldn't %s Kerberos principal for user %s",
		       "modify", after[U_NAME]);
      else 
	critical_alert(whoami, "incremental", "Couldn't %s Kerberos principal for user %s",
		       activate ? "enable" : "disable", after[U_NAME]);
    }
  else
    {
      if (activate == 2)
	com_err(whoami, 0, "Successfully %s Kerberos principal for user %s",
		"modified", after[U_NAME]);
      else
	com_err(whoami, 0, "Successfully %sd Kerberos principal for user %s",
                activate ? "enable" : "disable", after[U_NAME]);
    }

  exit(0);
}

void check_krb(void)
{
  int i;

  for (i = 0; file_exists(STOP_FILE); i++)
    {
      if (i > 30)
	{
	  critical_alert(whoami, "incremental",
			 "AFS incremental failed (%s exists): %s",
			 STOP_FILE, tbl_buf);
	  exit(1);
	}
      sleep(60);
    }
}

/* Modify Kerberos principal */
long modify_kerberos(char *username, int activate)
{ 
  void *kadm_server_handle = NULL;
  krb5_context context = NULL;
  kadm5_ret_t status;
  krb5_principal princ;
  kadm5_principal_ent_rec dprinc;
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
  strcpy(admin_princ, MOIRA_SVR_PRINCIPAL);
  realm_params.mask = 0;
#endif

  status = krb5_init_context(&context);
  if (status)
    return status;

  memset(&princ, 0, sizeof(princ));
  memset(&dprinc, 0, sizeof(dprinc));

  status = krb5_parse_name(context, username, &princ);
  if (status)
    return status;

  status = kadm5_init_with_skey(admin_princ, NULL, KADM5_ADMIN_SERVICE,
                                &realm_params, KADM5_STRUCT_VERSION,
                                KADM5_API_VERSION_2, NULL,
                                &kadm_server_handle);
  if (status)
    goto cleanup;

  status = kadm5_get_principal(kadm_server_handle, princ, &dprinc, KADM5_PRINCIPAL_NORMAL_MASK);
  if (status)
    goto cleanup;

  mask |= KADM5_ATTRIBUTES;
  if (activate == 2)
    {
      /* Force password change */
      dprinc.attributes |= KRB5_KDB_REQUIRES_PWCHANGE;
      dprinc.attributes &= ~KRB5_KDB_DISALLOW_ALL_TIX;
    }
  else if (activate == 1)
    {
      /* Enable principal */
      dprinc.attributes &= ~KRB5_KDB_DISALLOW_ALL_TIX;
      dprinc.attributes &= ~KRB5_KDB_REQUIRES_PWCHANGE;
    }
  else
    {
      /* Disable principal */
      dprinc.attributes |= KRB5_KDB_DISALLOW_ALL_TIX;
      dprinc.attributes &= ~KRB5_KDB_REQUIRES_PWCHANGE;
    }

  status = kadm5_modify_principal(kadm_server_handle, &dprinc, mask);

 cleanup:
  krb5_free_principal(context, princ);
  kadm5_free_principal_ent(kadm_server_handle, &dprinc);
  if (kadm_server_handle)
    kadm5_destroy(kadm_server_handle);

  return status;
}
