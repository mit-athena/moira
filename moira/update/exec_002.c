/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.1 1987-08-22 17:54:08 wesommer Exp $
 */

#ifndef lint
static char *rcsid_exec_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.1 1987-08-22 17:54:08 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include <sys/wait.h>
#include "gdb.h"
#include "update.h"
#include "sms_update_int.h"
#include "kludge.h"
#include "smsu_int.h"

extern CONNECTION conn;
extern int code, errno;

int
exec_002(str)
    char *str;
{
    union wait waitb;
    int n, pid;

    str += 8;
    while (*str == ' ')
	str++;
    pid = fork();
    switch (pid) {
    case -1:
	n = errno;
	log_priority = log_ERROR;
	com_err(whoami, errno, ": can't fork to run install script");
	code = send_object(conn, (char *)&n, INTEGER_T);
	if (code)
	    exit(1);
	return;
    case 0:
	execlp(str, str, (char *)NULL);
	n = errno;
	log_priority = log_ERROR;
	com_err(whoami, n, ": %s", str);
	(void) send_object(conn, (char *)&n, INTEGER_T);
	exit(1);
    default:
	do {
	    n = wait(&waitb);
	} while (n != -1 && n != pid);
	if (waitb.w_status) {
	    log_priority = log_ERROR;
	    com_err(whoami, 0, "child exited with status %d", waitb.w_status);
	    code = send_object(conn, (char *)&n, INTEGER_T);
	    if (code)
		exit(1);
	}
	else {
	    code = send_ok();
	    if (code)
		exit(1);
	}
    }
}
