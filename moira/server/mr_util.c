/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.27 1998-01-05 19:53:34 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_mr_util_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/mr_util.c,v 1.27 1998-01-05 19:53:34 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include "mr_server.h"
#include <com_err.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>

extern char *whoami;

char *requote(char *buf, register char *cp, int len)
{
  register int count = 0;
  register unsigned char c;
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
  register int i;
  register char *bp;

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
  if (code)
    fputs(error_message(code), stderr);
  if (fmt)
    _doprnt(fmt, pvar, stderr);
  putc('\n', stderr);
}


/* mr_trim_args: passed an argument vector, it will trim any trailing
 * spaces on the args by writing a null into the string.  If an argument
 * appears to be binary instead of ASCII, it will not be trimmed.
 */

int mr_trim_args(int argc, char **argv)
{
  register char **arg;
  register unsigned char *p, *lastch;

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
    a[i] = strsave(argv[i]);
  return a;
}
