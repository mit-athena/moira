/* $Id: fixhost.c,v 1.19 1998-02-27 20:53:42 danw Exp $
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
#include <sys/socket.h>
#include <sys/utsname.h>

#include <netdb.h>
#include <netinet/in.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.19 1998-02-27 20:53:42 danw Exp $");

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
  int has_dot = 0;
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

  if (strchr(host, '*') || strchr(host, '?') || !strcmp(host, "[NONE]"))
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
      for (cp = host; *cp; cp++)
	{
	  if (islower(*cp))
	    *cp = toupper(*cp);
	  has_dot |= (*cp == '.');
	}
      if (!has_dot)
	{
	  static char *domain = NULL;

	  if (domain == NULL)
	    {
	      struct utsname name;

	      uname(&name);
	      hp = gethostbyname(name.nodename);
	      if (hp)
		{
		  cp = strchr(hp->h_name, '.');
		  if (cp)
		    domain = strdup(++cp);
		  if (!domain)
		    domain = "";
		}
	      else
		domain = "";
	    }
	  tbuf = malloc(strlen(host) + strlen(domain) + 2);
	  if (!tbuf)
	    return NULL;
	  sprintf(tbuf, "%s.%s", host, domain);
	  free(host);
	  host = tbuf;
	}
      return host;
    }
}
