/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.6 1988-06-30 12:44:22 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef lint
static char *rcsid_qsubs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.6 1988-06-30 12:44:22 mar Exp $";
#endif lint

#include <sms.h>
#include "query.h"

extern struct query Queries1[], Queries2[];
extern int QueryCount1, QueryCount2;

struct query *
get_query_by_name(name, version)
    register char *name;
    int version;
{
    register struct query *q;
    register int i;

    if (version == SMS_VERSION_1) {
	q = Queries1;
	i = QueryCount1;
    } else {
	q = Queries2;
	i = QueryCount2;
    }	

    if (strlen(name) == 4) {
	while (--i >= 0) {
	    if (!strcmp(q->shortname, name)) return(q);
	    q++;
	}
    } else {
	while (--i >= 0) {
	    if (!strcmp(q->name, name)) return(q);
	    q++;
	}
    }

    return((struct query *)0);
}

list_queries(version, action, actarg)
    int version;
    int (*action)();
    int actarg;
{
  register struct query *q;
  register int i;
  static struct query **squeries1 = (struct query **)0;
  static struct query **squeries2 = (struct query **)0;
  register struct query **sq;
  char qnames[80];
  char *qnp;
  int count;
  int qcmp();

  if (version == SMS_VERSION_1) {
      count = QueryCount1;
      if (squeries1 == (struct query **)0) {
	  sq = (struct query **)malloc(count * sizeof (struct query *));
	  squeries1 = sq;
	  q = Queries1;
	  for (i = count; --i >= 0; )
	      *sq++ = q++;
	  qsort(squeries1, count, sizeof (struct query *), qcmp);
      }
      sq = squeries1;
  } else {
      count = QueryCount2;
      if (squeries2 == (struct query **)0) {
	  sq = (struct query **)malloc(count * sizeof (struct query *));
	  squeries2 = sq;
	  q = Queries2;
	  for (i = count; --i >= 0; )
	      *sq++ = q++;
	  qsort(squeries2, count, sizeof (struct query *), qcmp);
      }
      sq = squeries2;
  }

  qnp = qnames;
  for (i = count; --i >= 0; sq++) {
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

help_query(q, action, actarg)
    register struct query *q;
    int (*action)();
    int actarg;
{
    register int argcount;
    register int i;
    char argn[32];
    char qname[512];
    char argr[512];
    char *argv[32];

    argcount = q->argc;
    if (q->type == UPDATE || q->type == APPEND) argcount += q->vcnt;

    switch (argcount) {
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
	for (i = 1; i < argcount; i++) argv[i] = q->fields[i];
	sprintf(argn, "%s)", q->fields[argcount]);
	argv[argcount++] = argn;
	break;
    }
    if (q->type == RETRIEVE) {
	sprintf(argr, "%s => %s", argv[--argcount], q->fields[q->argc]);
	argv[argcount++] = argr;
	if (q->vcnt > 1)
	    for (i = q->argc + 1; i < q->vcnt + q->argc; i++)
		argv[argcount++] = q->fields[i];
    }
    (*action)(argcount, argv, actarg);
}

qcmp(q1, q2)
    struct query **q1;
    struct query **q2;
{
  return(strcmp((*q1)->name, (*q2)->name));
}
