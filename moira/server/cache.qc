/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v 1.2 1990-01-30 18:05:37 mar Exp $
 *
 *	Copyright (C) 1989 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v 1.2 1990-01-30 18:05:37 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "query.h"
#include "sms_server.h"


extern char *whoami, *strsave();
extern int ingres_errno, sms_errcode;


/*** NOTE **************************************************************
 *
 *    This code depends on each type starting with a unique letter.  If
 *    any new types are added to the system that begin with the same
 *    letter as one of the existing types:
 *		User
 *		List
 *		String
 *		Machine
 *		Cluster
 *		Filesystem
 *    then we will have to rework the code that only looks at the first
 *    letter of the types.
 *
 ***********************************************************************
 */

/* Cache parameters: */
#define CACHESIZE 101		/* number of cache slots */
#define NAMESZ 257		/* max size of a name */

struct item {
    char name[NAMESZ];
    char type[9];
    int nhash;
    int id;
    struct item *next;
    struct item *prev;
};

static struct item cachehead;
static int cachesize;

/* statistics counters */
int cachehits = 0, cachemisses = 0;


/* Name hash function. */

int hashname(name, type)
register char *name;
char *type;
{
    register int val = *type;

    while (*name)
      val = val<<5 - val + *name++ - '`';
    return(val);
}


/* Initialize the cache, flushing any old data, and report the statistics
 * if the cache was previously in use.
 */

flush_cache()
{
    register struct item *i;

    if (cachehits + cachemisses != 0)
      com_err(whoami, 0, "Flushing cache; %d hits, %d misses, %d%% hit rate",
	      cachehits, cachemisses,
	      (100 * cachehits) / (cachehits + cachemisses));
    else
      cachehead.next = cachehead.prev = &cachehead;
    cachehits = cachemisses = cachesize = 0;
    for (i = cachehead.next; i != &cachehead; i = i->next) {
	if (i->prev != &cachehead)
	  free(i->prev);
    }
    if (cachehead.prev != &cachehead)
      free(cachehead.prev);
    cachehead.next = cachehead.prev = &cachehead;
}


/* Do a name to ID translation.  id will be updated with the answer if
 * it is available, and as a side effect the cache is updated.
 */

int name_to_id(name, type, id)
char *name;
char *type;
int *id;
##{
    register struct item *i;
##  char *iname;
##  int j, rowcount;
    int h, ctr;

    h = hashname(name, type);
    for (i = cachehead.next; i != &cachehead; i = i->next) {
	if (i->nhash != h ||
	    strcmp(name, i->name) ||
	    strcasecmp(type, i->type))
	  continue;
	*id = i->id;
	cachehits++;
	i->next->prev = i->prev;
	i->prev->next = i->next;
	i->next = cachehead.next;
	i->prev = &cachehead;
	cachehead.next->prev = i;
	cachehead.next = i;
	return(SMS_SUCCESS);
    }

    cachemisses++;
    iname = name;

    switch (*type) {
    case 'U':
    case 'u':
##	repeat retrieve (j = users.users_id) where users.#login=@iname
	break;
    case 'L':
    case 'l':
##	repeat retrieve (j = list.list_id) where list.#name=@iname
	break;
    case 'M':
    case 'm':
##	repeat retrieve (j = machine.mach_id) where machine.#name=uppercase(@iname)
	break;
    case 'C':
    case 'c':
##	repeat retrieve (j = cluster.clu_id) where cluster.#name=@iname
	break;
    case 'F':
    case 'f':
##	repeat retrieve (j = filesys.filsys_id) where filesys.#label=@iname
	break;
    case 'S':
    case 's':
##	repeat retrieve (j = strings.string_id) where strings.#string=@iname
	break;
    default:
	return(SMS_INTERNAL);
    }
##  inquire_equel(rowcount = "rowcount")
    if (ingres_errno) return(sms_errcode);
    if (rowcount == 0)
      return(SMS_NO_MATCH);
    if (rowcount > 1)
      return(SMS_NOT_UNIQUE);
    *id = j;
    if (name[0] == '#' && !strcasecmp(type, "USER"))
      return(SMS_SUCCESS);
    if (cachesize < CACHESIZE) {
	i = (struct item *) malloc(sizeof(struct item));
	cachesize++;
    } else {
	i = cachehead.prev;
	cachehead.prev = i->prev;
	i->prev->next = &cachehead;
    }
    strcpy(i->name, name);
    strcpy(i->type, type);
    i->nhash = h;
    i->id = j;
    i->next = cachehead.next;
    i->prev = &cachehead;
    cachehead.next->prev = i;
    cachehead.next = i;
    return(SMS_SUCCESS);
