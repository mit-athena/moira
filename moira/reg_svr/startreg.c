/* $Id$
 *
 * This program starts the user registration server
 * in a "clean" environment, and then waits for it to exit.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <sys/resource.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

RCSID("$HeadURL$ $Id$");

#define PROG	"reg_svr"

int rdpipe[2];

void cleanup(int signal);

void cleanup(int signal)
{
  int stat, serrno = errno;
  char buf[BUFSIZ];

  buf[0] = '\0';

  while (waitpid(-1, &stat, WNOHANG) > 0)
    {
      if (WIFEXITED(stat))
	{
	  if (WEXITSTATUS(stat))
	    sprintf(buf, "exited with code %d\n", WEXITSTATUS(stat));
	}
      if (WIFSIGNALED(stat))
	{
	  sprintf(buf, "exited on signal %d%s\n", WTERMSIG(stat),
		  (WCOREDUMP(stat) ? "; Core dumped" : ""));
	}
      write(rdpipe[1], buf, strlen(buf));
      close(rdpipe[1]);
    }
  errno = serrno;
}

int main(int argc, char **argv)
{
  char buf[BUFSIZ];
  FILE *log, *prog;
  int logf, inf, i, done, pid;
  struct rlimit rl;

  struct sigaction action;
  int nfds;

  getrlimit(RLIMIT_NOFILE, &rl);
  nfds = rl.rlim_cur;

  action.sa_handler = cleanup;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGCHLD, &action, NULL);

  sprintf(buf, "%s/%s.log", MOIRA_DIR, PROG);
  logf = open(buf, O_CREAT|O_WRONLY|O_APPEND, 0640);
  if (logf < 0)
    {
      perror(buf);
      exit(1);
    }
  inf = open("/dev/null", O_RDONLY , 0);
  if (inf < 0)
    {
      perror("/dev/null");
      exit(1);
    }
  pipe(rdpipe);
  if (fork())
    exit(0);
  chdir("/");
  close(0);
  close(1);
  close(2);
  dup2(inf, 0);
  dup2(inf, 1);
  dup2(inf, 2);

  setpgrp();
  sprintf(buf, "%s/%s", BIN_DIR, PROG);

  if ((pid = fork()) == 0)
    {
      dup2(inf, 0);
      dup2(rdpipe[1], 1);
      dup2(1,2);
      for (i = 3; i < nfds; i++)
	close(i);
      execl(buf, PROG, 0);
      perror("cannot run reg_svr");
      exit(1);
    }
  if (pid < 0)
    {
      perror("startreg");
      exit(1);
    }

  log = fdopen(logf, "w");
  prog = fdopen(rdpipe[0], "r");

  do
    {
      char *time_s;
      time_t now;

      done = 0;
      errno = 0;
      if (!fgets(buf, BUFSIZ, prog))
	{
	  if (errno && errno != EINTR)
	    {
	      strcpy(buf, "Unable to read from program: ");
	      strcat(buf, strerror(errno));
	      strcat(buf, "\n");
	    }
	  else
	    break;
	}
      time(&now);
      time_s = ctime(&now) + 4;
      time_s[strlen(time_s) - 6] = '\0';
      fprintf(log, "%s <%d> %s", time_s, pid, buf);
      fflush(log);
    }
  while (!done);
  exit(0);
}
