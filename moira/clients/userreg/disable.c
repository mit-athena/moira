/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/disable.c,v 1.7 1998-01-05 14:51:10 danw Exp $
 *
 * disabled: check to see if registration is enabled right now.  Most of this
 * code is stolen from the cron daemon.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef _AUX_SOURCE
#include <time.h>
#endif /* _AUX_SOURCE */
#include <sys/file.h>
#include "files.h"

#define	LISTS	(2*BUFSIZ)
#define	MAXLIN	BUFSIZ

#define	EXACT	100
#define	ANY	101
#define	LIST	102
#define	RANGE	103
#define	EOS	104

time_t	itime, time();
struct	tm *loct;
struct	tm *localtime();
char	*malloc();
char	*realloc();
int	flag;
char	*list;
char	*listend;
unsigned listsize;

char *cmp();
int number(register char c, FILE *f);

/* This routine will determine if registration is enabled at this time.  If
 * NULL is returned, registration is OK.  Otherwise, the string returned
 * will indicate the time that registration will be re-enabled.
 */

char *disabled(msg)
char **msg;
{
    register char *cp;
    int hit;

    *msg = 0;
    init();
    append(DISABLE_FILE);
    *listend++ = EOS;
    *listend++ = EOS;

    (void) time(&itime);
    itime -= localtime(&itime)->tm_sec;
    loct = localtime(&itime);
    loct->tm_mon++;		 /* 1-12 for month */
    if (loct->tm_wday == 0)
      loct->tm_wday = 7;	/* sunday is 7, not 0 */
    hit = 0;
    for(cp = list; *cp != EOS;) {
	flag = 0;
	cp = cmp(cp, loct->tm_min);
	cp = cmp(cp, loct->tm_hour);
	cp = cmp(cp, loct->tm_mday);
	cp = cmp(cp, loct->tm_mon);
	cp = cmp(cp, loct->tm_wday);
	if(flag == 0) {
	    *msg = cp;
	    hit++;
	    break;
	}
	while(*cp++ != 0)
	  ;
    }
    if (!hit)
      return(NULL);
    while (hit) {
	itime += 60;		/* add a minute */
	loct = localtime(&itime);
	loct->tm_mon++;		/* 1-12 for month */
	if (loct->tm_wday == 0)
	  loct->tm_wday = 7;	/* sunday is 7, not 0 */
	hit = 0;
	for(cp = list; *cp != EOS;) {
	    flag = 0;
	    cp = cmp(cp, loct->tm_min);
	    cp = cmp(cp, loct->tm_hour);
	    cp = cmp(cp, loct->tm_mday);
	    cp = cmp(cp, loct->tm_mon);
	    cp = cmp(cp, loct->tm_wday);
	    if(flag == 0) {
		hit++;
		break;
	    }
	    while(*cp++ != 0)
	      ;
	}
    }
    return(ctime(&itime));
}

char *
cmp(p, v)
char *p;
{
	register char *cp;

	cp = p;
	switch(*cp++) {

	case EXACT:
		if (*cp++ != v)
			flag++;
		return(cp);

	case ANY:
		return(cp);

	case LIST:
		while(*cp != LIST)
			if(*cp++ == v) {
				while(*cp++ != LIST)
					;
				return(cp);
			}
		flag++;
		return(cp+1);

	case RANGE:
		if(*cp > v || cp[1] < v)
			flag++;
		return(cp+2);
	}
	if(cp[-1] != v)
		flag++;
	return(cp);
}

init()
{
	/*
	 * Don't free in case was longer than LISTS.  Trades off
	 * the rare case of crontab shrinking vs. the common case of
	 * extra realloc's needed in append() for a large crontab.
	 */
	if (list == 0) {
		list = malloc(LISTS);
		listsize = LISTS;
	}
	listend = list;
}

append(fn)
char *fn;
{
	register int i, c;
	register char *cp;
	register char *ocp;
	register int n;
	FILE *f, *fopen();

	if ((f = fopen(fn, "r")) == (FILE *) NULL)
		return;
	cp = listend;
loop:
	if(cp > list+listsize-MAXLIN) {
		int length = cp - list;

		listsize += LISTS;
		list = realloc(list, listsize);
		cp = list + length;
	}
	ocp = cp;
	for(i=0;; i++) {
		do
			c = getc(f);
		while(c == ' ' || c == '\t')
			;
		if(c == EOF || c == '\n')
			goto ignore;
		if(i == 5)
			break;
		if(c == '*') {
			*cp++ = ANY;
			continue;
		}
		if ((n = number(c, f)) < 0)
			goto ignore;
		c = getc(f);
		if(c == ',')
			goto mlist;
		if(c == '-')
			goto mrange;
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = EXACT;
		*cp++ = n;
		continue;

	mlist:
		*cp++ = LIST;
		*cp++ = n;
		do {
			if ((n = number(getc(f), f)) < 0)
				goto ignore;
			*cp++ = n;
			c = getc(f);
		} while (c==',');
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = LIST;
		continue;

	mrange:
		*cp++ = RANGE;
		*cp++ = n;
		if ((n = number(getc(f), f)) < 0)
			goto ignore;
		c = getc(f);
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = n;
	}
	while(c != '\n') {
		if(c == EOF)
			goto ignore;
		if(c == '%')
			c = '\n';
		*cp++ = c;
		c = getc(f);
	}
	*cp++ = '\n';
	*cp++ = 0;
	goto loop;

ignore:
	cp = ocp;
	while(c != '\n') {
		if(c == EOF) {
			(void) fclose(f);
			listend = cp;
			return;
		}
		c = getc(f);
	}
	goto loop;
}

number(c, f)
register char c;
FILE *f;
{
	register int n = 0;

	while (isdigit(c)) {
		n = n*10 + c - '0';
		c = getc(f);
	}
	(void) ungetc(c, f);
	if (n>=100)
		return(-1);
	return(n);
}

