/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.3 1988-08-23 11:49:52 mar Exp $
 */

#ifndef lint
static char *rcsid_exec_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.3 1988-08-23 11:49:52 mar Exp $";
#endif	lint

#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <gdb.h>
#include <sms.h>
#include "update.h"
#include "kludge.h"

extern CONNECTION conn;
extern int code, errno;

int
exec_002(str)
    char *str;
{
    union wait waitb;
    int n, pid, mask;

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
	return;
    case 0:
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
	if (waitb.w_status) {
	    n = waitb.w_retcode + sms_err_base;
	    log_priority = log_ERROR;
	    com_err(whoami, n, " child exited with status %d", waitb.w_retcode);
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
