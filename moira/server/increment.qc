/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/increment.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/increment.qc,v 1.12 1990-06-18 12:58:30 mar Exp $
 *
 *	Copyright (C) 1989 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 * 
 */

#ifndef lint
static char *rcsid_qrtn_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/increment.qc,v 1.12 1990-06-18 12:58:30 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <moira.h>
#include "query.h"
#include "mr_server.h"

extern char *whoami;
char *malloc();

int inc_pid = 0;
int inc_running = 0;
time_t inc_started;

#define MAXARGC 15

/* structures to save before args */
static char beforeb[MAXARGC][ARGLEN];
static char *before[MAXARGC];
##char *barg0, *barg1, *barg2, *barg3, *barg4;
##char *barg5, *barg6, *barg7, *barg8, *barg9;
##char *barg10, *barg11, *barg12, *barg13, *barg14;
static int beforec;
static char *beforetable;

/* structures to save after args */
static char afterb[MAXARGC][ARGLEN];
static char *after[MAXARGC];
##char *aarg0, *aarg1, *aarg2, *aarg3, *aarg4;
##char *aarg5, *aarg6, *aarg7, *aarg8, *aarg9;
##char *aarg10, *aarg11, *aarg12, *aarg13, *aarg14;
static int afterc;

/* structures to save entire sets of incremental changes */
struct save_queue *incremental_sq = NULL;
struct save_queue *incremental_exec = NULL;
struct iupdate {
    char *table;
    int beforec;
    char **before;
    int afterc;
    char **after;
    char *service;
};


incremental_init()
{
    int i;

    for (i = 0; i < MAXARGC; i++) {
	before[i] = &beforeb[i][0];
	after[i] = &afterb[i][0];
    }
    barg0 = before[0];
    barg1 = before[1];
    barg2 = before[2];
    barg3 = before[3];
    barg4 = before[4];
    barg5 = before[5];
    barg6 = before[6];
    barg7 = before[7];
    barg8 = before[8];
    barg9 = before[9];
    barg10 = before[10];
    barg11 = before[11];
    barg12 = before[12];
    barg13 = before[13];
    barg14 = before[14];
    aarg0 = after[0];
    aarg1 = after[1];
    aarg2 = after[2];
    aarg3 = after[3];
    aarg4 = after[4];
    aarg5 = after[5];
    aarg6 = after[6];
    aarg7 = after[7];
    aarg8 = after[8];
    aarg9 = after[9];
    aarg10 = after[10];
    aarg11 = after[11];
    aarg12 = after[12];
    aarg13 = after[13];
    aarg14 = after[14];
    if (incremental_sq == NULL)
      incremental_sq = sq_create();
    if (incremental_exec == NULL)
      incremental_exec = sq_create();
}


##incremental_before(table, qual, argv)
##char *table;
##char *qual;
char **argv;
##{
##  int id;
    char buffer[512], *name;

    beforetable = table;

    if (!strcmp(table, "users")) {
##	retrieve (barg0 = u.login, barg1 = text(u.uid),
##		  barg2 = u.shell, barg3 = u.last, barg4 = u.first,
##		  barg5 = u.middle, barg6 = text(u.status),
##		  barg7 = u.mit_id, barg8 = u.mit_year)
##		where qual
	beforec = 9;
    } else if (!strcmp(table, "machine")) {
##	retrieve (barg0 = m.name, barg1 = m.type) where qual
	beforec = 2;
    } else if (!strcmp(table, "cluster")) {
##	retrieve (barg0 = c.name, barg1 = c.desc, barg2 = c.location)
##	  	where qual
	beforec = 3;
    } else if (!strcmp(table, "mcmap")) {
	strcpy(barg0, argv[0]);
	strcpy(barg1, argv[1]);
	beforec = 2;
    } else if (!strcmp(table, "svc")) {
	strcpy(barg0, argv[0]);
	strcpy(barg1, argv[1]);
	strcpy(barg2, argv[2]);
	beforec = 3;
    } else if (!strcmp(table, "filesys")) {
##	range of fs is filesys
##	retrieve (barg0 = fs.label, barg1 = fs.type, barg2 = text(fs.mach_id),
##		  barg3 = fs.name, barg4 = fs.mount, barg5 = fs.access,
##		  barg6 = fs.comments, barg7 = text(fs.owner),
##		  barg8 = text(fs.owners), barg9 = text(fs.createflg),
##		  barg10 = fs.lockertype)
##	  where qual
	beforec = 11;
    } else if (!strcmp(table, "quota")) {
	strcpy(barg0, "?");
	strcpy(barg1, argv[1]);
	strcpy(barg2, "?");
	sprintf(buffer, "%s and filesys.filsys_id = q.filsys_id", qual);
	qual = buffer;
##	range of q is quota
##	retrieve (barg3 = text(q.quota), barg4 = filesys.name) where qual
	beforec = 5;
    } else if (!strcmp(table, "list")) {
##	retrieve (barg0 = l.name, barg1 = text(l.active),
##		  barg2 = text(l.public), barg3 = text(l.hidden),
##		  barg4 = text(l.maillist), barg5 = text(l.group),
##		  barg6 = text(l.gid), barg7 = l.acl_type,
##		  barg8 = text(l.acl_id), barg9 = l.desc)
##	  where qual
	beforec = 10;
    } else if (!strcmp(table, "members")) {
	id = (int) argv[0];
##	retrieve (barg3 = text(list.group)) where list.list_id = id
	name = malloc(0);
	id_to_name(id, "LIST", &name);
	strcpy(barg0, name);
	strcpy(barg1, argv[1]);
	id = (int) argv[2];
	if (!strcmp(barg1, "USER")) {
	    id_to_name(id, barg1, &name);
	} else if (!strcmp(barg1, "LIST")) {
	    id_to_name(id, barg1, &name);
	} else if (!strcmp(barg1, "STRING")) {
	    id_to_name(id, barg1, &name);
	} else if (!strcmp(barg1, "KERBEROS")) {
	    id_to_name(id, "STRING", &name);
	}
	strcpy(barg2, name);
	free(name);
	beforec = 4;
    } /* else
      com_err(whoami, 0, "unknown table in incremental_before"); */
