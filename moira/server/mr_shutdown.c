/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.2 1987-06-21 16:42:12 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.1  87/06/02  20:07:18  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_sms_shutdown_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_shutdown.c,v 1.2 1987-06-21 16:42:12 wesommer Exp $";
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
		(void) sprintf(buf,
			       "Unauthenticated shutdown request rejected",
			       cl->clname);
		com_err(whoami, 0, buf);
		cl->reply.sms_status = EACCES;
		return;
	}
	if (!strcmp(cl->clname, "wesommer@ATHENA.MIT.EDU") ||
	    !strcmp(cl->clname, "mike@ATHENA.MIT.EDU")) {
		(void) sprintf(buf, "Shut down by %s", cl->clname);
		com_err(whoami, 0, buf);
		(void) strcpy(buf, "Reason for shutdown: ");
		(void) strcat(buf, cl->args->sms_argv[0]);
		takedown = buf;
	} else {
		(void) sprintf(buf, "Shutdown request by %s rejected",
			       cl->clname);
		com_err(whoami, 0, buf);
		cl->reply.sms_status = EACCES;
	}
}
		
