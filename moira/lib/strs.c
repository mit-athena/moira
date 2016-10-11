/* $Id$
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
#include <string.h>

RCSID("$HeadURL$ $Id$");

/*
 * memmove for strings.
 */
char *strmove(char *dest, char *src) {
  return memmove(dest, src, strlen(src)+1);
}

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
      if (*save)
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

#ifndef HAVE_STRLCPY
/* Copy as much of SRC will fit into a DST of size SIZE, always
 * NUL-terminating. (Originally from OpenBSD.)
 */
size_t strlcpy(char *dst, const char *src, size_t size)
{
  size_t len = strlen(src);

  if (len < size)
    memcpy(dst, src, len + 1);
  else
    {
      memcpy(dst, src, size - 1);
      dst[size - 1] = '\0';
    }
  return len;
}
#endif

#ifndef HAVE_STRLCAT
/* Catenate as must of SRC will fit onto the end of DST, which is
 * in a buffer of size SIZE, always NUL-terminating. (Originally
 * from OpenBSD.)
 */
size_t strlcat(char *dst, const char *src, size_t size)
{
  size_t dlen = strlen(dst);
  size_t slen = strlen(src);

  if (dlen + slen < size)
    memcpy(dst + dlen, src, slen + 1);
  else
    {
      memcpy(dst + dlen, src, size - dlen - 1);
      dst[size - 1] = '\0';
    }
  return dlen + slen;
}
#endif
