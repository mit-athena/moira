/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v 1.6 1990-03-19 18:52:54 mar Exp $
 *
 * 
 * 	Utility functions used by the DCM.
 *
 *  (c) Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_utils_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/utils.c,v 1.6 1990-03-19 18:52:54 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <varargs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <moira.h>
#include <moira_site.h>
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


int maybe_lock_update(host, service, exclusive)
char *host, *service;
int exclusive;
{
    char lock[BUFSIZ];
    int fd;

    sprintf(lock, "%s/%s.%s", LOCK_DIR, host, service);
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


int mr_query_with_retry(name, argc, argv, proc, hint)
char *name;
int argc;
char **argv;
int (*proc)();
char *hint;
{
    int status, tries;

    for (tries = 0; tries < DEADLOCK_TRIES; tries++) {
	status = mr_query(name, argc, argv, proc, hint);
	if (status != MR_DEADLOCK)
	  return(status);
	sleep(DEADLOCK_WAIT);
    }
    return(status);
}
