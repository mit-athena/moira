/* $Id: fixhost.c,v 1.16 1998-02-08 19:31:15 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.16 1998-02-08 19:31:15 danw Exp $");

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
  int n_len;
  int has_dot = 0;
  char tbuf[BUFSIZ];
  struct utsname name;
  char *cp;

  if (strlen(host) > 2 && host[0] == '"' && host[strlen(host) - 1] == '"')
    {
      strcpy(tbuf, host + 1);
      free(host);
      tbuf[strlen(tbuf) - 1] = '\0';
      return strdup(tbuf);
    }

  if (strchr(host, '*') || strchr(host, '?') || strchr(host, '['))
    return host;

  hp = gethostbyname(host);

  if (hp)
    {
      n_len = strlen(hp->h_name) + 1;
      host = realloc(host, n_len);

      strcpy(host, hp->h_name);
      return host;
    }
  else
    {
      /* can't get name from nameserver; fix up the format a bit */
      for (cp = host; *cp; cp++)
	{
	  int c;
	  if (islower(c = *cp))
	    *cp = toupper(c);
	  has_dot |= (c == '.');
	}
      if (!has_dot)
	{
	  static char *domain = NULL;

	  if (domain == NULL)
	    {
	      uname(&name);
	      hp = gethostbyname(name.nodename);
	      cp = strchr(hp->h_name, '.');
	      if (cp)
		domain = strdup(++cp);
	      else
		domain = "";
	    }
	  sprintf(tbuf, "%s.%s", host, domain);
	  free(host);
	  host = strdup(tbuf);
	}
      return host;
    }
}
