/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/sync.qc,v 1.14 1993-02-05 03:30:47 probe Exp $
 *
 *
 *  (c) Copyright 1989 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>

#include <rx/xdr.h>
#include "ptint.h"
#include "ptserver.h"
#include "pterror.h"

#include <moira.h>
#include <moira_site.h>
#include <ctype.h>

#define min(x,y)	((x) < (y) ? (x) : (y))
char *whoami = "sync";

char *malloc(), *strsave();
int dbase_fd;

int ucount = 0;
int gcount = 0;
int mcount = 0;

struct hash *users;
struct hash *groups;

struct member {
    struct entry *user;
    struct entry *group;
    struct member *unext;
    struct member *gnext;
};
struct entry {
    long id;
    struct member *members;
};

main(argc, argv)
int argc;
char **argv;
{
##  char db[9];
    
    int status;
    int ingerr();
    long t;

    strcpy(db, "sms");

    if (!strcmp(argv[1], "-db")) {
	strncpy(db, argv[2], sizeof(db)-1);
	argc -= 2;
	argv += 2;
    }
    if (argc != 2) {
	fprintf(stderr, "usage: %s [-db sms] outfile\n", whoami);
	exit(MR_ARGS);
    }

    dbase_fd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0660);
    if (dbase_fd < 0) {
	perror("opening file %s", argv[1]);
	exit(1);
    }	
    IIseterr(ingerr);

    initialize_sms_error_table();
    initialize_pt_error_table();
    Initdb();					/* Initialize prdb */
    
    users = create_hash(10000);
    groups = create_hash(15000);
    
##  ingres db
##  set lockmode session where level = table
##  begin transaction

    do_passwd();
    do_groups();

    t = time(0);
    fprintf(stderr, "Done (%d users, %d groups, %d members): %s",
	    ucount, gcount, mcount, ctime(&t));

##  end transaction
##  exit

    exit(MR_SUCCESS);
}


do_passwd()
##{
##  char login[9];
##  int uid, id, status;
    long t;
    struct prentry tentry;
    struct entry *u;

    t = time(0);
    fprintf(stderr, "Doing users: %s", ctime(&t));
    
##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, id = u.users_id)
##	where u.#uid > 0 and (u.#status = 1 or u.#status = 2) {
	    lowercase(strtrim(login));

	    if (FindByID(0,uid))
		status = PRIDEXIST;
	    else
		status = CreateEntry(0,login,&uid,1/*idflag*/,0/*gflag*/,
				     SYSADMINID/*oid*/, SYSADMINID/*cid*/);
	    if (status)
		fprintf(stderr, "Error adding user %s uid %d: %s\n",
			login, uid, error_message(status));
	    else {
		u = (struct entry *)malloc(sizeof(struct entry));
		u->id = uid;
		u->members = 0;
		hash_store(users, id, u);
		ucount++;
	    }
##  }
##}



do_groups()
##{
    long status, pos;
    struct prentry gentry, uentry;
    struct entry *u, *g;
    struct member *m;
    struct bucket **p, *b;
    char namebuf[40];
    long aid, t;
##  char name[33];
##  int gid, id, lid, hide;

    t = time(0);
    fprintf(stderr, "Doing groups: %s", ctime(&t));

##  range of l is list
##  range of m is imembers
    /* get lock records */
##  retrieve (name = l.modtime) where l.list_id = 0
##  retrieve (name = users.modtime) where users.users_id = 0

