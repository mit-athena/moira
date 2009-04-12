/* $Id: sq.c,v 1.14 1999-07-12 16:26:22 danw Exp $
 *
 * Generic Queue Routines
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/lib/sq.c,v 1.14 1999-07-12 16:26:22 danw Exp $");

struct save_queue *sq_create(void)
{
  struct save_queue *sq;

  sq = malloc(sizeof(struct save_queue));
  if (!sq)
    return sq;
  sq->q_next = sq;
  sq->q_prev = sq;
  sq->q_lastget = 0;
  return sq;
}

int sq_save_data(struct save_queue *sq, void *data)
{
  struct save_queue *q;

  q = malloc(sizeof(struct save_queue));
  if (!q)
    return 0;
  q->q_next = sq;
  q->q_prev = sq->q_prev;
  sq->q_prev->q_next = q;
  sq->q_prev = q;
  q->q_data = data;
  return 1;
}

int sq_save_args(int argc, char *argv[], struct save_queue *sq)
{
  char **argv_copy;
  int i, n;

  argv_copy = malloc(argc * sizeof(char *));
  if (!argv_copy)
    return 0;
  for (i = 0; i < argc; i++)
    {
      n = strlen(argv[i]) + 1;
      argv_copy[i] = malloc(n);
      if (!argv_copy[i])
	{
	  for (i--; i >= 0; i--)
	    free(argv_copy[i]);
	  free(argv_copy);
	  return 0;
	}
      memcpy(argv_copy[i], argv[i], n);
    }

  return sq_save_data(sq, argv_copy);
}

int sq_save_unique_data(struct save_queue *sq, void *data)
{
  struct save_queue *q;

  for (q = sq->q_next; q != sq; q = q->q_next)
    {
      if (q->q_data == data)
	return 1;
    }

  return sq_save_data(sq, data);
}

int sq_save_unique_string(struct save_queue *sq, char *data)
{
  struct save_queue *q;

  for (q = sq->q_next; q != sq; q = q->q_next)
    {
      if (!strcmp(q->q_data, data))
	return 1;
    }

  return sq_save_data(sq, data);
}

/* in sq_get_data and sq_remove_data, `data' is actually a pointer to the
   variable to put the data in to. */

int sq_get_data(struct save_queue *sq, void *data)
{
  void **dptr = data;

  if (sq->q_lastget == NULL)
    sq->q_lastget = sq->q_next;
  else
    sq->q_lastget = sq->q_lastget->q_next;

  if (sq->q_lastget == sq)
    return 0;
  *dptr = sq->q_lastget->q_data;
  return 1;
}

int sq_remove_data(struct save_queue *sq, void *data)
{
  void **dptr = data;

  if (sq->q_next != sq)
    {
      *dptr = sq->q_next->q_data;
      sq->q_next = sq->q_next->q_next;
      free(sq->q_next->q_prev);
      sq->q_next->q_prev = sq;
      return 1;
    }
  return 0;
}

void sq_remove_last_data(struct save_queue *sq)
{
  struct save_queue *rem = sq->q_lastget;

  if (rem != NULL)
    {
      rem->q_next->q_prev = sq->q_lastget = rem->q_prev;
      rem->q_prev->q_next = rem->q_next;
      free(rem);
    }
}      

int sq_empty(struct save_queue *sq)
{
  if (sq->q_next == sq)
    return 1;
  else
    return 0;
}

void sq_destroy(struct save_queue *sq)
{
  struct save_queue *q;

  for (q = sq->q_next; q != sq; q = sq->q_next)
    {
      sq->q_next = q->q_next;
      free(q);
    }
  free(sq);
}

