/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/resync.qc,v 1.3 1990-09-21 15:20:13 mar Exp $
 *
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <rx/xdr.h>
#include <afs/ptint.h>
#include <afs/ptserver.h>
#include <afs/pterror.h>
#include <moira.h>
#include <moira_site.h>
#include <ctype.h>


#define min(x,y)	((x) < (y) ? (x) : (y))
#define abs(x)		(((x) > 0) ? (x) : -(x))
struct hash *users, *groups, *usersid, *groupsid;
char *whoami = "sync";

char *malloc(), *strsave(), *perm_malloc();
int dbase_fd;
int replaceflag = 0;
struct prheader prh;


main(argc, argv)
int argc;
char **argv;
{
    int i;
    char *file = NULL;
    int ingerr();

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'r':
		replaceflag++;
		break;
	    default:
		usage(argv[0]);
	    }
	} else {
	    if (file == NULL)
	      file = argv[i];
	    else
	      usage(argv[0]);
	}
    }

    i = O_RDONLY;
    if (replaceflag)
      i = O_RDWR;
    dbase_fd = open(file, i, 0660);
    if (dbase_fd < 0) {
	perror("opening file %s", file);
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

    exit(MR_SUCCESS);
}

usage(name)
char *name;
{
    fprintf(stderr, "usage: %s [-r] dbfile\n", name);
    exit(MR_ARGS);
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
	ingres_errno = MR_DEADLOCK;
	break;
    default:
	ingres_errno = MR_INGRES_ERR;
    }
    com_err(whoami, MR_INGRES_ERR, " code %d\n", *num);
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
	    u = (struct user *)perm_malloc(sizeof(struct user));
	    if (u == NULL)
	      nomem("loading users");
	    strncpy(u->login, login, 9);
	    u->flag = 0;
	    u->uid = uid;
	    hash_store(users, uid, u);
	    hash_store(usersid, id, u);
##  }
##}


#define CHUNKSIZE 5
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
	    g = (struct group *)perm_malloc(sizeof(struct group));
	    if (g == NULL)
	      nomem("loading groups");
	    bzero(g, sizeof(struct group));
	    strncpy(g->name, name, 33);
	    g->gid = gid;
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
		  c = (struct chunk *) perm_malloc(sizeof(struct chunk));
		  if (c == NULL)
		    nomem("loading members");
		  g->ch.xmem = c;
	      }
	      while (i >= CHUNKSIZE) {
		  i -= CHUNKSIZE;
		  if (c->xmem == NULL) {
		      c->xmem = (struct chunk *) perm_malloc(sizeof(struct chunk));
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
    int status;

    if (u->flag == 0) {
	printf("< User %s id %d is in Moira but not in prs\n",
	       u->login, u->uid);
	if (replaceflag) {
	    status = PR_INewEntry(NULL, u->login, u->uid, 0);
	    if (status) {
		prserror(status, "adding user %s uid %d to prs database",
			 u->login, u->uid);
	    }
	}
    }
}

checkgroup(id, g)
int id;
struct group *g;
{
    register int i, j, status;
    struct chunk *c;

    if (g->flag == 0) {
	printf("< Group %s id %d is in Moira but not in prs\n",
	       g->name, g->gid);
	if (replaceflag) {
	    char namebuf[64];
	    sprintf(namebuf, "system:%s", g->name);
	    status = PR_INewEntry(NULL, namebuf, -g->gid, SYSADMINID);
	    if (status)
	      prserror(status, "adding list %s gid %d to prs database",
		       namebuf, -g->gid);
	}
    }
    c = (struct chunk *)&(g->ch);
    for (i = 0; i < g->members; i++) {
	j = i % CHUNKSIZE;
	printf("< Group %s contains user %d in Moira but not in prs\n",
	       g->name, c->member[j]);
	if (replaceflag) {
	    status = PR_AddToGroup(NULL, c->member[j], -g->gid);
	    if (status)
	      prserror(status, "adding %d to group %d in prs",
		       c->member[j], -g->gid);
	}
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
	    if (g == NULL) {
		if (!strncmp(entry.name, "system:", 7)) {
		    printf("> Group %s, id %d is in prs but not moira\n",
			   entry.name, id);
		    if (replaceflag) {
			status = PR_Delete(NULL, id);
			if (status)
			  prserror(status, "deleting group %d from prs", id);
		    }
		}
	    } else if (strcasecmp(g->name, index(entry.name, ':')+1)) {
		printf("> GID %d in prs is %s, but GID %d in moira is %s\n",
		       id, entry.name, -id, g->name);
		if (replaceflag) {
		    printf("*** Dont' know how to fix this!!!!!\n");
		}
	    } else {
		g->flag = 1;
		if (entry.entries[0])
		  do_members(&entry, g);
	    }
	} else {
	    u = (struct user *) hash_lookup(users, abs(id));
	    if (u == NULL) {
		printf("> User %s, id %d is in prs but not moira\n",
		       entry.name, id);
		if (replaceflag) {
		    status = PR_Delete(NULL, id);
		    if (status)
		      prserror(status, "while deleting user %d from prs", id);
		}
	    } else if (strcasecmp(u->login, entry.name)) {
		printf("> User id %d in prs is %s, but UID %d in moira is %s\n",
		       id, entry.name, id, u->login);
		if (replaceflag) {
		    printf("*** don't know how to fix this!!!!!\n");
		}
	    } else
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
    register int i, j, status;

    if (uid == PRBADID)
      return;
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
	if (replaceflag) {
	    status = PR_RemoveFromGroup(NULL, uid, -g->gid);
	    if (status)
	      prserror(status, "While removing user %d from group %d in prs",
		       uid, g->gid);
	}
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
    if (replaceflag) {
	status = PR_RemoveFromGroup(NULL, uid, -g->gid);
	if (status)
	  prserror(status, "While removing user %d from group %d in prs",
		   uid, g->gid);
    }
}


#define MALLOC_BLOCK	102400

char *perm_malloc(size)
unsigned size;
{
    static char *pool = NULL;
    static unsigned pool_size = 0;
    register char *ret;

    if (size > pool_size) {
	pool = (char *) malloc(MALLOC_BLOCK);
	pool_size = MALLOC_BLOCK;
    }
    ret = pool;
    pool += size;
    pool = (char *)(((unsigned) (pool + 1)) & ~1);
    pool_size -= (pool - ret);
    return(ret);
}

