/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.6 1990-03-19 15:42:16 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_sms_shutdown_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.6 1990-03-19 15:42:16 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <strings.h>
#include <sys/errno.h>
#include "mr_server.h"

extern char *takedown;
extern char *whoami;

sigshut()
{
	takedown = "Shut down by signal.";
}

void
do_shutdown(cl)
	client *cl;
{
	static char buf[BUFSIZ];
	/*
	 * This feature is no longer supported.  Sorry.
	 */
	com_err(whoami, 0, "Shutdown request by %s rejected",
		cl->clname);
	cl->reply.mr_status = EACCES;
}
		
