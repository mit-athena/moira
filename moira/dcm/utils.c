/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v 1.2 1988-09-13 14:09:28 mar Exp $
 *
 * 
 * 	Utility functions used by the DCM.
 *
 *  (c) Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_utils_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v 1.2 1988-09-13 14:09:28 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <varargs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <zephyr/zephyr.h>
#include <sms.h>
#include "dcm.h"


int dbg = 0;
int log_flags;

extern char *strsave();

void dcm_com_err_hook(whoami, code, fmt, pvar)
	char *whoami;
	int code;
	char *fmt;
	caddr_t pvar;
{
	if (whoami) {
		fputs(whoami, stderr);
		fputs(": ", stderr);
	}
	if (code) {
		fputs(error_message(code), stderr);
	}
	if (fmt) {
		_doprnt(fmt, pvar, stderr);
	}
	putc('\n', stderr);
	fflush(stderr);
}
		
void leave(s)
char *s;
{
    extern int errno;

    if (*s)
      com_err(whoami, errno, "%s: exiting", s);
    else
      com_err(whoami, errno, "exiting");

    exit(errno);
}

void scream(argc, argv, hint)
int argc;
char **argv;
int hint;
{
    leave("Programmer botch");
}


char *itoa(i)
int i;
{
    char buf[20];

    sprintf(buf, "%d", i);
    return(strsave(buf));
}


char *tkt_string()
{
    return("/tmp/tkt_dcm");
}


int maybe_lock_update(dir, host, service, exclusive)
char *dir, *host, *service;
int exclusive;
{
    char lock[BUFSIZ];
    int fd;

    sprintf(lock, "%s/dcm/locks/%s.%s", dir, host, service);
    if ((fd = open(lock, O_TRUNC |  O_CREAT, 0)) < 0)
      com_err(whoami, errno, ": maybe_lock_update: opening %s", lock);
    else if (flock(fd, (exclusive ? LOCK_EX : LOCK_SH) | LOCK_NB) != 0) {
	if (errno != EWOULDBLOCK) 
	  com_err(whoami, errno, ": maybe_lock_update: flock");
	else if (dbg & DBG_VERBOSE)
	  com_err(whoami, 0, "%s already locked\n", lock);
	close(fd);
	return -1;
    } else if (dbg & DBG_VERBOSE) 
      com_err(whoami, 0, "%s now locked\n", lock);
    return fd;
}

