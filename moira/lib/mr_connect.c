/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.19 1998-01-07 17:13:29 danw Exp $
 *
 *	Copyright (C) 1987, 1990 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	This routine is part of the client library.  It handles
 *	creating a connection to the mr server.
 */

#ifndef lint
static char *rcsid_mr_connect_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.19 1998-01-07 17:13:29 danw Exp $";
#endif

#include <mit-copyright.h>
#include "mr_private.h"
#include <moira_site.h>
#include <string.h>
#include <stdlib.h>
#include <hesiod.h>

static char *mr_server_host = 0;

/*
 * Open a connection to the mr server.  Looks for the server name
 * 1) passed as an argument, 2) in environment variable, 3) by hesiod
 * 4) compiled in default (from moira_site.h).
 */

int mr_connect(char *server)
{
  extern int errno;
  char *p, **pp, sbuf[256];

  if (!mr_inited)
    mr_init();
  if (_mr_conn)
    return MR_ALREADY_CONNECTED;

  if (!server || (strlen(server) == 0))
    server = getenv("MOIRASERVER");

#ifdef HESIOD
  if (!server || (strlen(server) == 0))
    {
      pp = hes_resolve("moira", "sloc");
      if (pp)
	server = *pp;
    }
#endif

  if (!server || (strlen(server) == 0))
    server = MOIRA_SERVER;

  if (!strchr(server, ':'))
    {
      p = strchr(MOIRA_SERVER, ':');
      p++;
      sprintf(sbuf, "%s:%s", server, p);
      server = sbuf;
    }

  errno = 0;
  _mr_conn = start_server_connection(server, "");
  if (_mr_conn == NULL)
    return errno;
  if (connection_status(_mr_conn) == CON_STOPPED)
    {
      int status = connection_errno(_mr_conn);
      if (!status)
	status = MR_CANT_CONNECT;
      mr_disconnect();
      return status;
    }

  /*
   * stash hostname for later use
   */

  mr_server_host = strsave(server);
  if ((p = strchr(mr_server_host, ':')))
    *p = '\0';
  mr_server_host = canonicalize_hostname(mr_server_host);
  return 0;
}

int mr_disconnect(void)
{
  CHECK_CONNECTED;
  _mr_conn = sever_connection(_mr_conn);
  free(mr_server_host);
  mr_server_host = 0;
  return 0;
}

int mr_host(char *host, int size)
{
  CHECK_CONNECTED;

  /* If we are connected, mr_server_host points to a valid string. */
  strncpy(host, mr_server_host, size);
  return 0;
}

int mr_noop(void)
{
  int status;
  mr_params param_st;
  struct mr_params *params = NULL;
  struct mr_params *reply = NULL;

  CHECK_CONNECTED;
  params = &param_st;
  params->mr_version_no = sending_version_no;
  params->mr_procno = MR_NOOP;
  params->mr_argc = 0;
  params->mr_argl = NULL;
  params->mr_argv = NULL;

  if ((status = mr_do_call(params, &reply)) == 0)
    status = reply->mr_status;

  mr_destroy_reply(reply);

  return status;
}
