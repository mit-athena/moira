/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.3 1988-09-13 15:51:06 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_fixhost_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.3 1988-09-13 15:51:06 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/resolv.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>

extern char *malloc();
extern char *realloc();
extern char *strsave();

/*
 * Canonicalize hostname; if it is in the namespace, call the
 * nameserver to expand it; otherwise uppercase it and append the
 * default domain (using an, er, undocumented global of the
 * nameserver).
 *
 * Assumes that host was allocated using malloc(); it may be freed or
 * realloc'ed, so the old pointer should not be considered valid.
 */

char *
canonicalize_hostname(host)
    char *host;
{
    register struct hostent *hp;
    int n_len;
    int has_dot = 0;
    char tbuf[BUFSIZ];
    register char *cp;
    
    if (index(host, '*') || index(host, '?'))
      return(host);

    hp = gethostbyname(host);

    if (hp) {
	n_len = strlen(hp->h_name) + 1;
	host = realloc(host, (unsigned)n_len);
	
	(void) strcpy(host, hp->h_name);
	return host;
    } else {
	/* can't get name from nameserver; fix up the format a bit */
	for (cp = host; *cp; cp++) {
	    register int c;	/* pcc doesn't like register char */
	    if (islower(c = *cp)) *cp = toupper(c);
	    has_dot |= (c == '.');
	}
	if (!has_dot) {
	    (void) sprintf(tbuf, "%s.%s", host, _res.defdname);
	    free(host);
	    host = strsave(tbuf);
	}
	return host;
    }
}
