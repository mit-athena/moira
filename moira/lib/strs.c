/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.6 1990-05-02 13:12:11 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Miscellaneous string functions.
 */

#ifndef lint
static char *rcsid_strs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.6 1990-05-02 13:12:11 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/types.h>
#include <strings.h>
#include <ctype.h>

extern char *malloc(), *realloc();

/*
 * Random string functions which should be in the C library..
 */

/*
 * Make a copy of a string.
 */
char *
strsave(s)
    char *s;
{
    register int len;
    register char *p;
    /* Kludge for sloppy string semantics */
    if (!s) {
	    printf("NULL != \"\" !!!!\r\n");
	    p = malloc(1);
	    *p = '\0';
	    return p;
    }
    len = strlen(s) + 1;
    p = malloc((u_int)len);
    if (p) bcopy(s, p, len);
    return p;
}
/*
 * Trim whitespace off both ends of a string.
 */
char *strtrim(save)
    register char *save;
{
    register char *t, *s;

    s = save;
    while (isspace(*s)) s++;
    /* skip to end of string */
    if (*s == '\0') {
	*save = '\0';
	return(save);
    }

    for (t = s; *t; t++) continue; 
    while (t > s) {
	--t;
	if (!isspace(*t)) {
	    t++;
	    break;
	}
    }
    *t = '\0';
    return s;
}
