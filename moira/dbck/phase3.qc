/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase3.qc,v 1.3 1988-09-26 15:21:04 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "dbck.h"

#define NULL 0

static char phase3_qc_rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase3.qc,v 1.3 1988-09-26 15:21:04 mar Exp $";


empty_list_check(id, l, hint)
int id, hint;
struct list *l;
{
    if (l->members == 0 && l->list_id != 0)
      printf("Warning: List %s is empty\n", l->name);
}


/* Used by other parts of the program to check that a string_id is good.
 * This returns TRUE if it is, or FALSE if it is not, and as a side effect
 * increments the string reference count.
 */

int string_check(id)
int id;
{
    register struct string *s;

    s = (struct string *) hash_lookup(strings, id);
    if (s == NULL)
      return(0);
    s->refc++;
    return(1);
}


unref_string_check(id, s, hint)
int id, hint;
struct string *s;
{
    if (s->refc == 0) {
	printf("Unreferenced string %s id %d\n", s->name, id);
	if (single_fix("Delete", 1))
	  single_delete("strings", "string_id", id);
    }
}


noclu_mach_check(id, m, hint)
int id, hint;
struct machine *m;
{
    if (m->clucount == 0 && m->mach_id != 0)
      printf("Warning: machine %s is not in any clusters\n", m->name);
}


phase3()
##{
    printf("Phase 3 - Finding unused objects\n");

    if (warn) {
	dprintf("Checking machines...\n");
	hash_step(machines, noclu_mach_check, NULL);

	dprintf("Checking lists...\n");
	hash_step(lists, empty_list_check, NULL);
    }

    dprintf("Checking strings...\n");
    hash_step(strings, unref_string_check, NULL);

##}

