/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/hash.c,v 1.3 1988-08-04 10:48:21 mar Exp $
 *
 * Generic hash table routines.  Uses integer keys to store char * values.
 */

#include <ctype.h>
#include "sms_app.h"
#define NULL 0


/* Create a hash table.  The size is just a hint, not a maximum. */

struct hash *create_hash(size)
int size;
{
    struct hash *h;

    h = (struct hash *) malloc(sizeof(struct hash));
    h->size = size;
    h->data = (struct bucket **) malloc(size * sizeof(char *));
    bzero(h->data, size * sizeof(char *));
    return(h);
}

/* Lookup an object in the hash table.  Returns the value associated with
 * the key, or NULL (thus NULL is not a very good value to store...)
 */

char *hash_lookup(h, key)
struct hash *h;
register int key;
{
    register struct bucket *b;

    b = h->data[key % h->size];
    while (b && b->key != key)
      b = b->next;
    if (b && b->key == key)
      return(b->data);
    else
      return(NULL);
}


/* Update an existing object in the hash table.  Returns 1 if the object
 * existed, or 0 if not.
 */

int hash_update(h, key, value)
struct hash *h;
register int key;
char *value;
{
    register struct bucket *b;

    b = h->data[key % h->size];
    while (b && b->key != key)
      b = b->next;
    if (b && b->key == key) {
	b->data = value;
	return(1);
    } else
      return(0);
}


/* Store an item in the hash table.  Returns 0 if the key was not previously
 * there, or 1 if it was.
 */

int hash_store(h, key, value)
struct hash *h;
register int key;
char *value;
{
    register struct bucket *b, **p;

    p = &(h->data[key % h->size]);
    if (*p == NULL) {
	b = *p = (struct bucket *) malloc(sizeof(struct bucket));
	b->next = NULL;
	b->key = key;
	b->data = value;
	return(0);
    }

    for (b = *p; b && b->key != key; b = *p)
      p = (struct bucket **) *p;
    if (b && b->key == key) {
	b->data = value;
	return(1);
    }
    b = *p = (struct bucket *) malloc(sizeof(struct bucket));
    b->next = NULL;
    b->key = key;
    b->data = value;
    return(0);
}


/* Search through the hash table for a given value.  For each piece of
 * data with that value, call the callback proc with the corresponding key.
 */

hash_search(h, value, callback)
struct hash *h;
register char *value;
void (*callback)();
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    if (b->data == value)
	      (*callback)(b->key);
	}
    }
}


/* Step through the hash table, calling the callback proc with each key.
 */

hash_step(h, callback, hint)
struct hash *h;
void (*callback)();
char *hint;
{
    register struct bucket *b, **p;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b->next) {
	    (*callback)(b->key, b->data, hint);
	}
    }
}


/* Deallocate all of the memory associated with a table */

hash_destroy(h)
struct hash *h;
{
    register struct bucket *b, **p, *b1;

    for (p = &(h->data[h->size - 1]); p >= h->data; p--) {
	for (b = *p; b; b = b1) {
	    b1 = b->next;
	    free(b);
	}
    }
}