##  retrieve (name = l.#name, gid = l.#gid, lid = l.list_id, hide = l.hidden)
##	where l.group != 0 and l.active != 0 and l.#gid > 0 {
	    lowercase(strtrim(name));
	    sprintf(namebuf, "system:%s", name);
	    aid = -gid;

	    if (FindByID(0, aid))
		status = PRIDEXIST;
	    else
		status = CreateEntry(0,namebuf,&aid,1/*idflag*/,PRGRP/*gflag*/,
				     SYSADMINID/*oid*/, SYSADMINID/*cid*/);
	    if (status)
		fprintf(stderr, "Error adding group %s id %d: %s\n",
			namebuf, aid, error_message(status));

	    if ((status==0 || status==PRIDEXIST) &&
		(aid!=ANYUSERID && aid!=AUTHUSERID)) {

		g = (struct entry *)malloc(sizeof(struct entry));
		g->id = aid;
		g->members = 0;
		hash_store(groups, lid, g);
		gcount++;

		/* Set modes on hidden lists (S----) */
		if (hide) {
		    pos = FindByID(0, aid);
		    status = pr_Read(0, 0, pos, &gentry, sizeof(gentry));
		    if (!status) {
			gentry.flags = htonl(PRGRP|PRACCESS|PRP_STATUS_ANY);
			status = pr_Write(0, 0, pos, &gentry, sizeof(gentry));
		    }
		    if (status)
			fprintf(stderr,
				"Error setting flags on group %s: %s\n",
				namebuf, error_message(status));
		}
	    }
##  }

    t = time(0);
    fprintf(stderr, "Doing members: %s", ctime(&t));

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (struct entry *)hash_lookup(users, id)) &&
	  (g = (struct entry *)hash_lookup(groups, lid))) {
	      m = (struct member *)malloc(sizeof(struct member));
	      m->user = u;
	      m->group = g;
	      m->unext = u->members;
	      m->gnext = g->members;
	      u->members = g->members = m;
	      mcount++;
      }
##  }

    /* Do the bulk of the membership quickly.
     * Add PRSIZE members into the user/group record.  After that, we
     * require the use of continuation records, but we assume this is
     * few enough that we do this the slow way (AddToEntry).
     */
    for (p = &(users->data[users->size - 1]); p >= users->data; p--) {
	for (b = *p; b; b = b->next) {
	    if ((u = (struct entry *)b->data)->members == 0)
		continue;
	    pos = FindByID(0, u->id);
	    pr_Read(0, 0, pos, &uentry, sizeof(uentry));
	    for (t=0, m=u->members; m && t<PRSIZE; m=m->unext, t++)
		uentry.entries[t] = htonl(m->group->id);
	    uentry.count = htonl(t);
	    pr_Write(0, 0, pos, &uentry, sizeof(uentry));
	    if (m) {
		pr_ReadEntry(0, 0, pos, &uentry);
		while (m) {
		    AddToEntry(0, &uentry, pos, m->group->id);
		    m = m->unext;
		}
	    }
	}
    }
    for (p = &(groups->data[groups->size - 1]); p >= groups->data; p--) {
	for (b = *p; b; b = b->next) {
	    if ((g = (struct entry *)b->data)->members == 0)
		continue;
	    pos = FindByID(0, g->id);
	    pr_Read(0, 0, pos, &gentry, sizeof(gentry));
	    for (t=0, m=g->members; m && t<PRSIZE; m=m->gnext, t++)
		gentry.entries[t] = htonl(m->user->id);
	    gentry.count = htonl(t);
	    pr_Write(0, 0, pos, &gentry, sizeof(gentry));
	    if (m) {
		pr_ReadEntry(0, 0, pos, &gentry);
		while (m) {
		    AddToEntry(0, &gentry, pos, m->user->id);
		    m = m->gnext;
		}
	    }
	}
    }
##}

#if 0
do_kerberos()
##{
    long status, pos;
    struct prentry gentry, uentry;
    struct entry *u, *g;
    struct member *m;
    struct bucket **p, *b;
    char namebuf[40];
    int aid, t;
##  char name[129];
##  int gid, id, lid, hide;

    t = time(0);
    fprintf(stderr, "Doing kerberos members: %s", ctime(&t));

##  range of s is strings
##  range of m is imembers
    /* get lock records */
##  retrieve (name = list.modtime) where list.list_id = 0
##  retrieve (name = s.modtime) where s.string_id = 0

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "KERBEROS" {
	xxx;
	{
	}
##  }

##}
#endif
    
    
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
