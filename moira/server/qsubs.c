/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.5 1988-02-16 16:24:24 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 * Revision 1.5  88/02/16  15:12:58  mar
 * sq_save_unique_data and sq_save_unique_string didn't used
 * to cdr down the queue correctly.
 * 
 * Revision 1.4  87/08/29  00:04:14  mike
 * added sq_save_unique_string
 * 
 * Revision 1.3  87/08/22  17:44:39  wesommer
 * Cleaning up after mike again.
 * 
 * Revision 1.2  87/06/08  03:08:15  wesommer
 * Reindented; added header.
 * 
 */

#ifndef lint
static char *rcsid_qsubs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.5 1988-02-16 16:24:24 mar Exp $";
#endif lint

#include "query.h"

extern struct query Queries[];
extern int QueryCount;
#ifdef notdef
extern struct s_query S_Queries[];
extern int S_QueryCount;
#endif notdef

struct query *
get_query_by_name(name)
    register char *name;
{
    register struct query *q;
    register int i;

    q = Queries;
    i = QueryCount;

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

list_queries(action, actarg)
    int (*action)();
    int actarg;
{
  register struct query *q;
  register int i;
  static struct query **squeries = (struct query **)0;
  register struct query **sq;
  char qnames[80];
  char *qnp;
  int qcmp();

  if (squeries == (struct query **)0)
    {
      sq = (struct query **)malloc(QueryCount * sizeof (struct query *));
      squeries = sq;
      q = Queries;
      for (i = QueryCount; --i >= 0; )
	*sq++ = q++;
      qsort(squeries, QueryCount, sizeof (struct query *), qcmp);
    }

  q = Queries;
  sq = squeries;

  qnp = qnames;
  for (i = QueryCount; --i >= 0; sq++) {
      sprintf(qnames, "%s (%s)", (*sq)->name, (*sq)->shortname);
      (*action)(1, &qnp, actarg);
  }
}

help_query(q, action, actarg)
    register struct query *q;
    int (*action)();
    int actarg;
{
    register int argcount;
    register int i;
    char argn[32];
    char qname[80];
    char *argv[32];

    argcount = q->argc;
    if (q->type == UPDATE || q->type == APPEND) argcount += q->vcnt;

    switch (argcount) {
    case 0:
	sprintf(qname, "      %s ()", q->shortname);
	argv[0] = qname;
	(*action)(1, argv, actarg);
	break;

    case 1:
	sprintf(qname, "      %s (%s)", q->shortname, q->fields[0]);
	argv[0] = qname;
	(*action)(1, argv, actarg);
	break;

    case 2:
	sprintf(qname, "      %s (%s, %s)", q->shortname, q->fields[0],
		q->fields[1]);
	argv[0] = qname;
	(*action)(1, argv, actarg);
	break;

    default:
	sprintf(qname, "      %s (%s", q->shortname, q->fields[0]);
	argv[0] = qname;
	argcount--;
	for (i = 1; i < argcount; i++) argv[i] = q->fields[i];
	sprintf(argn, "%s)", q->fields[argcount]);
	argv[argcount] = argn;
	(*action)(argcount+1, argv, actarg);
	break;
    }
}

qcmp(q1, q2)
    struct query **q1;
    struct query **q2;
{
  return(strcmp((*q1)->name, (*q2)->name));
}

get_input_fields(q, argc, argv)
    register struct query *q;
    int *argc;
    char ***argv;
{
    *argv = q->fields;
    *argc = q->argc;
    if (q->type == UPDATE || q->type == APPEND)
	*argc += q->vcnt;
}

get_output_fields(q, argc, argv)
    register struct query *q;
    int *argc;
    char ***argv;
{
    if (q->type == RETRIEVE) {
	*argc = q->vcnt;
	*argv = &q->fields[q->argc];
    } else {
	*argc = 0;
	*argv = (char **)0;
    }
}

char *
get_field(q, argv, name)
    register struct query *q;
    char *argv[];
    char *name;
{
    register char **fp;
    register char *field;
    register int i;

    if (q->type != RETRIEVE) return((char *)0);

    if (*name == '*') name++;
    fp = &q->fields[q->argc];
    for (i = 0; i < q->vcnt; i++)     {
	field = *fp++;
	if (*field == '*') field++;
	if (!strcmp(field, name)) return(argv[i]);
    }
    return((char *)0);
}

put_field(q, argv, name, value)
    register struct query *q;
    char *argv[];
    char *name;
    char *value;
{
    register char **fp;
    register char *field;
    register int i;
    register int n;

    n = q->argc;
    if (q->type == UPDATE || q->type == APPEND) n += q->vcnt;

    if (*name == '*') name++;
    fp = q->fields;
    for (i = 0; i < n; i++) {
	field = *fp++;
	if (*field == '*') field++;
	if (!strcmp(field, name)) {
	    strcpy(argv[i], value);
	    return(0);
	}
    }
    return(-1);
}


/* Generic Queue Routines */

struct save_queue *
sq_create()
{
    register struct save_queue *sq;

    sq = (struct save_queue *)malloc(sizeof (struct save_queue));
    sq->q_next = sq;
    sq->q_prev = sq;
    sq->q_lastget = 0;
    return(sq);
}

sq_save_data(sq, data)
    register struct save_queue *sq;
    char *data;
{
    register struct save_queue *q;

    q = (struct save_queue *)malloc(sizeof (struct save_queue));
    q->q_next = sq;
    q->q_prev = sq->q_prev;
    sq->q_prev->q_next = q;
    sq->q_prev = q;
    q->q_data = data;
}

sq_save_args(argc, argv, sq)
    register struct save_queue *sq;
    register int argc;
    register char *argv[];
{
    register char **argv_copy;
    register int i;
    register int n;

    argv_copy = (char **)malloc(argc * sizeof (char *));
    for (i = 0; i < argc; i++) {
	n = strlen(argv[i]) + 1;
	argv_copy[i] = (char *)malloc(n);
	bcopy(argv[i], argv_copy[i], n);
    }

    sq_save_data(sq, argv_copy);
}

sq_save_unique_data(sq, data)
    register struct save_queue *sq;
    char *data;
{
    register struct save_queue *q;

    for (q = sq->q_next; q != sq; q = q->q_next)
	if (q->q_data == data) return;

    sq_save_data(sq, data);
}

sq_save_unique_string(sq, data)
    register struct save_queue *sq;
    char *data;
{
    register struct save_queue *q;

    for (q = sq->q_next; q != sq; q = q->q_next)
	if (!strcmp(q->q_data, data)) return;

    sq_save_data(sq, data);
}

sq_get_data(sq, data)
    register struct save_queue *sq;
    register char **data;
{
    if (sq->q_lastget == (struct save_queue *)0) {
	sq->q_lastget = sq->q_next;
    } else {
	sq->q_lastget = sq->q_lastget->q_next;
    }

    if (sq->q_lastget == sq) return(0);
    *data = sq->q_lastget->q_data;
    return(1);
}

sq_destroy(sq)
    register struct save_queue *sq;
{
    register struct save_queue *q;

    for (q = sq->q_next; q != sq; q = sq->q_next) {
	sq->q_next = q->q_next;
	free(q);			
    }
    free(sq);
}


/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
