/* $Id: mr_scall.c,v 1.31 1998-02-05 22:51:44 danw Exp $
 *
 * Do RPC
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include "mr_server.h"
#include "query.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.31 1998-02-05 22:51:44 danw Exp $");

extern char buf1[];
extern int nclients;
extern char *whoami;

extern int dbms_errno, mr_errcode;
static int row_count;

void do_call(client *cl);
void free_rtn_tuples(client *cp);
int retr_callback(int argc, char **argv, void *p_cp);
int list_users(int (*callbk)(int, char **, void *), char *callarg);
void do_retr(client *cl);
void do_access(client *cl);
void get_motd(client *cl);

/* Put this in a variable so that we can patch it if necessary */
int max_row_count = 4096;

/*
 * Welcome to the (finite state) machine (highest level).
 */
void do_client(client *cp)
{
  struct stat stbuf;

  free_rtn_tuples(cp);
  if (OP_STATUS(cp->pending_op) == OP_CANCELLED)
    {
      com_err(whoami, 0,
	      "Closed connection (now %d client%s, %d new queries, %d old)",
	      nclients - 1, nclients != 2 ? "s" : "", newqueries, oldqueries);
      clist_delete(cp);
      /* if we no longer have any clients, and we're supposed to
       * go down, then go down now.
       */
      if ((dormant == AWAKE) && (nclients == 0) &&
	  (stat(MOIRA_MOTD_FILE, &stbuf) == 0))
	{
	  com_err(whoami, 0, "motd file exists, slumbertime");
	  dormant = SLEEPY;
	}
      return;
    }
  switch (cp->action)
    {
    case CL_ACCEPT:
    case CL_SEND:
      /* Start recieving next request */
      initialize_operation(cp->pending_op, mr_start_recv,
			   (char *)&cp->args, NULL);
      queue_operation(cp->con, CON_INPUT, cp->pending_op);
      cp->action = CL_RECEIVE;
      break;
    case CL_RECEIVE:
      /* Data is here. Process it & start it heading back */
      do_call(cp); /* This may block for a while. */
      mr_destroy_reply(cp->args);
      cp->args = NULL;
      initialize_operation(cp->pending_op, mr_start_send,
			   (char *)&cp->reply, NULL);
      queue_operation(cp->con, CON_OUTPUT, cp->pending_op);
      cp->action = CL_SEND;
      break;
    }
}

char *procnames[] = {
  "noop",
  "auth",
  "shutdown",
  "query",
  "access",
  "dcm",
  "motd",
};


void do_call(client *cl)
{
  int pn;
  cl->reply.mr_argc = 0;
  cl->reply.mr_status = 0;
  cl->reply.mr_version_no = cl->args->mr_version_no;
  if (((pn = cl->args->mr_procno) < 0) || (pn > MR_MAX_PROC))
    {
      com_err(whoami, 0, "procno out of range");
      cl->reply.mr_status = MR_UNKNOWN_PROC;
      return;
    }
  if (log_flags & LOG_ARGS)
    {
      log_args(procnames[pn], cl->args->mr_version_no,
	       cl->args->mr_argc, cl->args->mr_argv);
    }
  else if (log_flags & LOG_REQUESTS)
    com_err(whoami, 0, "%s", procnames[pn]);

  if ((dormant == ASLEEP || dormant == GROGGY) &&
      pn != MR_NOOP && pn != MR_MOTD)
    {
      cl->reply.mr_status = MR_DOWN;
      if (log_flags & LOG_RES)
	com_err(whoami, MR_DOWN, "(query refused)");
      return;
    }

  /* make sure this gets cleared before every operation */
  dbms_errno = 0;

  switch (pn)
    {
    case MR_NOOP:
      cl->reply.mr_status = 0;
      return;

    case MR_AUTH:
      do_auth(cl);
      return;

    case MR_QUERY:
      do_retr(cl);
      return;

    case MR_ACCESS:
      do_access(cl);
      return;

    case MR_SHUTDOWN:
      do_shutdown(cl);
      return;

    case MR_DO_UPDATE:
      trigger_dcm(NULL, NULL, cl);
      return;

    case MR_MOTD:
      get_motd(cl);
      return;
    }
}

void free_rtn_tuples(client *cp)
{
  returned_tuples *temp;
  for (temp = cp->first; temp && OP_DONE(temp->op); )
    {
      returned_tuples *t1 = temp;
      temp = t1->next;
      if (t1 == cp->last)
	cp->last = NULL;

      mr_destroy_reply(t1->retval);
      delete_operation(t1->op);
      free(t1);
    }
  cp->first = temp;
}

