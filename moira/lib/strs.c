/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.1 1987-09-02 17:16:20 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	Miscellaneous string functions.
 */

#ifndef lint
static char *rcsid_strs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.1 1987-09-02 17:16:20 wesommer Exp $";
#endif lint


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
    register int len = strlen(s) + 1;
    register char *p = malloc((u_int)len);
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

