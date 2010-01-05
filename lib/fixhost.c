/* $Id: fixhost.c,v 1.23 2000-03-15 22:44:18 rbasch Exp $
 *
 * Canonicalize a hostname
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <sys/types.h>

#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif /* _WIN32 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.23 2000-03-15 22:44:18 rbasch Exp $");

static struct hostent *local_gethostbyname(void)
{
#ifdef HAVE_UNAME
  struct utsname name;
  uname(&name);
  return gethostbyname(name.nodename);
#else
  char hostname[128];
  gethostname(hostname, sizeof(hostname));
  hostname[sizeof(hostname)-1] = 0;
  return gethostbyname(hostname);
#endif
}

static char *local_domain(void)
{
  static char *domain = NULL;
  char *cp;
  struct hostent *hp;

  if (domain == NULL)
    {
      char hostbuf[256];

      if (mr_host(hostbuf, sizeof(hostbuf)) == MR_SUCCESS)
	{
	  cp = strchr(hostbuf, '.');
	  if (cp)
	    domain = strdup(++cp);
	}
      else
	{
	  hp = local_gethostbyname();
	  if (hp)
	    {
	      cp = strchr(hp->h_name, '.');
	      if (cp)
		domain = strdup(++cp);
	    }
	}
      if (!domain)
	domain = "";
    }

  return domain;
}

/*
 * Canonicalize hostname:
 *  if it is in double-quotes, then strip the quotes and return the name.
 *  if it is in the namespace, call the nameserver to expand it
 *  otherwise uppercase it and append the default domain (using an, er,
 *    undocumented global of the nameserver).
 *
 * Assumes that host was allocated using malloc(); it may be freed or
 * realloc'ed, so the old pointer should not be considered valid.
 */

char *canonicalize_hostname(char *host)
{
  struct hostent *hp;
  int len;
  char *tbuf, *cp;

  len = strlen(host);
  if (len > 2 && host[0] == '"' && host[len - 1] == '"')
    {
      tbuf = malloc(len - 1);
      if (!tbuf)
	return NULL;
      strncpy(tbuf, host + 1, len - 2);
      tbuf[len - 2] = '\0';
      free(host);
      return tbuf;
    }

  if (strchr(host, '*') || strchr(host, '?') || *host == '[')
    return host;

  hp = gethostbyname(host);

  if (hp)
    {
      host = realloc(host, strlen(hp->h_name) + 1);
      if (host)
	strcpy(host, hp->h_name);
      return host;
    }
  else
    {
      /* can't get name from nameserver; fix up the format a bit */
      cp = strchr(host, '.');
      if (!cp)
	{
	  tbuf = malloc(strlen(host) + strlen(local_domain()) + 2);
	  if (!tbuf)
	    return NULL;
	  sprintf(tbuf, "%s.%s", host, local_domain());
	  free(host);
	  host = tbuf;
	}
      else if (strcasecmp(cp + 1, local_domain()) != 0)
	return host;

      /* This is a host in our local domain, so capitalize it. */
      for (cp = host; *cp; cp++)
	*cp = toupper(*cp);
      return host;
    }
}
