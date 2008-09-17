/* $Id$
 *
 * Library-internal routines for categorizing machines in terms
 * of email.
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

RCSID("$Header$");

static int save_sloc_machine(int argc, char **argv, void *sq);
static int save_alias_value(int argc, char **argv, void *sq);

/* Given a canonicalized machine name, ask the Moira server if it is of type
 * POP, LOCAL, or MAILHUB -- if none of those, we assume it's FOREIGN.
 */
int mailtype(char *machine)
{
  char *name;
  int status, match = 0;
  static struct save_queue *pop = NULL, *local = NULL;
  static struct save_queue *mailhub = NULL, *mailhub_name = NULL;
  static struct save_queue *imap = NULL, *exchange = NULL;

  mrcl_clear_message();

  /* 1. Check if the machine is an IMAP server. */
  if (!imap)
    {
      char *service = "POSTOFFICE";
      imap = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, imap);
      if (status && (status != MR_NO_MATCH))
	{
	  mrcl_set_message("Could not read list of IMAP servers: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
	}
    }

  /* Because of how sq_get_data works, we need to go through the entire
   * queue even if we find a match, so that it gets reset for the next
   * call.
   */
  while (sq_get_data(imap, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }

  if (match)
    return MAILTYPE_IMAP;


  /* 2. Check if the machine is a POP server. */
  if (!pop)
    {
      char *service = "POP";
      pop = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, pop);
      if (status && (status != MR_NO_MATCH))
	{
	  mrcl_set_message("Could not read list of POP servers: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
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
    return MAILTYPE_POP;


  /* 3. Check if the machine is "LOCAL". */
  if (!local)
    {
      char *service = "LOCAL";
      local = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, local);
      if (status && (status != MR_NO_MATCH))
	{
	  mrcl_set_message("Could not read list of LOCAL servers: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
	}
    }

  while (sq_get_data(local, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return MAILTYPE_LOCAL;

  
  /* 4. Check if the machine is one of the mailhubs. */
  if (!mailhub)
    {
      char *service = "MAILHUB";
      mailhub = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, mailhub);
      if (!status || status == MR_NO_MATCH)
	{
	  service = "NMAILHUB";
	  status = mr_query("get_server_locations", 1, &service,
			    save_sloc_machine, mailhub);
	}

      if (status && (status != MR_NO_MATCH))
	{
	  mrcl_set_message("Could not read list of MAILHUB servers: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
	}

    }

  while (sq_get_data(mailhub, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return MAILTYPE_MAILHUB;


  /* 5. Check if the machine is one of the external names of the mailhubs. */
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
	  mrcl_set_message("Could not read list of mailhub names: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
	}
    }

  while (sq_get_data(mailhub_name, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return MAILTYPE_MAILHUB;

  /* 6. Check for EXCHANGE service. */
  if (!exchange)
    {
      char *service = "EXCHANGE";
      exchange = sq_create();
      status = mr_query("get_server_locations", 1, &service,
			save_sloc_machine, exchange);
      if (status && (status != MR_NO_MATCH))
	{
	  mrcl_set_message("Could not read list of EXCHANGE servers: %s",
			   error_message(status));
	  return MAILTYPE_ERROR;
	}
    }

  while (sq_get_data(exchange, &name))
    {
      if (!match && !strcasecmp(name, machine))
	match = 1;
    }
  if (match)
    return MAILTYPE_EXCHANGE;

  return MAILTYPE_SMTP;
}

static int save_sloc_machine(int argc, char **argv, void *sq)
{
  sq_save_data(sq, strdup(argv[1]));
  return MR_CONT;
}

static int save_alias_value(int argc, char **argv, void *sq)
{
  sq_save_data(sq, canonicalize_hostname(strdup(argv[2])));
  return MR_CONT;
}
