/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v 1.8 1998-01-05 19:53:07 danw Exp $
 *
 *	Copyright (C) 1987, 1990 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_sms_access_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_access.c,v 1.8 1998-01-05 19:53:07 danw Exp $";
#endif

#include <mit-copyright.h>
#include "mr_private.h"
#include <string.h>
#include <stdlib.h>

/*
 * Check access to a named query.
 */
int mr_access(char *name, int argc, char **argv)
{
  register char **nargv = malloc(sizeof(char *) * (argc + 1));
  register int status = 0;

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
  register mr_params *params = NULL;
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
