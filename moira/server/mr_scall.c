/* $Id: mr_scall.c,v 1.34 1998-05-30 18:17:21 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_scall.c,v 1.34 1998-05-30 18:17:21 danw Exp $");

extern int nclients;
extern client **clients;
extern char *whoami;

extern int dbms_errno, mr_errcode;

void do_call(client *cl);
void free_rtn_tuples(client *cp);
int retr_callback(int argc, char **argv, void *p_cl);
int list_users(client *cl);
void do_retr(client *cl, mr_params req);
void do_access(client *cl, mr_params req);
void get_motd(client *cl);

char *procnames[] = {
  "noop",
  "auth",
  "shutdown",
  "query",
  "access",
  "dcm",
  "motd",
};

int newqueries;

void client_read(client *cl)
{
  mr_params req;
  int status, pn;

  status = mr_receive(cl->con, &req);
  if (status != MR_SUCCESS)
    {
      cl->done = 1;
      if (status != MR_NOT_CONNECTED)
	com_err(whoami, status, "while reading from socket");
      return;
    }

  pn = req.u.mr_procno;
  if (pn < 0 || pn > MR_MAX_PROC)
    {
      com_err(whoami, 0, "procno out of range");
      client_reply(cl, MR_UNKNOWN_PROC);
      mr_destroy_reply(req);
      return;
    }
  log_args(procnames[pn], 2, req.mr_argc, req.mr_argv);

  if (dormant == ASLEEP && pn != MR_NOOP && pn != MR_MOTD)
    {
      client_reply(cl, MR_DOWN);
      com_err(whoami, MR_DOWN, "(query refused)");
      mr_destroy_reply(req);
      return;
    }

  /* make sure this gets cleared before every operation */
  dbms_errno = 0;

  switch (pn)
    {
    case MR_NOOP:
      client_reply(cl, MR_SUCCESS);
      break;

    case MR_AUTH:
      do_auth(cl, req);
      break;

    case MR_QUERY:
      do_retr(cl, req);
      break;

    case MR_ACCESS:
      do_access(cl, req);
      break;

    case MR_SHUTDOWN:
      do_shutdown(cl);
      break;

    case MR_DO_UPDATE:
      client_reply(cl, MR_PERM);
      break;

    case MR_MOTD:
      get_motd(cl);
      break;
    }
  mr_destroy_reply(req);
}

/* Set the final return status for a query. We always keep one more
   free slot in cl->tuples[] than we're using so that this can't fail */
void client_reply(client *cl, long status)
{
  cl->tuples[cl->ntuples].u.mr_status = status;
  cl->tuples[cl->ntuples].mr_argc = 0;
  cl->tuples[cl->ntuples].mr_argl = NULL;
  cl->tuples[cl->ntuples].mr_argv = NULL;
  cl->ntuples++;
}

void client_return_tuple(client *cl, int argc, char **argv)
{
  if (cl->done || dbms_errno)
    return;

  if (cl->ntuples == cl->tuplessize - 1)
    {
      int newsize = (cl->tuplessize + 4) * 2;
      mr_params *newtuples;

      newtuples = realloc(cl->tuples, newsize * sizeof(mr_params));
      if (!newtuples)
	{
	  free_rtn_tuples(cl);
	  dbms_errno = mr_errcode = MR_NO_MEM;
	  return;
	}
      cl->tuplessize = newsize;
      cl->tuples = newtuples;
    }

  cl->tuples[cl->ntuples].u.mr_status = MR_MORE_DATA;
  cl->tuples[cl->ntuples].mr_argc = argc;
  cl->tuples[cl->ntuples].mr_argl = NULL;
  cl->tuples[cl->ntuples].mr_argv = mr_copy_args(argv, argc);
  cl->ntuples++;
}

void client_write(client *cl)
{
  int status;

  status = mr_send(cl->con, &cl->tuples[cl->nexttuple]);
  if (status)
    {
      com_err(whoami, status, "writing to socket");
      cl->done = 1;
    }
  else
    {
      cl->nexttuple++;
      if (cl->nexttuple == cl->ntuples)
	free_rtn_tuples(cl);
    }
}

void free_rtn_tuples(client *cl)
{
  for (cl->ntuples--; cl->ntuples >= 0; cl->ntuples--)
    free_argv(cl->tuples[cl->ntuples].mr_argv,
	      cl->tuples[cl->ntuples].mr_argc);
  free(cl->tuples);

  cl->tuples = xmalloc(sizeof(mr_params));
  cl->tuplessize = 1;
  cl->ntuples = cl->nexttuple = 0;
}

void do_retr(client *cl, mr_params req)
{
  char *queryname;
  int status;

  if (req.mr_argc < 1)
    {
      client_reply(cl, MR_ARGS);
      com_err(whoami, MR_ARGS, "got nameless query");
      return;
    }
  queryname = req.mr_argv[0];
  newqueries++;

  if (!strcmp(queryname, "_list_users"))
    status = list_users(cl);
  else
    status = mr_process_query(cl, queryname, req.mr_argc - 1, req.mr_argv + 1,
			      retr_callback, cl);

  client_reply(cl, status);

  com_err(whoami, 0, "Query complete.");
}

int retr_callback(int argc, char **argv, void *p_cl)
{
  client *cl = p_cl;

  mr_trim_args(argc, argv);
  client_return_tuple(cl, argc, argv);
}

void do_access(client *cl, mr_params req)
{
  int status;

  if (req.mr_argc < 1)
    {
      client_reply(cl, MR_ARGS);
      com_err(whoami, MR_ARGS, "got nameless access");
      return;
    }

  status = mr_check_access(cl, req.mr_argv[0], req. mr_argc - 1,
			   req.mr_argv + 1);
  client_reply(cl, status);

  com_err(whoami, 0, "Access check complete.");
}

void get_motd(client *cl)
{
  int motd;
  char *buffer;
  struct stat statb;

  if (stat(MOIRA_MOTD_FILE, &statb) == -1)
    {
      client_reply(cl, MR_SUCCESS);
      return;
    }
  
  buffer = malloc(statb.st_size + 1);
  if (!buffer)
    {
      client_reply(cl, MR_NO_MEM);
      return;
    }

  motd = open(MOIRA_MOTD_FILE, 0, O_RDONLY);
  if (motd)
    {
      read(motd, buffer, statb.st_size);
      close(motd);
      buffer[statb.st_size] = '\0';
      client_return_tuple(cl, 1, &buffer);
      client_reply(cl, MR_SUCCESS);
    }
  else
    client_reply(cl, errno);

  free(buffer);
}

int list_users(client *cl)
{
  char *argv[5];
  char buf[30];
  char buf1[30];
  int i;
  char *cp;

  for (i = 0; i < nclients; i++)
    {
      client *c = clients[i];
      argv[0] = c->clname;
      argv[1] = inet_ntoa(c->haddr.sin_addr);
      argv[2] = buf;
      sprintf(buf, "port %d", ntohs(c->haddr.sin_port));
      argv[3] = ctime(&c->last_time_used);
      cp = strchr(argv[3], '\n');
      if (cp)
	*cp = '\0';
      argv[4] = buf1;
      sprintf(buf1, "[#%d]", c->id);
      client_return_tuple(cl, 5, argv);
    }
  return MR_SUCCESS;
}

