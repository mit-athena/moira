/* $Id: qsubs.c,v 1.14 1998-02-05 22:51:50 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"
#include "query.h"

#include <stdlib.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.14 1998-02-05 22:51:50 danw Exp $");

extern struct query Queries2[];
extern int QueryCount2;

int qcmp(const void *q1, const void *q2);

struct query *get_query_by_name(char *name, int version)
{
  struct query *q;
  int i;

  q = Queries2;
  i = QueryCount2;

  if (strlen(name) == 4)
    {
      while (--i >= 0)
	{
	  if (!strcmp(q->shortname, name))
	    return q;
	  q++;
	}
    }
  else
    {
      while (--i >= 0)
	{
	  if (!strcmp(q->name, name))
	    return q;
	  q++;
	}
    }

  return NULL;
}

void list_queries(int version, int (*action)(int, char *[], void *),
		  void *actarg)
{
  struct query *q;
  int i;
  static struct query **squeries2 = NULL;
  struct query **sq;
  char qnames[80];
  char *qnp;
  int count;

  count = QueryCount2;
  if (!squeries2)
    {
      sq = malloc(count * sizeof(struct query *));
      squeries2 = sq;
      q = Queries2;
      for (i = count; --i >= 0; )
	*sq++ = q++;
      qsort(squeries2, count, sizeof(struct query *), qcmp);
    }
  sq = squeries2;

  qnp = qnames;
  for (i = count; --i >= 0; sq++)
    {
      sprintf(qnames, "%s (%s)", (*sq)->name, (*sq)->shortname);
      (*action)(1, &qnp, actarg);
    }
  strcpy(qnames, "_help");
  (*action)(1, &qnp, actarg);
  strcpy(qnames, "_list_queries");
  (*action)(1, &qnp, actarg);
  strcpy(qnames, "_list_users");
  (*action)(1, &qnp, actarg);
}

void help_query(struct query *q, int (*action)(int, char *[], void *),
		void *actarg)
{
  int argcount;
  int i;
  char argn[32];
  char qname[512];
  char argr[512];
  char *argv[32];

  argcount = q->argc;
  if (q->type == UPDATE || q->type == APPEND)
    argcount += q->vcnt;

  switch (argcount)
    {
    case 0:
      sprintf(qname, "   %s, %s ()", q->name, q->shortname);
      argv[0] = qname;
      argcount = 1;
      break;

    case 1:
      sprintf(qname, "   %s, %s (%s)", q->name, q->shortname, q->fields[0]);
      argv[0] = qname;
      argcount = 1;
      break;

    case 2:
      sprintf(qname, "   %s, %s (%s, %s)", q->name, q->shortname,
	      q->fields[0], q->fields[1]);
      argv[0] = qname;
      argcount = 1;
      break;

    default:
      sprintf(qname, "   %s, %s (%s", q->name, q->shortname, q->fields[0]);
      argv[0] = qname;
      argcount--;
      for (i = 1; i < argcount; i++)
	argv[i] = q->fields[i];
      sprintf(argn, "%s)", q->fields[argcount]);
      argv[argcount++] = argn;
      break;
    }

  if (q->type == RETRIEVE)
    {
      sprintf(argr, "%s => %s", argv[--argcount], q->fields[q->argc]);
      argv[argcount++] = argr;
      if (q->vcnt > 1)
	{
	  for (i = q->argc + 1; i < q->vcnt + q->argc; i++)
	    argv[argcount++] = q->fields[i];
	}
    }
  (*action)(argcount, argv, actarg);
}

int qcmp(const void *q1, const void *q2)
{
  return strcmp((*(struct query **)q1)->name, (*(struct query **)q2)->name);
}
