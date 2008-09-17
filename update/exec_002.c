/* $Id$
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"
#include "update.h"

#include <sys/wait.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

RCSID("$Header$");

void exec_002(int conn, char *str)
{
  int waitb, n;
  sigset_t mask, oldmask;
  pid_t pid;
  long code;

  if (!have_authorization)
    {
      send_int(conn, MR_PERM);
      return;
    }
  if (config_lookup("noexec"))
    {
      send_int(conn, EPERM);
      com_err(whoami, EPERM, "Not allowed to execute");
      return;
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
      com_err(whoami, n, ": can't fork to run install script");
      code = send_int(conn, n);
      if (code)
	exit(1);
      return;

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
      com_err(whoami, n, ": %s", str);
      send_int(conn, n);
      exit(1);

    default:
      do
	n = wait(&waitb);
      while (n != -1 && n != pid);

      sigprocmask(SIG_UNBLOCK, &oldmask, &mask);
      if ((WIFEXITED(waitb) && (WEXITSTATUS(waitb) != 0)) ||
	  WIFSIGNALED(waitb))
	{
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
	  code = send_int(conn, n);
	  if (code)
	    exit(1);
	}
      else
	{
	  code = send_ok(conn);
	  if (code)
	    exit(1);
	}
    }
  return;
}
