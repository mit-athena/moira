/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/sync.qc,v 1.9 1992-06-29 10:19:17 probe Exp $
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

struct hash *users;
struct hash *groups;

main(argc, argv)
int argc;
char **argv;
{
##  char db[9];
    
    int status;
    int ingerr();

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

##  end transaction
##  exit

    exit(MR_SUCCESS);
}


do_passwd()
##{
##  char login[9];
##  int uid, id, status;
    int t;
    struct prentry tentry;

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
	    else
		hash_store(users, id, uid);
##  }
##}



do_groups()
##{
    long u, g, status, gpos, upos;
    struct prentry gentry, uentry;
    char namebuf[41];
    int aid, t;
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
	    else
		hash_store(groups, lid, aid);

	    /* If this list is hidden, set the pts entry to be s---- */
	    if (hide && (status==0 || status==PRIDEXIST)) {
		gpos = FindByID(0, aid);
		status = pr_ReadEntry(0, 0, gpos, &gentry);
		if (!status) {
		    gentry.flags = PRGRP|PRACCESS|PRP_STATUS_ANY;
		    status = pr_WriteEntry(0, 0, gpos, &gentry);
		}
		if (status)
		    fprintf(stderr,
			    "Error setting flags on group %s: %s\n",
			    namebuf, error_message(status));
	    }
##  }

    t = time(0);
    fprintf(stderr, "Doing members: %s", ctime(&t));

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (long) hash_lookup(users, id)) &&
	  (g = (long) hash_lookup(groups, lid))) {
	  if (g==ANYUSERID || g==AUTHUSERID || u==ANONYMOUSID) {
	      status = PRPERM;
	  } else if ((gpos = FindByID(0,g)) && (upos = FindByID(0,u))) {
	      status = pr_ReadEntry(0,0,upos,&uentry);
	      if (!status) status = pr_ReadEntry(0,0,gpos,&gentry);
	      if (!status) status = AddToEntry (0, &gentry, gpos, u);
	      if (!status) status = AddToEntry (0, &uentry, upos, g);
	  } else {
	      status = PRNOENT;
	  }
	  if (status)
	      fprintf(stderr, "Error adding uid %d to group %d: %s\n",
		      u, -g, error_message(status));
      }
##  }

    t = time(0);
    fprintf(stderr, "Done: %s", ctime(&t));

##}


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
