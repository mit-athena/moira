/* $Id: strs.c,v 1.16 1998-02-08 19:31:23 danw Exp $
 *
 * Miscellaneous string functions.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <ctype.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/strs.c,v 1.16 1998-02-08 19:31:23 danw Exp $");

/*
 * Trim whitespace off both ends of a string.
 */
char *strtrim(char *save)
{
  char *t, *s;

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
  char *p;

  for (p = s; *p; p++)
    {
      if (islower(*p))
	*p = toupper(*p);
    }
  return s;
}


char *lowercase(char *s)
{
  char *p;

  for (p = s; *p; p++)
    {
      if (isupper(*p))
	*p = tolower(*p);
    }
  return s;
}
