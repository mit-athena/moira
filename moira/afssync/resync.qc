/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/resync.qc,v 1.1 1989-09-26 12:11:33 mar Exp $
 *
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"
#include "prerror.h"
#include <sms.h>
#include <sms_app.h>
#include <ctype.h>


#define min(x,y)	((x) < (y) ? (x) : (y))
#define abs(x)		(((x) > 0) ? (x) : -(x))
struct hash *users, *groups, *usersid, *groupsid;
char *whoami = "sync";

char *malloc(), *strsave();
int dbase_fd;
struct prheader prh;


main(argc, argv)
int argc;
char **argv;
{
    int ingerr();

    if (argc != 2) {
	fprintf(stderr, "usage: %s dbfile\n", argv[0]);
	exit(SMS_ARGS);
    }

    dbase_fd = open(argv[1], O_RDONLY, 0660);
    if (dbase_fd < 0) {
	perror("opening file %s", argv[1]);
	exit(1);
    }	
    IIseterr(ingerr);
    initialize_sms_error_table ();

##  ingres sms
##  set lockmode session where level = table
##  begin transaction

    get_users();
    get_groups();
    get_members();

##  end transaction
##  exit

    pr_Read(NULL, 0, 0, &prh, sizeof(prh));
    check_users_and_groups();

    exit(SMS_SUCCESS);
}


/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */
#define INGRES_DEADLOCK 4700

static int ingerr(num)
    int *num;
{
    char buf[256];
    int ingres_errno;

    switch (*num) {
    case INGRES_DEADLOCK:
	ingres_errno = SMS_DEADLOCK;
	break;
    default:
	ingres_errno = SMS_INGRES_ERR;
    }
    com_err(whoami, SMS_INGRES_ERR, " code %d\n", *num);
    exit(ingres_errno);
}


prserror(status, msg, arg1, arg2)
int status;
char *msg;
unsigned long arg1, arg2;
{
    char buf[512];

    sprintf(buf, msg, arg1, arg2);
    switch (status) {
    case PREXIST:
	msg = "name already exists";
	break;
    case PRIDEXIST:
	msg = "ID already exists";
	break;
    case PRNOIDS:
	msg = "no IDs available";
	break;
    case PRDBFAIL:
	msg = "database failed";
	break;
    case PRNOENT:
	msg = "no space left in database";
	break;
    case PRPERM:
	msg = "permission denied";
	break;
    case PRNOTGROUP:
	msg = "not a group";
	break;
    case PRNOTUSER:
	msg = "not a user";
	break;
    case PRBADNAM:
	msg = "bad name";
	break;
    case 0:
	msg = "no error";
	break;
    default:
	msg = "unknown code";
	break;
    }
    fprintf(stderr, "%s (%d): %s\n", msg, status, buf);
}


nomem(msg)
char *msg;
{
    fprintf(stderr, "Out of memory: %s\n", msg);
    exit(1);
}


struct user {
    char login[9];
    char flag;
    short uid;
};


get_users()
##{
##  char login[9];
##  int uid, id;
    struct user *u;

    fprintf(stderr, "Loading users from Moira\n");
    users = create_hash(10000);
    usersid = create_hash(10000);
##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, id = u.users_id)
##	where u.#status = 1 {
	    strtrim(login);
	    u = (struct user *)malloc(sizeof(struct user));
	    if (u == NULL)
	      nomem("loading users");
	    strncpy(u->login, login, 9);
	    u->flag = 0;
	    u->uid = uid;
	    hash_store(users, uid, u);
	    hash_store(usersid, id, u);
##  }
##}


#define CHUNKSIZE 10
struct chunk {
    short member[CHUNKSIZE];
    struct chunk *xmem;
};

struct group {
    char name[33];
    char flag;
    short gid;
    short members;
    struct chunk ch;
};


get_groups()
##{
##  char name[33];
##  int gid, id;
    struct group *g;

    fprintf(stderr, "Loading groups from Moira\n");
    groups = create_hash(10000);
    groupsid = create_hash(10000);
##  range of l is list
##  retrieve (name = l.#name, gid = l.#gid, id = l.list_id)
##	where l.#active = 1 and l.#group = 1 {
	    strtrim(name);
	    g = (struct group *)malloc(sizeof(struct group));
	    if (g == NULL)
	      nomem("loading groups");
	    bzero(g, sizeof(struct group));
	    strncpy(g->name, name, 33);
	    hash_store(groups, gid, g);
	    hash_store(groupsid, id, g);
##  }
##}


get_members()
##{
    struct user *u;
    struct group *g;
##  int id, lid;

    fprintf(stderr, "Fetching members from Moira\n");

