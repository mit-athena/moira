/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/sync.fast.qc,v 1.1 1992-06-29 10:22:10 probe Exp $
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

long pos;		/* cheader.eofPtr */
long usercount;		/* cheader.usercount */
long groupcount;	/* cheader.groupcount */

struct hash *users;
struct hash *groups;
struct hash *idpos;

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
    
    idpos = create_hash(20000);
    users = create_hash(10000);
    groups = create_hash(15000);
    
    pos = ntohl(cheader.eofPtr);
    usercount = ntohl(cheader.usercount);
    groupcount = ntohl(cheader.groupcount);

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
    int t, i;
    struct prentry tentry;

    t = time(0);
    fprintf(stderr, "Doing users: %s", ctime(&t));
    
##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, id = u.users_id)
##	where u.#uid > 0 and (u.#status = 1 or u.#status = 2) {
	    lowercase(strtrim(login));

	    if (uid==ANONYMOUSID)
		status = PRIDEXIST;
	    else {
		bzero(&tentry, sizeof(tentry));
		hash_store(idpos, pos);
		strcpy(tentry.name, login);
		tentry.id = uid;
		tentry.owner = SYSADMINID;
		tentry.creator = SYSADMINID;
		tentry.flags = PRQUOTA;
		tentry.ngroups = tentry.nusers = 20;
		if (tentry.id > ntohl(cheader.maxID))
		    cheader.maxID = htonl(tentry.id);
		usercount++;

		i = IDHash(tentry.id);
		tentry.nextID = ntohl(cheader.idHash[i]);
		cheader.idHash[i] = htonl(pos);

		i = NameHash(tentry.name);
		tentry.nextName = ntohl(cheader.nameHash[i]);
		cheader.nameHash[i] = htonl(pos);
		
		pr_WriteEntry(0, 0, pos, &tentry);
		pos += ENTRYSIZE;
		status = 0;
	    }

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
    int aid, t, i;
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

	    if (aid==ANYUSERID || aid==AUTHUSERID)
		status = PRIDEXIST;
	    else {
		bzero(&gentry, sizeof(gentry));
		hash_store(idpos, pos);
		strcpy(gentry.name, namebuf);
		gentry.id = aid;
		gentry.owner = SYSADMINID;
		gentry.creator = SYSADMINID;
		if (hide)
		    gentry.flags = PRGRP|PRACCESS|PRP_STATUS_ANY;
		else
		    gentry.flags = PRGRP;
		if (gentry.id < ntohl(cheader.maxGroup))
		    cheader.maxGroup = htonl(gentry.id);
		groupcount++;

		i = IDHash(gentry.id);
		gentry.nextID = ntohl(cheader.idHash[i]);
		cheader.idHash[i] = htonl(pos);

		i = NameHash(gentry.name);
		gentry.nextName = ntohl(cheader.nameHash[i]);
		cheader.nameHash[i] = htonl(pos);

		pr_WriteEntry(0, 0, pos, &gentry);
		AddToOwnerChain(0,gentry.id,gentry.owner);
		pos += ENTRYSIZE;
		status = 0;
	    }
	    if (status)
		fprintf(stderr, "Error adding group %s id %d: %s\n",
			namebuf, aid, error_message(status));
	    else
		hash_store(groups, lid, aid);

##  }

    cheader.groupcount = htonl(groupcount);
    cheader.usercount = htonl(usercount);
    cheader.eofPtr = htonl(pos);
    pr_Write(0, 0, 0, &cheader, sizeof(cheader));
    
    t = time(0);
    fprintf(stderr, "Doing members: %s", ctime(&t));

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (long) hash_lookup(users, id)) &&
	  (g = (long) hash_lookup(groups, lid))) {
	  if ((gpos = get_id(g)) && (upos = get_id(u))) {
	      pr_ReadEntry(0,0,upos,&uentry);
	      pr_ReadEntry(0,0,gpos,&gentry);
	      AddToEntry (0, &gentry, gpos, u);
	      AddToEntry (0, &uentry, upos, g);
	      status = 0;
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


get_id(id)
{
    long i;

    if (i=(long)hash_lookup(idpos, id))
        return i;
    hash_store(idpos, id, i=FindByID(0, id));
    return i;
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
