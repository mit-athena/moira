/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.18 1998-01-05 19:53:53 danw Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_exec_002_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/exec_002.c,v 1.18 1998-01-05 19:53:53 danw Exp $";
#endif

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
extern int code, errno, uid, log_priority, have_authorization;
extern char *whoami;

int exec_002(char *str)
{
  int waitb;
  sigset_t mask, oldmask;
  int n, pid;

  if (!have_authorization)
    {
      reject_call(MR_PERM);
      return 0;
    }
  if (config_lookup("noexec"))
    {
      code = EPERM;
      send_object(conn, (char *)&code, INTEGER_T);
      com_err(whoami, code, "Not allowed to execute");
      return 0;
    }
  str += 8;
  while (*str == ' ')
    str++;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &oldmask);
  pid = fork();
  switch (pid)
    {
    case -1:
      n = errno;
      sigprocmask(SIG_UNBLOCK, &oldmask, &mask);
      log_priority = log_ERROR;
      com_err(whoami, errno, ": can't fork to run install script");
      code = send_object(conn, (char *)&n, INTEGER_T);
      if (code)
	exit(1);
      return 0;
    case 0:
      if (setuid(uid) < 0)
	{
	  com_err(whoami, errno, "Unable to setuid to %d\n", uid);
	  exit(1);
	}
      sigprocmask(SIG_UNBLOCK, &oldmask, &mask);
      execlp(str, str, NULL);
      n = errno;
      sigprocmask(SIG_UNBLOCK, &oldmask, &mask);
      log_priority = log_ERROR;
      com_err(whoami, n, ": %s", str);
      send_object(conn, (char *)&n, INTEGER_T);
      exit(1);
    default:
      do
	n = wait(&waitb);
      while (n != -1 && n != pid);
      sigprocmask(SIG_UNBLOCK, &oldmask, &mask);
      if ((WIFEXITED(waitb) && (WEXITSTATUS(waitb) != 0)) ||
	  WIFSIGNALED(waitb))
	{
	  log_priority = log_ERROR;
	  if (WIFSIGNALED(waitb))
	    {
	      n = MR_COREDUMP;
	      com_err(whoami, n, " child exited on signal %d",
		      WTERMSIG(waitb));
	    }
	  else
	    {
	      n = WEXITSTATUS(waitb) + ERROR_TABLE_BASE_sms;
	      com_err(whoami, n, " child exited with status %d",
		      WEXITSTATUS(waitb));
	    }
	  code = send_object(conn, (char *)&n, INTEGER_T);
	  if (code)
	    exit(1);
	}
      else
	{
	  code = send_ok();
	  if (code)
	    exit(1);
	}
    }
}
