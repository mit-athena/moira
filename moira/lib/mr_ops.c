/* $Id$
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

/* Invoke a DCM update. */

int mr_do_update(void)
{
  int status;
  mr_params params, reply;

  CHECK_CONNECTED;
  params.u.mr_procno = MR_DO_UPDATE;
  params.mr_argc = 0;
  params.mr_argl = NULL;
  params.mr_argv = NULL;

  if ((status = mr_do_call(&params, &reply)) == MR_SUCCESS)
    status = reply.u.mr_status;

  mr_destroy_reply(reply);

  return status;
}


/* Get the Moira motd.  This returns a Moira status, and motd will either
 * point to NULL or the motd in a static buffer.
 */

int mr_motd(char **motd)
{
  int status;
  mr_params params, reply;
  static char *buffer = NULL;

  *motd = NULL;
  CHECK_CONNECTED;
  params.u.mr_procno = MR_MOTD;
  params.mr_argc = 0;
  params.mr_argl = NULL;
  params.mr_argv = NULL;

  if ((status = mr_do_call(&params, &reply)))
    goto punt;

  while ((status = reply.u.mr_status) == MR_MORE_DATA)
    {
      if (reply.mr_argc > 0)
	{
	  buffer = realloc(buffer, reply.mr_argl[0] + 1);
	  if (!buffer)
	    {
	      mr_disconnect();
	      return ENOMEM;
	    }
	  strcpy(buffer, reply.mr_argv[0]);
	  *motd = buffer;
	}
      mr_destroy_reply(reply);
      if (mr_receive(_mr_conn, &reply) != MR_SUCCESS)
	{
	  mr_disconnect();
	  return MR_ABORTED;
	}
    }
punt:
  mr_destroy_reply(reply);

  return status;
}

/* Exchange query version info with the server. */

int mr_version(int version)
{
  int status;
  mr_params params, reply;
  char vbuf[10], *arg;

  CHECK_CONNECTED;

  sprintf(vbuf, "%d", version);
  arg = strdup(vbuf);
  params.u.mr_procno = MR_SETVERSION;
  params.mr_argc = 1;
  params.mr_argl = NULL;
  params.mr_argv = &arg;

  status = mr_do_call(&params, &reply);
  free(arg);

  if (status == MR_SUCCESS)
    {
      status = reply.u.mr_status;

      if (status == MR_VERSION_LOW && getenv("MOIRA_LOW_VERSION_WARNING"))
	fprintf(stderr, "Warning: This client is out of date.\n");
    }
  mr_destroy_reply(reply);

  return status;
}
