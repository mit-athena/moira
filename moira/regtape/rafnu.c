/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v 1.1 1990-01-30 16:01:40 mar Exp $
 */

#ifndef lint
static char *rcsid_rafnu_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/rafnu.c,v 1.1 1990-01-30 16:01:40 mar Exp $";

#endif	lint


/* ReadAndFixNextUser reads in a student record line in the format
 * of the registrar's tape and hacks on the info to make it conform
 * to athenareg's standards.  For example, the line
  
123321123 ALBERGHETTI III                        JOANNA MARIA             97
  
 * gets processed into:
 *
 * -->
 * {nil, nil, nil, nil,
 *  last -> "Alberghetti",
 *  first -> "Joanna III",
 *  mid_init -> "M",
 *  nil,
 *  crypt_ssn -> "aj7.NBbasJbn8"	; encrypted with salt "aj"
 *  year -> "97",
 *  nil}
 *
 */

#include <stdio.h>
#include <ctype.h>

#define LAST_LEN		15
#define FIRST_LEN		15
#define FULL_NAME_LEN		31

#define REG_TAPE_LINE_LEN	77
#define SSN_OFFSET		0
#define SSN_ILEN		9
#define LAST_NAME_OFFSET	10
#define LAST_NAME_MAX_ILEN	39
#define FIRST_NAME_OFFSET	49
#define FIRST_NAME_MAX_ILEN	25
#define MIT_YEAR_OFFSET		74
#define MIT_YEAR_ILEN		2

#define BUFSIZE  1024

int 
ReadAndFixNextUser(s, querc, querv)
    FILE *s;
    int querc;
    char **querv;

{
    char ibuf[BUFSIZE];		/* input line buffer */
    static lno = 0;		/* line number */
    int len;			/* line length */
    char issn[SSN_ILEN + 1];
    char ilnm[LAST_NAME_MAX_ILEN + 1];
    char ifnm[FIRST_NAME_MAX_ILEN + 1 + 6];	/* extra for ", Jr.", et al */
    char iyr[MIT_YEAR_ILEN + 1];
    int ends_jr = 0;		/* 1 if name ends in "JR" or "JR." */
    int ends_iii = 0;		/* 1 if name ends in "III" */
    int ends_iv = 0;		/* 1 if name ends in "IV" */
    char salt[2];
    register char *p;
    char *strcat();
    char *strcpy();
    char *strncpy();
    char *crypt();

    for (;;) {			/* ... well, at least until we get a good
				 * input line */
	if (fgets(ibuf, BUFSIZE, s) == NULL) {
	    fprintf(stderr, "Reached EOF after line %d.\n", lno);
	    return (EOF);	/* reached EOF */
	}
/* 
 * POTENTIAL  BUG!!!   fgets will not remove \n from input stream
 *                      May need to manually get that character off
 */
	lno++;

	if ((len = strlen(ibuf)) != REG_TAPE_LINE_LEN) {
	    fprintf(stderr, "bad length\n");
	    goto complain_and_continue;
	}

	/* grab and check input SSN */

	issn[SSN_ILEN] = '\0';
	(void) strncpy(issn, ibuf + SSN_OFFSET, SSN_ILEN);
	for (p = issn; *p; p++) {
	    if (!isdigit(*p)) {
		fprintf(stderr, "bad char \"%c\"\n", *p);
		goto complain_and_continue;
	    }
	}

	/* check for blank before last name */

	if (!isspace(ibuf[LAST_NAME_OFFSET - 1])) {
	    fprintf(stderr, "not space \"%c\"\n", ibuf[LAST_NAME_OFFSET - 1]);
	    goto complain_and_continue;
	}

	/* grab input last name */

	ilnm[LAST_NAME_MAX_ILEN] = '\0';
	(void) strncpy(ilnm, ibuf + LAST_NAME_OFFSET, LAST_NAME_MAX_ILEN);
	for (p = ilnm; *p; p++) {
	    if (!isalpha(*p) && *p != '-' && *p != ' ' &&
		*p != '.' && *p != '\'') {
		fprintf(stderr, "bad char in name \"%c\"\n", *p);
		goto complain_and_continue;
	    }
	}

	/* grab input first name */

	ifnm[FIRST_NAME_MAX_ILEN] = '\0';
	(void) strncpy(ifnm, ibuf + FIRST_NAME_OFFSET, FIRST_NAME_MAX_ILEN);

	for (p = ifnm; *p; p++) {
	    if (!isalpha(*p) && *p != '-' && *p != ' ' && *p != '.') {
		fprintf(stderr, "bad char in fname \"%c\"\n", *p);
		goto complain_and_continue;
	    }
	}

	/* grab MIT year */

	iyr[MIT_YEAR_ILEN] = '\0';
	(void) strncpy(iyr, ibuf + MIT_YEAR_OFFSET, MIT_YEAR_ILEN);
	if (strcmp(iyr, "GG") && strcmp(iyr, "90") && strcmp(iyr, "88")
	    && strcmp(iyr, "89") && strcmp(iyr, "91")) {
	    fprintf(stderr, "bad year \"%s\"\n", iyr);
	    goto complain_and_continue;
	}

	/* Okay, got everything, now hack on it */

	/* Last name ... */

	TrimTrailingSpace(ilnm);
	LookForJrAndIII(ilnm, &ends_jr, &ends_iii, &ends_iv);
	LookForSt(ilnm);
	LookForO(ilnm);
	FixCase(ilnm);
	(void) strncpy(querv[4], ilnm, LAST_LEN);

	/* First name  & middle initial ... */

	TrimTrailingSpace(ifnm);
	LookForJrAndIII(ifnm, &ends_jr, &ends_iii, &ends_iv);

	GetMidInit(ifnm, querv[6]);
	FixCase(ifnm);
	(void) strncpy(querv[5], ifnm, FIRST_LEN);


	/* okay, finish up first name */
	AppendJrOrIII(ifnm, &ends_jr, &ends_iii, &ends_iv);

	/* MIT Year ... */

	if (!strcmp(iyr, "GG")) {
	    (void) strcpy(querv[9], "G\0");
	}
	else {
	    (void) strcpy(querv[9], iyr);
	}

	/* MIT-ID (Social Security Number) ... */

	salt[0] = tolower(*ilnm);
	salt[1] = tolower(*ifnm);
	(void) strcpy(querv[8], crypt(issn + 2, salt));	/* last 7 digits only */
	(void) strcat(querv[8], "\0");
	/* That's all, folks */


	return (lno);

complain_and_continue:
	fprintf(stderr, "\007ERROR on input line no. %d, len %d:\n",
		lno, len);
	fprintf(stderr, "%s\n", ibuf);
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

GetMidInit(nm, mi)
    register char *nm;		/* truncate at first space, if any such */
    register char *mi;		/* set to first char after first space, if
				 * any such */
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

AppendJrOrIII(nm, phas_jr, phas_iii, phas_iv)
    register char *nm;
    register int *phas_jr;
    register int *phas_iii;
    register int *phas_iv;
{
    if (*phas_jr) {
	(void) strcat(nm, ", Jr.");
    }
    else if (*phas_iii) {
	(void) strcat(nm, " III");
    }
    else if (*phas_iv) {
	(void) strcat(nm, " IV");
    }
}

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

LookForJrAndIII(nm, pends_jr, pends_iii, pends_iv)
    register char *nm;
    register int *pends_jr;
    register int *pends_iii;
    register int *pends_iv;
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
