/* $Id $
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

#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_query.c,v 1.14 1998-02-05 22:51:30 danw Exp $");

int mr_query_internal(int argc, char **argv,
		      int (*callback)(int, char **, void *), void *callarg);

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
  char **nargv = malloc(sizeof(char *) * (argc + 1));
  int status = 0;

  nargv[0] = name;
  memcpy(nargv + 1, argv, sizeof(char *) * argc);
  status = mr_query_internal(argc + 1, nargv, callproc, callarg);
  free(nargv);
  return status;
}

/*
 * This routine makes a Moira query.
 *
 * argv[0] is the query name.
 * argv[1..argc-1] are the query arguments.
 *
 * callproc is called once for each returned value, with arguments
 * argc, argv, and callarg.
 * If it returns a non-zero value, further calls to it are not done, and
 * all future data from the server is ignored (there should be some
 * way to send it a quench..)
 */

int mr_query_internal(int argc, char **argv,
		      int (*callproc)(int, char **, void *), void *callarg)
{
  int status;
  mr_params params_st;
  mr_params *params = NULL;
  mr_params *reply = NULL;
  int stopcallbacks = 0;

  if (level)
    return MR_QUERY_NOT_REENTRANT;

  CHECK_CONNECTED;
  level++;

  params = &params_st;
  params->mr_version_no = sending_version_no;
  params->mr_procno = MR_QUERY;
  params->mr_argc = argc;
  params->mr_argl = NULL;
  params->mr_argv = argv;

  if ((status = mr_do_call(params, &reply)))
    goto punt;

  while ((status = reply->mr_status) == MR_MORE_DATA)
    {
      if (!stopcallbacks && callproc)
	stopcallbacks = (*callproc)(reply->mr_argc, reply->mr_argv, callarg);
      mr_destroy_reply(reply);
      reply = NULL;

      initialize_operation(_mr_recv_op, mr_start_recv, &reply, NULL);
      queue_operation(_mr_conn, CON_INPUT, _mr_recv_op);

      complete_operation(_mr_recv_op);
      if (OP_STATUS(_mr_recv_op) != OP_COMPLETE)
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
  return status;
}
