/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.2 1987-06-08 03:08:15 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_qsubs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsubs.c,v 1.2 1987-06-08 03:08:15 wesommer Exp $";
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

get_input_fields(q, argc, argv)
    register struct query *q;
    int *argc;
    char ***argv;
{
    *argv = q->fields;
    *argc = q->sargc + q->argc;
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
	*argv = &q->fields[q->sargc + q->argc];
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
    fp = &q->fields[q->sargc + q->argc];
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

    n = q->sargc + q->argc;
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
