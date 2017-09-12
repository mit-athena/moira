/* $Id$
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#define KTEXT void*
#include <mr_krb.h>
#include <krb5.h>
#include <update.h>

RCSID("$HeadURL$ $Id$");

static char realm[REALM_SZ];
static char master[INST_SZ] = "sms";
static char service[ANAME_SZ] = "rcmd";
krb5_context context = NULL;

static int get_mr_krb5_tgt(krb5_context context, krb5_ccache ccache);

int get_mr_krb5_update_ticket(char *host, krb5_data *auth)
{
  krb5_auth_context auth_con = NULL;
  krb5_ccache ccache = NULL;
  krb5_error_code code;
  int pass = 1;

  code = krb5_init_context(&context);
  if (code)
    goto out;

  code = krb5_auth_con_init(context, &auth_con);
  if (code)
    goto out;

  code = krb5_cc_default(context, &ccache);
  if (code)
    goto out;

 try_it:
  code = krb5_mk_req(context, &auth_con, 0, "host", host, NULL, ccache,
		     auth);
  if (code)
    {
      if (pass == 1)
	{
	  if ((code = get_mr_krb5_tgt(context, ccache)))
	    {
	      com_err(whoami, code, "can't get Kerberos v5 TGT");
	      return code;
	    }
	  pass++;
	  goto try_it;
	}
      com_err(whoami, code, "in krb5_mk_req");
    }

 out:
  if (ccache)
    krb5_cc_close(context, ccache);
  if (auth_con)
    krb5_auth_con_free(context, auth_con);
  return code;
}

int get_mr_krb5_tgt(krb5_context context, krb5_ccache ccache)
{
  krb5_creds my_creds;
  krb5_principal me = NULL;
  krb5_error_code code;

  memset(&my_creds, 0, sizeof(my_creds));

  code = krb5_parse_name(context, master, &me);
  if (code)
    goto out;
  
  code = krb5_get_init_creds_keytab(context, &my_creds, me, NULL, 0, NULL, NULL);
  if (code)
    goto out;
  
  code = krb5_cc_initialize(context, ccache, me);
  if (code)
    goto out;

  code = krb5_cc_store_cred(context, ccache, &my_creds);
  if (code)
    goto out;

 out:
  if (me)
    krb5_free_principal(context, me);
  krb5_free_cred_contents(context, &my_creds);

  return code;
}

int get_mr_update_ticket(char *host, KTEXT ticket)
{
  return MR_NO_KRB4;
}
