/* $Id: utils.c,v 1.3 2000-01-06 21:26:33 danw Exp $
 *
 * Random client utilities.
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <mrclient.h>

#include <stdlib.h>
#include <stdio.h>

#include <com_err.h>
#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/lib/utils.c,v 1.3 2000-01-06 21:26:33 danw Exp $");

extern char *whoami;

int mrcl_connect(char *server, char *client, int version, int auth)
{
  int status;
  char *motd;

  status = mr_connect(server);
  if (status)
    {
      com_err(whoami, status, "while connecting to Moira");
      return MRCL_FAIL;
    }

  status = mr_motd(&motd);
  if (status)
    {
      mr_disconnect();
      com_err(whoami, status, "while checking server status");
      return MRCL_FAIL;
    }
  if (motd)
    {
      fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
	      motd);
      mr_disconnect();
      return MRCL_FAIL;
    }

  status = mr_version(version);
  if (status)
    {
      if (status == MR_UNKNOWN_PROC)
	{
	  if (version > 2)
	    status = MR_VERSION_HIGH;
	  else
	    status = MR_SUCCESS;
	}

      if (status == MR_VERSION_HIGH)
	{
	  com_err(whoami, 0, "Warning: This client is running newer code than the server.");
	  com_err(whoami, 0, "Some operations may not work.");
	}
      else if (status && status != MR_VERSION_LOW)
	{
	  com_err(whoami, status, "while setting query version number.");
	  mr_disconnect();
	  return MRCL_FAIL;
	}
    }

  if (auth)
    {
      status = mr_auth(client);
      if (status)
	{
	  com_err(whoami, status, "while authenticating to Moira.");
	  mr_disconnect();
	  return MRCL_AUTH_ERROR;
	}
    }

  return MRCL_SUCCESS;
}

char *mrcl_krb_user(void)
{
  int status;
  static char pname[ANAME_SZ];

  status = tf_init(TKT_FILE, R_TKT_FIL);
  if (status == KSUCCESS)
    {
      status = tf_get_pname(pname);
      tf_close();
    }

  if (status != KSUCCESS)
    {
      /* In case mr_init hasn't been called yet. */
      initialize_krb_error_table();
      com_err(whoami, status, "reading Kerberos ticket file.");
      return NULL;
    }

  return pname;
}
