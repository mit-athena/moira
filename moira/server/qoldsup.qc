/*
 *  These are query support routines that are specific to the old version
 *  of the protocol.  This file may be removed once support for the old
 *  protocol has been discontinued.
 *
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qoldsup.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qoldsup.qc,v 1.2 1988-09-13 17:43:04 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qoldsup.qc,v 1.2 1988-09-13 17:43:04 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include "query.h"
#include "sms_server.h"
#include <ctype.h>


extern char *whoami;


add_locker() { return(SMS_UNKNOWN_PROC); }
add_user_group() { return(SMS_UNKNOWN_PROC); }


/* Specialized Access Routines */


/**
 ** access_maillist - access_list + disallow adding user-group to maillists
 **
 ** Inputs:
 **   argv[0] - list_id
 **   cl - client name
 **
 **/

access_maillist(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int list_id;
##  int exists;
##  char list_name[32];
    int status;

    if ((status = access_list(q, argv, cl)) != SMS_SUCCESS)
	return(status);
    if (strcmp(q->name, "add_maillist"))
	return(status);

    list_id = *(int *)argv[0];
##  range of l is list
##  repeat retrieve (exists = any(l.#list_id 
##			      where l.group != 0 and l.#list_id = @list_id))
    if (!exists) return(SMS_SUCCESS);
##  repeat retrieve (exists = any(users.login where users.login = l.name and
##						    l.#list_id = @list_id))
    return ((exists) ? SMS_USER_GROUP : SMS_SUCCESS);
##}



/* Setup Routines */



/* FOLLOWUP ROUTINES */

/* followup_gpob: fixes argv[2] based on the IDs currently there and the
 * type in argv[1].  Then completes the upcall to the user.
 *
 * argv[2] is of the form "123:234" where the first integer is the machine
 * ID if it is a pop box, and the second is the string ID if it is an SMTP
 * box.  argv[1] should be "POP", "SMTP", or "NONE".  Boxes of type NONE
 * are skipped.
 *
 * followup_old_gpob does the same thing for old style queries.  The format
 * is a little different and there is one more argument, but the idea is
 * the same.
 */

followup_old_gpob(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
    char **argv, *p, *index();
    char *ptype;
##  char mach[129], box[129];
##  int id, rowcount;

    /* for each row */
    while (sq_get_data(sq, &argv)) {
	ptype = argv[1];
	sms_trim_args(2, argv);

	if (!strcmp(ptype, "POP")) {
	    id = atoi(argv[2]);
##	    repeat retrieve (mach=machine.name) where machine.mach_id=@id
	    strcpy(box, argv[0]);
	} else if (!strcmp(ptype, "SMTP")) {
	    id = atoi(argv[3]);
##	    repeat retrieve (box=strings.string) where strings.string_id=@id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		return(SMS_STRING);
	    p = index(box, '@');
	    if (p) {
		*p++ = 0;
		strcpy(mach, p);
	    } else
		strcpy(mach, "");
	} else /* ptype == "NONE" */ {
	    goto skip;
	}

	free(argv[2]);
	argv[2] = mach;
	free(argv[3]);
	argv[3] = box;
	(*action)(q->vcnt, argv, actarg);
    skip:
	/* free saved data */
	free(argv[0]);
	free(argv[1]);
	free(argv);
    }

    sq_destroy(sq);
    return (SMS_SUCCESS);
##}


/* Specialized query routines */

/* set_user_pobox - this does all of the real work.
 * argv = user_id, type, machine, box
 */
int set_user_pobox(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  int user, id, rowcount;
##  char *host;
##  char buffer[256];

    host = argv[2];
    user = *(int *)argv[0];

    if (!strcmp(argv[1], "POP")) {
##	repeat retrieve (id=machine.mach_id) where machine.name=uppercase(@host)
##	inquire_equel(rowcount = "rowcount")
	if (rowcount == 0)
	    return(SMS_MACHINE);
##	replace users (potype="POP", pop_id=id) where users.users_id = user
    } else if (!strcmp(argv[1], "SMTP")) {
	sprintf(buffer, "%s@%s", argv[3], host);
##      range of s is strings
##      repeat retrieve (id = s.string_id) where s.string = @buffer
##      inquire_equel (rowcount = "rowcount")
	if (rowcount == 0) {
##          range of v is values
##          repeat retrieve (id = v.value) where v.name = "strings_id"
	    id++;
##          repeat replace v (value = @id) where v.name = "strings_id"
##	    append to strings (string_id = id, string = buffer)
	}
##	repeat replace users (potype="SMTP",box_id=@id) 
##	       where users.users_id = @user
    } else /* argv[1] == "NONE" */ {
##	repeat replace users (potype="NONE") where users.users_id = @user
    }
    set_old_pop_usage(q, argv, 1);
    set_pobox_modtime(q, argv, cl);
    return(SMS_SUCCESS);
##}

followup_delete_pobox(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
{
    set_old_pop_usage(q, argv, -1);
    set_pobox_modtime(q, argv, cl);
    return(SMS_SUCCESS);
}

/** set_old_pop_usage - incr/decr usage count for pop server in serverhosts
 ** table
 **
 ** Inputs:
 **   q->name - "add_pobox" or "delete_pobox"
 **   argv[1] - type
 **   argv[2] - mach_id
 **
 ** Description:
 **   - incr/decr value field in serverhosts table for pop/mach_id
 **
 **/

set_old_pop_usage(q, argv, count)
    struct query *q;
    char *argv[];
##{
##  int mach_id;
##  int n;

    if (bcmp(argv[1], "POP", 3)) return(SMS_SUCCESS);

    mach_id = *(int *)argv[2];
    n = count;

##  range of sh is serverhosts
##  repeat replace sh (value1 = sh.value1 + @n)
##         where sh.service = "pop" and sh.#mach_id = @mach_id

    return(SMS_SUCCESS);
##}


/* Finds "acl_type:acl_name" in the argv (index depends on query) and
 * replaces it with the list or user name.
 */

followup_fix_acl(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
    char **argv, *p, *index(), *malloc();
##  char *name, *acl;
##  int id, i, rowcount;
    int idx;

    if (!strcmp(q->shortname, "gaml"))
	idx = 1;
    else if (!strcmp(q->shortname, "gamd"))
	idx = 1;
    else
	idx = 2;

    while (sq_get_data(sq, &argv)) {
	name = argv[idx];
	p = index(name, ':');
	*p++ = 0;
	id = atoi(p);
	if ((acl = malloc(33)) == NULL)
	    return(SMS_NO_MEM);
	if (!strcmp(name, "LIST")) {
##	    repeat retrieve (acl = list.#name) where list.list_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(acl, "???");
	} else if (!strcmp(name, "USER")) {
##	    repeat retrieve (acl = users.login) where users.users_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(acl, "???");
	} else
	  strcpy(acl, "???");
	free(argv[idx]);
	argv[idx] = acl;

	/* send the data */
	(*action)(q->vcnt, argv, actarg);

	/* free saved data */
	for (i = 0; i < q->vcnt; i++) 
	    free(argv[i]);
	free(argv);
    }

    sq_destroy(sq);
    return (SMS_SUCCESS);
##}


/**
 ** get_list_is_group
 ** get_list_is_maillist
 **
 ** Inputs:
 **   argv[0] - list_id
 **
 ** Returns:
 **   {true | false}
 **
 **/

get_list_is_group(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  int exists, list_id, rowcount;
    char *result;

    list_id = *(int *)argv[0];

##  range of l is list
##  repeat retrieve (exists = l.group) where l.#list_id = @list_id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NOT_UNIQUE);

    result = (exists) ? "true" : "false";
    (*action)(1, &result, actarg);
    return(SMS_SUCCESS);
##}

get_list_is_maillist(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  int exists, list_id, rowcount;
    char *result;

    list_id = *(int *)argv[0];

##  range of l is list
##  repeat retrieve (exists = l.maillist) where l.#list_id = @list_id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
	return(SMS_NOT_UNIQUE);

    result = (exists) ? "true" : "false";
    (*action)(1, &result, actarg);
    return(SMS_SUCCESS);
##}


/* implements the get lists of administrator query.  It's fairly 
 * straightforward, but too complex for the regular query table.
 * First retrieve any lists with a USER acl which matches the 
 * specified user.  Then retrieve any lists with an acl which is a
 * list which has the specified user as a member.
 */

get_lists_of_administrator(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  int user;
##  char name[33];
    char *args;

##  range of l is list
##  range of m is members
    user  = *(int *) argv[0];
    args = name;

##  repeat retrieve (name = l.#name) 
##	where l.acl_type = "USER" and l.acl_id = @user {
	    (*action)(1, &args, actarg);
##  }

##  repeat retrieve (name = l.#name)
##	where l.acl_type = "LIST" and l.acl_id = m.#list_id
##	    and m.member_type = "USER" and m.member_id = @user {
	    (*action)(1, &args, actarg);
##  }
    return(SMS_SUCCESS);
##}


/**
 ** Setup routine for add_group
 **
 ** Inputs: none
 **
 ** Description: allocate next gid and store in values table
 **
 **/

setup_add_group(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int ngid, exists, rowcount, list_id;
    int status;

##  range of l is list
##  range of v is values
    list_id = *(int *)argv[0];

##  repeat retrieve (exists = l.group) where l.#list_id = @list_id
    if (exists)
	return(SMS_EXISTS);

##  repeat retrieve (ngid = v.value) where v.name = "gid"
    exists = 1;
    while (exists) {
	ngid++;
##      repeat retrieve (exists = any(l.#gid where l.#gid = @ngid))
    }

##  repeat replace v (value = @ngid) where v.name = "gid"
##  inquire_equel (rowcount = "rowcount")
    if (rowcount != 1) return SMS_INGRES_ERR;
    else return(SMS_SUCCESS);
##}

/**
 ** get_groups_of_user - optimized query for retrieval of all groups to
 **                      which a user belongs
 **
 **/

get_groups_of_user(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  int users_id;
##  char list_name[33];
##  char gid[11];
##  int rowcount;
    char *targv[2];

    users_id = *(int *)argv[0];
    targv[0] = list_name;
    targv[1] = gid;

##  range of m is members
##  range of l is list

##  repeat retrieve (list_name = l.name, gid = text(l.#gid))
##         where m.member_id = @users_id and m.member_type = "USER" and
##               m.list_id = l.list_id and l.group != 0
##         sort by #list_name
##  {
	 (*action)(2, targv, actarg);
##  }
##  inquire_equel (rowcount = "rowcount")

    return ((rowcount = 0) ? SMS_NO_MATCH : SMS_SUCCESS);
##}

get_groups_of_all_users(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  char login[9];
##  char group[33];
##  char gid[11];
    char *targv[3];
##  int errorno;

    targv[0] = login;
    targv[1] = group;
    targv[2] = gid;

##  range of u is users
##  range of l is list
##  range of m is members

##  set lockmode session where readlock = nolock

##  repeat retrieve (login = u.#login, group = l.name, gid = text(l.#gid))
##         where m.member_type = "USER" and m.member_id = u.users_id and
##               u.status != 0 and m.list_id = l.list_id and l.group != 0
##         sort by #login, #group
##  {
	 (*action)(3, targv, actarg);
##  }

##  inquire_equel (errorno = "errorno")
##  set lockmode session where readlock = system

    return((errorno) ? SMS_INGRES_ERR : SMS_SUCCESS);
##}


/* expand_list_flags - takes the flag value stuffed into list.active of
 * 	the list just created, and expands that value into hidden & public,
 *	then sets the modtime on the list.
 */
expand_list_flags(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  int id, flags, active, public, hidden, who;
##  char *entity;

    if (!strcmp(q->shortname, "ulis")) {
	id = *(int *)argv[0];
    } else {
##    repeat retrieve (id = values.value) where values.name = "list_id"
    }

##  repeat retrieve (flags = l.#active) where l.list_id = @id
    active = flags & 1;
    public = (flags & 2) >> 1;
    hidden = (flags & 4) >> 2;
    entity = cl->entity;
    who = cl->users_id;
##  repeat replace l (#active = @active, #public = @public, #hidden = @hidden,
##	modtime = "now", modby = @who, modwith = @entity)
##	 where l.list_id = @id
    return(SMS_SUCCESS);
##}


/**
 ** add_new_quota
 ** delete_current_quota - adjust nfsphys values on xxx_quota queries.
 **
 ** Inputs:
 **   argv[0] - mach_id
 **   argv[1] - device
 **   argv[2] - users_id
 **   argv[3] - quota (add_new_quota only)
 **
 ** Description:
 **   delete_current_quota:
 **     - find nfsquota entry
 **     - decrement nfsphys.allocated by nfsquota.quota
 **   add_new_quota
 **     - increment nfsphys.allocated by quota
 **
 **/

add_new_quota(q, argv, cl)
    struct query *q;
    register char *argv[];
    client *cl;
##{
##  int mach_id;
##  char *device;
##  int quota;

    mach_id = *(int*)argv[0];
    device = argv[1];
    quota = *(int *)argv[3];

##  range of np is nfsphys
##  repeat replace np (allocated = np.allocated + @quota)
##      where np.#mach_id = @mach_id and np.#device = @device

    return(SMS_SUCCESS);
##}

delete_current_quota(q, argv, cl)
    struct query *q;
    register char *argv[];
    client *cl;
##{
##  int mach_id;
##  int users_id;
##  char *device;
##  int quota;

    mach_id = *(int *)argv[0];
    device = argv[1];
    users_id = *(int *)argv[2];

##  range of np is nfsphys
##  range of nq is nfsquota
##  repeat retrieve (quota = nq.#quota) 
##      where nq.#mach_id = @mach_id and nq.#device = @device and
##            nq.#users_id = @users_id
##  repeat replace np (allocated = np.allocated - @quota)
##      where np.#mach_id = @mach_id and np.#device = @device
 
    return(SMS_SUCCESS);
##}


/**
 ** delete_locker - special query routine for deleting a user locker
 **
 ** Inputs:
 **   argv[0] - users_id
 ** 
 ** Description:
 **   - get login name from users_id
 **   - get filesys entry from login
 **   - use filesys.mach_id and filesys.name to determine machine/device
 **     pair for nfsphys and nfsquota
 **   - delete filesys entry (label=<login>)
 **   - decrement allocated in nfsphys by quota
 **   - delete nfsquota entry
 **
 ** Errors:
 **   - SMS_FILESYS - no filesys exists for user
 ** 
 **/

delete_locker(q, argv)
    register struct query *q;
    register char *argv[];
##{
##  int users_id;
##  int mach_id;
##  int quota;
##  int rowcount;
##  char login[9];
##  char lname[64];
##  char ndev[32];
    char *rindex();
    register char *c;

    /* copy arguments */
    users_id = *(int *)argv[0];

##  range of u is users
##  range of f is filesys
##  range of np is nfsphys
##  range of nq is nfsquota
##  range of tbs is tblstats

    /* get login name */
##  repeat retrieve (login = u.#login) where u.#users_id = @users_id

    /* get mach_id and locker name from filesys entry; then delete it */
##  repeat retrieve (mach_id = f.#mach_id, lname = f.#name)
##         where f.#label = @login
##  inquire_equel (rowcount = "rowcount")
    if (rowcount == 0) return(SMS_FILESYS);
##  repeat delete f where f.#label = @login

    /* get prefix directory */
    c = rindex(lname, '/');
    *c = 0;

    /* get nfs device */
##  repeat retrieve (ndev = np.device) 
##         where np.#mach_id = @mach_id and np.dir = @lname

    /* get quota from nfsquota entry; then delete entry */
##  repeat retrieve (quota = nq.#quota)
##         where nq.#mach_id = @mach_id and nq.#device = @ndev and
##               nq.#users_id = @users_id
##  repeat delete nq where nq.#mach_id = @mach_id and nq.#device = @ndev and
##                  nq.#users_id = @users_id

    /* decrement nfsphys.allocated */
##  repeat replace np (allocated = np.allocated - @quota)
##         where np.#mach_id = @mach_id and np.#device = @ndev

    /* adjust table statistics */
##  repeat replace tbs (deletes = tbs.deletes + 1, modtime = "now")
##             where tbs.table = "filesys"
##  repeat replace tbs (updates = tbs.updates + 1, modtime = "now")
##             where tbs.table = "nfsphys"
##  repeat replace tbs (deletes = tbs.deletes + 1, modtime = "now")
##             where tbs.table = "nfsquota"

    return(SMS_SUCCESS);
##}
