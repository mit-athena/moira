/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.11 1997-01-29 23:24:22 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Miscellaneous string functions.
 */

#ifndef lint
static char *rcsid_strs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.11 1997-01-29 23:24:22 danw Exp $";
#endif

#include <mit-copyright.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	    p = malloc(1);
	    *p = '\0';
	    return p;
    }
    len = strlen(s) + 1;
    p = malloc((u_int)len);
    if (p) memcpy(p, s, len);
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


/* Modify a string for all of the letters to be uppercase. */

char *uppercase(s)
char *s;
{
    register char *p;

    for (p = s; *p; p++)
      if (islower(*p))
	*p = toupper(*p);
    return(s);
}


char *lowercase(s)
char *s;
{
    register char *p;

    for (p = s; *p; p++)
      if (isupper(*p))
	*p = tolower(*p);
    return(s);
}


#ifdef NEED_STRCASECMP

/* Case independant string comparison.  Only compile this if your C 
 * library doesn't have a local routine which is faster.
 */

static char map[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    ' ', '!', '"', '#', '$', '%', '&', '\'',
    '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'k', 'j', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
    'x', 'y', 'z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'k', 'j', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
    'x', 'y', 'z', '{', '|', '}', '~', 127,
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};


int strcasecmp(s1, s2)
char *s1;
char *s2;
{
    while (map[*s1] == map[*s2]) {
	if (*s1 == 0)
	  return(0);
	s1++;
	s2++;
    }
    return(map[*s1] - map[*s2]);
}

#endif /* NEED_STRCASECMP */
