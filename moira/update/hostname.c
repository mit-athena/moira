/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v 1.1 1987-08-22 17:54:19 wesommer Exp $
 */

#ifndef lint
static char *rcsid_hostname_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v 1.1 1987-08-22 17:54:19 wesommer Exp $";
#endif	lint

/* PrincipalHostname, borrowed from rcmd.c in Kerberos code */
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <strings.h>
char *
PrincipalHostname(alias)
     char *alias;
{
     struct hostent *h;
     char *phost = alias;
     if ((h=gethostbyname(alias)) != (struct hostent *)NULL) {
	  char *p = index(h->h_name, '.');
	  if (p)
	       *p = NULL;
	  p = phost = h->h_name;
	  do {
	       if (isupper(*p))
		    *p = tolower(*p);
	  } while (*p++);
     }
     return(phost);
}