int retr_callback(int argc, char **argv, void *p_cp)
{
  client *cp = p_cp;
  mr_params *arg_tmp;
  returned_tuples *tp;
  OPERATION op_tmp;
  char **nargv;
  int i;

  if (row_count++ >= max_row_count)
    {
      dbms_errno = mr_errcode = MR_NO_MEM;
      return MR_ABORT;
    }

  /*
   * This takes too much advantage of the fact that
   * serialization of the data happens during the queue operation.
   */
  arg_tmp = malloc(sizeof(mr_params));
  tp = malloc(sizeof(returned_tuples));
  nargv = malloc(argc * sizeof(char *));

  op_tmp = create_operation();

  if (mr_trim_args(argc, argv) == MR_NO_MEM)
    com_err(whoami, MR_NO_MEM, "while trimming args");
  if (log_flags & LOG_RESP)
    log_args("return: ", cp->args->mr_version_no, argc, argv);

  tp->op = op_tmp;
  tp->retval = arg_tmp;
  tp->next = NULL;

  arg_tmp->mr_status = MR_MORE_DATA;
  arg_tmp->mr_version_no = cp->args->mr_version_no;
  arg_tmp->mr_argc = argc;
  arg_tmp->mr_argv = nargv;
  for (i = 0; i < argc; i++)
    {
      int len = strlen(argv[i]) + 1;
      nargv[i] = malloc(len);
      memcpy(nargv[i], argv[i], len);
    }
  arg_tmp->mr_flattened = NULL;
  arg_tmp->mr_argl = NULL;

  if (cp->last)
    {
      cp->last->next = tp;
      cp->last = tp;
    } else
      cp->last = cp->first = tp;

  reset_operation(op_tmp);
  initialize_operation(op_tmp, mr_start_send, (char *)arg_tmp, NULL);
  queue_operation(cp->con, CON_OUTPUT, op_tmp);
  return MR_CONT;
}

int list_users(int (*callbk)(int, char **, void *), char *callarg)
{
  char *argv[6];
  char buf[30];
  char buf1[30];
  int i;
  extern client **clients;
  char *cp;

  for (i = 0; i < nclients; i++)
    {
      client *cl = clients[i];
      if (cl->clname)
	argv[0] = cl->clname;
      else argv[0] = "unauthenticated";

      argv[1] = inet_ntoa(cl->haddr.sin_addr);
      argv[2] = buf;
      sprintf(buf, "port %d", ntohs(cl->haddr.sin_port));
      argv[3] = ctime(&cl->last_time_used);
      cp = strchr(argv[3], '\n');
      if (cp)
	*cp = '\0';
      argv[4] = buf1;
      sprintf(buf1, "[#%d]", cl->id);
      (*callbk)(5, argv, callarg);
    }
  return 0;
}

void do_retr(client *cl)
{
  char *queryname;

  cl->reply.mr_argc = 0;
  cl->reply.mr_status = 0;
  row_count = 0;

  if (cl->args->mr_argc < 1)
    {
      cl->reply.mr_status = MR_ARGS;
      com_err(whoami, MR_ARGS, "got nameless query");
      return;
    }
  queryname = cl->args->mr_argv[0];

  if (cl->args->mr_version_no == MR_VERSION_2)
    newqueries++;
  else
    oldqueries++;

  if (!strcmp(queryname, "_list_users"))
    cl->reply.mr_status = list_users(retr_callback, (char *)cl);
  else
    {
      cl->reply.mr_status = mr_process_query(cl, queryname,
					     cl->args->mr_argc - 1,
					     cl->args->mr_argv + 1,
					     retr_callback, cl);
    }
  if (row_count >= max_row_count)
    {
      critical_alert("moirad", "attempted query %s with %d rows\n",
		     queryname, row_count);
    }

  if (log_flags & LOG_RES)
    com_err(whoami, 0, "Query complete.");
}

void do_access(client *cl)
{
  if (cl->args->mr_argc < 1)
    {
      cl->reply.mr_status = MR_ARGS;
      com_err(whoami, MR_ARGS, "got nameless access");
      return;
    }
  cl->reply.mr_argc = 0;

  cl->reply.mr_status = mr_check_access(cl, cl->args->mr_argv[0],
					cl->args->mr_argc - 1,
					cl->args->mr_argv + 1);

  com_err(whoami, 0, "Access check complete.");
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
  int pid;
  char prog[128];

  cl->reply.mr_argc = 0;

  if ((cl->reply.mr_status = check_query_access(&pseudo_query, 0, cl)))
    return cl->reply.mr_status;

  sprintf(prog, "%s/startdcm", BIN_DIR);
  pid = vfork();
  switch (pid)
    {
    case 0:
      execl(prog, "startdcm", 0);
      exit(1);

    case -1:
      cl->reply.mr_status = errno;
      return 0;

    default:
      return 0;
    }
}


void get_motd(client *cl)
{
  int motd, len;
  char buffer[1024];
  char *arg[1];

  arg[0] = buffer;
  cl->reply.mr_status = 0;
  motd = open(MOIRA_MOTD_FILE, 0, O_RDONLY);
  if (motd < 0)
    return;
  len = read(motd, buffer, sizeof(buffer) - 1);
  close(motd);
  buffer[len] = 0;
  row_count = 0;
  retr_callback(1, arg, cl);
  cl->reply.mr_status = 0;
}
