/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.15 1997-01-29 23:29:00 danw Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_exec_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.15 1997-01-29 23:29:00 danw Exp $";
#endif

#include <mit-copyright.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#ifdef _AIX
#undef _BSD
#endif
#include <sys/wait.h>
#ifdef _AIX
#define _BSD 44
#endif
#include <signal.h>
#include <gdb.h>
#include <moira.h>
#include "update.h"

extern CONNECTION conn;
extern int code, errno, uid, log_priority;
extern char *whoami;

#if defined(vax) || defined(ibm032)
#define WEXITSTATUS(waitb) ((waitb).w_retcode)
#endif


int
exec_002(str)
    char *str;
{
#ifdef POSIX
    int waitb;
    sigset_t mask,oldmask;
#else
    union wait waitb;
    int mask;
#endif
    int n, pid;

    if (config_lookup("noexec")) {
	code = EPERM;
	code = send_object(conn, (char *)&code, INTEGER_T);
	com_err(whoami, code, "Not allowed to execute");
	return(0);
    }
    str += 8;
    while (*str == ' ')
	str++;
#ifdef POSIX
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK,&mask,&oldmask);
#else
    mask = sigblock(sigmask(SIGCHLD));
#endif
    pid = fork();
    switch (pid) {
    case -1:
	n = errno;
#ifdef POSIX
	sigprocmask(SIG_UNBLOCK,&oldmask,&mask);
#else
	sigsetmask(mask);
#endif
	log_priority = log_ERROR;
	com_err(whoami, errno, ": can't fork to run install script");
	code = send_object(conn, (char *)&n, INTEGER_T);
	if (code)
	    exit(1);
	return(0);
    case 0:
	if (setuid(uid) < 0) {
	    com_err(whoami, errno, "Unable to setuid to %d\n", uid);
	    exit(1);
	}
#ifdef POSIX
	sigprocmask(SIG_UNBLOCK,&oldmask,&mask);
#else
	sigsetmask(mask);
#endif
	execlp(str, str, (char *)NULL);
	n = errno;
#ifdef POSIX
	sigprocmask(SIG_UNBLOCK,&oldmask,&mask);
#else
	sigsetmask(mask);
#endif
	log_priority = log_ERROR;
	com_err(whoami, n, ": %s", str);
	(void) send_object(conn, (char *)&n, INTEGER_T);
	exit(1);
    default:
	do {
	    n = wait(&waitb);
	} while (n != -1 && n != pid);
#ifdef POSIX
	sigprocmask(SIG_UNBLOCK,&oldmask,&mask);
#else
	sigsetmask(mask);
#endif
#ifdef POSIX
	if ( (WIFEXITED(waitb) && (WEXITSTATUS(waitb)!=0)) || WIFSIGNALED(waitb)  ) {
	    /* This is not really correct.  It will cause teh moira server to
	       report a bogus error message if the script died on a signal.
	       However this is the same thing that occurs in the non-POSIX
	       case, and I don't know how to come up with a useful error based
	       on the signal recieved.
	    */
	    n = WEXITSTATUS(waitb) + ERROR_TABLE_BASE_sms;
	    log_priority = log_ERROR;
	    com_err(whoami, n, " child exited with status %d",
		    WEXITSTATUS(waitb));
#else
	if (waitb.w_status) {
	    n = waitb.w_retcode + ERROR_TABLE_BASE_sms;
	    log_priority = log_ERROR;
	    com_err(whoami, n, " child exited with status %d",
		    waitb.w_retcode);
#endif
	    code = send_object(conn, (char *)&n, INTEGER_T);
	    if (code) {
		exit(1);
	    }
	} else {
	    code = send_ok();
	    if (code)
	      exit(1);
	}
    }
}