##}


incremental_clear_before()
{
    beforec = 0;
}

incremental_clear_after()
{
    incremental_after("clear", 0);
}



##incremental_after(table, qual, argv)
##char *table;
##char *qual;
char **argv;
##{
    char buffer[2048], *name;
##  int id, i;
    struct iupdate *iu;
    char **copy_argv();

    if (!strcmp(table, "users")) {
##	retrieve (aarg0 = u.login, aarg1 = text(u.uid),
##		  aarg2 = u.shell, aarg3 = u.last, aarg4 = u.first,
##		  aarg5 = u.middle, aarg6 = text(u.status),
##		  aarg7 = u.mit_id, aarg8 = u.mit_year)
##		where qual
	afterc = 9;
    } else if (!strcmp(table, "machine")) {
##	retrieve (aarg0 = m.name, aarg1 = m.type) where qual
	afterc = 2;
    } else if (!strcmp(table, "cluster")) {
##	retrieve (aarg0 = c.name, aarg1 = c.desc, aarg2 = c.location)
##	  	where qual
	afterc = 3;
    } else if (!strcmp(table, "mcmap")) {
	strcpy(aarg0, argv[0]);
	strcpy(aarg1, argv[1]);
	afterc = 2;
    } else if (!strcmp(table, "svc")) {
	strcpy(aarg0, argv[0]);
	strcpy(aarg1, argv[1]);
	strcpy(aarg2, argv[2]);
	afterc = 3;
    } else if (!strcmp(table, "filesys")) {
##	range of fs is filesys
##	retrieve (aarg0 = fs.label, aarg1 = fs.type,
##		  aarg2 = text(fs.mach_id),
##		  aarg3 = fs.name, aarg4 = fs.mount, aarg5 = fs.access,
##		  aarg6 = fs.comments, aarg7 = text(fs.owner),
##		  aarg8 = text(fs.owners), aarg9 = text(fs.createflg),
##		  aarg10 = fs.lockertype)
##	  where qual
	afterc = 11;
    } else if (!strcmp(table, "quota")) {
	strcpy(aarg0, "?");
	strcpy(aarg1, argv[1]);
	strcpy(aarg2, "?");
	sprintf(buffer, "%s and filesys.filsys_id = q.filsys_id", qual);
	qual = buffer;
##	range of q is quota
##	retrieve (aarg3 = text(q.quota), aarg4 = filesys.name) where qual
	afterc = 5;
    } else if (!strcmp(table, "list")) {
##	retrieve (aarg0 = l.name, aarg1 = text(l.active),
##		  aarg2 = text(l.public), aarg3 = text(l.hidden),
##		  aarg4 = text(l.maillist), aarg5 = text(l.group),
##		  aarg6 = text(l.gid), aarg7 = l.acl_type,
##		  aarg8 = text(l.acl_id), aarg9 = l.desc)
##	  where qual
	afterc = 10;
    } else if (!strcmp(table, "members")) {
	id = (int) argv[0];
##	retrieve (aarg3 = text(list.group)) where list.list_id = id
	name = malloc(0);
	id_to_name(id, "LIST", &name);
	strcpy(aarg0, name);
	strcpy(aarg1, argv[1]);
	id = (int) argv[2];
	if (!strcmp(aarg1, "USER")) {
	    id_to_name(id, aarg1, &name);
	} else if (!strcmp(aarg1, "LIST")) {
	    id_to_name(id, aarg1, &name);
	} else if (!strcmp(aarg1, "STRING")) {
	    id_to_name(id, aarg1, &name);
	} else if (!strcmp(aarg1, "KERBEROS")) {
	    id_to_name(id, "STRING", &name);
	}
	strcpy(aarg2, name);
	free(name);
	afterc = 4;
    } else if (!strcmp(table, "clear")) {
	afterc = 0;
	table = beforetable;
    } /* else
      com_err(whoami, 0, "unknown table in incremental_after"); */

    iu = (struct iupdate *) malloc(sizeof(struct iupdate));
    iu->table = strsave(table);
    iu->beforec = beforec;
    iu->before = copy_argv(before, beforec);
    iu->afterc = afterc;
    iu->after = copy_argv(after, afterc);
    sq_save_data(incremental_sq, iu);

