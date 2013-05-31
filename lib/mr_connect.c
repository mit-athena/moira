/* $Id: mr_connect.c 4101 2013-03-06 16:37:24Z zacheiss $
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifndef closesocket
#define closesocket close
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif

#ifdef HAVE_HESIOD
#include <hesiod.h>
#endif

#ifdef _WIN32
/* This is declared in wshelper's headers, but those also include a
 * definition of the putlong macro which conflicts with Moira's.
 */
struct hostent * WINAPI rgethostbyname(char *name);
#endif

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/lib/mr_connect.c $ $Id: mr_connect.c 4101 2013-03-06 16:37:24Z zacheiss $");

#define DEFAULT_SERV "moira_db"
#define DEFAULT_PORT 775

int _mr_conn = 0;
static char *mr_server_host = NULL;

#ifdef _WIN32
static int mr_conn_wsaStart = 0;
#endif

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
#ifdef _WIN32
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
#endif

  if (_mr_conn)
    return MR_ALREADY_CONNECTED;
  if (!mr_inited)
    mr_init();

  if (!server || (strlen(server) == 0))
    server = getenv("MOIRASERVER");

#ifdef _WIN32
  wVersionRequested = MAKEWORD(2, 2);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0)
    return MR_CANT_CONNECT;
  mr_conn_wsaStart++;
#endif

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

  if (strchr(server, ':'))
    {
      int len = strcspn(server, ":");
      sbuf = malloc(len + 1);
      strncpy(sbuf, server, len);
      sbuf[len] = '\0';
      port = strchr(server, ':') + 1;
      server = sbuf;
    }
  else
    port = strchr(MOIRA_SERVER, ':') + 1;

  _mr_conn = mr_connect_internal(server, port);
  free(sbuf);
  if (!_mr_conn)
    return MR_CANT_CONNECT;

  return MR_SUCCESS;
}

int mr_connect_internal(char *server, char *port)
{
  int size, more;
  struct sockaddr_in target;
  struct hostent *shost;
  char actualresponse[53];
  char *host = NULL;
  int fd = SOCKET_ERROR;
  int ok = 0;
  int on = 1; /* Value variable for setsockopt() */

#if defined(_WIN32)
  shost = rgethostbyname(server);
#else
  shost = gethostbyname(server);
#endif
  if (!shost)
    goto cleanup;

  /* Get the host info in case some library decides to clobber shost. */
  memcpy(&target.sin_addr, shost->h_addr, shost->h_length);
  target.sin_family = shost->h_addrtype;
  host = strdup(shost->h_name);

  if (port[0] == '#')
    target.sin_port = htons((unsigned short)atoi(port + 1));
  else
    {
      struct servent *s;
      target.sin_port = 0;
      s = getservbyname(port, "tcp");
      if (s)
	target.sin_port = s->s_port;
#ifdef HAVE_HESIOD
      if (!target.sin_port)
        {
          s = hes_getservbyname(port, "tcp");
          if (s)
            target.sin_port = s->s_port;
        }
#endif
      if (!target.sin_port && !strcasecmp(port, DEFAULT_SERV))
	target.sin_port = htons(DEFAULT_PORT);
      if (!target.sin_port)
	goto cleanup;
    }

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    goto cleanup;

  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(int)) < 0)
    goto cleanup;

  if (connect(fd, (struct sockaddr *)&target, sizeof(target)) < 0)
    goto cleanup;

  /* Do magic mrgdb initialization */
  size = send(fd, challenge, sizeof(challenge), 0);
  if (size != sizeof(challenge))
    goto cleanup;
  for (size = 0; size < sizeof(actualresponse); size += more)
    {
      more = recv(fd, actualresponse + size, sizeof(actualresponse) - size, 0);
      if (more <= 0)
	break;
    }
  if (size != sizeof(actualresponse))
    goto cleanup;
  if (memcmp(actualresponse, response, sizeof(actualresponse)))
    goto cleanup;

  ok = 1;
  mr_server_host = host;

 cleanup:
  if (!ok)
    {
      if (host)
	free(host);
      if (fd != SOCKET_ERROR)
	closesocket(fd);
      return 0;
    }
  /* You win */
  return fd;
}

int mr_disconnect(void)
{
  CHECK_CONNECTED;
  closesocket(_mr_conn);
  _mr_conn = 0;
  free(mr_server_host);
  mr_server_host = NULL;
#ifdef _WIN32
  if (mr_conn_wsaStart > 0)
    {
      WSACleanup();
      mr_conn_wsaStart--;
    }
#endif
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
  sin.sin_family = AF_INET;
  if (port[0] == '#')
    sin.sin_port = atoi(port + 1);
  else
    {
      struct servent *s;
      s = getservbyname(port, "tcp");
      if (s)
	sin.sin_port = s->s_port;
      else
#ifndef HAVE_HESIOD
	return -1;
#else
      {
        s = hes_getservbyname(port, "tcp");
	if (s)
	  sin.sin_port = s->s_port;
	else
	  return -1;
      }
#endif /* HAVE_HESIOD */
    }
  
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    return -1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int)) < 0)
    {
      closesocket(s);
      return -1;
    }
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
      closesocket(s);
      return -1;
    }
  if (listen(s, 5) < 0)
    {
      closesocket(s);
      return -1;
    }

  return s;
}

/* mr_accept returns -1 on accept() error, 0 on bad connection,
   or connection fd on success */

int mr_accept(int s, struct sockaddr_in *sin)
{
  int conn = -1, addrlen = sizeof(struct sockaddr_in), nread, status;
  char *buf = NULL;

  while (conn < 0)
    {
      conn = accept(s, (struct sockaddr *)sin, &addrlen);
      if (conn < 0 && errno != EINTR
#ifdef ERESTART
	  && errno != ERESTART
#endif
#ifdef ECONNABORTED
	  && errno != ECONNABORTED
#endif
	  )
	return -1;
    }

  do
    status = mr_cont_accept(conn, &buf, &nread);
  while (status == -1);

  return status;
}

/* mr_cont_accept returns 0 if it has failed, an fd if it has succeeded,
   or -1 if it is still making progress */

int mr_cont_accept(int conn, char **buf, int *nread)
{
  long len, more;

  if (!*buf)
    {
      char lbuf[4];
      if (recv(conn, lbuf, 4, 0) != 4)
	{
	  closesocket(conn);
	  return 0;
	}
      getlong(lbuf, len);
      len += 4;

      if (len < 58 || len > 1000)
	{
	  closesocket(conn);
	  return 0;
	}

      *buf = malloc(len);
      if (!*buf)
	{
	  closesocket(conn);
	  return 0;
	}
      putlong(*buf, len);
      *nread = 4;
      return -1;
    }
  else
    getlong(*buf, len);

  more = recv(conn, *buf + *nread, len - *nread, 0);

  switch (more)
    {
    case 0:
      /* If we read 0 bytes, the remote end has gone away. */
      break;
    case -1:
      /* If errno is EINTR, return -1 and try again, otherwise we failed. */
      if (errno == EINTR)
	return -1;
      else
	{
	  closesocket(conn);
	  free(*buf);
	  return 0;
	}
      break;
    default:
      *nread += more;
      if (*nread != len)
	return -1;
      break;
    }

  if (memcmp(*buf + 4, challenge + 4, 34))
    {
      closesocket(conn);
      free(*buf);
      return 0;
    }

  /* good enough */
  free(*buf);

  if (send(conn, response, sizeof(response), 0) != sizeof(response))
    {
      closesocket(conn);
      return 0;
    }
  return conn;
}
