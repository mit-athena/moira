/* $Id: pobox.c,v 1.3 2000-02-22 17:45:50 rbasch Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com_err.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/lib/pobox.c,v 1.3 2000-02-22 17:45:50 rbasch Exp $");

enum { POTYPE_ERROR, POTYPE_POP, POTYPE_LOCAL, POTYPE_MAILHUB, POTYPE_SMTP };
static int potype(char *machine);
static int save_sloc_machine(int argc, char **argv, void *sq);
static int save_alias_value(int argc, char **argv, void *sq);

extern char *whoami;

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
	  com_err(whoami, 0, "No at sign (@) in address \"%s\".", p);
	  status = MRCL_REJECT;
	  goto cleanup;
	}

      if (strlen(m) > 0)
	{
	  machine = canonicalize_hostname(strdup(m));
	}
      else
	{
	  com_err(whoami, 0, "No hostname in address \"%s@\".", p);
	  status = MRCL_REJECT;
	  goto cleanup;
	}
      
      switch (potype(machine))
	{
	case POTYPE_POP:
	  if (strcmp(p, user))
	    {
	      com_err(whoami, 0, "The name on the POP box (%s) must match "
		      "the username (%s).", p, user);
	      status = MRCL_REJECT;
	      goto cleanup;
	    }
	  /* Fall through */

	case POTYPE_LOCAL:
	  if ((m = strchr(machine, '.')))
	    *m = '\0';
	  machine = realloc(machine, strlen(machine) + 6);
	  strcat(machine, ".LOCAL");
	  break;

	case POTYPE_MAILHUB:
	  if (!strcmp(p, user))
	    {
	      com_err(whoami, 0, "The address \"%s@%s\" would create a mail "
		      "loop.", p, machine);
	      com_err(NULL, 0, "Set a POP pobox if you want local mail "
		      "delivery.");
	      status = MRCL_REJECT;
	      goto cleanup;
	    }
	  else
	    {
	      com_err(whoami, 0, "Cannot forward mail to a local mailing "
		      "address (%s@%s).", p, machine);
	      status = MRCL_REJECT;
	      goto cleanup;
	    }

	case POTYPE_SMTP:
	  if (*m != '"' && strcasecmp(m, machine))
	    {
	      com_err(whoami, 0, "Warning: hostname %s canonicalized to %s\n",
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

/* Given a canonicalized machine name, ask the Moira server if it is of type
 * POP, LOCAL, or MAILHUB -- if none of those, we assume it's foreign.
 */
static int potype(char *machine)
{
  char *name;
  int status, match = 0;
  static struct save_queue *pop = NULL, *local = NULL;
  static struct save_queue *mailhub = NULL, *mailhub_name = NULL;

  /* 1. Check if the machine is a POP server. */
  if (!pop)
    {
      char *service = "POP";
      pop = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, pop);
      if (status && (status != MR_NO_MATCH))
	{
	  com_err(whoami, status, "while reading list of POP servers");
	  return POTYPE_ERROR;
	}
    }

  /* Because of how sq_get_data works, we need to go through the entire
   * queue even if we find a match, so that it gets reset for the next
   * call.
   */
  while (sq_get_data(pop, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return POTYPE_POP;


  /* 2. Check if the machine is "LOCAL". */
  if (!local)
    {
      char *service = "LOCAL";
      local = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, local);
      if (status && (status != MR_NO_MATCH))
	{
	  com_err(whoami, status, "while reading list of LOCAL servers");
	  return POTYPE_ERROR;
	}
    }

  while (sq_get_data(local, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return POTYPE_LOCAL;

  
  /* 3. Check if the machine is one of the mailhubs. */
  if (!mailhub)
    {
      char *service = "MAILHUB";
      mailhub = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, mailhub);
      if (status && (status != MR_NO_MATCH))
	{
	  com_err(whoami, status, "while reading list of MAILHUB servers");
	  return POTYPE_ERROR;
	}
    }

  while (sq_get_data(mailhub, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return POTYPE_MAILHUB;


  /* 4. Check if the machine is one of the external names of the mailhubs. */
  if (!mailhub_name)
    {
      char *argv[3];
      mailhub_name = sq_create();
      argv[0] = "mailhub";
      argv[1] = "TYPE";
      argv[2] = "*";
      status = mr_query("get_alias", 3, argv, save_alias_value, mailhub_name);
      if (status && (status != MR_NO_MATCH))
	{
	  com_err(whoami, status, "while reading list of mailhub names");
	  return POTYPE_ERROR;
	}
    }

  while (sq_get_data(mailhub_name, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return POTYPE_MAILHUB;

  return POTYPE_SMTP;
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
