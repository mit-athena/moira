/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.8 1998-01-06 20:40:03 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_nfsparttype_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.8 1998-01-06 20:40:03 danw Exp $";
#endif

#include <mit-copyright.h>
#include <moira.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

extern char *strsave();
extern char *strtrim();

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
  return strsave(buf);
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
  return strsave(temp);
}
