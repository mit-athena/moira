/* $Id: mr_main.c,v 1.55 2009-05-04 20:49:12 zacheiss Exp $
 *
 * Moira server process.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_KRB4
#include <krb.h>
#endif
#include <krb5.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_main.c,v 1.55 2009-05-04 20:49:12 zacheiss Exp $");

client *cur_client;

char *whoami;
char *takedown;
FILE *journal;

time_t now;

char *host;
krb5_context context = NULL;
char *krb_realm = NULL;

/* Client array and associated data. This needs to be global for _list_users */
client **clients;
int nclients, clientssize;

int dormant;
int child_exited_abnormally = 0;
int child_pid, child_signal, child_status;

void reapchild(int x);
void godormant(int x);
void gowakeup(int x);
void clist_append(client *cp);
void mr_setup_signals(void);

/*
 * Main Moira server loop.
 *
 * Initialize the world, then start accepting connections and
 * making progress on current connections.
 */

int main(int argc, char **argv)
{
  int status, i, listener;
  time_t tardy;
  char *port, *p;
  extern char *database;
  struct stat stbuf;
  struct utsname uts;
  fd_set readfds, writefds, xreadfds, xwritefds;
  int nfds, counter = 0;

  whoami = argv[0];
  /*
   * Error handler init.
   */
  mr_init();
  set_com_err_hook(mr_com_err);
  setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

  port = strchr(MOIRA_SERVER, ':') + 1;

  for (i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i], "-db") && i + 1 < argc)
	{
	  database = argv[i + 1];
	  i++;
	}
      else if (!strcmp(argv[i], "-p") && i + 1 < argc)
	{
	  port = argv[i + 1];
	  i++;
	}
      else
	{
	  com_err(whoami, 0, "Usage: moirad [-db database][-p port]");
	  exit(1);
	}
    }

  status = krb5_init_context(&context);
  if (status)
    {
      com_err(whoami, status, "Initializing krb5 context.");
      exit(1);
    }

  status = krb5_get_default_realm(context, &krb_realm);
  if (status)
    {
      com_err(whoami, status, "Getting default Kerberos realm.");
      exit(1);
    }

  /*
   * Database initialization.  Only init if database should be open.
   */

  if (stat(MOIRA_MOTD_FILE, &stbuf) != 0)
    {
      if ((status = mr_open_database()))
	{
	  com_err(whoami, status, "trying to open database.");
	  exit(1);
	}
      sanity_check_database();
    }
  else
    {
      dormant = ASLEEP;
      com_err(whoami, 0, "sleeping, not opening database");
    }

  sanity_check_queries();

  /*
   * Get moira server hostname for authentication
   */
  if (uname(&uts) < 0)
    {
      com_err(whoami, errno, "Unable to get local hostname");
      exit(1);
    }
  host = canonicalize_hostname(xstrdup(uts.nodename));
  for (p = host; *p && *p != '.'; p++)
    {
      if (isupper(*p))
	*p = tolower(*p);
    }
  *p = '\0';

  /*
   * Set up client array handler.
   */
  nclients = 0;
  clientssize = 10;
  clients = xmalloc(clientssize * sizeof(client *));

  mr_setup_signals();

  journal = fopen(JOURNAL, "a");
  if (!journal)
    {
      com_err(whoami, errno, "opening journal file");
      exit(1);
    }

  /*
   * Establish template connection.
   */
  listener = mr_listen(port);
  if (listener == -1)
    {
      com_err(whoami, MR_ABORTED, "trying to create listening connection");
      exit(1);
    }
  FD_ZERO(&xreadfds);
  FD_ZERO(&xwritefds);
  FD_SET(listener, &xreadfds);
  nfds = listener + 1;

  com_err(whoami, 0, "started (pid %d)", getpid());
  com_err(whoami, 0, rcsid);
  if (dormant != ASLEEP)
    send_zgram("MOIRA", "server started");
  else
    send_zgram("MOIRA", "server started, but database closed");

  /*
   * Run until shut down.
   */
  while (!takedown)
    {
      int i;
      struct timeval timeout = {60, 0}; /* 1 minute */

      /* If we're supposed to go down and we can, do it */
      if (((dormant == AWAKE) && (nclients == 0) &&
	   (stat(MOIRA_MOTD_FILE, &stbuf) == 0)) ||
	  (dormant == SLEEPY))
	{
	  mr_close_database();
	  com_err(whoami, 0, "database closed");
	  mr_setup_signals();
	  send_zgram("MOIRA", "database closed");
	  dormant = ASLEEP;
	}

      /* Block until something happens. */
      memcpy(&readfds, &xreadfds, sizeof(readfds));
      memcpy(&writefds, &xwritefds, sizeof(writefds));
      if (select(nfds, &readfds, &writefds, NULL, &timeout) == -1)
	{
	  if (errno != EINTR)
	    com_err(whoami, errno, "in select");
	  if (!inc_running || now - inc_started > INC_TIMEOUT)
	    next_incremental();
	  continue;
	}

      if (takedown)
	break;

      if (child_exited_abnormally)
	{
	  critical_alert("moirad", "%d: child exits with signal %d status %d",
			 child_pid, child_signal, child_status);
	  child_exited_abnormally = 0;
	}

      time(&now);
      if (!inc_running || now - inc_started > INC_TIMEOUT)
	next_incremental();
      tardy = now - 30 * 60;

      /* If we're asleep and we should wake up, do it */
      if ((dormant == ASLEEP) && (stat(MOIRA_MOTD_FILE, &stbuf) == -1) &&
	  (errno == ENOENT))
	{
	  mr_open_database();
	  com_err(whoami, 0, "database open");
	  mr_setup_signals();
	  send_zgram("MOIRA", "database open again");
	  dormant = AWAKE;
	}

      /* Handle any new connections */
      if (FD_ISSET(listener, &readfds))
	{
	  int newconn, addrlen = sizeof(struct sockaddr_in);
	  struct sockaddr_in addr;
	  client *cp;

	  newconn = accept(listener, (struct sockaddr *)&addr, &addrlen);
	  if (newconn == -1)
	    com_err(whoami, errno, "accepting new connection");
	  else if (newconn > 0)
	    {
	      if (newconn + 1 > nfds)
		nfds = newconn + 1;
	      FD_SET(newconn, &xreadfds);

	      /* Add a new client to the array */
	      nclients++;
	      if (nclients > clientssize)
		{
		  clientssize = 2 * clientssize;
		  clients = xrealloc(clients, clientssize * sizeof(client *));
		}

	      clients[nclients - 1] = cp = xmalloc(sizeof(client));
	      memset(cp, 0, sizeof(client));
	      cp->con = newconn;
	      cp->id = counter++;
	      cp->last_time_used = now;
	      cp->haddr = addr;
	      cp->tuplessize = 1;
	      cp->tuples = xmalloc(sizeof(mr_params));
	      memset(cp->tuples, 0, sizeof(mr_params));
	      cp->state = CL_ACCEPTING;
	      cp->version = 2;

	      cur_client = cp;
	      com_err(whoami, 0,
		      "New connection from %s port %d (now %d client%s)",
		      inet_ntoa(cp->haddr.sin_addr),
		      (int)ntohs(cp->haddr.sin_port),
		      nclients, nclients != 1 ? "s" : "");
	    }
	}

      /* Handle any existing connections. */
      for (i = 0; i < nclients; i++)
	{
	  cur_client = clients[i];

	  if (FD_ISSET(clients[i]->con, &writefds))
	    {
	      client_write(clients[i]);
	      if (!clients[i]->ntuples)
		{
		  FD_CLR(clients[i]->con, &xwritefds);
		  FD_SET(clients[i]->con, &xreadfds);
		}
	      clients[i]->last_time_used = now;
	    }

	  if (FD_ISSET(clients[i]->con, &readfds))
	    {
	      if (clients[i]->state == CL_ACCEPTING)
		{
		  switch(mr_cont_accept(clients[i]->con,
					&clients[i]->hsbuf,
					&clients[i]->hslen))
		    {
		    case -1:
		      break;

		    case 0:
		      clients[i]->state = CL_CLOSING;
		      break;

		    default:
		      clients[i]->state = CL_ACTIVE;
		      clients[i]->hsbuf = NULL;
		      break;
		    }
		}
	      else
		{
		  client_read(clients[i]);
		  if (clients[i]->ntuples)
		    {
		      FD_CLR(clients[i]->con, &xreadfds);
		      FD_SET(clients[i]->con, &xwritefds);
		    }
		  clients[i]->last_time_used = now;
		}
	    }

	  if (clients[i]->last_time_used < tardy)
	    {
	      com_err(whoami, 0, "Shutting down connection due to inactivity");
	      clients[i]->state = CL_CLOSING;
	    }

	  if (clients[i]->state == CL_CLOSING)
	    {
	      client *old;

	      com_err(whoami, 0, "Closed connection (now %d client%s, "
		      "%d queries)", nclients - 1, nclients != 2 ? "s" : "",
		      newqueries);

	      shutdown(clients[i]->con, 2);
	      close(clients[i]->con);
	      FD_CLR(clients[i]->con, &xreadfds);
	      FD_CLR(clients[i]->con, &xwritefds);
	      free_rtn_tuples(clients[i]);
	      free(clients[i]->tuples);
	      free(clients[i]->hsbuf);
	      old = clients[i];
	      clients[i] = clients[--nclients];
	      free(old);
	    }

	  cur_client = NULL;
	  if (takedown)
	    break;
	}
    }

  com_err(whoami, 0, "%s", takedown);
  if (dormant != ASLEEP)
    mr_close_database();
  send_zgram("MOIRA", takedown);
  return 0;
}

