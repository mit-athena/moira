/* $Id $
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

#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v 1.11 1998-02-05 22:51:25 danw Exp $");

int mr_access_internal(int argc, char **argv);

/*
 * Check access to a named query.
 */
int mr_access(char *name, int argc, char **argv)
{
  char **nargv = malloc(sizeof(char *) * (argc + 1));
  int status = 0;

  nargv[0] = name;
  memcpy(nargv + 1, argv, sizeof(char *) * argc);
  status = mr_access_internal(argc + 1, nargv);
  free(nargv);
  return status;
}

/*
 * Check access to a named query, where the query name is argv[0]
 * and the arguments are the rest of argv[].
 */
int mr_access_internal(int argc, char **argv)
{
  int status;
  mr_params params_st;
  mr_params *params = NULL;
  mr_params *reply = NULL;

  CHECK_CONNECTED;

  params = &params_st;
  params->mr_version_no = sending_version_no;
  params->mr_procno = MR_ACCESS;
  params->mr_argc = argc;
  params->mr_argl = NULL;
  params->mr_argv = argv;

  if ((status = mr_do_call(params, &reply)) == 0)
    status = reply->mr_status;

  mr_destroy_reply(reply);

  return status;
}
