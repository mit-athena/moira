/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dumprest.qc,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dumprest.qc,v 1.1 1987-07-13 03:53:09 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_dumprest_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dumprest.qc,v 1.1 1987-07-13 03:53:09 wesommer Exp $";
#endif lint

#include <stdio.h>
#include <sys/file.h>
#include <strings.h>

punt(msg)
	char *msg;
{
	perror(msg);
##	exit
	exit(1);
}




/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
