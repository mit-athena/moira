/* $Id $
 *
 * Deal with NFS partition types
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <mit-copyright.h>
#include <moira.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.9 1998-02-05 22:51:31 danw Exp $");

struct pair {
  int type;
  char *name;
};

/*
 * Table of fs type names.
 */

static struct pair fs_names[] = {
  { MR_FS_STUDENT, "Student" },
  { MR_FS_FACULTY, "Faculty" },
  { MR_FS_STAFF, "Staff" },
  { MR_FS_MISC, "Other" },
  { MR_FS_GROUPQUOTA, "GroupQuota" },
  /* Insert new entries before the 0,0 pair */
  { 0, 0 },
};

/*
 * Given a numeric string containing a filesystem status value, return
 * a string indicating what allocation type it is.
 */
char *format_filesys_type(char *fs_status)
{
  char buf[BUFSIZ];
  struct pair *pp;

  int n_names = 0;
  int stat = atoi(fs_status);

  buf[0] = '\0';

  for (pp = fs_names; pp->type; pp++)
    {
      if (stat & pp->type)
	{
	  if (n_names)
	    strcat(buf, ", ");
	  strcat(buf, pp->name);
	  n_names++;
	  stat &= ~pp->type;
	}
    }
  if (stat)
    {
      char buf1[100];

      if (n_names)
	strcat(buf, ", ");
      sprintf(buf1, "Unknown bits 0x%x", stat);
      strcat(buf, buf1);
    }
  if (!n_names)
    strcpy(buf, "none");
  return strdup(buf);
}

/*
 * Given a string describing a filesystem allocation type, return the
 * numeric value.
 */
char *parse_filesys_type(char *fs_type_name)
{
  struct pair *pp;
  char *cp = fs_type_name;
  char temp[BUFSIZ];
  int flags = 0;

  do
    {
      /* Copy next component of type to temp */
      char *t = strchr(cp, ',');
      if (t)
	{
	  memcpy(temp, cp, t - cp);
	  temp[t - cp] = '\0';
	  cp = t + 1; /* one after the comma */
	}
      else
	{
	  strcpy(temp, cp);
	  cp = NULL;
	}

      t = strtrim(temp);	/* nuke leading and trailing whitespace */

      for (pp = fs_names; pp->type; pp++)
	{
	  if (!strcasecmp(pp->name, t))
	    {
	      flags |= pp->type;
	      break;
	    }
	}
    }
  while (cp);
  sprintf(temp, "%d", flags);
  return strdup(temp);
}
