/* $Id $
 *
 * This routine is part of the client library.  It handles
 * the protocol operations: invoking an update and getting the
 * Moira message of the day.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_ops.c,v 1.13 1998-02-05 22:51:29 danw Exp $");

/* Invoke a DCM update. */

int mr_do_update(void)
{
  int status;
  mr_params param_st;
  struct mr_params *params = NULL;
  struct mr_params *reply = NULL;

  CHECK_CONNECTED;
  params = &param_st;
  params->mr_version_no = sending_version_no;
  params->mr_procno = MR_DO_UPDATE;
  params->mr_argc = 0;
  params->mr_argl = NULL;
  params->mr_argv = NULL;

  if ((status = mr_do_call(params, &reply)) == 0)
    status = reply->mr_status;

  mr_destroy_reply(reply);

  return status;
}


/* Get the Moira motd.  This returns a Moira status, and motd will either
 * point to NULL or the motd in a static buffer.
 */

int mr_motd(char **motd)
{
  int status;
  mr_params param_st;
  struct mr_params *params = NULL;
  struct mr_params *reply = NULL;
  static char buffer[1024];

  *motd = NULL;
  CHECK_CONNECTED;
  params = &param_st;
  params->mr_version_no = sending_version_no;
  params->mr_procno = MR_MOTD;
  params->mr_argc = 0;
  params->mr_argl = NULL;
  params->mr_argv = NULL;

  if ((status = mr_do_call(params, &reply)))
    goto punt;

  while ((status = reply->mr_status) == MR_MORE_DATA)
    {
      if (reply->mr_argc > 0)
	{
	  strncpy(buffer, reply->mr_argv[0], sizeof(buffer));
	  *motd = buffer;
	}
      mr_destroy_reply(reply);
      reply = NULL;

      initialize_operation(_mr_recv_op, mr_start_recv, &reply, NULL);
      queue_operation(_mr_conn, CON_INPUT, _mr_recv_op);

      complete_operation(_mr_recv_op);
      if (OP_STATUS(_mr_recv_op) != OP_COMPLETE)
	{
	  mr_disconnect();
	  status = MR_ABORTED;
	  return status;
	}
    }
punt:
  mr_destroy_reply(reply);
  /* for backwards compatability */
  if (status == MR_UNKNOWN_PROC)
    return 0;
  else
    return status;
}
