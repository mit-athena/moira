/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v 1.4 1998-01-05 14:56:48 danw Exp $
 */

#ifndef lint
static char *rcsid_rafnu_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v 1.4 1998-01-05 14:56:48 danw Exp $";

#endif	lint


#include <stdio.h>
#include <ctype.h>

FixCase(p)
    register char *p;
{
    register int cflag;		/* convert to lcase, unless at start or
				 * following */

    /* a space or punctuation mark (e.g., '-') */

    for (cflag = 0; *p; p++) {
	if (cflag && isupper(*p)) {
	    *p = tolower(*p);
	}
	else if (isspace(*p) || ispunct(*p)) {
	    cflag = 0;
	}
	else {
	    cflag = 1;
	}
    }
}

LookForJrAndIII(nm, pends_sr, pends_jr, pends_iii, pends_iv)
    register char *nm;
    register int *pends_sr;
    register int *pends_jr;
    register int *pends_iii;
    register int *pends_iv;
{
    register int len = strlen(nm);

    if (len >= 4 && !strcmp(nm + len - 3, " SR")) {
	*pends_sr = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 4 && !strcmp(nm + len - 3, " JR")) {
	*pends_jr = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 4 && !strcmp(nm + len - 3, " IV")) {
	*pends_iv = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 5 && !strcmp(nm + len - 4, " SR.")) {
	*pends_sr = 1;
	nm[len - 4] = '\0';
    }
    else if (len >= 5 && !strcmp(nm + len - 4, " JR.")) {
	*pends_jr = 1;
	nm[len - 4] = '\0';
    }
    else if (len >= 5 && !strcmp(nm + len - 4, " III")) {
	*pends_iii = 1;
	nm[len - 4] = '\0';
    }
}

LookForSt(nm)			/* ST PIERRE, etc. */
    register char *nm;
{
    char temp[256];

    if (!strcmp(nm, "ST ")) {
	(void) strcpy(temp, nm + 3);
	(void) strcpy(nm, "ST. ");
	(void) strcat(nm, temp);
    }
}

LookForO(nm)			/* O BRIEN, etc. */
    register char *nm;
{
    if (!strcmp(nm, "O ") && isalpha(nm[2])) {
	nm[1] = '\'';
    }
}
