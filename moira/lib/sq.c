/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/sq.c,v 1.6 1991-03-08 10:33:07 mar Exp $
 *
 * Generic Queue Routines
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

extern char *malloc();

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

    sq_save_data(sq, (char *)argv_copy);
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

sq_remove_data(sq, data)
    register struct save_queue *sq;
    register char **data;
{
    if (sq->q_next != sq) {
	*data = sq->q_next->q_data;
	sq->q_next = sq->q_next->q_next;
	free(sq->q_next->q_prev);
	sq->q_next->q_prev = sq;
	return(1);
    }
    return(0);
}

int sq_empty(sq)
    register struct save_queue *sq;
{
    if (sq->q_next == sq)
      return(1);
    else
      return(0);
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

