/* $Id: member.c,v 1.7 2009-08-11 18:29:47 zacheiss Exp $
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
#include <ctype.h>

#include <krb5.h>

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/clients/lib/member.c,v 1.7 2009-08-11 18:29:47 zacheiss Exp $");

int mrcl_validate_string_member(char *str)
{
  char *p, *lname, *ret;

  for (ret = str; *ret; ret++)
    {
      if (iscntrl(*ret))
	{
	  mrcl_set_message("STRING \"%s\" contains control characters, "
			   "which are not allowed.", str);
	  return MRCL_REJECT;
	}
    }

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
		       str);
      return MRCL_WARN;
    }

  mrcl_clear_message();
  return MRCL_SUCCESS;
}

int mrcl_validate_kerberos_member(char *str, char **ret)
{
  char *p;
  int code = 0;
  krb5_context context = NULL;
  char *default_realm = NULL;

  mrcl_clear_message();

  for (p = str; *p; p++)
    {
      if (isspace(*p) || *p == ',')
	{
	  mrcl_set_message("KERBEROS member \"%s\" may not contain whitespace "
			   "or commas.", str);
	  return MRCL_REJECT;
	}
    }

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

      code = krb5_init_context(&context);
      if (code)
        goto out;

      code = krb5_get_default_realm(context, &default_realm);
      if (code)
        goto out;

      *ret = malloc(strlen(str) + strlen(default_realm) + 2);
      sprintf(*ret, "%s@%s", str, default_realm);

      mrcl_set_message("Warning: default realm \"%s\" added to principal "
		       "\"%s\"", default_realm, str);

    out:
      if (default_realm)
        free(default_realm);
      if (context)
        krb5_free_context(context);
      if (!code)
        return code;
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
