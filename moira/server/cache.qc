/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v 1.1 1989-12-28 17:02:43 mar Exp $
 *
 *	Copyright (C) 1989 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/cache.qc,v 1.1 1989-12-28 17:02:43 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "query.h"
#include "sms_server.h"


extern char *whoami, *strsave();
extern int ingres_errno, sms_errcode;


/*** NOTE **************************************************************
 *
 *	This code depends each type starting with a unique letter.  If
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
    int id;
    int use;
};

static struct item  cache[CACHESIZE];
static struct item *ncache[CACHESIZE];
static struct item *icache[CACHESIZE];

/* use counter for implementing LRU */
static int use = 1;
/* statistics counters */
int cachehits = 0, cachemisses = 0;

/* ID hash function */
#define hashid(id, type) (((id) + *(type)) % CACHESIZE)


/* Name hash function. */

int hashname(name, type)
register char *name;
char *type;
{
    register int val = *type;

    while (*name)
      val = val<<5 - val + *name++ - '`';
    return(val % CACHESIZE);
}


/* Initialize the cache, flushing any old data, and report the statistics
 * if the cache was previously in use.
 */

flush_cache()
{
    int i;

    if (cachehits + cachemisses != 0)
      com_err(whoami, 0, "Flushing cache; %d hits, %d misses, %d%% hit rate",
	      cachehits, cachemisses,
	      (100 * cachehits) / (cachehits + cachemisses));
    cachehits = cachemisses = 0;
    for (i = 0; i < CACHESIZE; i++) {
	cache[i].use = 0;
	ncache[i] = NULL;
	icache[i] = NULL;
    }
}


/* Do a name to ID translation.  id will be updated with the answer if
 * it is available, and as a side effect the cache is updated.
 */

int name_to_id(name, type, id)
char *name;
char *type;
int *id;
##{
    struct item *i, *mini;
##  char *iname;
##  int j, rowcount;
    int ctr, minv;

    i = ncache[hashname(name, type)];
    if (i && !strcmp(name, i->name) && !strcasecmp(type, i->type)) {
	*id = i->id;
	i->use = use++;
	cachehits++;
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
    minv = use;
    for (i = &cache[0]; i < &cache[CACHESIZE]; i++) {
	if (i->use < minv) {
	    minv = i->use;
	    mini = i;
	    if (minv == 0)
	      break;
	}
    }
    mini->id = j;
    mini->use = use++;
    strcpy(mini->name, name);
    strcpy(mini->type, type);
    ncache[hashname(name, type)] = icache[hashid(j, type)] = mini;
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
    struct item *i, *mini;
##  char iname[NAMESZ];
##  int j, rowcount;
    int ctr, minv;

    i = icache[hashid(id, type)];
    if (i && id == i->id && !strcasecmp(type, i->type)) {
	free(*name);
	*name = strsave(i->name);
	i->use = use++;
	cachehits++;
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
    minv = use;
    for (i = &cache[0]; i < &cache[CACHESIZE]; i++) {
	if (i->use < minv) {
	    minv = i->use;
	    mini = i;
	    if (minv == 0)
	      break;
	}
    }
    mini->id = id;
    mini->use = use++;
    strcpy(mini->name, *name);
    strcpy(mini->type, type);
    ncache[hashname(name, type)] = icache[hashid(id, type)] = mini;
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
    int minv;
    struct item *i, *mini;

    i = icache[hashid(id, type)];
    if (i->id == id &&
	!strcasecmp(i->type, type) &&
	!strcmp(i->name, name))
      return(SMS_SUCCESS);
    minv = use;
    if (i)
      mini = i;
    else
      for (i = &cache[0]; i < &cache[CACHESIZE]; i++) {
	  if (i->use < minv) {
	      minv = i->use;
	      mini = i;
	      if (minv == 0)
		break;
	  }
      }
    mini->id = id;
    mini->use = use++;
    strcpy(mini->name, name);
    strcpy(mini->type, type);
    ncache[hashname(name, type)] = icache[hashid(id, type)] = mini;
}


/* Flush something that may or may not already be in the cache. */

flush_name(name, type)
char *name;
char *type;
{
    struct item *i;

    i = ncache[hashname(name, type)];
    if (i && !strcmp(name, i->name) && !strcasecmp(type, i->type)) {
	ncache[hashname(name, type)] = NULL;
	icache[hashid(i->id, type)] = NULL;
	i->use = 0;
	return(SMS_SUCCESS);
    }
}
