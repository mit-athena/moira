/* $Id: mr_util.c,v 1.31 1998-02-23 19:24:33 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.31 1998-02-23 19:24:33 danw Exp $");

extern char *whoami;

char *requote(char *cp)
{
  int len = 0;
  char *out = xmalloc(4 * strlen(cp) + 3), *op = out;

  *op++ = '"';
  len++;
  while (*cp)
    {
      if (*cp == '\\' || *cp == '"')
	{
	  *op++ = '\\';
	  len++;
	}
      if (isprint(*cp))
	{
	  *op++ = *cp++;
	  len++;
	}
      else
	{
	  sprintf(op, "\\%03o", *cp++);
	  op += 4;
	  len += 4;
	}
    }

  strcpy(op, "\"");
  len += 2;

  out = realloc(out, len); /* shrinking, so can't fail */
  return out;
}

void log_args(char *tag, int version, int argc, char **argv)
{
  char *buf, **qargv;
  int i, len;
  char *bp;

  qargv = xmalloc(argc * sizeof(char *));

  for (i = len = 0; i < argc; i++)
    {
      qargv[i] = requote(argv[i]);
      len += strlen(qargv[i]) + 2;
    }

  buf = xmalloc(len + 1);

  for (i = 0, *buf = '\0'; i < argc; i++)
    {
      if (i)
	strcat(buf, ", ");
      strcat(buf, qargv[i]);
      free(qargv[i]);
    }
  free(qargv);

  com_err(whoami, 0, "%s[%d]: %s", tag, version, buf);
  free(buf);
}

void mr_com_err(const char *whoami, long code, const char *fmt, va_list pvar)
{
  extern client *cur_client;

  if (whoami)
    {
      fputs(whoami, stderr);
      if (cur_client)
	fprintf(stderr, "[#%d]", cur_client->id);
      fputs(": ", stderr);
    }
  if (code) {
    fputs(error_message(code), stderr);
    fputs(" ", stderr);
  }
  if (fmt)
    vfprintf(stderr, fmt, pvar);
  putc('\n', stderr);
}


/* mr_trim_args: passed an argument vector, it will trim any trailing
 * spaces on the args by writing a null into the string.  If an argument
 * appears to be binary instead of ASCII, it will not be trimmed.
 */

int mr_trim_args(int argc, char **argv)
{
  char **arg;
  unsigned char *p, *lastch;

  for (arg = argv; argc--; arg++)
    {
      for (lastch = p = (unsigned char *) *arg; *p; p++)
	{
	  /* If any byte in the string has the high bit set, assume
	   * that it is binary and we do not want to trim it.
	   * Setting p = lastch will cause us not to trim the string
	   * when we break out of this inner loop.
	   */
	  if (*p >= 0x80)
	    {
	      p = lastch;
	      break;
	    }
	  if (!isspace(*p))
	    lastch = p;
	}
      if (p != lastch)
	{
	  if (isspace(*lastch))
	    *lastch = '\0';
	  else
	    if (*(++lastch))
	      *lastch = '\0';
	}
    }
  return 0;
}


/* returns a copy of the argv and all of it's strings */

char **mr_copy_args(char **argv, int argc)
{
  char **a;
  int i;

  a = xmalloc(argc * sizeof(char *));
  for (i = 0; i < argc; i++)
    a[i] = xstrdup(argv[i]);
  return a;
}


/* malloc or die! */
void *xmalloc(size_t bytes)
{
  void *buf = malloc(bytes);

  if (buf)
    return buf;

  critical_alert("moirad", "Out of memory");
  exit(1);
}

void *xrealloc(void *ptr, size_t bytes)
{
  void *buf = realloc(ptr, bytes);

  if (buf)
    return buf;

  critical_alert("moirad", "Out of memory");
  exit(1);
}

char *xstrdup(char *str)
{
  char *buf = strdup(str);

  if (buf)
    return buf;

  critical_alert("moirad", "Out of memory");
  exit(1);
}
