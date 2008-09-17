/* $Id$
 *
 * Utility routines used by the MOIRA extraction programs.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <stdio.h>
#include <unistd.h>

#include "util.h"

RCSID("$Header$");

/* Julian day of the UNIX epoch (January 1, 1970) */
#define UNIX_EPOCH 2440588

extern void sqlglm(char buf[], int *, int *);

void fix_file(char *targetfile)
{
  char oldfile[64], filename[64];

  sprintf(oldfile, "%s.old", targetfile);
  sprintf(filename, "%s~", targetfile);
  if (rename(targetfile, oldfile) == 0)
    {
      if (rename(filename, targetfile) < 0)
	{
	  rename(oldfile, targetfile);
	  perror("Unable to install new file (rename failed)\n");
	  fprintf(stderr, "Filename = %s\n", targetfile);
	  exit(MR_CCONFIG);
	}
    }
  else
    {
      if (rename(filename, targetfile) < 0)
	{
	  perror("Unable to rename old file\n");
	  fprintf(stderr, "Filename = %s\n", targetfile);
	  exit(MR_CCONFIG);
	}
    }
  unlink(oldfile);
}


char *dequote(char *s)
{
  char *last = s;

  while (*s)
    {
      if (*s == '"')
	*s = '\'';
      else if (*s != ' ')
	last = s;
      s++;
    }
  if (*last == ' ')
    *last = '\0';
  else
    *(++last) = '\0';
  return s;
}

time_t unixtime(char *timestring)
{
  time_t t;

  t = strtol(timestring, &timestring, 10) - UNIX_EPOCH;
  t = t * 24 + strtol(timestring, &timestring, 10);
  t = t * 60 + strtol(timestring, &timestring, 10);
  t = t * 60 + strtol(timestring, &timestring, 10);

  return t;
}

void db_error(int code)
{
  extern char *whoami;
  char buf[256];
  int bufsize = 256, len = 0;

  if (code == -1013)
    {
      com_err(whoami, 0, "build cancelled by user");
      exit(MR_ABORT);
    }

  com_err(whoami, MR_DBMS_ERR, " code %d\n", code);
  sqlglm(buf, &bufsize, &len);
  buf[len] = 0;
  com_err(whoami, 0, "SQL error text = %s", buf);
  critical_alert("DCM", "%s build encountered DATABASE ERROR %d\n%s",
		 whoami, code, buf);
  exit(MR_DBMS_ERR);
}
