/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v $
 *	$Author: mike $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.10 1987-08-25 15:56:54 mike Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
Revision 1.9  87/08/22  17:41:34  wesommer
More of Mike's changes. 

Revision 1.8  87/08/18  15:05:20  wesommer
Fixed definition of add_locker.

Revision 1.7  87/08/04  01:49:41  wesommer
Rearranged messages.

Revision 1.6  87/08/04  01:10:02  wesommer
Changes by mike; checked in prior to my hacking.

Revision 1.5  87/07/30  14:54:13  wesommer
Added debugging code in an attempt to catch a flakey problem.

Revision 1.4  87/07/30  00:30:21  wesommer
replaced appends = appends+1 with appends = tbs.appends+1

Revision 1.3  87/07/30  00:26:11  wesommer
Changes by mike prior to "appends" fix.

Revision 1.2  87/07/29  16:00:39  wesommer
Fixed add_locker.

Revision 1.1  87/07/29  15:13:57  wesommer
Initial revision

 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.10 1987-08-25 15:56:54 mike Exp $";
#endif lint

#include "query.h"
#include "sms_server.h"
#include <ctype.h>

#define SMS_SUCCESS 0

extern char *whoami;

/* Specialized Access Routines */

/**
 ** access_user - verify that client name equals specified login name
 **
 ** Used by: update_user_shell
 **          update_finger_by_login
 **
 **  - since field validation routines are called first, a users_id is
 **    now in argv[0] instead of the login name.  Therefore, we must 
 **    convert the client name to a users_id.
 **/

