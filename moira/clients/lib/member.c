/* $Id$
 *
 * Shared routines for playing with list membership.
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>
#include "mrclient-internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <krb5.h>

RCSID("$HeadURL$ $Id$");

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

/* MIT ID must be 9 digits, all numeric. */
#define ID_MEMBER_LENGTH 9

int mrcl_validate_id_member(char *type, char **ret_type, char *member, char **ret_member)
{
  int len, status;
  char *p, *args[1], *argv[30];
  char *canon;

  mrcl_clear_message();

  len = strlen(member);
  if (len != ID_MEMBER_LENGTH) 
    {
      mrcl_set_message("ID \"%s\" must be 9 digits long.", member);
      return MRCL_REJECT;
    }

  for (p = member; *p; p++)
    {
      if (!isdigit(*p))
	{
	  mrcl_set_message("ID \"%s\" contains non-numeric characters, "
			   "which are not allowed.", member);
	  return MRCL_REJECT;
	}
    }

  /* Allow user to disable this behavior at runtime. */
  canon = getenv("MOIRA_CANONICALIZE_ID_MEMBER");
  if (canon && !strcmp(canon, "0"))
    {
      *ret_type = strdup(type);
      *ret_member = strdup(member);
      return MRCL_SUCCESS;
    }

  args[0] = member;
  status = mr_query("get_user_account_by_id", 1, args, save_query_info, argv);
  if (status && (status != MR_NO_MATCH))
    {
      mrcl_set_message("Could not look up user accounts for ID \"%s\": %s",
		       member, error_message(status));
      return MRCL_MOIRA_ERROR;
    }

  /* No user accounts for this ID, return it as is. */
  if (status == MR_NO_MATCH)
    {
      *ret_type = strdup(type);
      *ret_member = strdup(member);
      return MRCL_SUCCESS;
    }

  /* If the account isn't active yet, same deal. */
  status = atoi(argv[U_STATE]);
  if (status == US_NO_LOGIN_YET || status == US_NO_LOGIN_YET_KERBEROS_ONLY)
    {
      *ret_type = strdup(type);
      *ret_member = strdup(member);
      return MRCL_SUCCESS;
    }

  /* If we got here, we successfully resolved the MIT ID to an active USER. */
  mrcl_set_message("Resolved ID \"%s\" to USER \"%s\".", member, argv[U_NAME]);
  *ret_type = strdup("USER");
  *ret_member = strdup(argv[U_NAME]);

  return MRCL_SUCCESS;
}

/* Parse a line of input, fetching a member.  NULL is returned if a member
 * is not found.  ';' is a comment character.
 */

struct mrcl_ace_type *mrcl_parse_member(char *s)
{
  struct mrcl_ace_type *m;
  char *p, *lastchar;
  
  while (*s && isspace(*s))
    s++;
  lastchar = p = s;
  while (*p && *p != '\n' && *p != ';')
    {
      if (isprint(*p) && !isspace(*p))
        lastchar = p++;
      else
        p++;
    }
  lastchar++;
  *lastchar = '\0';
  if (p == s || strlen(s) == 0)
    return NULL;
  
  if (!(m = malloc(sizeof(struct mrcl_ace_type))))
    return NULL;
  m->tag = strdup("");

  if ((p = strchr(s, ':')))
    {
      *p = '\0';
      m->name = ++p;
      if (!strcasecmp("user", s))
        m->type = MRCL_M_USER;
      else if (!strcasecmp("list", s))
        m->type = MRCL_M_LIST;
      else if (!strcasecmp("string", s))
        m->type = MRCL_M_STRING;
      else if (!strcasecmp("kerberos", s))
        m->type = MRCL_M_KERBEROS;
      else if (!strcasecmp("machine", s))
        m->type = MRCL_M_MACHINE;
      else if (!strcasecmp("id", s))
	m->type = MRCL_M_ID;
      else if (!strcasecmp("none", s))
        m->type = MRCL_M_NONE;
      else
        {
          m->type = MRCL_M_ANY;
          *(--p) = ':';
          m->name = s;
        }
      m->name = strdup(m->name);
    }
  else
    {
      m->name = strdup(s);
      m->type = strcasecmp(s, "none") ? MRCL_M_ANY : MRCL_M_NONE;
    }
  return m;
}

int save_query_info(int argc, char **argv, void *hint)
{
  int i;
  char **nargv = hint;

  if (atoi(argv[U_STATE]) != US_REGISTERED && atoi(argv[U_STATE]) != US_REGISTERED_KERBEROS_ONLY)
    return MR_CONT;

  for(i = 0; i < argc; i++)
    nargv[i] = strdup(argv[i]);

  return MR_CONT;
}
