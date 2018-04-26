/* $Id$
 *
 * Don't know why this function is not in libkrb.a.  It's the inverse
 * of kname_parse() which is there.
 *
 * Copyright (C) 1993-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <stdio.h>
#include <string.h>

#include <mr_krb.h>

RCSID("$HeadURL$ $Id$");

/* Turn a principal, instance, realm triple into a single non-ambiguous
 * string.  This is the inverse of kname_parse().  It returns a pointer
 * to a static buffer, or NULL on error.
 */

char *mr_kname_unparse(char *p, char *i, char *r)
{
  static char name[MAX_K_NAME_SZ];
  char *s;

  s = name;
  if (!p || strlen(p) > ANAME_SZ)
    return NULL;
  while (*p)
    {
      switch (*p)
	{
	case '@':
	  *s++ = '\\';
	  *s++ = '@';
	  break;
	case '.':
	  *s++ = '\\';
	  *s++ = '.';
	  break;
	case '\\':
	  *s++ = '\\';
	  *s++ = '\\';
	  break;
	default:
	  *s++ = *p;
	}
      p++;
    }
  if (i && *i)
    {
      if (strlen(i) > INST_SZ)
	return NULL;
      *s++ = '.';
      while (*i)
	{
	  switch (*i)
	    {
	    case '@':
	      *s++ = '\\';
	      *s++ = '@';
	      break;
	    case '.':
	      *s++ = '\\';
	      *s++ = '.';
	      break;
	    case '\\':
	      *s++ = '\\';
	      *s++ = '\\';
	      break;
	    default:
	      *s++ = *i;
	    }
	  i++;
	}
    }
  *s++ = '@';
  if (!r || strlen(r) > REALM_SZ)
    return NULL;
  while (*r)
    {
      switch (*r)
	{
	case '@':
	  *s++ = '\\';
	  *s++ = '@';
	  break;
	case '\\':
	  *s++ = '\\';
	  *s++ = '\\';
	  break;
	default:
	  *s++ = *r;
	}
      r++;
    }
  *s = '\0';
  return &name[0];
}
