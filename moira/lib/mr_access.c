/* $Id$
 *
 * Check access to a Moira query
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "mr_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

/*
 * Check access to a named query.
 */
int mr_access(char *name, int argc, char **argv)
{
  mr_params params, reply;
  int status;

  CHECK_CONNECTED;

  params.u.mr_procno = MR_ACCESS;
  params.mr_argc = argc + 1;
  params.mr_argv = malloc(sizeof(char *) * (argc + 1));
  if (!params.mr_argv)
    return ENOMEM;
  params.mr_argv[0] = name;
  memcpy(params.mr_argv + 1, argv, sizeof(char *) * argc);
  params.mr_argl = NULL;

  if ((status = mr_do_call(&params, &reply)) == MR_SUCCESS)
    status = reply.u.mr_status;

  mr_destroy_reply(reply);
  free(params.mr_argv);

  return status;
}
