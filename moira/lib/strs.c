/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.3 1987-09-03 02:35:50 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Miscellaneous string functions.
 */

#ifndef lint
static char *rcsid_strs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.3 1987-09-03 02:35:50 wesommer Exp $";
#endif lint

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
char *strtrim(s)
    register char *s;
{
    register char *t;
    
    while (isspace(*s)) s++;
    /* skip to end of string */

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

/*
 * Case insensitive string compare.
 */

int cistrcmp(cp1, cp2)
    char *cp1, *cp2;
{
    register int c1, c2;
    
    do {
	if (isupper(c1 = (*cp1++))) c1 = tolower(c1);
	if (isupper(c2 = (*cp2++))) c2 = tolower(c2);
	if (c1 != c2) return c1-c2;
    } while (c1 && c2);
    return 0;
}

