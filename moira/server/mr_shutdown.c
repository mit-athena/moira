/* $Id: mr_shutdown.c,v 1.11 1998-02-15 17:49:15 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <errno.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.11 1998-02-15 17:49:15 danw Exp $");

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

