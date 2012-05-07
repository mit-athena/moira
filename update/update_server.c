/* $Id: update_server.c 4077 2012-05-07 16:53:10Z zacheiss $
 *
 * Copyright 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#ifdef HAVE_KRB4
#include <des.h>
#endif
#include "update.h"

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/update/update_server.c $ $Id: update_server.c 4077 2012-05-07 16:53:10Z zacheiss $");

char *whoami, *hostname;

int have_authorization = 0;
#ifdef HAVE_KRB4
des_cblock session;
#endif
int uid = 0;

void child_handler(int signal);
static void syslog_com_err_proc(const char *progname, long code,
				const char *fmt, va_list args);

struct _dt {
  char *str;
  void (*proc)(int, char *);
} dispatch_table[] = {
  { "AUTH_002", auth_002 },
  { "AUTH_003", auth_003 },
  { "XFER_002", xfer_002 },
  { "XFER_003", xfer_003 },
  { "EXEC_002", exec_002 },
  { "quit", quit },
  { NULL, (void (*)(int, char *))abort }
};

int main(int argc, char **argv)
{
  char *str, *p;
  size_t len;
  struct _dt *d;
  struct utsname name;
  int s, conn;
  struct sigaction sa;

  whoami = strrchr(argv[0], '/');
  if (whoami)
    whoami++;
  else
    whoami = argv[0];

  /* interpret arguments here */
  if (argc != 1)
    {
      fprintf(stderr, "Usage:  %s\n", whoami);
      exit(1);
    }

  if (!config_lookup("nofork"))
    {
      if (fork())
	exit(0);
      setsid();
    }

  uname(&name);
  hostname = name.nodename;

  umask(0022);
  mr_init();

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = child_handler;
  sigaction(SIGCHLD, &sa, NULL);

  /* If the config file contains a line "user username", the
   * daemon will run with that user's UID.
   */
  if ((p = config_lookup("user")))
    {
      struct passwd *pw;
      pw = getpwnam(p);
      if (!pw)
	{
	  com_err(whoami, errno, "Unable to find user %s\n", p);
	  exit(1);
	}
      uid = pw->pw_uid;
    }

  /* If the config file contains a line "port portname", the daemon
   * will listen on the named port rather than SERVICE_NAME ("moira_update")
   */
  if (!(p = config_lookup("port")))
    p = SERVICE_NAME;

  s = mr_listen(p);
  if (s == -1)
    {
      com_err(whoami, errno, "creating listening socket");
      exit(1);
    }

  set_com_err_hook(syslog_com_err_proc);
  openlog(whoami, LOG_PID, LOG_DAEMON);

  /* now loop waiting for connections */
  while (1)
    {
      struct sockaddr_in client;
      long len;
      char *buf;

      conn = mr_accept(s, &client);
      if (conn == -1)
	{
	  com_err(whoami, errno, "accepting on listening socket");
	  exit(1);
	}
      else if (conn == 0)
	continue;

      if (config_lookup("nofork") || (fork() <= 0))
	break;

      close(conn);
    }

  /* If the config file contains a line "chroot /dir/name", the
   * daemon will run chrooted to that directory.
   */
  if ((p = config_lookup("chroot")))
    {
      if (chroot(p) < 0)
	{
	  com_err(whoami, errno, "unable to chroot to %s", p);
	  exit(1);
	}
    }

  com_err(whoami, 0, "got connection");

  while (1)
    {
      char *cp, *str;
      size_t len;
      int code;

      code = recv_string(conn, &str, &len);
      if (code)
	{
	  com_err(whoami, code, "receiving command");
	  close(conn);
	  exit(1);
	}

      cp = strchr(str, ' ');
      if (cp)
	*cp = '\0';
      for (d = dispatch_table; d->str; d++)
	{
	  if (!strcmp(d->str, str))
	    {
	      if (cp)
		*cp = ' ';
	      (d->proc)(conn, str);
	      goto ok;
	    }
	}
      com_err(whoami, 0, "unknown request received: %s", str);
      code = send_int(conn, MR_UNKNOWN_PROC);
      if (code)
	com_err(whoami, code, "sending UNKNOWN_PROC");
    ok:
      free(str);
    }
}

int send_ok(int conn)
{
  return send_int(conn, 0);
}

/*
 * quit request:
 *
 * syntax:
 * >>> quit
 * <<< (int)0
 * any arguments are ignored
 *
 * function:
 *	closes connection from MR
 */

void quit(int conn, char *str)
{
  send_ok(conn);
  close(conn);
  com_err(whoami, 0, "Closing connection.");
  exit(0);
}

void fail(int conn, int err, char *msg)
{
  com_err(whoami, err, "%s", msg);
  close(conn);
  exit(1);
}

void child_handler(int signal)
{
  int status;

  while (waitpid(-1, &status, WNOHANG) > 0)
    ;
}

static void syslog_com_err_proc(const char *progname, long code,
				const char *fmt, va_list args)
{
  char buf[BUFSIZ + 1];

  buf[BUFSIZ] = '\0';

  vsnprintf(buf, BUFSIZ, fmt, args);
  syslog(LOG_NOTICE, "%s: %s %s", progname ? progname : "",
	 code ? error_message(code) : "", buf);
}