access_user(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
    register struct krbname *krb;
##  int client_id;
##  char *client_name;
##  int rowcount;

    client_name = cl->kname.name;
##  repeat retrieve (client_id = users.users_id) 
##         where users.login = @client_name
##  inquire_equel (rowcount = "rowcount")
    if (rowcount != 1) return(SMS_PERM);
    if (client_id != *(int *)argv[0]) return(SMS_PERM);

    return(SMS_SUCCESS);
##}
    
/**
 ** access_pop - same as access_user plus verifies that a user has only one
 **		 mailbox of type "POP"
 **
 ** Inputs:
 **   argv[0] - users_id
 **   argv[1] - type
 **   argv[2] - mach_id
 **   argv[3] - box
 **
 ** Description:
 **   - if q->name = "add_pobox" and type = "POP", 
 **        verify that no POP box already exists for user
 **   - call access_user
 **
 **/

access_pop(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int users_id;
##  int mach_id;
##  char *box;
##  int exists;

    if (!bcmp(q->name, "add_pobox", 10) && !bcmp(argv[1], "POP", 4)) {
	users_id = *(int *)argv[0];
	mach_id = *(int *)argv[2];
	box = argv[3];
##      range of p is pobox
##      repeat retrieve (exists = any(p.#box where p.#users_id = @users_id
##				      and p.type = "POP" 
##				      and p.#mach_id = @mach_id
##				      and p.#box = @box))
	if (exists) return(SMS_EXISTS);
    }

    return(access_user(q, argv, cl));
##}

/**
 ** access_list - check access for adding or deleting list members
 **
 ** Inputs: argv[0] - list_id
 **         cl->krb.name - client name
 **
 ** - check that client is a member of the access control list
 ** - OR, if q->shortname == {amtl | dfml} and
 **       if list.flags & LF_PUBLIC, allow access if client = member
 **
 **/ 

access_list(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int list_id;
##  int acl_id;
##  int flags;
    int member_id;
    char *client_type;
    int client_id;
    int status;
    int exists;

    list_id = *(int *)argv[0];
##  repeat retrieve (acl_id = list.#acl_id, flags = list.#flags) 
##	   where list.#list_id = @list_id

    /* parse client structure */
    status = get_client(cl, &client_type, &client_id);
    if (status != SMS_SUCCESS) return(status);

    /* if amtl or dmfl and list is public allow client to add or delete self */
    if (!bcmp("amtl", q->shortname, 4) || !bcmp("dmfl", q->shortname, 4)) {
	if ((flags & LF_PUBLIC) && !bcmp("USER", argv[1], 4)) {
	    member_id = *(int *)argv[2];
	    if (member_id == client_id) return(SMS_SUCCESS);
	}
    }

    /* check for client in access control list */
    exists = find_member(acl_id, client_type, client_id, 0);
    if (!exists) return(SMS_PERM);

    return(SMS_SUCCESS);
##}

/**
 ** access_maillist - access_list + disallow adding user-group to maillists
 **
 ** Inputs:
 **   argv[0] - list_id
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

    status = access_list(q, argv, cl);
    if (status != SMS_SUCCESS) return(status);
    if (bcmp(q->name, "add_maillist", 12)) return(status);

    list_id = *(int *)argv[0];
##  range of g is groups
##  repeat retrieve (exists = any(g.#list_id where g.#list_id = @list_id))
    if (!exists) return(SMS_SUCCESS);
##  repeat retrieve (list_name = list.name) where list.#list_id = @list_id
##  repeat retrieve (exists = any(users.login where users.login = @list_name))
    return ((exists) ? SMS_USER_GROUP : SMS_SUCCESS);
##}

/**
 ** Setup routine for add_group
 **
 ** Inputs: none
 **
 ** Description: allocate next gid and store in values table
 **
 **/

setup_add_group(q, argv, cl, access_check)
    struct query *q;
    char *argv[];
    client *cl;
    int access_check;
##{
##  int ngid;
##  int exists;
    int status;

    status = access_list(q, argv, cl);

    if (status != SMS_SUCCESS || access_check) return(status);

##  range of g is groups
##  range of v is values
##  repeat retrieve (ngid = v.value) where v.name = "gid"
    exists = 1;
    while (exists) {
	ngid++;
##      repeat retrieve (exists = any(g.#gid where g.#gid = @ngid))
    }

##  repeat replace v (value = @ngid) where v.name = "gid"
    return(SMS_SUCCESS);
##}

/**
 ** Setup routine for add_user
 **
 ** Inputs: argv[0] - login
 **         argv[1] - uid
 **
 ** Description:
 **
 ** - if argv[1] == "#" then set argv[1] = next(uid)
 ** - if argv[0] == "#" then set argv[0] = "#<uid>"
 **
 **/

setup_add_user(q, argv, cl, access_check)
    struct query *q;
    register char *argv[];
    client *cl;
    int access_check;
##{
##  int nuid;
##  int exists;

    if (access_check) return(SMS_SUCCESS);

    if (!bcmp(argv[1], "#", 2)) {
##      range of u is users
##      range of v is values
##      repeat retrieve (nuid = v.value) where v.name = "uid"
	exists = 1;
	while (exists) {
	    nuid++;
##          repeat retrieve (exists = any(u.#uid where u.#uid = @nuid))
	}
##	repeat replace v (value = @nuid) where v.name = "uid"
	sprintf(argv[1], "%d", nuid);
    }

    if (!bcmp(argv[0], "#", 2)) {
	sprintf(argv[0], "#%s", argv[1]);
    }

    return(SMS_SUCCESS);
##}

/**
 ** followup_add_user - add finger entry, set_user_modtime
 ** followup_delete_user - delete finger entry
 **
 ** Inputs:
 **   argv[0] - login (add_user)
 **   argv[0] - users_id (delete_user)
 **
 **/

followup_add_user(q, argv)
    struct query *q;
    char *argv[];
##{
##  char *login;
##  int users_id;
##  char first[33];
##  char middle[33];
##  char last[33];
##  char fullname[128];
    register char *cp1;
    register char *cp2;

    login = argv[0];

    /* get user information */
##  range of u is users
##  repeat retrieve (users_id = u.#users_id, last = u.#last,
##                   first = u.#first, middle = u.#middle) 
##         where u.#login = @login

    /* build fullname */
    cp2 = fullname;
    cp1 = first;
    while (*cp1) *cp2++ = *cp1++;
    *cp2++ = ' ';
    cp1 = middle;
    if (*cp1 == 0) cp2--;
    while (*cp1) *cp2++ = *cp1++;
    *cp2++ = ' ';
    cp1 = last;
    while (*cp2++ = *cp1++) ;

    /* create a finger entry */
##  repeat append finger (#users_id = @users_id, #fullname = @fullname)

    /* set modtime (creation time) on user */
##  repeat replace u (modtime = "now") where u.#users_id = @users_id

    return(SMS_SUCCESS);
##}

followup_delete_user(q, argv)
    struct query *q;
    char *argv[];
##{
##  int users_id;

    users_id = *(int *)argv[0];
##  repeat delete finger where finger.#users_id = @users_id
    return(SMS_SUCCESS);
##}

/**
 ** setup_add_filesys - verify existance of referenced file systems
 ** setup_update_filesys - same, except argv[1..5] --> argv[2..6]
 **
 ** Inputs:     Add         Update
 **   argv[0] - label       label
 **   argv[1] - type        new label
 **   argv[2] - mach_id     type
 **   argv[3] - name        mach_id
 **   argv[4] - mount       name
 **   argv[5] - access      mount
 **   argv[6] -             access
 **
 ** Description:
 **   - for type = RVD:
 **        * verify mach_id/name in rvdvirt
 **        * verify access in {r, x, R, X}
 **   - for type = NFS:
 **        * extract directory prefix from name
 **        * verify mach_id/dir in nfsphys
 **        * verify access in {r, w, R, W}
 **
 ** Errors:
 **   SMS_RVD - no such rvd
 **   SMS_NFS - specified directory not exported
 **   SMS_FILESYS_ACCESS - invalid filesys access
 **
 **/

setup_add_filesys(q, argv)
    struct query *q;
    char *argv[];
{
    char *type;
    int mach_id;
    char *name;
    char *access;  

    type = argv[1];
    mach_id = *(int *)argv[2];
    name = argv[3];
    access = argv[5];

    if (!bcmp(type, "RVD", 3))
	return (check_rvd(mach_id, name, access));
    else if (!bcmp(type, "NFS", 3))
	return (check_nfs(mach_id, name, access));
    else
	return(SMS_SUCCESS);
}

setup_update_filesys(q, argv)
    struct query *q;
    char *argv[];
{
    char *type;
    int mach_id;
    char *name;
    char *access;  

    type = argv[2];
    mach_id = *(int *)argv[3];
    name = argv[4];
    access = argv[6];

    if (!bcmp(type, "RVD", 3))
	return (check_rvd(mach_id, name, access));
    else if (!bcmp(type, "NFS", 3))
	return (check_nfs(mach_id, name, access));
    else
	return(SMS_SUCCESS);
}

##check_rvd(mach_id, name, access)
##  int mach_id;
##  char *name;
    char *access;
##{
##  int rowcount;
    char caccess;

##  range of rv is rvdvirt
##  retrieve (rowcount = any(rv.#name where rv.#mach_id = mach_id and
##                           rv.#name = name))
    if (rowcount == 0) return(SMS_RVD);

    caccess = (isupper(*access)) ? tolower(*access) : *access;
    if (caccess != 'r' && caccess != 'x') return(SMS_FILESYS_ACCESS);

    return(SMS_SUCCESS);
##}

##check_nfs(mach_id, name, access)
##  int mach_id;
    char *name;
    char *access;
##{
##  int rowcount;
##  char dir[32];
    char caccess;
    register char *cp1;
    register char *cp2;

    caccess = (isupper(*access)) ? tolower(*access) : *access;
    if (caccess != 'r' && caccess != 'w') return(SMS_FILESYS_ACCESS);

##  range of np is nfsphys
##  retrieve (dir = np.#dir) where np.#mach_id = mach_id
##  {
	 cp1 = name;
	 cp2 = dir;
	 while (*cp2) {
	     if (*cp1++ != *cp2) break;
	     cp2++;
	 }
	 if (*cp2 == 0) return(SMS_SUCCESS);
##  }

    return(SMS_NFS);
##}

/* Followup Routines */

set_user_modtime(q, argv)
    struct query *q;
    char *argv[];
##{
##  char *login;

    login = argv[0];
##  repeat replace u (modtime = "now") where u.#login = @login
    return(SMS_SUCCESS);
##}

set_user_modtime_by_id(q, argv)
    struct query *q;
    char *argv[];
##{
##  int users_id;

    users_id = *(int *)argv[0];
##  repeat replace users (modtime = "now") where users.#users_id = @users_id
    return(SMS_SUCCESS);
##}

set_list_modtime(q, argv)
    struct query *q;
    char *argv[];
##{
##  char *list_name;

    list_name = argv[0];
##  repeat replace list (modtime = "now") where list.name = @list_name
    return(SMS_SUCCESS);
##}

set_list_modtime_by_id(q, argv)
    struct query *q;
    char *argv[];
##{
##  int list_id;

    list_id = *(int *)argv[0];
##  repeat replace list (modtime = "now") where list.#list_id = @list_id
    return(SMS_SUCCESS);
##}

set_finger_modtime(q, argv)
    struct query *q;
    char *argv[];
##{
##  int users_id;

    users_id = *(int *)argv[0];
##  repeat replace f (modtime = "now") where f.#users_id = @users_id
    return(SMS_SUCCESS);
##}

/**
 ** followup_amtl - followup for amtl and dmfl; when adding a list 
 **                 member to a maillist, add list to maillist table,
 **                 unless list is a user-group.
 **                 Then set_list_modtime_by_id.
 **
 ** Inputs:
 **   argv[0] - list_id
 **   argv[1] - member_type
 **   argv[2] - member_id
 **
 **/

followup_amtl(q, argv)
    struct query *q;
    char *argv[];
##{
##  int list_id;
##  int member_id;
##  int exists;
##  char list_name[33];

    list_id = *(int *)argv[0];

##  repeat replace list (modtime = "now") where list.#list_id = @list_id

    /* if query is not amtl or if member_type is not LIST then return */
    if (bcmp(q->shortname, "amtl", 4) || bcmp(argv[1], "LIST", 4)) 
	return(SMS_SUCCESS);

    member_id = *(int *)argv[2];
##  range of l is list
##  range of ml is maillists
##  range of g is groups

    /* is parent list a mailing list? */
##  repeat retrieve (exists = any(ml.#list_id where ml.#list_id=@list_id))

    /* if not then return */
    if (!exists) return(SMS_SUCCESS);

    /* is member_list a user-group? */
    /* is it a group? */
##  repeat retrieve (exists = any(g.#list_id where g.#list_id = @member_id))
    if (exists) {
	/* get list_name */
##      repeat retrieve (list_name = l.#name) where l.#list_id = @member_id
	/* is list_name a username? */
##      repeat retrieve (exists = any(users.login 
##				      where users.login = @list_name))
	/* yes, return error */
	if (exists) return(SMS_USER_GROUP);
    }

    /* list is not a user-group; add list to maillist table */
##  repeat append maillists (#list_id = @member_id, ltid = l.tid)
##         where l.#list_id = @member_id

    return(SMS_SUCCESS);
##}

/**
 ** followup_add_pobox
 ** followup_delete_pobox - followup routines for pobox queries
 **
 ** Description:
 **   add_pobox: set pobox creation time
 **              increment pop usage in serverhosts
 **
 **   delete_pobox: decrement pop usage in serverhosts
 **
 **/

followup_add_pobox(q, argv)
    struct query *q;
    char *argv[];
{
    set_pobox_creation(q, argv);
    set_pop_usage(q, argv, 1);
    return(SMS_SUCCESS);
}

followup_delete_pobox(q, argv)
    struct query *q;
    char *argv[];
{
    set_pop_usage(q, argv, -1);
    return(SMS_SUCCESS);
}

set_pobox_creation(q, argv)
    struct query *q;
    char *argv[];
##{
##  int users_id;
##  int mach_id;
##  char *type;
##  char *box;

    users_id = *(int *)argv[0];
    type = argv[1];
    mach_id = *(int *)argv[2];
    box = argv[3];

##  range of p is pobox
##  repeat replace p (created = "now") 
##         where p.#users_id = @users_id and p.#type = @type and
##               p.#mach_id = @mach_id and p.#box = @box

    return (SMS_SUCCESS);
##}

/**
 ** set_pop_usage - incr/decr usage count for pop server in serverhosts talbe
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

set_pop_usage(q, argv, count)
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

/**
 ** delete_user_poboxes - delete all poboxes for a user
 **
 ** Inputs:
 **   argv[0] - users_id
 **
 **/

delete_user_poboxes(q, argv)
    struct query *q;
    char *argv[];
##{
##  int users_id;
##  int n;
##  int mach_id;
    register int i;
    int mach_ids[10];

    users_id = *(int *)argv[0];

    /* get machine ids for pop server(s) on which the user currently exists */
##  range of p is pobox
    i = 0;
##  repeat retrieve (mach_id = p.#mach_id) 
##         where p.#users_id = @users_id and p.type = "POP"
##  {
	 mach_ids[i++] = mach_id;
	 if (i == 10) {
##           endretrieve
	 }
##  }

    /* decrement counts on serverhost entries */
##  range of sh is serverhosts
    while (--i >= 0) {
	mach_id = mach_ids[i];
##      repeat replace sh (value1 = sh.value1 - 1)
##             where sh.service = "pop" and sh.#mach_id = @mach_id
    }

    /* delete user's poboxes */
##  repeat delete p where p.#users_id = @users_id

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

add_new_quota(q, argv)
    struct query *q;
    register char *argv[];
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

delete_current_quota(q, argv, cl, access_check)
    struct query *q;
    register char *argv[];
    client *cl;
    int access_check;
##{
##  int mach_id;
##  int users_id;
##  char *device;
##  int quota;

    if (access_check) return(SMS_SUCCESS);

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
 ** add_hostaccess - create entry in hostaccess table upon adding a new
 **                  machine to the serverhosts table where service =
 **                  "hostaccess".
 **
 ** Inputs:
 **   argv[0] - service
 **   argv[1] - mach_id
 **
 **/

add_hostaccess(q, argv)
    struct query *q;
    char *argv[];
##{
##  int mach_id;

    /* only work with service = "hostaccess" */
    if (bcmp(argv[0], "hostaccess", 10)) return(SMS_SUCCESS);

    mach_id = *(int *)argv[1];
##  repeat append hostaccess (#mach_id = @mach_id, status = 0)
##  repeat replace tblstats (modtime = "now", appends = tblstats.appends + 1)
##         where tblstats.table = "hostaccess"
    return(SMS_SUCCESS);
##}

/* followup to delete_server_host_info */

delete_hostaccess(q, argv)
    struct query *q;
    char *argv[];
##{
##  int mach_id;

    /* only work with service = "hostaccess" */
    if (bcmp(argv[0], "hostaccess", 10)) return(SMS_SUCCESS);

    mach_id = *(int *)argv[1];
##  repeat delete hostaccess where hostaccess.#mach_id = @mach_id
##  repeat replace tblstats (modtime = "now", deletes = tblstats.deletes + 1)
##         where tblstats.table = "hostaccess"
    return(SMS_SUCCESS);
##}

followup_ushp(q, argv)
    struct query *q;
    char *argv[];
##{
##  int mach_id;
##  int status;

    mach_id = *(int *)argv[0];
##  range of ha is hostaccess
##  repeat retrieve (status = ha.#status) where ha.#mach_id = @mach_id
    status |= 1;
##  repeat replace ha (#status = @status) where ha.#mach_id = @mach_id
    return(SMS_SUCCESS);
##}

followup_usha(q, argv)
    struct query *q;
    char *argv[];
##{
##  int mach_id;
##  int status;

    mach_id = *(int *)argv[0];
##  range of ha is hostaccess
##  repeat retrieve (status = ha.#status) where ha.#mach_id = @mach_id
    status |= 2;
##  repeat replace ha (#status = @status) where ha.#mach_id = @mach_id
    return(SMS_SUCCESS);
##}

/**
 ** delete_list_members - called after the delete_list query to clean up
 **			  members table.
 **
 ** Inputs: argv[0] - list_id
 **
 ** Description:
 **   - foreach string member: decr string refc; ifzero, delete string
 **   - delete all members entries for this list_id
 **
 **/

delete_list_members(q, argv)
    struct query *q;
    register char *argv[];
##{
##  int list_id;
##  int string_id;
##  int refc;
##  int rowcount;
    struct save_queue *sq;
    struct save_queue *sq_create();

    list_id = *(int *)argv[0];
    sq = sq_create();

##  range of m is members
##  repeat retrieve (string_id = m.member_id)
##         where m.#list_id = @list_id and m.member_type = "STRING"
##  {
	 sq_save_data(sq, string_id);
##  }

    while (sq_get_data(sq, &string_id)) {
##	range of s is strings
##	repeat retrieve (refc = s.#refc) where s.#string_id = @string_id
##      inquire_equel (rowcount = "rowcount")
	if (rowcount == 0) continue;
	if (--refc == 0) {
##	    repeat delete s where s.#string_id = @string_id
	} else {
##	    repeat replace s (#refc = @refc) where s.#string_id = @string_id
	}
    }
    sq_destroy(sq);

##  repeat delete m where m.#list_id = @list_id

    return(SMS_SUCCESS);
##}

/**
 ** followup_grvd - Support routine for get_rvd_servers query
 **
 ** Inputs:
 **    q     - grvd query structure
 **    sq    - save_queue struture: contains list of {machine, oper_acl_id,
 **            admin_acl_id, shutdown_acl_id} records.
 **    v     - validate structure (not used)
 **    action - action routine
 **    actarg - action routine argument
 **
 ** Description:
 **   - translate acl_ids to list names
 **
 **/

followup_grvd(q, sq, v, action, actarg)
    struct query *q;
    struct save_queue *sq;
    struct validate *v;
    int (*action)();
    int actarg;
##{
    char **argv;
    char *targv[4];
##  char oper[33];
##  char admin[33];
##  char shutdown[33];
##  int list_id;

    targv[0] = oper;
    targv[1] = admin;
    targv[2] = shutdown;

##  range of l is list

    while (sq_get_data(sq, &argv)) {
	sscanf(argv[0], "%d", &list_id);
##      repeat retrieve (oper = l.name) where l.#list_id = @list_id
	sscanf(argv[1], "%d", &list_id);
##      repeat retrieve (admin = l.name) where l.#list_id = @list_id
	sscanf(argv[2], "%d", &list_id);
##      repeat retrieve (shutdown = l.name) where l.#list_id = @list_id
	
	(*action)(3, targv, actarg);
	free(argv[0]);
	free(argv[1]);
	free(argv[2]);
    }

    sq_destroy(sq);
    return(SMS_SUCCESS);
##}

followup_gars(q, sq, v, action, actarg)
    struct query *q;
    struct save_queue *sq;
    struct validate *v;
    int (*action)();
    int actarg;
##{
    char **argv;
    char *targv[4];
##  char oper[33];
##  char admin[33];
##  char shutdown[33];
##  int list_id;

    targv[1] = oper;
    targv[2] = admin;
    targv[3] = shutdown;

##  range of l is list

    while (sq_get_data(sq, &argv)) {
	sscanf(argv[1], "%d", &list_id);
##      repeat retrieve (oper = l.name) where l.#list_id = @list_id
	sscanf(argv[2], "%d", &list_id);
##      repeat retrieve (admin = l.name) where l.#list_id = @list_id
	sscanf(argv[3], "%d", &list_id);
##      repeat retrieve (shutdown = l.name) where l.#list_id = @list_id
	
	targv[0] = argv[0];
	(*action)(4, targv, actarg);
	free(argv[0]);
	free(argv[1]);
	free(argv[2]);
	free(argv[3]);
    }

    sq_destroy(sq);
    return(SMS_SUCCESS);
##}

/**
 ** set_next_object_id - set next object id in values table
 **
 ** Inputs: object - object name in values table
 **
 ** - called before an APPEND operation to set the next object id to
 **   be used for the new record
 **
 **/

set_next_object_id(object)
    char *object;
##{
##  char *name;

    name = object;
##  range of v is values
##  repeat replace v (value = v.value + 1) where v.#name = @name
    return(SMS_SUCCESS);
##}

/**
 ** get_query_need - check modtime of query's associated table against given
 **                  time and return true if greater (false if not)
 **
 ** Inputs:
 **   argv[0] - query name
 **   argv[1] - time to compare against
 **
 **/

get_query_need(q, argv, action, actarg)
    struct query *q;
    register char *argv[];
    int (*action)();
##{
    struct query *q1;
##  char *last_get_time;
##  char *table;
##  int need;
    char *result;
    struct query *get_query_by_name();

    q1 = get_query_by_name(argv[0]);

    last_get_time = argv[1];
    table = q1->rtable;

    if (q1->type != RETRIEVE) return(SMS_NO_MATCH);

##  range of tbs is tblstats
##  repeat retrieve (need = any(tbs.modtime where tbs.#table = @table and
##				tbs.modtime > @last_get_time))
    
    result = (need) ? "true" : "false";
    (*action)(1, &result, actarg);
    return(SMS_SUCCESS);
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

get_list_is_group(q, argv, action, actarg)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
##{
##  int exists;
##  int list_id;
    char *result;

    list_id = *(int *)argv[0];

##  range of g is groups
##  repeat retrieve (exists = any(g.#list_id where g.#list_id = @list_id))

    result = (exists) ? "true" : "false";
    (*action)(1, &result, actarg);
    return(SMS_SUCCESS);
##}

get_list_is_maillist(q, argv, action, actarg)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
##{
##  int exists;
##  int list_id;
    char *result;

    list_id = *(int *)argv[0];

##  range of ml is maillists
##  repeat retrieve (exists = any(ml.#list_id where ml.#list_id = @list_id))

    result = (exists) ? "true" : "false";
    (*action)(1, &result, actarg);
    return(SMS_SUCCESS);
##}


/**
 ** add_locker - special query routine for creating a user locker
 **
 ** Inputs:
 **   argv[0] - users_id
 **   argv[1] - machine_id
 **   argv[2] - device
 **   argv[3] - initial quota
 ** 
 ** Description:
 **   - get prefix directory (dir) for mount point on specified machine/device
 **   - create filesys entry (label=<login>, type=NFS, machine=<machine>,
 **     mount=<dir>/<login>, access=w, acl=dbadmin)
 **   - increment allocated in nfsphys by quota
 **   - create nfsquota entry
 **
 ** Errors:
 **   - SMS_NFSPHYS - machine/device does not exist in nfsphys
 **   - SMS_FILESYS_EXISTS - file system already exists
 ** 
 **/

add_locker(q, argv)
    register struct query *q;
    char *argv[];
##{
##  int users_id;
##  int mach_id;
##  char *device;
##  int quota;
##  int rowcount;
##  char login[9];
##  char dir[32];
##  int allocated;
##  char locker[64];
##  char mount[64];
##  int user_acl;

    /* copy arguments */
    users_id = *(int *)argv[0];
    mach_id = *(int *)argv[1];
    device = argv[2];
    sscanf(argv[3], "%d", &quota);

##  range of u is users
##  range of f is filesys
##  range of np is nfsphys
##  range of tbs is tblstats

    /* get login name */
##  repeat retrieve (login = u.#login) where u.#users_id = @users_id

    /* get user's acl id */
##  repeat retrieve (user_acl = list.list_id) where list.name = @login

    /* get filesystem directory prefix; give error if machine/device
       pair not in nfsphys table */
    printf("np.mach_id = %d and np.device = %s\n", mach_id, device);
    
##  repeat retrieve (dir = np.#dir, allocated = np.#allocated) 
##         where np.#mach_id = @mach_id and np.#device = @device
##  inquire_equel (rowcount = "rowcount")
    if (rowcount == 0) return(SMS_NFSPHYS);

    /* make sure a filesys with user's name does not already exist */
##  repeat retrieve (rowcount = any(f.label where f.label = @login))
    if (rowcount != 0) return(SMS_FILESYS_EXISTS);

    /* create a new filesys */
    sprintf(locker, "%s/%s", dir, login);
    sprintf(mount, "/mit/%s", login);
##  repeat append filesys 
##             (#label = @login, type = "NFS", #mach_id = @mach_id,
##		name = @locker, access = "w", order = 1, #mount = @mount,
##		acl_id = @user_acl)
##  repeat replace tbs (appends = tbs.appends + 1, modtime = "now")
##             where tbs.table = "filesys"

    /* increment usage count in nfsphys table */
    allocated += quota;
##  replace np (#allocated = allocated) 
##	    where np.#mach_id = mach_id and np.#device = device
##  repeat replace tbs (updates = tbs.updates + 1, modtime = "now")
##             where tbs.table = "nfsphys"

    /* create nfsquota entry */
##  append nfsquota (#users_id = users_id, #mach_id = mach_id,
##		     #device = device, #quota = quota)
##  repeat replace tbs (appends = tbs.appends + 1, modtime = "now")
##             where tbs.table = "nfsquota"

    return(SMS_SUCCESS);
##}

/**
 ** delete_locker - special query routine for deleting a user locker
 **
 ** Inputs:
 **   argv[0] - users_id
 **   argv[1] - machine_id
 **   argv[2] - device
 **   argv[3] - quota
 ** 
 ** Description:
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
##  char *device;
##  int quota;
##  int rowcount;
##  char login[9];

    /* copy arguments */
    users_id = *(int *)argv[0];
    mach_id = *(int *)argv[1];
    device = argv[2];
    sscanf(argv[3], "%d", &quota);

##  range of u is users
##  range of f is filesys
##  range of np is nfsphys
##  range of nq is nfsquota
##  range of tbs is tblstats

    /* get login name */
##  repeat retrieve (login = u.#login) where u.#users_id = @users_id

    /* delete the filesys entry */
##  repeat delete f where f.label = @login
##  inquire_equel (rowcount = "rowcount")
    if (rowcount == 0) return(SMS_FILESYS);
##  repeat replace tbs (deletes = tbs.deletes + 1, modtime = "now")
##             where tbs.table = "filesys"

    /* decrement usage count in nfsphys table */
##  replace np (#allocated = np.#allocated - quota) 
##	    where np.#mach_id = mach_id and np.#device = device
##  repeat replace tbs (updates = tbs.updates + 1, modtime = "now")
##             where tbs.table = "nfsphys"

    /* delete nfsquota entry */
##  delete nq where nq.#users_id = users_id and nq.#mach_id = mach_id and
##                  nq.#device = device
##  repeat replace tbs (deletes = tbs.deletes + 1, modtime = "now")
##             where tbs.table = "nfsquota"

    return(SMS_SUCCESS);
##}

/**
 ** add_user_group - create a group for a user and add user to group
 **
 ** Inputs:
 **   argv[0] - login
 **
 ** Description:
 **   - verify specified user exists
 **   - create a list of same name as user
 **   - add user as a member of the list
 **
 **/

add_user_group(q, argv)
    struct query *q;
    char *argv[];
##{
##  char *login;
##  int exists;
##  int users_id;
##  int list_id;
##  int gid;

    login = argv[0];

    /* verify user exists */
##  repeat retrieve (users_id = users.#users_id) where users.#login = @login
##  inquire_equel (exists = "rowcount")
    if (exists != 1) return(SMS_USER);

    /* verify list does not exist */
##  repeat retrieve (exists = any(list.name where list.name = @login))
    if (exists) return(SMS_LIST);

    /* get new list_id */
##  repeat retrieve (list_id = values.value) where values.name = "list_id"
    list_id++;
##  repeat replace values (value = @list_id) where values.name = "list_id"

    /* create the list */
##  range of tbs is tblstats
##  repeat append list (name = @login, #list_id = @list_id, flags = 1,
##                      desc = "User Group", acl_id = @list_id,
##			expdate = "today" + "5 years", modtime = "now")
##  repeat replace tbs (modtime = "now", appends = tbs.appends + 1)
##         where tbs.table = "list"

    /* add user to list */
##  repeat append members (#list_id = @list_id, member_type = "USER",
##			   member_id = @users_id)
##  repeat replace tbs (modtime = "now", appends = tbs.appends + 1)
##         where tbs.table = "members"

    /* get new gid */
##  range of g is groups
##  range of v is values
##  repeat retrieve (gid = v.value) where v.name = "gid"
    exists = 1;
    while (exists) {
	gid++;
##      repeat retrieve (exists = any(g.#gid where g.#gid = @gid))
    }
##  repeat replace v (value = @gid) where v.name = "gid"

    /* add list to group table */
##  repeat append groups (#list_id = @list_id, ltid = list.tid, #gid = @gid)
##      where list.#list_id = @list_id
##  repeat replace tbs (modtime = "now", appends = tbs.appends + 1)
##         where tbs.table = "members"

    /* and we're done */
    return(SMS_SUCCESS);
##}


/**
 ** get_members_of_list - optimized query for retrieval of list members
 **
 ** Inputs:
 **   argv[0] - list_id
 **
 ** Description:
 **   - retrieve USER members, then LIST members, then STRING members
 **
 **/

get_members_of_list(q, argv, action, actarg)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
##{
##  int list_id;
##  char member_name[129];
    char *targv[2];

    list_id = *(int *)argv[0];
    targv[0] = "USER";
    targv[1] = member_name;

##  range of m is members
##  repeat retrieve (member_name = users.login)
##             where m.#list_id = @list_id and m.member_type = "USER"
##                   and m.member_id = users.users_id
##             sort by #member_name
##  {
	 (*action)(2, targv, actarg);
##  }

    targv[0] = "LIST";
##  repeat retrieve (member_name = list.name)
##             where m.#list_id = @list_id and m.member_type = "LIST"
##                   and m.member_id = list.#list_id
##             sort by #member_name
##  {
	 (*action)(2, targv, actarg);
##  }

    targv[0] = "STRING";
##  repeat retrieve (member_name = strings.string)
##             where m.#list_id = @list_id and m.member_type = "STRING"
##                   and m.member_id = strings.string_id
##             sort by #member_name
##  {
	 (*action)(2, targv, actarg);
##  }

    return(SMS_SUCCESS);
##}

/**
 ** get_groups_of_user - optimized query for retrieval of all groups to
 **                      which a user belongs
 **
 **/

get_groups_of_user(q, argv, action, actarg)
    struct query *q;
    char *argv[];
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

##  repeat retrieve (list_name = list.name, gid = text(groups.#gid))
##         where m.member_id = @users_id and m.member_type = "USER" and
##               m.list_id = groups.list_id and groups.ltid = list.tid
##         sort by #list_name
##  {
	 (*action)(2, targv, actarg);
##  }
##  inquire_equel (rowcount = "rowcount")

    return ((rowcount = 0) ? SMS_NO_MATCH : SMS_SUCCESS);
##}

get_groups_of_all_users(q, argv, action, actarg)
    struct query *q;
    char *argv[];
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
##  range of g is groups

##  repeat retrieve (login = u.#login, group = l.name, gid = text(g.#gid))
##         where m.member_type = "USER" and m.member_id = u.users_id and
##               u.status != 0 and m.list_id = g.list_id and
##               g.ltid = l.tid
##         sort by #login, #group
##  {
	 (*action)(3, targv, actarg);
##  }

##  inquire_equel (errorno = "errorno")

    return((errorno) ? SMS_INGRES_ERR : SMS_SUCCESS);
##}

/**
 ** get_all_poboxes - optimized query for retrieval of all poboxes
 **
 ** Description:
 **   - retrieve LOCAL boxes, then POP boxes, then FOREIGN boxes
 **
 **/

get_all_poboxes(q, argv, action, actarg)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
##{
##  char login[9];
##  char machine[129];
##  char box[129];
    char *targv[4];

    targv[0] = login;
    targv[2] = machine;
    targv[3] = box;

    targv[1] = "LOCAL";
##  range of p is pobox
##  repeat retrieve (login=users.#login, machine = #machine.name, box=p.#box)
##             where p.type = "LOCAL" and p.users_id = users.users_id
##                   and p.mach_id = #machine.mach_id
##  {
	 (*action)(4, targv, actarg);
##  }

    targv[1] = "POP";
##  repeat retrieve (login=users.#login, machine = #machine.name, box=p.#box)
##             where p.type = "POP" and p.users_id = users.users_id
##                   and p.mach_id = #machine.mach_id
##  {
	 (*action)(4, targv, actarg);
##  }

    targv[1] = "FOREIGN";
##  repeat retrieve (login=users.#login, machine=strings.string, box=p.#box)
##             where p.type = "FOREIGN" and p.users_id = users.users_id
##                   and p.mach_id = strings.string_id
##  {
	 (*action)(4, targv, actarg);
##  }

    return(SMS_SUCCESS);
##}

get_new_poboxes(q, argv, action, actarg)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
##{
##  char *created;
##  char login[9];
##  char machine[129];
##  char box[129];
    char *targv[4];

    created = argv[0];

    targv[0] = login;
    targv[2] = machine;
    targv[3] = box;

    targv[1] = "LOCAL";
##  range of p is pobox
##  repeat retrieve (login=users.#login, machine = #machine.name, box=p.#box)
##             where p.type = "LOCAL" and p.users_id = users.users_id
##                   and p.mach_id = #machine.mach_id and
##                   p.#created > @created
##  {
	 (*action)(4, targv, actarg);
##  }

    targv[1] = "POP";
##  repeat retrieve (login=users.#login, machine = #machine.name, box=p.#box)
##             where p.type = "POP" and p.users_id = users.users_id
##                   and p.mach_id = #machine.mach_id and
##                   p.#created > @created
##  {
	 (*action)(4, targv, actarg);
##  }

    targv[1] = "FOREIGN";
##  repeat retrieve (login=users.#login, machine=strings.string, box=p.#box)
##             where p.type = "FOREIGN" and p.users_id = users.users_id
##                   and p.mach_id = strings.string_id and
##                   p.#created > @created
##  {
	 (*action)(4, targv, actarg);
##  }

    return(SMS_SUCCESS);
##}

/* Validation Routines */

validate_row(q, argv, v)
    register struct query *q;
    char *argv[];
    register struct validate *v;
##{
##  char *rvar;
##  char *table;
##  char *name;
##  char qual[128];
##  int rowcount;

    /* build where clause */
    build_qual(v->qual, v->argc, argv, qual);

    /* setup ingres variables */
    rvar = q->rvar;
    table = q->rtable;
    name = v->field;

    if (log_flags & LOG_RES)
	/* tell the logfile what we're doing */
	com_err(whoami, 0, "validating row: %s", qual);
    
    /* look for the record */
##  range of rvar is table
##  retrieve (rowcount = count(rvar.name where qual))
    if (rowcount == 0) return(SMS_NO_MATCH);
    if (rowcount > 1) return(SMS_NOT_UNIQUE);
    return(SMS_EXISTS);
##}

validate_fields(q, argv, vo, n)
    struct query *q;
    register char *argv[];
    register struct valobj *vo;
    register int n;
{
    register int status;

    while (--n >= 0) {
	switch (vo->type) {
	case V_NAME:
	    if (log_flags & LOG_RES)
		com_err(whoami, 0, "validating %s in %s: %s", 
		    vo->namefield, vo->table, argv[vo->index]);
	    status = validate_name(argv, vo);
	    break;

	case V_ID:
	    if (log_flags & LOG_RES)
		com_err(whoami, 0, "validating %s in %s: %s", 
		    vo->idfield, vo->table, argv[vo->index]);
	    status = validate_id(argv, vo);
	    break;

	case V_DATE:
	    if (log_flags & LOG_RES)
		com_err(whoami, 0, "validating date: %s", argv[vo->index]);
	    status = validate_date(argv, vo);
	    break;

	case V_TYPE:
	    if (log_flags & LOG_RES)
		com_err(whoami, 0, "validating %s type: %s",
		    vo->table, argv[vo->index]);
	    status = validate_type(argv, vo);
	    break;

	case V_TYPEDATA:
	    if (log_flags & LOG_RES)
		com_err(whoami, 0, "validating typed data (%s): %s",
		    argv[vo->index - 1], argv[vo->index]);
	    status = validate_typedata(q, argv, vo);
	    break;

	case V_FOLLOWUP:
	    status = SMS_EXISTS;
	    break;

	case V_SORT:
	    status = SMS_EXISTS;
	    break;

	}

	if (status != SMS_EXISTS) return(status);
	vo++;
    }

    return(SMS_SUCCESS);
}

validate_id(argv, vo)
    char *argv[];
    register struct valobj *vo;
##{
##  char *name;
##  char *table;
##  char *namefield;
##  char *idfield;
##  int id;
##  int rowcount;

    name = argv[vo->index];
    table = vo->table;
    namefield = vo->namefield;
    idfield = vo->idfield;
##  retrieve (id = table.idfield) where table.namefield = name
##  inquire_equel (rowcount = "rowcount")
    if (rowcount != 1) return(vo->error);
    *(int *)argv[vo->index] = id;
    return(SMS_EXISTS);
##}

validate_name(argv, vo)
    char *argv[];
    register struct valobj *vo;
##{
##  char *name;
##  char *table;
##  char *namefield;
##  int rowcount;

    name = argv[vo->index];
    table = vo->table;
    namefield = vo->namefield;
##  retrieve (rowcount = countu(table.namefield 
##            where table.namefield = name))
    return ((rowcount == 1) ? SMS_EXISTS : vo->error);
##}

validate_date(argv, vo)
    char *argv[];
    struct valobj *vo;
##{
##  char *idate;
##  double dd;
##  int errorno;

    idate = argv[vo->index];

##  retrieve (dd = interval("years", date(idate) - date("today")))
##  inquire_equel (errorno = "errorno")
    if (errorno != 0 || dd > 5.0) return(SMS_DATE);
    return(SMS_SUCCESS);
##}

validate_type(argv, vo)
    char *argv[];
    register struct valobj *vo;
##{
##  char *typename;
##  char *value;
##  int rowcount;
    register char *c;

    typename = vo->table;
    value = argv[vo->index];

    /* uppercase type fields */
    for (c = value; *c; c++) if (islower(*c)) *c = toupper(*c);

##  range of a is alias
##  repeat retrieve (rowcount = count(a.trans where a.name = @typename and
##				      a.type = "TYPE" and
##				      a.trans = @value))
    return ((rowcount == 1) ? SMS_EXISTS : vo->error);
##}

/* validate member or type-specific data field */

validate_typedata(q, argv, vo)
    register struct query *q;
    register char *argv[];
    register struct valobj *vo;
##{
##  char *name;
##  char *field_type;
##  char data_type[17];
##  int id;
##  int refc;
##  int rowcount;
    register char *c;

    /* get named object */
    name = argv[vo->index];

    /* get field type string (known to be at index-1) */
    field_type = argv[vo->index-1];

    /* get corresponding data type associated with field type name */
##  repeat retrieve (data_type = alias.trans) 
##         where alias.#name = @field_type and alias.type = "TYPEDATA"
##  inquire_equel (rowcount = "rowcount")
    if (rowcount != 1) return(SMS_TYPE);

    /* now retrieve the record id corresponding to the named object */

    if (!strcmp(data_type, "user")) {
	/* USER */
##	repeat retrieve (id = users.users_id) where users.login = @name
##	inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) return(SMS_USER);

    } else if (!strcmp(data_type, "list")) {
	/* LIST */
##	repeat retrieve (id = list.list_id) where list.#name = @name
##	inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) return(SMS_LIST);

    } else if (!strcmp(data_type, "machine")) {
	/* MACHINE */
	for (c = name; *c; c++) if (islower(*c)) *c = toupper(*c);
##      repeat retrieve (id = machine.mach_id) where machine.#name = @name
##      inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) return(SMS_MACHINE);

    } else if (!strcmp(data_type, "string")) {
	/* STRING */
##      range of s is strings
##      repeat retrieve (id = s.string_id, refc = s.#refc) 
##             where s.string = @name
##      inquire_equel (rowcount = "rowcount")
	if (rowcount == 0) {
	    if (q->type != APPEND) return(SMS_STRING);
##          range of v is values
##          retrieve (id = v.value) where v.#name = "strings_id"
	    id++;
##          replace v (value = id) where v.#name = "strings_id"
##	    append to strings (string_id = id, string = name, #refc = 1)
	} else if (rowcount == 1) {
	    if (q->type == APPEND || q->type == DELETE) {
		refc += (q->type == APPEND) ? 1 : -1;
		if (refc > 0) {
##		    replace s (#refc = refc) where s.string_id = id
		} else {
##		    delete s where s.string_id = id
		}
	    }
	}
    } else {
	return(SMS_TYPE);
    }

    /* now set value in argv */
    *(int *)argv[vo->index] = id;
    
    return (SMS_EXISTS);
##}


translate_ids(q, sq, v, action, actarg)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
##{
##  char *name;
##  char *field_type;
##  char data_type[17];
##  int id;
##  int rowcount;
    register int i;
    struct valobj *vo;
    char **argv;

    for (i = 0; i < v->objcnt; i++) {
	vo = &v->valobj[i];
	if (vo->type == V_FOLLOWUP) break;
    }

    /* for each row */
    while (sq_get_data(sq, &argv)) {

	/* get object id */
	i = vo->index;
	sscanf(argv[i], "%d", &id);
	free(argv[i]);
	name = (char *)malloc(129);
	argv[i] = name;

	/* get field type string (known to be at index-1) */
	field_type = argv[vo->index-1];

	/* get corresponding data type associated with field type name */
##      repeat retrieve (data_type = alias.trans) 
##             where alias.#name = @field_type and alias.type = "TYPEDATA"
##      inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) {
	    sprintf(name, "%d", id);
	    (*action)(q->vcnt, argv, actarg);
	    continue;
	}

	/* retrieve object name */

	if (!strcmp(data_type, "user")) {
	    /* USER */
##          repeat retrieve (name = users.login) where users.users_id = @id
##	    inquire_equel (rowcount = "rowcount")

	} else if (!strcmp(data_type, "list")) {
	    /* LIST */
##          repeat retrieve (name = list.#name) where list.list_id = @id
##	    inquire_equel (rowcount = "rowcount")

	} else if (!strcmp(data_type, "machine")) {
	    /* MACHINE */
##          repeat retrieve (name = machine.#name) where machine.mach_id = @id
##          inquire_equel (rowcount = "rowcount")

	} else if (!strcmp(data_type, "string")) {
	    /* STRING */
##          repeat retrieve (name = strings.string) 
##                 where strings.string_id = @id
##          inquire_equel (rowcount = "rowcount")

	} else {
	    rowcount = 0;
	}

	/* if there wasn't a corresponding object name, then use the id */
	if (rowcount != 1) sprintf(name, "%d", id);

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

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
