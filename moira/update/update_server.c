/* $Id: update_server.c,v 1.17 1998-02-05 22:52:03 danw Exp $
 *
 * Copyright 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include "update_server.h"

#include <sys/stat.h>

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <des.h>
#include <gdb.h>
#include "update.h"

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/update_server.c,v 1.17 1998-02-05 22:52:03 danw Exp $");

extern int errno;

CONNECTION conn;
int code, log_priority;
char *whoami;

int have_authorization = 0;
C_Block session;
int have_file = 0;
int done = 0;
int uid = 0;

#define send_int(n) \
     (_send_int = (n), send_object(conn, (char *)&_send_int, INTEGER_T))
int _send_int;

struct _dt {
  char *str;
  int (*proc)(char *);
} dispatch_table[] = {
  { "AUTH_002", auth_002 },
  { "XFER_002", xfer_002 },
  { "XFER_003", xfer_003 },
  { "EXEC_002", exec_002 },
  { "quit", quit },
  { NULL, (int (*)(char *))abort }
};

/* general scratch space -- useful for building error messages et al... */
char buf[BUFSIZ];

int main(int argc, char **argv)
{
  STRING str;
  struct _dt *d;
  char *p;

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
  else
    gdb_debug(GDB_NOFORK);

  umask(0022);
  initialize_sms_error_table();
  initialize_krb_error_table();
  mr_update_initialize();

  /* wait for connection */
  gdb_init();
  /* If the config file contains a line "port portname", the daemon
   * will listen on the named port rather than SERVICE_NAME "moira_update"
   */
  if (!(p = config_lookup("port")))
    p = SERVICE_NAME;
  conn = create_forking_server(p, 0);

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

  if (!conn)
    {
      com_err(whoami, errno, "can't get connection");
      exit(1);
    }
  if (connection_status(conn) == CON_STOPPED)
    {
      com_err(whoami, connection_errno(conn), ": can't get connection");
      exit(1);
    }

  mr_log_info("got connection");
  /* got a connection; loop forever */
  while (1)
    {
      char *cp;
      code = receive_object(conn, (char *)&str, STRING_T);
      if (code)
	{
	  com_err(whoami, connection_errno(conn), "receiving command");
	  sever_connection(conn);
	  exit(1);
	}
      cp = strchr(STRING_DATA(str), ' ');
      if (cp)
	*cp = '\0';
      for (d = dispatch_table; d->str; d++)
	{
	  if (!strcmp(d->str, STRING_DATA(str)))
	    {
	      if (cp)
		*cp = ' ';
	      (d->proc)(STRING_DATA(str));
	      goto ok;
	    }
	}
      sprintf(buf, "unknown request received: %s\n", STRING_DATA(str));
      mr_log_error(buf);
      code = send_int(MR_UNKNOWN_PROC);
      if (code)
	com_err(whoami, connection_errno(conn), "sending UNKNOWN_PROC");
    ok:
      string_free(&str);
    }
}

int send_ok(void)
{
  static int zero = 0;
  return code = send_object(conn, (char *)&zero, INTEGER_T);
}


void initialize(void)
{
  /* keep have_authorization around */
  have_file = 0;
  done = 0;
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
int quit(char *str)
{
  send_ok();
  sever_connection(conn);
  mr_log_info("Closing connection.");
  exit(0);
}


/*
 * lose(msg)
 *
 * put <msg> to log as error, break connection, and exit
 */

void lose(char *msg)
{
  com_err(whoami, code, msg);
  if (conn)
    sever_connection(conn);
  exit(1);
}

/*
 * report_error(msg)
 *
 * send back (external) <code>; if error, punt big with <lose(msg)>
 */

void report_error(char *msg)
{
  code = send_object(conn, (char *)&code, INTEGER_T);
  if (code)
    {
      code = connection_errno(conn);
      lose(msg);
    }
}

/*
 * reject_call(c)
 *
 * set (external) <code> to <c> and call <report_error>
 */

void reject_call(int c)
{
  code = c;
  report_error("call rejected");
}