#ifdef DEBUG
    sprintf(buffer, "INCREMENTAL(%s, [", table);
    for (i = 0; i < beforec; i++) {
	if (i == 0)
	  strcat(buffer, strtrim(before[0]));
	else {
	    strcat(buffer, ", ");
	    strcat(buffer, strtrim(before[i]));
	}
    }
    strcat(buffer, "], [");
    for (i = 0; i < afterc; i++) {
	if (i == 0)
	  strcat(buffer, strtrim(after[0]));
	else {
	    strcat(buffer, ", ");
	    strcat(buffer, strtrim(after[i]));
	}
    }
    strcat(buffer, "])");
    com_err(whoami, 0, buffer);
#endif DEBUG
##}


/* Called when the current transaction is committed to start any queued
 * incremental updates.  This caches the update table the first time it
 * is called.
 */

struct inc_cache {
    struct inc_cache *next;
    char *table;
    char *service;
};


incremental_update()
{
    static int inited = 0;
    static struct inc_cache *cache;
    struct inc_cache *c;
##  char tab[17], serv[17];
    struct iupdate *iu;

    if (!inited) {
	inited++;

##	retrieve (tab = incremental.table, serv = incremental.service) {
    	    c = (struct inc_cache *)malloc(sizeof(struct inc_cache));
	    c->next = cache;
	    c->table = strsave(strtrim(tab));
	    c->service = strsave(strtrim(serv));
	    cache = c;
##	}
    }

    while (sq_remove_data(incremental_sq, &iu)) {
	for (c = cache; c; c = c->next) {
	    if (!strcmp(c->table, iu->table)) {
		iu->service = c->service;
		sq_save_data(incremental_exec, iu);
	    }
	}
    }
    if (inc_running == 0)
      next_incremental();
}


next_incremental()
{
    struct iupdate *iu;
    char *argv[MAXARGC * 2 + 4], cafter[3], cbefore[3], prog[BUFSIZ];
    int i;

    if (incremental_exec == NULL)
      incremental_init();

    if (sq_empty(incremental_exec) ||
	(inc_running && now - inc_started < INC_TIMEOUT))
      return;

    if (inc_running)
      com_err(whoami, 0, "incremental timeout on pid %d", inc_pid);

    sq_remove_data(incremental_exec, &iu);
    argv[1] = iu->table;
    sprintf(cbefore, "%d", iu->beforec);
    argv[2] = cbefore;
    sprintf(cafter, "%d", iu->afterc);
    argv[3] = cafter;
    for (i = 0; i < iu->beforec; i++)
      argv[4 + i] = iu->before[i];
    for (i = 0; i < iu->afterc; i++)
      argv[4 + iu->beforec + i] = iu->after[i];

    sprintf(prog, "%s/%s.incr", BIN_DIR, iu->service);
#ifdef DEBUG
    com_err(whoami, 0, "forking %s", prog);
#endif
    argv[0] = prog;
    argv[4 + iu->beforec + iu->afterc] = 0;
    inc_pid = vfork();
    switch (inc_pid) {
    case 0:
	execv(prog, argv);
	exit(1);
    case -1:
	com_err(whoami, 0, "Failed to start incremental update");
	break;
    default:
	inc_running = 1;
	inc_started = now;
    }

    free_argv(iu->before, iu->beforec);
    free_argv(iu->after, iu->afterc);
    free(iu->table);
    free(iu);

}


/* Called when the current transaction is aborted to throw away any queued
 * incremental updates
 */

incremental_flush()
{
    struct iupdate *iu;

    while (sq_get_data(incremental_sq, &iu)) {
	free_argv(iu->before, iu->beforec);
	free_argv(iu->after, iu->afterc);
	free(iu->table);
	free(iu);
    }
    sq_destroy(incremental_sq);
    incremental_sq = sq_create();
}


char **copy_argv(argv, argc)
char **argv;
int argc;
{
    char **ret = (char **)malloc(sizeof(char *) * argc);
    while (--argc >= 0)
      ret[argc] = strsave(strtrim(argv[argc]));
    return(ret);
}

free_argv(argv, argc)
char **argv;
int argc;
{
    while (--argc >= 0)
      free(argv[argc]);
    free(argv);
}
