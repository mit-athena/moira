/* $Id: mr_util.c,v 1.30 1998-02-15 17:49:16 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.30 1998-02-15 17:49:16 danw Exp $");

extern char *whoami;

char *requote(char *buf, char *cp, int len)
{
  int count = 0;
  unsigned char c;
  if (len <= 2)
    return buf;
  *buf++ = '"';
  count++;
  len--;
  for (; (count < 258) && (len > 1) && (c = *cp); cp++, --len, ++count)
    {
      if (c == '\\' || c == '"')
	*buf++ = '\\';
      if (isprint(c))
	*buf++ = c;
      else
	{
	  sprintf(buf, "\\%03o", c);
	  buf = strchr(buf, '\0');
	}
    }
  if (len > 1)
    {
      *buf++ = '"';
      count++;
      len--;
    }
  if (len > 1)
    *buf = '\0';
  return buf;
}

void log_args(char *tag, int version, int argc, char **argv)
{
  char buf[BUFSIZ];
  int i;
  char *bp;

  i = strlen(tag);
  sprintf(buf, "%s[%d]: ", tag, version);
  for (bp = buf; *bp; bp++)
    ;

  for (i = 0; i < argc && ((buf - bp) + BUFSIZ) > 2; i++)
    {
      if (i != 0)
	{
	  *bp++ = ',';
	  *bp++ = ' ';
	}
      bp = requote(bp, argv[i], (buf - bp) + BUFSIZ);
    }
  *bp = '\0';
  com_err(whoami, 0, "%s", buf);
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

  a = malloc(argc * sizeof(char *));
  if (!a)
    return a;
  for (i = 0; i < argc; i++)
    a[i] = strdup(argv[i]);
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
