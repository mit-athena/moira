/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/hash.c,v 1.13 1998-01-06 20:39:58 danw Exp $
 *
 * Generic hash table routines.  Uses integer keys to store char * values.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <ctype.h>
#include <moira.h>
#include <string.h>
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif
#define hash_func(h, key) (key >= 0 ? (key % h->size) : (-key % h->size))

/* Create a hash table.  The size is just a hint, not a maximum. */

struct hash *create_hash(int size)
{
  struct hash *h;

  h = malloc(sizeof(struct hash));
  if (!h)
    return NULL;
  h->size = size;
  h->data = malloc(size * sizeof(char *));
  if (!h->data)
    {
      free(h);
      return NULL;
    }
  memset(h->data, 0, size * sizeof(char *));
  return h;
}

/* Lookup an object in the hash table.  Returns the value associated with
 * the key, or NULL (thus NULL is not a very good value to store...)
 */

char *hash_lookup(struct hash *h, int key)
{
  struct bucket *b;

  b = h->data[hash_func(h, key)];
  while (b && b->key != key)
    b = b->next;
  if (b && b->key == key)
    return b->data;
  else
    return NULL;
}


/* Update an existing object in the hash table.  Returns 1 if the object
 * existed, or 0 if not.
 */

int hash_update(struct hash *h, int key, char *value)
{
  struct bucket *b;

  b = h->data[hash_func(h, key)];
  while (b && b->key != key)
    b = b->next;
  if (b && b->key == key)
    {
      b->data = value;
      return 1;
    }
  else
    return 0;
}


/* Store an item in the hash table.  Returns 0 if the key was not previously
 * there, 1 if it was, or -1 if we ran out of memory.
 */

int hash_store(struct hash *h, int key, char *value)
{
  struct bucket *b, **p;

  p = &(h->data[hash_func(h, key)]);
  if (!*p)
    {
      b = *p = malloc(sizeof(struct bucket));
      if (!b)
	return -1;
      b->next = NULL;
      b->key = key;
      b->data = value;
      return 0;
    }

  for (b = *p; b && b->key != key; b = *p)
    p = (struct bucket **) *p;
  if (b && b->key == key)
    {
      b->data = value;
      return 1;
    }
  b = *p = malloc(sizeof(struct bucket));
  if (!b)
    return -1;
  b->next = NULL;
  b->key = key;
  b->data = value;
  return 0;
}


/* Search through the hash table for a given value.  For each piece of
 * data with that value, call the callback proc with the corresponding key.
 */

hash_search(struct hash *h, char *value, void (*callback)())
{
  struct bucket *b, **p;

  for (p = &(h->data[h->size - 1]); p >= h->data; p--)
    {
      for (b = *p; b; b = b->next)
	{
	  if (b->data == value)
	    (*callback)(b->key);
	}
    }
}


/* Step through the hash table, calling the callback proc with each key.
 */

hash_step(struct hash *h, void (*callback)(), char *hint)
{
  struct bucket *b, **p;

  for (p = &(h->data[h->size - 1]); p >= h->data; p--)
    {
      for (b = *p; b; b = b->next)
	(*callback)(b->key, b->data, hint);
    }
}


/* Deallocate all of the memory associated with a table */

hash_destroy(struct hash *h)
{
  struct bucket *b, **p, *b1;

  for (p = &(h->data[h->size - 1]); p >= h->data; p--)
    {
      for (b = *p; b; b = b1)
	{
	  b1 = b->next;
	  free(b);
	}
    }
}