##}


/* Perform an ID to name mapping.  name should be a pointer to a pointer to
 * malloc'ed data.  The buffer it refers to will be freed, and a new buffer
 * allocated with the answer.
 */

int id_to_name(id, type, name)
int id;
char *type;
char **name;
##{
    register struct item *i;
##  char iname[NAMESZ];
##  int j, rowcount;
    int ctr;

    for (i = cachehead.next; i != &cachehead; i = i->next) {
	if (i->id != id || strcasecmp(type, i->type)) continue;
	free(*name);
	*name = strsave(i->name);
	cachehits++;
	i->next->prev = i->prev;
	i->prev->next = i->next;
	i->next = cachehead.next;
	i->prev = &cachehead;
	cachehead.next->prev = i;
	cachehead.next = i;
	return(SMS_SUCCESS);
    }

    cachemisses++;
    j = id;

    switch (*type) {
    case 'U':
    case 'u':
##	repeat retrieve (iname = users.login) where users.users_id=@j
	break;
    case 'L':
    case 'l':
##	repeat retrieve (iname = list.name) where list.list_id=@j
	break;
    case 'M':
    case 'm':
##	repeat retrieve (iname = machine.name) where machine.mach_id=@j
	break;
    case 'C':
    case 'c':
##	repeat retrieve (iname = cluster.name) where cluster.clu_id=@j
	break;
    case 'F':
    case 'f':
##	repeat retrieve (iname = filesys.label) where filesys.filsys_id=@j
	break;
    case 'S':
    case 's':
##	repeat retrieve (iname = strings.string) where strings.string_id=@j
	break;
    default:
	return(SMS_INTERNAL);
    }
##  inquire_equel(rowcount = "rowcount")
    if (ingres_errno) return(sms_errcode);
    if (rowcount == 0) {
	free(*name);
	sprintf(iname, "#%d", j);
	*name = strsave(iname);
	return(SMS_NO_MATCH);
    }
    if (rowcount != 1)
      return(SMS_INTERNAL);
    free(*name);
    *name = strsave(strtrim(iname));
    if (**name == '#' && !strcasecmp(type, "USER"))
      return(SMS_SUCCESS);
    if (cachesize < CACHESIZE) {
	i = (struct item *) malloc(sizeof(struct item));
	cachesize++;
    } else {
	i = cachehead.prev;
	cachehead.prev = i->prev;
	i->prev->next = &cachehead;
    }
    strcpy(i->name, *name);
    strcpy(i->type, type);
    i->nhash = hashname(*name, type);
    i->id = id;
    i->next = cachehead.next;
    i->prev = &cachehead;
    cachehead.next->prev = i;
    cachehead.next = i;
    return(SMS_SUCCESS);
##}


/* Explicitly add something to the cache without doing a lookup in the
 * database.
 */

cache_entry(name, type, id)
char *name;
char *type;
int id;
{
    register struct item *i;

    for (i = cachehead.next; i != &cachehead; i = i->next)
      if (i->id == id && !strcmp(i->type, type))
	return(SMS_SUCCESS);
    if (cachesize < CACHESIZE) {
	i = (struct item *) malloc(sizeof(struct item));
	cachesize++;
    } else {
	i = cachehead.prev;
	cachehead.prev = i->prev;
	i->prev->next = &cachehead;
    }
    strcpy(i->name, name);
    strcpy(i->type, type);
    i->nhash = hashname(name, type);
    i->id = id;
    i->next = cachehead.next;
    i->prev = &cachehead;
    cachehead.next->prev = i;
    cachehead.next = i;
    return(SMS_SUCCESS);
}


/* Flush something that may or may not already be in the cache. */

flush_name(name, type)
char *name;
char *type;
{
    int h;
    register struct item *i;

    h = hashname(name, type);

    for (i = cachehead.next; i != &cachehead; i = i->next) {
	if (!strcmp(name, i->name) && !strcasecmp(type, i->type)) {
	    cachesize--;
	    i->next->prev = i->prev;
	    i->prev->next = i->next;
	    free(i);
	    return(SMS_SUCCESS);
	}
    }
}
