/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/sync.qc,v 1.2 1989-09-26 12:11:04 mar Exp $
 *
 * This generates the zone files necessary to load a hesiod server.
 * The following zones are generated: passwd, uid, pobox, group,
 * grplist, gid, filsys, cluster, pcap, sloc, service.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
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

#define USERSMS 14487

#define min(x,y)	((x) < (y) ? (x) : (y))
struct hash *users = NULL;
char *whoami = "sync";

char *malloc(), *strsave();
int dbase_fd;


main(argc, argv)
int argc;
char **argv;
{
    int status;
    int ingerr();

    if (argc != 2) {
	fprintf(stderr, "usage: %s outfile\n", argv[0]);
	exit(SMS_ARGS);
    }

    dbase_fd = open(argv[1], O_RDWR|O_CREAT, 0660);
    if (dbase_fd < 0) {
	perror("opening file %s", argv[1]);
	exit(1);
    }	
    IIseterr(ingerr);
    initialize_sms_error_table ();

##  ingres sms
##  set lockmode session where level = table
##  begin transaction

    do_passwd();
    status = PR_AddToGroup(NULL, USERSMS, SYSADMINID);
    if (status) {
	prserror(status, "adding SMS to system:administrators", 0, 0);
    }
    do_groups();

##  end transaction
##  exit

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



do_passwd()
##{
##  char login[9];
##  int uid, id, status;

    fprintf(stderr, "Doing users\n");
    users = create_hash(10000);
##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, id = u.users_id)
##	where u.#status = 1 {
	    strtrim(login);
	    hash_store(users, id, uid);
	    status = PR_INewEntry(NULL, login, uid, 0);
	    if (status) {
		prserror(status, "adding user %s uid %d", login, uid);
	    }
##  }
##}



do_groups()
##{
    struct hash *groups;
    long u, g, status;
##  char name[33], namebuf[128];
##  int gid, id, lid;

    fprintf(stderr, "Doing groups\n");

    /* make space for group list */
    groups = create_hash(15000);

    /* retrieve simple groups */
##  range of l is list
##  range of m is imembers
    /* get lock records */
##  retrieve (name = l.modtime) where l.list_id = 0
##  retrieve (name = users.modtime) where users.users_id = 0

##  retrieve (name = l.#name, gid = l.#gid, lid = l.list_id)
##	where l.group != 0 and l.active != 0 {
	    strtrim(name);
	    sprintf(namebuf, "system:%s", name);
	    hash_store(groups, lid, -gid);
	    status = PR_INewEntry(NULL, namebuf, -gid, SYSADMINID);
	    if (status)
	      prserror(status, "adding list %s gid %d", namebuf, -gid);
##  }


    fprintf(stderr, "Doing members\n");

##  retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (long) hash_lookup(users, id)) &&
	  (g = (long) hash_lookup(groups, lid))) {
	  status = PR_AddToGroup(NULL, u, g);
	  if (status) {
	      prserror(status, "adding %d to group %d", u, -g);
	  }
      }
##  }

##}

