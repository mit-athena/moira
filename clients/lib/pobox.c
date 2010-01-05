/* $Id: pobox.c,v 1.8 2008-08-22 17:49:11 zacheiss Exp $
 *
 * Shared routines for pobox changing.
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

#include <com_err.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/lib/pobox.c,v 1.8 2008-08-22 17:49:11 zacheiss Exp $");

static int save_sloc_machine(int argc, char **argv, void *sq);
static int save_alias_value(int argc, char **argv, void *sq);

extern char *whoami;

#ifdef _WIN32
#define strtok_r(s, tokens, resume) strtok(s, tokens)
#endif /* _WIN32 */

int mrcl_validate_pobox_smtp(char *user, char *address, char **ret)
{
  char *addr, *retaddr, *p, *lasts = NULL;
  char *machine = NULL, *m;
  int oldlen, len, status = MRCL_SUCCESS;

  /* Make a private copy of address that we can mangle. */
  addr = strdup(address);

  retaddr = strdup("");
  len = 1;

  /* For each comma-delimited address, canonicalize the hostname and
   * verify that the address won't cause mail problems.
   */
  for (p = strtok_r(addr, ", ", &lasts); p; p = strtok_r(NULL, ",", &lasts))
    {
      m = strchr(p, '@');
      if (m)
	{
	  *m++ = '\0';		/* get rid of the @ sign */
	  m = strtrim(m);	/* get rid of whitespace */
	}
      else
	{
	  mrcl_set_message("No at sign (@) in address \"%s\".", p);
	  status = MRCL_REJECT;
	  goto cleanup;
	}

      if (strlen(m) > 0)
	{
	  machine = canonicalize_hostname(strdup(m));
	}
      else
	{
	  mrcl_set_message("No hostname in address \"%s@\".", p);
	  status = MRCL_REJECT;
	  goto cleanup;
	}
      
      switch (mailtype(machine))
	{
	case MAILTYPE_IMAP:
	  mrcl_set_message("Cannot forward mail to IMAP server %s.\n " 
			   "Use \"chpobox -p\" if you are trying to unset "
			   "your mail forwarding.", machine);
	  status = MRCL_REJECT;
	  goto cleanup;

	case MAILTYPE_POP:
	  if (strcmp(p, user))
	    {
	      mrcl_set_message("The name on the POP box (%s) must match "
			       "the username (%s).", p, user);
	      status = MRCL_REJECT;
	      goto cleanup;
	    }
	  /* Fall through */

	case MAILTYPE_LOCAL:
	  if ((m = strchr(machine, '.')))
	    *m = '\0';
	  machine = realloc(machine, strlen(machine) + 6);
	  strcat(machine, ".LOCAL");
	  break;

	case MAILTYPE_MAILHUB:
	case MAILTYPE_EXCHANGE:
	  if (!strcmp(p, user))
	    {
	      mrcl_set_message("The address \"%s@%s\" would create a mail "
			       "loop.\nSet a POP pobox if you want local "
			       "mail delivery.", p, machine);
	      status = MRCL_REJECT;
	      goto cleanup;
	    }
	  else
	    {
	      mrcl_set_message("Cannot forward mail to a local mailing "
			       "address (%s@%s).", p, machine);
	      status = MRCL_REJECT;
	      goto cleanup;
	    }

	case MAILTYPE_SMTP:
	  if (*m != '"' && strcasecmp(m, machine))
	    {
	      mrcl_set_message("Warning: hostname %s canonicalized to %s\n",
			       m, machine);
	    }
	  break;

	default:
	  status = MRCL_MOIRA_ERROR;
	  goto cleanup;
	}

      oldlen = len;
      len += (oldlen > 1 ? 2 : 0) + strlen(p) + strlen(machine) + 1;
      retaddr = realloc(retaddr, len);
      sprintf(retaddr + oldlen - 1, "%s%s@%s", oldlen > 1 ? ", " : "",
	      p, machine);
      free(machine);
      machine = NULL; /* Make sure it doesn't get freed again later. */
    }

 cleanup:
  free(addr);
  if (status == MRCL_SUCCESS)
    *ret = retaddr;
  else
    free(retaddr);
  free(machine);

  return status;
}

static int save_sloc_machine(int argc, char **argv, void *sq)
{
  sq_save_data(sq, strdup(argv[1]));
  return MR_CONT;
}

static int save_alias_value(int argc, char **argv, void *sq)
{
  sq_save_data(sq, strdup(argv[2]));
  return MR_CONT;
}
