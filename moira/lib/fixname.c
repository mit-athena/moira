/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v $
 *	$Author: jweiss $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v 1.6 1994-09-16 16:30:10 jweiss Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_fixname_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v 1.6 1994-09-16 16:30:10 jweiss Exp $";
#endif lint

#include <mit-copyright.h>
#include <string.h>
#include <ctype.h>

#define LAST_LEN		100
#define FIRST_LEN		100

void FixName(ilnm, ifnm, last, first, middle)
	char *ilnm, *ifnm;
	char *first, *last, *middle;
{
	int ends_jr=0, ends_iii=0, ends_iv=0, ends_ii=0, ends_v=0;

	Upcase(ilnm);
	Upcase(ifnm);
	
	/* Last name ... */

	TrimTrailingSpace(ilnm);
	LookForJrAndIII(ilnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);
	LookForSt(ilnm);
	LookForO(ilnm);
	FixCase(ilnm);
	strncpy(last, ilnm, LAST_LEN);

		/* First name  & middle initial ... */

	TrimTrailingSpace(ifnm);
	LookForJrAndIII(ifnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);

        GetMidInit(ifnm, middle);

	FixCase(ifnm);
#ifdef notdef
		/* okay, finish up first name */
	AppendJrOrIII(ifnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);
#endif notdef
	strncpy(first, ifnm, FIRST_LEN);
}

FixCase(p)
register char *p;
{
    register int cflag;	/* convert to lcase, unless at start or following */
			/* a space or punctuation mark (e.g., '-') */

    for (cflag = 0; *p; p++) {
	if (cflag && isupper(*p))
	  *p = tolower(*p);
	else if (!cflag && islower(*p))
	  *p = toupper(*p);
	if (isalpha(*p))
	  cflag = 1;
	else
	  cflag = 0;
    }
}

LookForJrAndIII(nm, pends_jr, pends_ii, pends_iii, pends_iv, pends_v)
register char *nm;
register int *pends_jr;
int *pends_ii;
register int *pends_iii;
register int *pends_iv;
int *pends_v;
{
    register int len = strlen(nm);

    if (len >= 4 && !strcmp(nm + len - 3, " JR")) {
	*pends_jr = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 4 && !strcmp(nm + len - 3, " IV")) {
	*pends_iv = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 5 && !strcmp(nm + len - 4, " JR.")) {
	*pends_jr = 1;
	nm[len - 4] = '\0';
    }
    else if (len >= 5 && !strcmp(nm + len - 4, " III")) {
	*pends_iii = 1;
	nm[len - 4] = '\0';
    }
    else if (len >= 4 && !strcmp(nm + len - 3, " II")) {
	*pends_ii = 1;
	nm[len - 3] = '\0';
    }
    else if (len >= 3 && !strcmp(nm + len - 2, " V")) {
	*pends_v = 1;
	nm[len - 2] = '\0';
    }
}

LookForSt(nm)		/* ST PIERRE, etc. */
register char *nm;
{
    char temp[256];

    if (!strcmp(nm,"ST ")) {
	strcpy(temp, nm + 3);
	strcpy(nm, "ST. ");
	strcat(nm, temp);
    }
}

LookForO(nm)		/* O BRIEN, etc. */
register char *nm;
{
    if (!strcmp(nm, "O ") && isalpha(nm[2])) {
	nm[1] = '\'';
    }
}

TrimTrailingSpace(ip)
register char *ip;
{
    register char *p;
    for (p = ip + strlen(ip) - 1; p >= ip && isspace(*p); p--) {
	*p = '\0';
    }
}

Upcase(cp)
	char *cp;
{
	register int c;
	
	for ( ; c= *cp; cp++)
		if (islower(c)) *cp = toupper(c);
}

GetMidInit(nm, mi)
register char *nm;	/* truncate at first space, if any such */
register char *mi;	/* set to first char after first space, if any such */
{
    while (*nm && !isspace(*nm)) {
	nm++;
    }
    if (*nm) {
	*nm++ = '\0';
    }
    while (*nm && isspace(*nm)) {
	nm++;
    }
    if (*nm) {
	*mi++ = *nm;
    }
    *mi = '\0';
}
