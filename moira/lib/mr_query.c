/* $Id: mr_query.c,v 1.17 1998-02-15 17:49:06 danw Exp $
 *
 * Perform a Moira query
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.17 1998-02-15 17:49:06 danw Exp $");

/*
 * This routine is the primary external interface to the mr library.
 *
 * It builds a new argument vector with the query handle prepended,
 * and calls mr_query_internal.
 */
static int level = 0;

int mr_query(char *name, int argc, char **argv,
	     int (*callproc)(int, char **, void *), void *callarg)
{
  int status, stopcallbacks = 0;
  mr_params params, reply;

  CHECK_CONNECTED;
  if (level)
    return MR_QUERY_NOT_REENTRANT;

  params.u.mr_procno = MR_QUERY;
  params.mr_argc = argc + 1;
  params.mr_argl = NULL;
  params.mr_argv = malloc(sizeof(char *) * (argc + 1));
  if (!params.mr_argv)
    return ENOMEM;
  params.mr_argv[0] = name;
  memcpy(params.mr_argv + 1, argv, sizeof(char *) * argc);

  level++;
  if ((status = mr_do_call(&params, &reply)))
    goto punt;

  while ((status = reply.u.mr_status) == MR_MORE_DATA)
    {
      if (!stopcallbacks)
	stopcallbacks = (*callproc)(reply.mr_argc, reply.mr_argv, callarg);
      mr_destroy_reply(reply);

      if (mr_receive(_mr_conn, &reply) != MR_SUCCESS)
	{
	  mr_disconnect();
	  status = MR_ABORTED;
	  goto punt_1;
	}
    }

punt:
  mr_destroy_reply(reply);
punt_1:
  level--;
  free(params.mr_argv);

  return status;
}
