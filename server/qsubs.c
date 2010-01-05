/* $Id: qsubs.c,v 1.21 2009-12-29 17:29:33 zacheiss Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "mr_server.h"
#include "query.h"

#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.21 2009-12-29 17:29:33 zacheiss Exp $");

extern struct query Queries[];
extern int QueryCount;

int qcmp(const void *q1, const void *q2);

struct query *get_query_by_name(char *name, int version)
{
  int i;

  i = QueryCount;

  if (strlen(name) == 4)
    {
      while (--i >= 0)
	{
	  if (!strcmp(Queries[i].shortname, name) &&
	      Queries[i].version <= version)
	    return &Queries[i];
	}
    }
  else
    {
      while (--i >= 0)
	{
	  if (!strcmp(Queries[i].name, name) &&
	      Queries[i].version <= version)
	    return &Queries[i];
	}
    }

  return NULL;
}

void list_queries(client *cl, int (*action)(int, char *[], void *),
		  void *actarg)
{
  struct query **squeries = NULL;
  int qcount;
  struct query *q, **sq;
  char qnames[80];
  char *qnp;
  int i;

  squeries = sq = xmalloc(QueryCount * sizeof(struct query *));
  q = Queries;
  for (i = 0; i < QueryCount; i++)
    {
      if (q->version > cl->version)
	{
	  q++;
	  continue;
	}
      if (i > 0 && strcmp((*sq)->name, q->name))
	sq++;
      *sq = q++;
    }
  qcount = (sq - squeries) + 1;
  qsort(squeries, qcount, sizeof(struct query *), qcmp);

  sq = squeries;

  qnp = qnames;
  for (i = qcount; --i >= 0; sq++)
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

  free(squeries);
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
  if (q->type == MR_Q_UPDATE || q->type == MR_Q_APPEND)
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

  if (q->type == MR_Q_RETRIEVE)
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
