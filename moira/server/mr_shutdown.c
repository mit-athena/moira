/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.8 1998-01-05 19:53:33 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_shutdown_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.8 1998-01-05 19:53:33 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/errno.h>
#include "mr_server.h"

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
  cl->reply.mr_status = EACCES;
}

