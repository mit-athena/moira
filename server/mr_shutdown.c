/* $Id: mr_shutdown.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <errno.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/server/mr_shutdown.c $ $Id: mr_shutdown.c 3956 2010-01-05 20:56:56Z zacheiss $");

extern char *takedown;
extern char *whoami;

void sigshut(int sig)
{
  takedown = "Shut down by signal.";
}

void do_shutdown(client *cl)
{
  /*
   * This feature is no longer supported.  Sorry.
   */
  com_err(whoami, 0, "Shutdown request by %s rejected", cl->clname);
  client_reply(cl, EACCES);
}

