/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.3 1987-07-30 00:26:11 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
Revision 1.2  87/07/29  16:00:39  wesommer
Fixed add_locker.

Revision 1.1  87/07/29  15:13:57  wesommer
Initial revision

 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.3 1987-07-30 00:26:11 wesommer Exp $";
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

    if (access_check) return(SMS_SUCCESS);

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
 ** set_pop_usage - incr/decr usage count for pop server in serverhosts talbe
 **
 ** Inputs:
 **   q->name - "add_pobox" or "delete_pobox"
 **   argv[2] - mach_id
 **
 ** Description:
 **   - incr/decr value field in serverhosts table for pop/mach_id
 **
 **/

set_pop_usage(q, argv)
    struct query *q;
    char *argv[];
##{
##  int mach_id;

    mach_id = *(int *)argv[2];
##  range of sh is serverhosts

    if (!bcmp(q->name, "add_pobox", 10)) {
##      repeat replace sh (value1 = sh.value1 + 1)
##             where sh.service = "pop" and sh.#mach_id = @mach_id
    } else if (!bcmp(q->name, "delete_pobox", 13)) {
##      repeat replace sh (value1 = sh.value1 - 1)
##             where sh.service = "pop" and sh.#mach_id = @mach_id
    }

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
 ** grvd_support - Support routine for get_rvd_servers query
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

grvd_support(q, sq, v, action, actarg)
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

    if (q1->type != RETRIEVE || table == (char *)0) return(SMS_NO_MATCH);

##  range of tbs is tblstats
##  repeat retrieve (need = any(tbs.modtime where tbs.#table = @table and
##				tbs.modtime > @last_get_time))
    
    result = (need) ? "true" : "false";
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
##  repeat retrieve (dir = np.#dir, allocated = np.#allocated) 
##         where np.#mach_id = @mach_id and np.#device = device
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
##  repeat replace tbs (appends = appends + 1, modtime = "now")
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

    /* tell the logfile what we're doing */
    com_err(whoami, 0, "validating row");
    com_err(whoami, 0, qual);

    /* look for the record */
##  range of rvar is table
##  retrieve (rowcount = count(rvar.name where qual))
    com_err(whoami, 0, "row validated");
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
    char buf[64];

    while (--n >= 0) {
	switch (vo->type) {
	case V_NAME:
	    sprintf(buf, "validating %s in %s: %s", 
		    vo->namefield, vo->table, argv[vo->index]);
	    com_err(whoami, 0, buf);
	    status = validate_name(argv, vo);
	    break;

	case V_ID:
	    sprintf(buf, "validating %s in %s: %s", 
		    vo->idfield, vo->table, argv[vo->index]);
	    com_err(whoami, 0, buf);
	    status = validate_id(argv, vo);
	    break;

	case V_TYPE:
	    sprintf(buf, "validating %s type: %s",
		    vo->table, argv[vo->index]);
	    com_err(whoami, 0, buf);
	    status = validate_type(argv, vo);
	    break;

	case V_TYPEDATA:
	    sprintf(buf, "validating type-specific data: %s",
		    argv[vo->index]);
	    com_err(whoami, 0, buf);
	    status = validate_typedata(q, argv, vo);
	    break;

	case V_FOLLOWUP:
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
