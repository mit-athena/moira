/* $Id: mr_connect.c,v 1.24 1998-05-26 18:14:09 danw Exp $
 *
 * This routine is part of the client library.  It handles
 * creating a connection to the moira server.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include "mr_private.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_HESIOD
#include <hesiod.h>
#endif

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/mr_connect.c,v 1.24 1998-05-26 18:14:09 danw Exp $");

int _mr_conn = 0;
static char *mr_server_host = NULL;

/* mrgdb compatibility magic

   The data looks like this:

   client -> server
     00000036 				[length of rest of packet]
     00000004 				[number of fields]
     01 01 01 01			[types of fields: 4 strings]
     "server_id\0parms\0host\0user\0"	[field names]
     00000001				[length of null-terminated server_id]
     "\0"				[server_id: ignored anyway]
     00000001				[length of null-terminated parms]
     "\0"				[parms: ignored anyway]
     00000001				[length of null-terminated client host]
     "\0"				[host: ignored anyway]
     00000001				[length of null-terminated client name]
     "\0"				[user: ignored anyway]

   server -> client
     00000031 				[length of rest of packet]
     00000003 				[number of fields]
     00 01 01 				[types of fields: int and 2 strings]
     "disposition\0server_id\0parms\0"	[field names]
     00000001				[GDB_ACCEPTED]
     00000001				[length of null-terminated server_id]
     "\0"				[server_id: ignored anyway]
     00000001				[length of null-terminated parms]
     "\0"				[parms: ignored anyway]

*/

static char challenge[58] = "\0\0\0\066\0\0\0\004\001\001\001\001server_id\0parms\0host\0user\0\0\0\0\001\0\0\0\0\001\0\0\0\0\001\0\0\0\0\001\0";
static char response[53] = "\0\0\0\061\0\0\0\003\0\001\001disposition\0server_id\0parms\0\0\0\0\001\0\0\0\001\0\0\0\0\001\0";

/*
 * Open a connection to the moira server.  Looks for the server name
 * 1) passed as an argument, 2) in environment variable, 3) by hesiod
 * 4) compiled in default
 */

int mr_connect(char *server)
{
  char *port, **pp, *sbuf = NULL;
  struct hostent *shost;

  if (_mr_conn)
    return MR_ALREADY_CONNECTED;
  if (!mr_inited)
    mr_init();

  if (!server || (strlen(server) == 0))
    server = getenv("MOIRASERVER");

#ifdef HAVE_HESIOD
  if (!server || (strlen(server) == 0))
    {
      pp = hes_resolve("moira", "sloc");
      if (pp)
	server = *pp;
    }
#endif

  if (!server || (strlen(server) == 0))
    server = MOIRA_SERVER;

  shost = gethostbyname(server);
  if (!shost)
    return MR_CANT_CONNECT;

  if (strchr(server, ':'))
    {
      int len = strcspn(server, ":");
      sbuf = malloc(len + 1);
      strncpy(sbuf, server, len);
      sbuf[len - 1] = '\0';
      server = sbuf;
      port = strchr(server, ':') + 1;
    }
  else
    port = strchr(MOIRA_SERVER, ':') + 1;

  _mr_conn = mr_connect_internal(server, port);
  free(sbuf);
  if (!_mr_conn)
    return MR_CANT_CONNECT;

  /* stash hostname for later use */
  mr_server_host = strdup(shost->h_name);
  return MR_SUCCESS;
}

int mr_connect_internal(char *server, char *port)
{
  int fd, size, more;
  struct sockaddr_in target;
  struct hostent *shost;
  char actualresponse[53];

  shost = gethostbyname(server);
  if (!shost)
    return 0;

  if (port[0] == '#')
    target.sin_port = atoi(port + 1);
  else
    {
      struct servent *s;
      s = getservbyname(port, "tcp");
      if (s)
	target.sin_port = s->s_port;
      else
	return 0;
    }

  memcpy(&target.sin_addr, shost->h_addr, shost->h_length);
  target.sin_family = shost->h_addrtype;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return 0;

  if (connect(fd, (struct sockaddr *)&target, sizeof(target)) < 0)
    {
      close(fd);
      return 0;
    }

  /* Do magic mrgdb initialization */
  size = write(fd, challenge, sizeof(challenge));
  if (size != sizeof(challenge))
    {
      close(fd);
      return 0;
    }
  for (size = 0; size < sizeof(actualresponse); size += more)
    {
      more = read(fd, actualresponse + size, sizeof(actualresponse) - size);
      if (!more)
	break;
    }
  if (size != sizeof(actualresponse))
    {
      close(fd);
      return 0;
    }
  if (memcmp(actualresponse, response, sizeof(actualresponse)))
    {
      close(fd);
      return 0;
    }

  /* You win */
  return fd;
}

int mr_disconnect(void)
{
  CHECK_CONNECTED;
  close(_mr_conn);
  _mr_conn = 0;
  free(mr_server_host);
  mr_server_host = NULL;
  return MR_SUCCESS;
}

int mr_host(char *host, int size)
{
  CHECK_CONNECTED;

  /* If we are connected, mr_server_host points to a valid string. */
  strncpy(host, mr_server_host, size);
  host[size - 1] = '\0';
  return MR_SUCCESS;
}

int mr_noop(void)
{
  int status;
  mr_params params, reply;

  CHECK_CONNECTED;
  params.u.mr_procno = MR_NOOP;
  params.mr_argc = 0;
  params.mr_argl = NULL;
  params.mr_argv = NULL;

  if ((status = mr_do_call(&params, &reply)) == MR_SUCCESS)
    status = reply.u.mr_status;

  mr_destroy_reply(reply);

  return status;
}


/* Server side */

int mr_listen(char *port)
{
  struct sockaddr_in sin;
  int s, on = 1;

  memset(&sin, 0, sizeof(sin));
  if (port[0] == '#')
    sin.sin_port = atoi(port + 1);
  else
    {
      struct servent *s;
      s = getservbyname(port, "tcp");
      if (s)
	sin.sin_port = s->s_port;
      else
	return -1;
    }

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    return -1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int)) < 0)
    {
      close(s);
      return -1;
    }
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
      close(s);
      return -1;
    }
  if (listen(s, 5) < 0)
    {
      close(s);
      return -1;
    }

  return s;
}

int mr_accept(int s, struct sockaddr_in *sin)
{
  int conn, addrlen = sizeof(struct sockaddr_in);
  char lbuf[4], *buf;
  long len, size, more;

  conn = accept(s, (struct sockaddr *)sin, &addrlen);
  if (conn < 0)
    return -1;

  /* Now do mrgdb accept protocol */
  /* XXX timeout */

  if (read(conn, lbuf, 4) != 4)
    {
      close(conn);
      return -1;
    }
  getlong(lbuf, len);

  buf = malloc(len);
  if (!buf || len < 54)
    {
      close(conn);
      free(buf);
      return -1;
    }

  for (size = 0; size < len; size += more)
    {
      more = read(conn, buf + size, len - size);
      if (!more)
	break;
    }
  if (size != len)
    {
      close(conn);
      free(buf);
      return 0;
    }

  if (memcmp(buf, challenge + 4, 34))
    {
      close(conn);
      free(buf);
      return 0;
    }

  /* good enough */
  free(buf);

  if (write(conn, response, sizeof(response)) != sizeof(response))
    {
      close(conn);
      return -1;
    }
  return conn;
}
