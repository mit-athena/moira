/* $Id: mr_glue.c,v 1.25 1998-02-23 19:24:29 danw Exp $
 *
 * Glue routines to allow the database stuff to be linked in to
 * a program expecting a library level interface.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"
#include "query.h"

#include <sys/wait.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char *krb_get_lrealm(char *, int);

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_glue.c,v 1.25 1998-02-23 19:24:29 danw Exp $");

static int already_connected = 0;

#define CHECK_CONNECTED { if (!already_connected) return MR_NOT_CONNECTED; }

static client pseudo_client;
extern char *whoami;
extern time_t now;

void reapchild(void);
int callback(int argc, char **argv, void *arg);

int mr_connect(char *server)
{
  int status;
  extern int query_timeout;
  struct sigaction action;

  if (already_connected)
    return MR_ALREADY_CONNECTED;

  initialize_sms_error_table();
  initialize_krb_error_table();
  memset(&pseudo_client, 0, sizeof(pseudo_client));

  query_timeout = 0;
  status =  mr_open_database();
  if (!status)
    already_connected = 1;

  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGCHLD);
  action.sa_handler = reapchild;
  if (sigaction(SIGCHLD, &action, NULL) < 0)
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }
  return status;
}

int mr_disconnect(void)
{
  CHECK_CONNECTED;
  mr_close_database();
  already_connected = 0;
  return 0;
}

int mr_noop(void)
{
  CHECK_CONNECTED;
  return 0;
}

/*
 * This routine is rather bogus, as it only fills in who you claim to be.
 */
int mr_auth(char *prog)
{
  struct passwd *pw;
  char buf[MAX_K_NAME_SZ];

  CHECK_CONNECTED;
  pw = getpwuid(getuid());
  if (!pw)
    return KDC_PR_UNKNOWN + ERROR_TABLE_BASE_krb;
  strcpy(pseudo_client.kname.name, pw->pw_name);
  krb_get_lrealm(pseudo_client.kname.realm, 1);
  krb_get_lrealm(krb_realm, 1);

  strcpy(buf, pw->pw_name);
  strcat(buf, "@");
  strcat(buf, pseudo_client.kname.realm);
  strcpy(pseudo_client.clname, buf);
  pseudo_client.users_id = 0;
  name_to_id(pseudo_client.kname.name, USERS_TABLE, &pseudo_client.users_id);
  pseudo_client.client_id = pseudo_client.users_id;
  strncpy(pseudo_client.entity, prog, 8);
  return 0;
}

struct hint {
  int (*proc)(int, char **, void *);
  char *hint;
};

int callback(int argc, char **argv, void *arg)
{
  struct hint *hint = arg;
  if (mr_trim_args(argc, argv) == MR_NO_MEM)
    com_err(whoami, MR_NO_MEM, "while trimmming args");
  return (*hint->proc)(argc, argv, hint->hint);
}


int mr_query(char *name, int argc, char **argv,
	     int (*callproc)(int, char **, void *), void *callarg)
{
  struct hint hints;

  time(&now);
  hints.proc = callproc;
  hints.hint = callarg;
  next_incremental();
  return mr_process_query(&pseudo_client, name, argc,
			  mr_copy_args(argv, argc), callback,
			  (char *)&hints);
}

int mr_access(char *name, int argc, char **argv)
{
  time(&now);
  return mr_check_access(&pseudo_client, name, argc,
			 mr_copy_args(argv, argc));
}

/* trigger_dcm is also used as a followup routine to the
 * set_server_host_override query, hence the two dummy arguments.
 */

struct query pseudo_query = {
  "trigger_dcm",
  "tdcm",
};

int trigger_dcm(struct query *q, char *argv[], client *cl)
{
  int pid, status;
  char prog[MAXPATHLEN];

  if ((status = check_query_access(&pseudo_query, 0, cl)))
    return status;

  sprintf(prog, "%s/startdcm", BIN_DIR);
  pid = vfork();
  switch (pid)
    {
    case 0:
      execl(prog, "startdcm", 0);
      exit(1);

    case -1:
      return errno;

    default:
      return MR_SUCCESS;
    }
}


void reapchild(void)
{
  int status, pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
      if (pid == inc_pid)
	{
	  inc_running = 0;
	  next_incremental();
	}
      if (WTERMSIG(status) != 0 || WEXITSTATUS(status) != 0)
	{
	  com_err(whoami, 0, "%d: child exits with signal %d status %d",
		  pid, WTERMSIG(status), WEXITSTATUS(status));
	}
    }
}
