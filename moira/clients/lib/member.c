/* $Id: member.c,v 1.1 2000-08-10 02:05:35 zacheiss Exp $
 *
 * Shared routines for playing with list membership.
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <mrclient.h>
#include "mrclient-internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/lib/member.c,v 1.1 2000-08-10 02:05:35 zacheiss Exp $");

static char default_realm[REALM_SZ];

int mrcl_validate_string_member(char *str)
{
  char *p, *lname;

  p = strchr(str, '@');
  if (p)
    {
      char *host = canonicalize_hostname(strdup(++p));

      if (mailtype(host) != MAILTYPE_SMTP)
	{
	  free(host);
	  lname = strdup(str);
	  *strchr(str, '@') = '\0';
	  mrcl_set_message("STRING \"%s\" should be USER or LIST \"%s\" "
			   "because it is a local name.", lname, str);
	  free(lname);
	  return MRCL_REJECT;
	}
      free(host);
    }
  else if (!strpbrk(str, "%!"))
    {
      mrcl_set_message("STRING \"%s\" is not a foreign mail address.\nAdding "
		       "it to a mailing list may cause the list to break.",
		       lname);
      return MRCL_REJECT;
    }

  mrcl_clear_message();
  return MRCL_SUCCESS;
}

int mrcl_validate_kerberos_member(char *str, char **ret)
{
  char *p;

  mrcl_clear_message();

  p = strchr(str, '@');
  if (!p)
    {
      /* An IP address is not a Kerberos principal, but we allow it
       * for AFS purposes.
       */
      if (strtoul(str, &p, 10) < 256 && (*p == '.') &&
	  strtoul(p + 1, &p, 10) < 256 && (*p == '.') &&
	  strtoul(p + 1, &p, 10) < 256 && (*p == '.') &&
	  strtoul(p + 1, &p, 10) < 256 && !*p)
	{
	  *ret = strdup(str);
	  return MRCL_SUCCESS;
	}

      if (!*default_realm)
	krb_get_lrealm(default_realm, 1);

      *ret = malloc(strlen(str) + strlen(default_realm) + 2);
      sprintf(*ret, "%s@%s", str, default_realm);

      mrcl_set_message("Warning: default realm \"%s\" added to principal "
		       "\"%s\"", default_realm, str);
      return MRCL_SUCCESS;
    }

  /* Check capitalization. */
  *ret = strdup(str);
  p = strchr(*ret, '@');
  while (*++p)
    {
      if (islower(*p))
	{
	  *p = toupper(*p);
	  mrcl_set_message("Warning: set realm in \"%s\" to all caps.", *ret);
	}
    }

  return MRCL_SUCCESS;
}
