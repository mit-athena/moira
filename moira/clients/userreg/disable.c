/* $Id: disable.c,v 1.11 1998-03-10 21:16:50 danw Exp $
 *
 * disabled: check to see if registration is enabled right now.  Most of this
 * code is stolen from the cron daemon.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "userreg.h"

#define	LISTS	(2 * BUFSIZ)
#define	MAXLIN	BUFSIZ

#define	EXACT	100
#define	ANY	101
#define	LIST	102
#define	RANGE	103
#define	EOS	104

time_t	itime;
struct	tm *loct;
int	flag;
char	*list;
char	*listend;
unsigned listsize;

char *cmp(char *p, int v);
void init(void);
void append(char *fn);
int number(char c, FILE *f);

/* This routine will determine if registration is enabled at this time.  If
 * NULL is returned, registration is OK.  Otherwise, the string returned
 * will indicate the time that registration will be re-enabled.
 */

char *disabled(char **msg)
{
  char *cp;
  int hit;

  *msg = 0;
  init();
  append(DISABLE_FILE);
  *listend++ = EOS;
  *listend++ = EOS;

  time(&itime);
  itime -= localtime(&itime)->tm_sec;
  loct = localtime(&itime);
  loct->tm_mon++;		 /* 1-12 for month */
  if (loct->tm_wday == 0)
    loct->tm_wday = 7;	/* sunday is 7, not 0 */
  hit = 0;
  for (cp = list; *cp != EOS;)
    {
      flag = 0;
      cp = cmp(cp, loct->tm_min);
      cp = cmp(cp, loct->tm_hour);
      cp = cmp(cp, loct->tm_mday);
      cp = cmp(cp, loct->tm_mon);
      cp = cmp(cp, loct->tm_wday);
      if (flag == 0)
	{
	  *msg = cp;
	  hit++;
	  break;
	}
      while (*cp++)
	;
    }
  if (!hit)
    return NULL;
  while (hit)
    {
      itime += 60;		/* add a minute */
      loct = localtime(&itime);
      loct->tm_mon++;		/* 1-12 for month */
      if (loct->tm_wday == 0)
	loct->tm_wday = 7;	/* sunday is 7, not 0 */
      hit = 0;
      for(cp = list; *cp != EOS;)
	{
	  flag = 0;
	  cp = cmp(cp, loct->tm_min);
	  cp = cmp(cp, loct->tm_hour);
	  cp = cmp(cp, loct->tm_mday);
	  cp = cmp(cp, loct->tm_mon);
	  cp = cmp(cp, loct->tm_wday);
	  if (!flag)
	    {
	      hit++;
	      break;
	    }
	  while(*cp++ != 0)
	    ;
	}
    }
  return ctime(&itime);
}

char *cmp(char *p, int v)
{
  char *cp;

  cp = p;
  switch (*cp++)
    {
    case EXACT:
      if (*cp++ != v)
	flag++;
      return cp;

    case ANY:
      return cp;

    case LIST:
      while (*cp != LIST)
	{
	  if (*cp++ == v)
	    {
	      while (*cp++ != LIST)
		;
	      return cp;
	    }
	}
      flag++;
      return cp + 1;

    case RANGE:
      if (*cp > v || cp[1] < v)
	flag++;
      return cp + 2;
    }
  if (cp[-1] != v)
    flag++;
  return cp;
}

void init(void)
{
  /*
   * Don't free in case was longer than LISTS.  Trades off
   * the rare case of crontab shrinking vs. the common case of
   * extra realloc's needed in append() for a large crontab.
   */
  if (list == 0)
    {
      list = malloc(LISTS);
      listsize = LISTS;
    }
  listend = list;
}

void append(char *fn)
{
  int i, c;
  char *cp;
  char *ocp;
  int n;
  FILE *f;

  if (!(f = fopen(fn, "r")))
    return;
  cp = listend;
loop:
  if (cp > list + listsize - MAXLIN)
    {
      int length = cp - list;

      listsize += LISTS;
      list = realloc(list, listsize);
      cp = list + length;
    }
  ocp = cp;
  for (i = 0; ; i++)
    {
      do
	c = getc(f);
      while(c == ' ' || c == '\t');
      if (c == EOF || c == '\n')
	goto ignore;
      if (i == 5)
	break;
      if (c == '*')
	{
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
      do
	{
	  if ((n = number(getc(f), f)) < 0)
	    goto ignore;
	  *cp++ = n;
	  c = getc(f);
	}
      while (c == ',');
      if (c != '\t' && c != ' ')
	goto ignore;
      *cp++ = LIST;
      continue;

    mrange:
      *cp++ = RANGE;
      *cp++ = n;
      if ((n = number(getc(f), f)) < 0)
	goto ignore;
      c = getc(f);
      if (c != '\t' && c != ' ')
	goto ignore;
      *cp++ = n;
    }
  while (c != '\n')
    {
      if (c == EOF)
	goto ignore;
      if (c == '%')
	c = '\n';
      *cp++ = c;
      c = getc(f);
    }
  *cp++ = '\n';
  *cp++ = 0;
  goto loop;

ignore:
  cp = ocp;
  while (c != '\n')
    {
      if (c == EOF)
	{
	  fclose(f);
	  listend = cp;
	  return;
	}
      c = getc(f);
    }
  goto loop;
}

int number(char c, FILE *f)
{
  int n = 0;

  while (isdigit(c))
    {
      n = n * 10 + c - '0';
      c = getc(f);
    }
  ungetc(c, f);
  if (n >= 100)
    return -1;
  return n;
}