void reapchild(int x)
{
  int status, pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
      if (pid == inc_pid)
	inc_running = 0;
      if (!takedown && (WTERMSIG(status) != 0 || WEXITSTATUS(status) != 0))
	{
	  child_exited_abnormally = 1;
	  child_pid = pid;
	  child_signal = WTERMSIG(status);
	  child_status = WEXITSTATUS(status);
	}
    }
}


void godormant(int x)
{
  switch (dormant)
    {
    case AWAKE:
    case GROGGY:
      com_err(whoami, 0, "requested to go dormant");
      break;
    case ASLEEP:
      com_err(whoami, 0, "already asleep");
      break;
    case SLEEPY:
      break;
    }
  dormant = SLEEPY;
}


void gowakeup(int x)
{
  switch (dormant)
    {
    case ASLEEP:
    case SLEEPY:
      com_err(whoami, 0, "Good morning");
      break;
    case AWAKE:
      com_err(whoami, 0, "already awake");
      break;
    case GROGGY:
      break;
    }
  dormant = GROGGY;
}


void mr_setup_signals(void)
{
  struct sigaction action;

  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);

  /* There should probably be a few more of these. */

  action.sa_handler = sigshut;
  if ((sigaction(SIGTERM, &action, NULL) < 0) ||
      (sigaction(SIGINT, &action, NULL) < 0) ||
      (sigaction(SIGHUP, &action, NULL) < 0))
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }

  action.sa_handler = godormant;
  if (sigaction(SIGUSR1, &action, NULL) < 0)
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }

  action.sa_handler = gowakeup;
  if (sigaction(SIGUSR2, &action, NULL) < 0)
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }

  action.sa_handler = SIG_IGN;
  if (sigaction(SIGPIPE, &action, NULL) < 0)
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }

  action.sa_handler = reapchild;
  sigaddset(&action.sa_mask, SIGCHLD);
  if (sigaction(SIGCHLD, &action, NULL) < 0)
    {
      com_err(whoami, errno, "Unable to establish signal handlers.");
      exit(1);
    }
}
