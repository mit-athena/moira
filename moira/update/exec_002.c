/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.11 1992-09-22 14:16:03 mar Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_exec_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.11 1992-09-22 14:16:03 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <gdb.h>
#include <moira.h>
#include "update.h"

extern CONNECTION conn;
extern int code, errno, uid;
extern char *whoami;

#if defined(vax) || defined(ibm032)
#define WEXITSTATUS(waitb) ((waitb).w_retcode)
#endif


int
exec_002(str)
    char *str;
{
#ifdef _AIX
    int waitb;
#else
    union wait waitb;
#endif
    int n, pid, mask;

    if (config_lookup("noexec")) {
	code = EPERM;
	code = send_object(conn, (char *)&code, INTEGER_T);
	com_err(whoami, code, "Not allowed to execute");
	return(0);
    }
    str += 8;
    while (*str == ' ')
	str++;
    mask = sigblock(sigmask(SIGCHLD));
    pid = fork();
    switch (pid) {
    case -1:
	n = errno;
	sigsetmask(mask);
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
	sigsetmask(mask);
	execlp(str, str, (char *)NULL);
	n = errno;
	sigsetmask(mask);
	log_priority = log_ERROR;
	com_err(whoami, n, ": %s", str);
	(void) send_object(conn, (char *)&n, INTEGER_T);
	exit(1);
    default:
	do {
	    n = wait(&waitb);
	} while (n != -1 && n != pid);
	sigsetmask(mask);
	if (WEXITSTATUS(waitb)) {
	    n = WEXITSTATUS(waitb) + ERROR_TABLE_BASE_sms;
	    log_priority = log_ERROR;
	    com_err(whoami, n, " child exited with status %d",
		    WEXITSTATUS(waitb));
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
