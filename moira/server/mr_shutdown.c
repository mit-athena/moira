/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.3 1987-08-04 02:41:29 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  87/06/21  16:42:12  wesommer
 * Performance work, rearrangement of include files.
 * 
 * Revision 1.1  87/06/02  20:07:18  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_shutdown_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.3 1987-08-04 02:41:29 wesommer Exp $";
#endif lint

#include <strings.h>
#include <sys/errno.h>
#include "sms_server.h"

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

	if (cl->args->sms_argc != 1) {
		cl->reply.sms_status = EINVAL;
		return;
	}
		
	if (!cl->clname) {
		com_err(whoami, 0, "Unauthenticated shutdown request rejected",
			cl->clname);
		cl->reply.sms_status = EACCES;
		return;
	}
	if (!strcmp(cl->clname, "wesommer@ATHENA.MIT.EDU") ||
	    !strcmp(cl->clname, "mike@ATHENA.MIT.EDU")) {
		com_err(whoami, 0, "Shut down by %s", cl->clname);
		(void) strcpy(buf, "Reason for shutdown: ");
		(void) strcat(buf, cl->args->sms_argv[0]);
		takedown = buf;
	} else {
		com_err(whoami, 0, "Shutdown request by %s rejected",
			cl->clname);
		cl->reply.sms_status = EACCES;
	}
}
		
