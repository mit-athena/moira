/* $Id$
 *
 * This program starts the moira server in a "clean" environment.
 * and then waits for it to exit.
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
#include <unistd.h>
#include <time.h>

RCSID("$HeadURL$ $Id$");

#define PROG	"moirad"
#define MOIRAD_LOG MOIRA_DIR "/moira.log"

int rdpipe[2];
static volatile int do_reopen;
char *whoami;
void cleanup(int signal);
void reopen_logfile(int signal);

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
	    {
	      sprintf(buf, "exited with code %d\n", WEXITSTATUS(stat));
	      critical_alert(whoami, "startmoira", "%s", buf);
	    }
	}
      if (WIFSIGNALED(stat))
	{
	  sprintf(buf, "exited on signal %d%s\n", WTERMSIG(stat),
		  (WCOREDUMP(stat) ? "; Core dumped" : ""));
	  if (WCOREDUMP(stat))
	    critical_alert(whoami, "startmoira", "%s", buf);
	}
      write(rdpipe[1], buf, strlen(buf));
      close(rdpipe[1]);
    }
  errno = serrno;
}

void reopen_logfile(int signal)
{
  do_reopen = 1;
}

int main(int argc, char *argv[])
{
  char buf[BUFSIZ];
  FILE *log, *prog;
  int logf, inf, i, done, pid;
  struct rlimit rl;

  struct sigaction action;
  int nfds;

  whoami = argv[0];

  getrlimit(RLIMIT_NOFILE, &rl);
  nfds = rl.rlim_cur;

  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = cleanup;
  sigaction(SIGCHLD, &action, NULL);
  action.sa_flags = SA_RESTART;
  action.sa_handler = reopen_logfile;
  sigaction(SIGHUP, &action, NULL);

  logf = open(MOIRAD_LOG, O_CREAT|O_WRONLY|O_APPEND, 0640);
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
      dup2(1, 2);
      for (i = 3; i < nfds; i++)
	close(i);
      execl(buf, PROG, 0);
      perror("cannot run moirad");
      exit(1);
    }
  if (pid < 0)
    {
      perror("moira_starter");
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

      if (do_reopen)
        {
          fclose(log);
          close(logf);
          logf = open(MOIRAD_LOG, O_CREAT|O_WRONLY|O_APPEND, 0640);
          if (logf < 0)
            {
              perror(buf);
              exit(1);
            }
          log = fdopen(logf, "w");
	  do_reopen = 0;
        }

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
      fprintf(log, "%s %s", time_s, buf);
      fflush(log);
    }
  while (!done);
  exit(0);
}
