/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v 1.3 1994-09-16 22:01:55 jweiss Exp $
 */
/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#ifndef lint
static char *rcsid_hostname_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/hostname.c,v 1.3 1994-09-16 22:01:55 jweiss Exp $";
#endif	lint

/* PrincipalHostname, borrowed from rcmd.c in Kerberos code */
#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
char *
PrincipalHostname(alias)
     char *alias;
{
     struct hostent *h;
     char *phost = alias;
     if ((h=gethostbyname(alias)) != (struct hostent *)NULL) {
	  char *p = strchr(h->h_name, '.');
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
