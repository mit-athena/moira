/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.13 1998-01-05 19:53:16 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Miscellaneous string functions.
 */

#ifndef lint
static char *rcsid_strs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.13 1998-01-05 19:53:16 danw Exp $";
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
char *strsave(char *s)
{
  register int len;
  register char *p;
  /* Kludge for sloppy string semantics */
  if (!s)
    {
      p = malloc(1);
      *p = '\0';
      return p;
    }
  len = strlen(s) + 1;
  p = malloc(len);
  if (p)
    memcpy(p, s, len);
  return p;
}

/*
 * Trim whitespace off both ends of a string.
 */
char *strtrim(register char *save)
{
  register char *t, *s;

  s = save;
  while (isspace(*s))
    s++;
  /* skip to end of string */
  if (*s == '\0')
    {
      *save = '\0';
      return save;
    }

  for (t = s; *t; t++)
    continue;
  while (t > s)
    {
      --t;
      if (!isspace(*t))
	{
	  t++;
	  break;
	}
    }
  if (*t)
    *t = '\0';
  return s;
}


/* Modify a string for all of the letters to be uppercase. */

char *uppercase(char *s)
{
  register char *p;

  for (p = s; *p; p++)
    {
      if (islower(*p))
	*p = toupper(*p);
    }
  return s;
}


char *lowercase(char *s)
{
  register char *p;

  for (p = s; *p; p++)
    {
      if (isupper(*p))
	*p = tolower(*p);
    }
  return s;
}
