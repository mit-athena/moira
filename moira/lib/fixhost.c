/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.12 1998-01-05 14:54:47 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_fixhost_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixhost.c,v 1.12 1998-01-05 14:54:47 danw Exp $";
#endif

#include <mit-copyright.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <string.h>
#include <ctype.h>
#include <moira.h>

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

char *
canonicalize_hostname(host)
    char *host;
{
    register struct hostent *hp;
    int n_len;
    int has_dot = 0;
    char tbuf[BUFSIZ];
    struct utsname name;
    register char *cp;
    
    if (strlen(host) > 2 && host[0] == '"' && host[strlen(host)-1] == '"') {
	strcpy(tbuf, host+1);
	free(host);
	tbuf[strlen(tbuf)-1] = 0;
	return(strsave(tbuf));
    }

    if (strchr(host, '*') || strchr(host, '?') || strchr(host, '['))
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
	    static char *domain = NULL;

	    if (domain == NULL) {
		(void) uname(&name);
		hp = gethostbyname(name.nodename);
		cp = strchr(hp->h_name, '.');
		if (cp)
		  domain = strsave(++cp);
		else
		  domain = "";
	    }
	    (void) sprintf(tbuf, "%s.%s", host, domain);
	    free(host);
	    host = strsave(tbuf);
	}
	return host;
    }
}