##  range of m is imembers

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (struct user *) hash_lookup(usersid, id)) &&
	  (g = (struct group *) hash_lookup(groupsid, lid))) {
	  if (g->members < CHUNKSIZE) {
	      g->ch.member[g->members] = u->uid;
	  } else {
	      int i = g->members - CHUNKSIZE;
	      struct chunk *c;

	      if ((c = g->ch.xmem) == NULL) {
		  c = (struct chunk *) malloc(sizeof(struct chunk));
		  if (c == NULL)
		    nomem("loading members");
		  g->ch.xmem = c;
	      }
	      while (i >= CHUNKSIZE) {
		  i -= CHUNKSIZE;
		  if (c->xmem == NULL) {
		      c->xmem = (struct chunk *) malloc(sizeof(struct chunk));
		      if (c->xmem == NULL)
			nomem("loading members");
		  }
		  c = c->xmem;
	      }
	      c->member[i] = u->uid;
	  }
	  g->members++;
      }
##  }

##}



checkuser(id, u)
int id;
struct user *u;
{
    if (u->flag == 0)
      printf("< User %s id %d is in Moira but not in prs\n", u->login, u->uid);
}

checkgroup(id, g)
int id;
struct group *g;
{
    register int i, j;
    struct chunk *c;

    if (g->flag == 0)
      printf("< Group %s id %d is in Moira but not in prs\n", g->name, g->gid);
    c = (struct chunk *)&(g->ch);
    for (i = 0; i < g->members; i++) {
	j = i % CHUNKSIZE;
	printf("< Group %s contains user %d in Moira but not in prs\n",
	       g->name, c->member[j]);
	if (j == CHUNKSIZE - 1)
	  c = c->xmem;
    }
}

check_users_and_groups()
{
    register int i;
    int offset;

    fprintf(stderr, "Scanning PRS database\n");

    for (i = 0; i < HASHSIZE; i++) {
	offset = ntohl(prh.idHash[i]);
	while (offset)
	  offset = check_entry(offset);
    }
    hash_step(users, checkuser, NULL);
    hash_step(groups, checkgroup, NULL);
}

check_entry(offset)
int offset;
{
    struct prentry entry;
    int flags, id, status;
    struct user *u;
    struct group *g;

    status = pr_Read(NULL, 0, offset, &entry, sizeof(entry));
    if (status)
      prserror(status, "reading prentry at %d", offset, 0);
    flags = ntohl(entry.flags);
    id = ntohl(entry.id);
    if ((flags & PRFREE) == 0) {
	if (flags & PRGRP) {
	    g = (struct group *) hash_lookup(groups, abs(id));
	    if (g == NULL)
	      printf("> Group %s, id %d is in prs but not moira\n",
		     entry.name, id);
	    else if (strcasecmp(g->name, index(entry.name, ':')+1))
	      printf("> Group id %d in prs is %s, but GID %d in moira is %s\n",
		     id, entry.name, -id, g->name);
	    else {
		g->flag = 1;
		if (entry.entries[0])
		  do_members(&entry, g);
	    }
	} else {
	    u = (struct user *) hash_lookup(users, abs(id));
	    if (u == NULL)
	      printf("> User %s, id %d is in prs but not moira\n",
		     entry.name, id);
	    else if (strcasecmp(u->login, entry.name))
	      printf("> User id %d in prs is %s, but UID %d in moira is %s\n",
		     id, entry.name, id, u->login);
	    else
	      u->flag = 1;
	}
    }
    return(ntohl(entry.nextID));
}


do_members(e, g)
struct prentry *e;
struct group *g;
{
    register int i;
    int offset;
    struct contentry prco;

    for (i = 0; i < PRSIZE; i++) {
	if (e->entries[i] == 0)
	  return;
	mark_member(g, ntohl(e->entries[i]));
    }
    offset = ntohl(e->next);
    while (offset) {
	pr_Read(NULL, 0, offset, &prco, sizeof(struct contentry));
	for (i = 0; i < COSIZE; i++) {
	    if (prco.entries[i] == 0)
	      return;
	    mark_member(g, ntohl(prco.entries[i]));
	}
	offset = ntohl(prco.next);
    }
}


mark_member(g, uid)
struct group *g;
int uid;
{
    register int i, j;

    if (g->members >= CHUNKSIZE) {
	short *s = NULL, s1;
	struct chunk *c = (struct chunk *)&(g->ch);

	for (i = 0; i < g->members; i++) {
	    j = i % CHUNKSIZE;
	    if (c->member[j] == uid) {
		s = &(c->member[j]);
	    }
	    if (j == CHUNKSIZE - 1) {
		s1 = c->member[CHUNKSIZE - 1];
		c = c->xmem;
	    }
	}
	if (j != CHUNKSIZE - 1)
	  s1 = c->member[j];
	if (s) {
	    *s = s1;
	    g->members--;
	    return;
	}
	printf("> Group %s contains user %d in prs but not in Moira\n", g->name,
	       uid);
	return;
    }
    for (i = 0; i < g->members; i++) {
	if (g->ch.member[i] == uid) {
	    if (g->members == i + 1)
	      g->ch.member[i] = 0;
	    else {
		g->ch.member[i] = g->ch.member[g->members];
		g->ch.member[g->members] = 0;
	    }
	    g->members--;
	    return;
	}
    }
    printf("> Group %s contains user %d in prs but not in Moira\n", g->name,
	   uid);
}
