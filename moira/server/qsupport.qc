/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.26 1988-08-24 18:00:31 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef lint
static char *rcsid_qsupport_qc = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qsupport.qc,v 1.26 1988-08-24 18:00:31 mar Exp $";
#endif lint

#include "query.h"
#include "sms_server.h"
#include <ctype.h>


extern char *whoami, *strsave();


/* Specialized Access Routines */

/* access_user - verify that client name equals specified login name
 *
 *  - since field validation routines are called first, a users_id is
 *    now in argv[0] instead of the login name.
 */

access_user(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
{
    if (cl->users_id != *(int *)argv[0])
	return(SMS_PERM);
    else
	return(SMS_SUCCESS);
}

    

/* access_login - verify that client name equals specified login name
 *
 *   argv[0...n] contain search info.  q->
 */

access_login(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int rowcount, id;
##  char qual[256];

    build_qual(q->qual, q->argc, argv, qual);
##  retrieve (id = u.users_id) where qual
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1 || id != cl->users_id)
	return(SMS_PERM);
    else
	return(SMS_SUCCESS);
##}

    

/* access_list - check access for most list operations
 *
 * Inputs: argv[0] - list_id
 *	    q - query name
 *	    argv[2] - member ID (only for queries "amtl" and  "dmfl")
 *	    argv[7] - group IID (only for query "ulis")
 *          cl - client name
 *
 * - check that client is a member of the access control list
 * - OR, if the query is add_member_to_list or delete_member_from_list
 *	and the list is public, allow access if client = member
 */ 

access_list(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int list_id, acl_id, flags, rowcount, gid;
##  char acl_type[9];
    char *client_type;
    int client_id, status;

    list_id = *(int *)argv[0];
##  repeat retrieve (acl_id = list.#acl_id, acl_type = list.#acl_type,
##		     gid = list.#gid, flags = list.#public) 
##	   where list.#list_id = @list_id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
      return(SMS_INTERNAL);

    /* parse client structure */
    if ((status = get_client(cl, &client_type, &client_id)) != SMS_SUCCESS)
	return(status);

    /* if amtl or dmfl and list is public allow client to add or delete self */
    if ((!strcmp("amtl", q->shortname) || !strcmp("dmfl", q->shortname)) &&
	(flags && !strcmp("USER", argv[1]))) {
	if (*(int *)argv[2] == client_id) return(SMS_SUCCESS);
    /* if update_list, don't allow them to change the GID */
    } else if (!strcmp("ulis", q->shortname)) {
	if ((!strcmp(argv[7], UNIQUE_GID) && (gid != -1)) ||
	    (strcmp(argv[7], UNIQUE_GID) && (gid != atoi(argv[7]))))
	  return(SMS_PERM);
    }

    /* check for client in access control list */
    status = find_member(acl_type, acl_id, client_type, client_id, 0);
    if (!status) return(SMS_PERM);

    return(SMS_SUCCESS);
##}


/* access_visible_list - allow access to list only if it is not hidden,
 *	or if the client is on the ACL
 *
 * Inputs: argv[0] - list_id
 *         cl - client identifier
 */ 

access_visible_list(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int list_id, acl_id, flags, rowcount;
##  char acl_type[9];
    char *client_type;
    int client_id, status;

    list_id = *(int *)argv[0];
##  repeat retrieve (flags = list.hidden, acl_id = list.#acl_id, 
##	acl_type = list.#acl_type) where list.#list_id = @list_id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1)
      return(SMS_INTERNAL);
    if (!flags)
	return(SMS_SUCCESS);

    /* parse client structure */
    if ((status = get_client(cl, &client_type, &client_id)) != SMS_SUCCESS)
	return(status);

    /* check for client in access control list */
    status = find_member(acl_type, acl_id, client_type, client_id, 0);
    if (!status)
	return(SMS_PERM);

    return(SMS_SUCCESS);
##}


/* access_vis_list_by_name - allow access to list only if it is not hidden,
 *	or if the client is on the ACL
 *
 * Inputs: argv[0] - list name
 *         cl - client identifier
 */ 

access_vis_list_by_name(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int acl_id, flags, rowcount;
##  char acl_type[9], *listname;
    char *client_type;
    int client_id, status;

    listname = argv[0];
##  repeat retrieve (flags = list.hidden, acl_id = list.#acl_id, 
##	acl_type = list.#acl_type) where list.#name = @listname
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 1)
      return(SMS_WILDCARD);
    if (rowcount == 0)
      return(SMS_NO_MATCH);
    if (!flags)
	return(SMS_SUCCESS);

    /* parse client structure */
    if ((status = get_client(cl, &client_type, &client_id)) != SMS_SUCCESS)
	return(status);

    /* check for client in access control list */
    status = find_member(acl_type, acl_id, client_type, client_id, 0);
    if (!status)
	return(SMS_PERM);

    return(SMS_SUCCESS);
##}


/* access_member - allow user to access member of type "USER" and name matches
 * username, or to access member of type "LIST" and list is one that user is
 * on the acl of, or the list is visible.
 */

access_member(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
{
    if (!strcmp(argv[0], "LIST") || !strcmp(argv[0], "RLIST"))
      return(access_visible_list(q, &argv[1], cl));

    if (!strcmp(argv[0], "USER") || !strcmp(argv[0], "RUSER")) {
	if (cl->users_id == *(int *)argv[1])
	  return(SMS_SUCCESS);
    }

    return(SMS_PERM);
}


/* access_qgli - special access routine for Qualified_get_lists.  Allows
 * access iff argv[0] == "TRUE" and argv[2] == "FALSE".
 */

access_qgli(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
{
    if (!strcmp(argv[0], "TRUE") && !strcmp(argv[2], "FALSE"))
      return(SMS_SUCCESS);
    return(SMS_PERM);
}


/* access_service - allow access if user is on ACL of service.  Don't
 * allow access if a wildcard is used.
 */

access_service(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int acl_id, rowcount;
##  char *name, acl_type[9];
    int client_id, status;
    char *client_type;

    name = argv[0];
##  repeat retrieve (acl_id = servers.#acl_id, acl_type = servers.#acl_type)
##	where servers.#name = uppercase(@name)
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 1)
      return(SMS_PERM);

    /* parse client structure */
    if ((status = get_client(cl, &client_type, &client_id)) != SMS_SUCCESS)
	return(status);

    /* check for client in access control list */
    status = find_member(acl_type, acl_id, client_type, client_id, 0);
    if (!status) return(SMS_PERM);

    return(SMS_SUCCESS);
##}



/* access_filesys - verify that client is owner or on owners list of filesystem
 *	named by argv[0]
 */

access_filesys(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int rowcount, users_id, list_id;
##  char *name;
    int status, client_id;
    char *client_type;

    name = argv[0];
##  repeat retrieve (users_id = filesys.owner, list_id = filesys.owners)
##	where filesys.label = @name
##  inquire_equel(rowcount = "rowcount")

    if (rowcount != 1)
      return(SMS_PERM);
    if (users_id == cl->users_id)
      return(SMS_SUCCESS);
    if ((status = get_client(cl, &client_type, &client_id)) != SMS_SUCCESS)
	return(status);
    status = find_member("LIST", list_id, client_type, client_id, 0);
    if (status)
      return(SMS_SUCCESS);
    return(SMS_PERM);
##}

    

/* Setup Routines */

/* Setup routine for add_user
 *
 * Inputs: argv[0] - login
 *         argv[1] - uid
 *
 * Description:
 *
 * - if argv[1] == UNIQUE_UID then set argv[1] = next(uid)
 * - if argv[0] == UNIQUE_LOGIN then set argv[0] = "#<uid>"
 */

setup_ausr(q, argv, cl)
    struct query *q;
    register char *argv[];
    client *cl;
##{
##  int nuid, rowcount;

    if (!strcmp(argv[1], UNIQUE_UID) || atoi(argv[1]) == -1) {
	if (set_next_object_id("uid", "users"))
	  return(SMS_INGRES_ERR);
##      repeat retrieve (nuid = values.value) where values.name = "uid"
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  return(SMS_INTERNAL);
	sprintf(argv[1], "%d", nuid);
    }

    if (!strcmp(argv[0], UNIQUE_LOGIN) || atoi(argv[1]) == -1) {
	sprintf(argv[0], "#%s", argv[1]);
    }

    return(SMS_SUCCESS);
##}


/* setup_dusr - verify that the user is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dusr(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag, id;

    id = *(int *)argv[0];

    /* For now, only allow users to be deleted if their status is 0 */
##  repeat retrieve (flag = u.status) where u.users_id = @id
    if (flag != 0)
      return(SMS_IN_USE);

##  repeat delete nfsquota where nfsquota.users_id = @id
##  repeat retrieve (flag = any(members.member_id where members.member_id=@id
##			 and members.member_type = "USER"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(filesys.label where filesys.owner=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(list.name where list.acl_id=@id and
##			list.acl_type = "USER"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(servers.name where servers.acl_id=@id and
##			servers.acl_type = "USER"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag=any(hostaccess.acl_id where hostaccess.acl_id=@id and
##			hostaccess.acl_type = "USER"))
    if (flag)
	return(SMS_IN_USE);
    else
	return(SMS_SUCCESS);
##}


/* setup_spop: verify that there is already a valid POP machine_id in the
 * pop_id field.  Also take care of keeping track of the post office usage.
 */
int setup_spop(q, argv)
struct query *q;
char **argv;
##{
##  int id, mid, flag;
##  char type[9];

    id = *(int *)argv[0];
##  repeat retrieve (type = u.potype, mid = u.pop_id,
##		     flag = any(machine.name where machine.mach_id = u.pop_id 
##				     and u.pop_id != 0 and u.users_id = @id))
##	where u.users_id = @id
    if (!flag)
      return(SMS_MACHINE);
    if (strcmp(strtrim(type), "POP"))
      set_pop_usage(mid, 1);
    return(SMS_SUCCESS);
##}


/* setup_dpob:  Take care of keeping track of the post office usage.
 */
int setup_dpob(q, argv)
struct query *q;
char **argv;
##{
##  int id, user;
##  char type[9];

    user = *(int *)argv[0];
##  repeat retrieve (type = u.potype, id = u.pop_id)
##		where u.users_id = @user

    if (!strcmp(strtrim(type), "POP"))
      set_pop_usage(id, -1);
    return(SMS_SUCCESS);
##}


/* setup_dmac - verify that the machine is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dmac(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag, id;

    id = *(int *)argv[0];
##  repeat retrieve (flag = any(users.login where users.potype = "POP" 
##						  and users.pop_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(serverhosts.mach_id
##				 where serverhosts.mach_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(nfsphys.mach_id where nfsphys.mach_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(hostaccess.mach_id where hostaccess.mach_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(printcap.mach_id where printcap.mach_id=@id))
    if (flag)
	return(SMS_IN_USE);

##  repeat delete mcmap where mcmap.mach_id = @id
    return(SMS_SUCCESS);
##}


/* setup_dclu - verify that the cluster is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dclu(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag, id;

    id = *(int *)argv[0];
##  repeat retrieve (flag = any(mcmap.mach_id where mcmap.clu_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(svc.clu_id where svc.clu_id=@id))
    if (flag)
	return(SMS_IN_USE);
    else
	return(SMS_SUCCESS);
##}


/* setup_alis - if argv[5] is non-zero and argv[6] is UNIQUE_ID, then allocate
 * a new gid and put it in argv[6].  Otherwise if argv[6] is UNIQUE_ID but
 * argv[5] is not, then remember that UNIQUE_ID is being stored by putting
 * a -1 there.  Remember that this is also used for ulis, with the indexes
 * at 6 & 7.
 */

int setup_alis(q, argv)
struct query *q;
char **argv;
##{
##  int ngid;
    char *malloc();
    int idx;

    if (!strcmp(q->shortname, "alis"))
      idx = 6;
    else if (!strcmp(q->shortname, "ulis"))
      idx = 7;

    if (!strcmp(argv[idx], UNIQUE_GID) || atoi(argv[idx]) == -1) {
	if (atoi(argv[idx - 1])) {
	    if (set_next_object_id("gid", "list"))
	      return(SMS_INGRES_ERR);
##	    repeat retrieve (ngid = values.value) where values.name = "gid"
	    sprintf(argv[idx], "%d", ngid);
	} else {
	    strcpy(argv[idx], "-1");
	}
    }

    return(SMS_SUCCESS);
##}


/* setup_dlist - verify that the list is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dlis(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag, id;

    id = *(int *)argv[0];
##  repeat retrieve (flag = any(members.member_id where members.member_id=@id
##			 and members.member_type = "LIST"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(members.member_id where members.list_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(filesys.label where filesys.owners=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(capacls.tag where capacls.list_id=@id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(list.name where list.acl_id=@id and
##			list.acl_type = "LIST" and list.list_id != @id))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(servers.name where servers.acl_id=@id and
##			servers.acl_type = "LIST"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag=any(hostaccess.acl_id where hostaccess.acl_id=@id and
##			hostaccess.acl_type = "LIST"))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = any(zephyr.class
##		where zephyr.xmt_type = "LIST" and zephyr.xmt_id = @id or
##		      zephyr.sub_type = "LIST" and zephyr.sub_id = @id or
##		      zephyr.iws_type = "LIST" and zephyr.iws_id = @id or
##		      zephyr.iui_type = "LIST" and zephyr.iui_id = @id))
    if (flag)
        return(SMS_IN_USE);
    else
	return(SMS_SUCCESS);
##}


/* setup_dsin - verify that the service is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dsin(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag;
##  char *name;

    name = argv[0];
##  repeat retrieve (flag = any(serverhosts.service 
##				where serverhosts.service=uppercase(@name)))
    if (flag)
	return(SMS_IN_USE);
##  repeat retrieve (flag = servers.inprogress) where servers.#name = @name
    if (flag)
	return(SMS_IN_USE);
    else
	return(SMS_SUCCESS);
##}


/* setup_dshi - verify that the service-host is no longer being referenced 
 * and may safely be deleted.
 */

int setup_dshi(q, argv)
    struct query *q;
    char **argv;
##{
##  int flag, id;
##  char *name;

    name = argv[0];
    id = *(int *)argv[1];
##  repeat retrieve (flag=serverhosts.inprogress) 
##	where serverhosts.service=uppercase(@name) and serverhosts.mach_id=@id
    if (flag)
	return(SMS_IN_USE);
    else
	return(SMS_SUCCESS);
##}


/**
 ** setup_add_filesys - verify existance of referenced file systems
 **
 ** Inputs:     Add
 **   argv[1] - type
 **   argv[2] - mach_id
 **   argv[3] - name
 **   argv[5] - access
 **
 ** Description:
 **   - for type = RVD:
 **	   * allow anything
 **   - for type = NFS:
 **        * extract directory prefix from name
 **        * verify mach_id/dir in nfsphys
 **        * verify access in {r, w, R, W}
 **
 **  Side effect: sets variable var_phys_id to the ID of the physical
 **	filesystem (nfsphys_id for NFS, 0 for RVD)
 **
 ** Errors:
 **   SMS_NFS - specified directory not exported
 **   SMS_FILESYS_ACCESS - invalid filesys access
 **
 **/

##static int var_phys_id;

setup_afil(q, argv)
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
    var_phys_id = 0;

    if (!strcmp(type, "NFS"))
	return (check_nfs(mach_id, name, access));
    else
	return(SMS_SUCCESS);
}


/* Verify the arguments, depending on the FStype.  Also, if this is an
 * NFS filesystem, then update any quotas for that filesystem to reflect
 * the new phys_id.
 */

setup_ufil(q, argv)
    struct query *q;
    char *argv[];
##{
    int mach_id, status;
    char *type, *name, *access;  
##  int fid;

    type = argv[2];
    mach_id = *(int *)argv[3];
    name = argv[4];
    access = argv[6];
    var_phys_id = 0;

    if (!strcmp(type, "NFS")) {
	status = check_nfs(mach_id, name, access);
	fid = *(int *)argv[0];
##	replace nfsquota (phys_id = var_phys_id) where nfsquota.filsys_id = fid
	return(status);
    } else
      return(SMS_SUCCESS);
##}


/* Find the NFS physical partition that the named directory is on.
 * This is done by comparing the dir against the mount point of the
 * partition.  To make sure we get the correct match when there is 
 * more than one, we sort the query in reverse order by dir name.
 */

##check_nfs(mach_id, name, access)
##  int mach_id;
    char *name;
    char *access;
##{
##  char dir[32];
    char caccess;
    register int status;
    register char *cp1;
    register char *cp2;

    caccess = (isupper(*access)) ? tolower(*access) : *access;
    if (caccess != 'r' && caccess != 'w') return(SMS_FILESYS_ACCESS);

    status = SMS_NFS;
##  range of np is nfsphys
##  repeat retrieve (var_phys_id = np.#nfsphys_id, dir = trim(np.#dir))
##	where np.#mach_id = @mach_id sort by #dir:d {
	 cp1 = name;
	 cp2 = dir;
	 while (*cp2) {
	     if (*cp1++ != *cp2) break;
	     cp2++;
	 }
	 if (*cp2 == 0) {
	     status = SMS_SUCCESS;
##           endretrieve
	 }
##  }

    return(status);
##}


/* setup_dfil: free any quota records associated with a filesystem
 * when it is deleted.
 */

setup_dfil(q, argv, cl)
    struct query  *q;
    char **argv;
    client *cl;
##{
##  int id;

    id = *(int *)argv[0];
##  range of q is nfsquota
##  range of fs is filesys
##  range of n is nfsphys
##  repeat replace n (allocated=n.allocated-sum(q.quota where q.filsys_id=@id))
##	where n.nfsphys_id = fs.phys_id and fs.filsys_id = @id

##  repeat delete q where q.filsys_id = @id
    return(SMS_SUCCESS);
##}


/* setup_dnfp: check to see that the nfs physical partition does not have
 * any filesystems assigned to it before allowing it to be deleted.
 */

setup_dnfp(q, argv, cl)
    struct query  *q;
    char **argv;
    client *cl;
##{
##  int id, exists;

    id = *(int *)argv[0];
##  repeat retrieve (exists = any(filesys.label where filesys.phys_id = @id))
    if (exists)
      return(SMS_IN_USE);
    return(SMS_SUCCESS);
##}


/* setup_dnfq: Remove allocation from nfsphys before deleting quota.
 *   argv[0] = filsys_id
 *   argv[1] = users_id
 */

setup_dnfq(q, argv, cl)
    struct query  *q;
    char **argv;
    client *cl;
##{
##  int quota, fs, user;

    fs = *(int *)argv[0];
    user = *(int *)argv[1];

##  range of q is nfsquota
##  repeat retrieve (quota = q.#quota) where q.users_id = @user and
##	q.filsys_id = @fs
##  repeat replace nfsphys (allocated = nfsphys.allocated - @quota)
##	where nfsphys.nfsphys_id = filesys.#phys_id and filesys.filsys_id = @fs
    return(SMS_SUCCESS);
##}



/* FOLLOWUP ROUTINES */

/* generic set_modtime routine.  This takes the table name from the query,
 * and will update the modtime, modby, and modwho fields in the entry in
 * the table whose name field matches argv[0].
 */

set_modtime(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  char *name, *entity, *table;
##  int who;

    entity = cl->entity;
    who = cl->users_id;
    table = q->rtable;
    name = argv[0];

##  replace table (modtime = "now", modby = who, modwith = entity)
##	 where table.#name = name
    return(SMS_SUCCESS);
##}

/* generic set_modtime_by_id routine.  This takes the table name from
 * the query, and the id name from the validate record,
 * and will update the modtime, modby, and modwho fields in the entry in
 * the table whose id matches argv[0].
 */

set_modtime_by_id(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *entity, *table, *id_name;
##  int who, id;

    entity = cl->entity;
    who = cl->users_id;
    table = q->rtable;
    id_name = q->validate->object_id;

    id = *(int *)argv[0];
##  replace table (modtime = "now", modby = who, modwith = entity)
##	 where table.id_name = id
    return(SMS_SUCCESS);
##}


/* Sets the finger modtime on a user record.  The users_id will be in argv[0].
 */

set_finger_modtime(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int users_id, who;
##  char *entity;

    entity = cl->entity;
    who = cl->users_id;
    users_id = *(int *)argv[0];

##  repeat replace u (fmodtime = "now", fmodby = @who, fmodwith = @entity)
##	 where u.#users_id = @users_id
    return(SMS_SUCCESS);
##}


/* Sets the pobox modtime on a user record.  The users_id will be in argv[0].
 */

set_pobox_modtime(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  int users_id, who;
##  char *entity;

    entity = cl->entity;
    who = cl->users_id;
    users_id = *(int *)argv[0];

##  repeat replace users (pmodtime = "now", pmodby = @who, pmodwith = @entity)
##	 where users.#users_id = @users_id
    return(SMS_SUCCESS);
##}


/* Sets the modtime on a machine record.  The machine name is in argv[0].
 * This routine is different from the generic set_modtime in that the
 * name is uppercased first.
 */

set_mach_modtime(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *host, *entity;
##  int who;

    entity = cl->entity;
    who = cl->users_id;

    host = argv[0];
##  repeat replace m (modtime = "now", modby = @who, modwith = @entity)
##	 where m.name = uppercase(@host)
    return(SMS_SUCCESS);
##}


/* Sets the modtime on the machine whose mach_id is in argv[0].  This routine
 * is necessary for add_machine_to_cluster becuase the table that query
 * operates on is "mcm", not "machine".
 */

set_mach_modtime_by_id(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *entity;
##  int who, id;

    entity = cl->entity;
    who = cl->users_id;

    id = *(int *)argv[0];
##  range of m is machine
##  repeat replace m (modtime = "now", modby = @who, modwith = @entity)
##	 where m.mach_id = @id
    return(SMS_SUCCESS);
##}


/* Sets the modtime on the cluster whose mach_id is in argv[0].  This routine
 * is necessary for add_cluster_data and delete_cluster_data becuase the
 * table that query operates on is "svc", not "cluster".
 */

set_cluster_modtime_by_id(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *entity;
##  int who, id;

    entity = cl->entity;
    who = cl->users_id;

    id = *(int *)argv[0];
##  range of c is cluster
##  repeat replace c (modtime = "now", modby = @who, modwith = @entity)
##	 where c.clu_id = @id
    return(SMS_SUCCESS);
##}


/* sets the modtime on the serverhost where the service name is in argv[0]
 * and the mach_id is in argv[1].
 */

set_serverhost_modtime(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *entity, *serv;
##  int who, id;

    entity = cl->entity;
    who = cl->users_id;

    serv = argv[0];
    id = *(int *)argv[1];
##  repeat replace sh (modtime = "now", modby = @who, modwith = @entity)
##	 where sh.service = uppercase(@serv) and sh.mach_id = @id
    return(SMS_SUCCESS);
##}


/* sets the modtime on the nfsphys where the mach_id is in argv[0] and the
 * directory name is in argv[1].
 */

set_nfsphys_modtime(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *entity, *dir;
##  int who, id;

    entity = cl->entity;
    who = cl->users_id;

    id = *(int *)argv[0];
    dir = argv[1];
##  repeat replace np (modtime = "now", modby = @who, modwith = @entity)
##	 where np.#dir = @dir and np.mach_id = @id
    return(SMS_SUCCESS);
##}


/* sets the modtime on a filesystem, where argv[0] contains the filesys
 * label.
 */

set_filesys_modtime(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  char *label, *entity;
##  int who;

    entity = cl->entity;
    who = cl->users_id;

    label = argv[0];
    if (!strcmp(q->shortname, "ufil"))
      label = argv[1];

##  repeat replace fs (modtime = "now", modby = @who, modwith = @entity,
##		       #phys_id = @var_phys_id)  where fs.#label = @label
    return(SMS_SUCCESS);
##}


/* sets the modtime on a zephyr class, where argv[0] contains the class
 * name.
 */

set_zephyr_modtime(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  char *class, *entity;
##  int who;

    entity = cl->entity;
    who = cl->users_id;

    class = argv[0];

##  repeat replace z (modtime = "now", modby = @who, modwith = @entity)
##		       where z.#class = @class
    return(SMS_SUCCESS);
##}


/* fixes the modby field.  This will be the second to last thing in the
 * argv, the argv length is determined from the query structure.  It is
 * passed as a pointer to an integer.  This will either turn it into a
 * username, or # + the users_id.
 */
followup_fix_modby(q, sq, v, action, actarg, cl)
    struct query *q;
    register struct save_queue *sq;
    struct validate *v;
    register int (*action)();
    register int actarg;
    client *cl;
##{
    register int i, j;
    char **argv, *malloc();
##  int id, rowcount;
##  char *name;

    i = q->vcnt - 2;
    while (sq_get_data(sq, &argv)) {
	id = atoi(argv[i]);
	free(argv[i]);
	argv[i] = malloc(9);
	name = argv[i];
##	repeat retrieve (name = users.login) where users.users_id = @id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1) {
	    sprintf(argv[i], "#%d", id);
	}
	(*action)(q->vcnt, argv, actarg);
	for (j = 0; j < q->vcnt; j++)
	  free(argv[j]);
	free(argv);
    }
    sq_destroy(sq);
    return(SMS_SUCCESS);
##}


/**
 ** followup_ausr - add finger and pobox entries, set_user_modtime
 **
 ** Inputs:
 **	argv[0] - login (add_user)
 **	argv[3] - last name
 **	argv[4] - first name
 **	argv[5] - middle name
 **
 **/

followup_ausr(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int who;
##  char *login, *entity;
##  char fullname[129];

    login = argv[0];
    who = cl->users_id;
    entity = cl->entity;

    /* build fullname */
    if (strlen(argv[4]) && strlen(argv[5]))
	sprintf(fullname, "%s %s %s", argv[4], argv[5], argv[3]);
    else if (strlen(argv[4]))
	sprintf(fullname, "%s %s", argv[4], argv[3]);
    else
	sprintf(fullname, "%s", argv[3]);

    /* create finger entry, pobox & set modtime on user */
##  repeat replace u (modtime = "now", modby=@who, modwith=@entity,
##	     #fullname=@fullname, mit_affil = u.mit_year,
##	     fmodtime="now", fmodby=@who, fmodwith=@entity,
##	     potype="NONE", pmodtime="now", pmodby=@who, pmodwith=@entity)
##	where u.#login = @login

    return(SMS_SUCCESS);
##}


/* followup_gpob: fixes argv[2] based on the IDs currently there and the
 * type in argv[1].  Then completes the upcall to the user.
 *
 * argv[2] is of the form "123:234" where the first integer is the machine
 * ID if it is a pop box, and the second is the string ID if it is an SMTP
 * box.  argv[1] should be "POP", "SMTP", or "NONE".  Boxes of type NONE
 * are skipped.
 */

followup_gpob(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
    char **argv, *index();
    char *ptype, *p;
##  char box[129], *name;
##  int mid, sid, rowcount;

    /* for each row */
    while (sq_get_data(sq, &argv)) {
	sms_trim_args(2, argv);
	ptype = argv[1];
	p = index(argv[2], ':');
	*p++ = 0;
	mid = atoi(argv[2]);
	sid = atoi(p);
	free(argv[2]);

	if (!strcmp(ptype, "POP")) {
##	    repeat retrieve (box=machine.#name) where machine.mach_id=@mid
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      return(SMS_MACHINE);
	} else if (!strcmp(ptype, "SMTP")) {
##	    repeat retrieve (box=strings.string) where strings.string_id=@sid
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      return(SMS_STRING);
	} else /* ptype == "NONE" */ {
	    goto skip;
	}

	if (!strcmp(q->shortname, "gpob")) {
	    sid = atoi(argv[4]);
	    free(argv[4]);
	    argv[4] = malloc(9);
	    name = argv[4];
##	    repeat retrieve (name = users.login) where users.users_id = @sid
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      sprintf(name, "#%d", sid);
	}

	argv[2] = box;
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


/* followup_glin: fix the ace_name in argv[8].  argv[7] will contain the
 * ace_type: "LIST", "USER", or "NONE".  Decode the id in argv[8] into the
 * proper name based on the type, and repace that string in the argv.
 * Also fixes the modby field by called followup_fix_modby.
 */

followup_glin(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
    char **argv, *malloc(), *realloc(), *type;
##  char *name;
##  int id, rowcount;
    int i, idx;

    idx = 8;
    if (!strcmp(q->shortname, "gsin"))
      idx = 12;

    while (sq_get_data(sq, &argv)) {
	sms_trim_args(q->vcnt, argv);

	id = atoi(argv[i = q->vcnt - 2]);
	free(argv[i]);
	name = argv[i] = malloc(9);
##	repeat retrieve (name = users.login) where users.users_id = @id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  sprintf(argv[i], "#%d", id);

	id = atoi(argv[idx]);
	type = argv[idx - 1];
	if ((name = malloc(33)) == NULL)
	    return(SMS_NO_MEM);

	if (!strcmp(type, "LIST")) {
##	    repeat retrieve (name = list.#name) where list.list_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(name, "???");
	} else if (!strcmp(type, "USER")) {
##	    repeat retrieve (name = users.login) where users.users_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(name, "???");
	} else if (!strcmp(type, "NONE")) {
	    strcpy(name, "NONE");
	} else
	  strcpy(name, "???");
	free(argv[idx]);
	argv[idx] = name;

	if (!strcmp(q->shortname, "glin") && atoi(argv[6]) == -1) {
	    argv[6] = realloc(argv[6], strlen(UNIQUE_GID) + 1);
	    strcpy(argv[6], UNIQUE_GID);
	}

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


/** followup_amtl - followup for amtl and dmfl; when adding a list 
 **                 member to a maillist, make member list a maillist also
 **                 unless list is a user-group.
 **                 Then set_list_modtime_by_id.
 **
 ** Inputs:
 **   argv[0] - list_id
 **   argv[1] - member_type
 **   argv[2] - member_id
 **
 **/

followup_amtl(q, argv, cl)
    struct query *q;
    char *argv[];
    client *cl;
##{
##  int list_id;
##  int member_id;
##  int exists, who;
##  char *entity;

    list_id = *(int *)argv[0];
    entity = cl->entity;
    who = cl->users_id;

##  range of l is list
##  repeat replace l (modtime = "now", modby = @who, modwith = @entity)
##	 where l.#list_id = @list_id

    /* if query is not amtl or if member_type is not LIST then return */
    if (bcmp(q->shortname, "amtl", 4) || bcmp(argv[1], "LIST", 4)) 
	return(SMS_SUCCESS);

    member_id = *(int *)argv[2];

    /* is parent list a mailing list? */
##  repeat retrieve (exists = l.maillist) where l.#list_id=@list_id
    if (!exists)
	return(SMS_SUCCESS);

    /* list is not a user-group; add list to maillist table */
##  repeat replace l (maillist = 1) where l.#list_id = @member_id
    return(SMS_SUCCESS);
##}


/* followup_anfq: Add allocation to nfsphys after creating quota.
 *   argv[0] = filsys_id
 *   argv[2] = ascii(quota)
 */

followup_anfq(q, argv, cl)
    struct query  *q;
    char **argv;
    client *cl;
##{
##  int quota, user, fs, who;
##  char *entity;

    fs = *(int *)argv[0];
    user = *(int *)argv[1];
    quota = atoi(argv[2]);
    who = cl->users_id;
    entity = cl->entity;

##  repeat replace nq (modtime = "now", modby = @who, modwith = @entity)
##	where nq.filsys_id = @fs and nq.users_id = @user
##  repeat replace nfsphys (allocated = nfsphys.allocated + @quota)
##	where nfsphys.nfsphys_id = filesys.#phys_id and filesys.filsys_id = @fs
    return(SMS_SUCCESS);
##}


/* followup_gzcl:
 */

followup_gzcl(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
##  char *name;
##  int rowcount, id;
    char **argv;
    int i;

    while (sq_get_data(sq, &argv)) {
	sms_trim_args(q->vcnt, argv);

	id = atoi(argv[i = q->vcnt - 2]);
	free(argv[i]);
	name = argv[i] = malloc(9);
##	repeat retrieve (name = users.login) where users.users_id = @id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  sprintf(argv[i], "#%d", id);

	for (i = 1; i < 8; i+=2) {
	    id = atoi(argv[i+1]);
	    free(argv[i+1]);
	    if ((name = argv[i+1] = malloc(33)) == NULL)
	      return(SMS_NO_MEM);
	    if (!strcmp(argv[i], "LIST")) {
##		repeat retrieve (name = list.#name) where list.list_id = @id
##		inquire_equel(rowcount = "rowcount")
		if (rowcount != 1)
		  strcpy(name, "???");
	    } else if (!strcmp(argv[i], "USER")) {
##		repeat retrieve (name = users.login) where users.users_id = @id
##		inquire_equel(rowcount = "rowcount")
		if (rowcount != 1)
		  strcpy(name, "???");
	    } else if (!strcmp(argv[i], "NONE")) {
		strcpy(name, "NONE");
	    } else {
		strcpy(name, "???");
	    }
	}

	/* send the data */
	(*action)(q->vcnt, argv, actarg);

	/* free saved data */
	for (i = 0; i < q->vcnt; i++) 
	    free(argv[i]);
	free(argv);
    }
    sq_destroy(sq);
    return(SMS_SUCCESS);
##}


/* followup_gsha:
 */

followup_gsha(q, sq, v, action, actarg, cl)
    register struct query *q;
    register struct save_queue *sq;
    register struct validate *v;
    register int (*action)();
    int actarg;
    client *cl;
##{
##  char *name;
##  int rowcount, id;
    char **argv;
    int i;

    while (sq_get_data(sq, &argv)) {
	sms_trim_args(q->vcnt, argv);

	id = atoi(argv[4]);
	free(argv[4]);
	name = argv[4] = malloc(9);
##	repeat retrieve (name = users.login) where users.users_id = @id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  sprintf(argv[4], "#%d", id);

	id = atoi(argv[2]);
	free(argv[2]);
	if ((name = argv[2] = malloc(33)) == NULL)
	  return(SMS_NO_MEM);
	if (!strcmp(argv[1], "LIST")) {
##	    repeat retrieve (name = list.#name) where list.list_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      strcpy(name, "???");
	} else if (!strcmp(argv[1], "USER")) {
##	    repeat retrieve (name = users.login) where users.users_id = @id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      strcpy(name, "???");
	} else if (!strcmp(argv[1], "NONE")) {
	    strcpy(name, "NONE");
	} else {
	    strcpy(name, "???");
	}

	/* send the data */
	(*action)(q->vcnt, argv, actarg);

	/* free saved data */
	for (i = 0; i < q->vcnt; i++) 
	    free(argv[i]);
	free(argv);
    }
    sq_destroy(sq);
    return(SMS_SUCCESS);
##}



/* Special query routines */

/* set_pobox - this does all of the real work.
 *	 argv = user_id, type, box
 * if type is POP, then box should be a machine, and its ID should be put in
 * pop_id.  If type is SMTP, then box should be a string and its ID should
 * be put in box_id.  If type is NONE, then box doesn't matter.
 */

int set_pobox(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  int user, id, rowcount;
##  char *box, potype[9];

    box = argv[2];
    user = *(int *)argv[0];

##  repeat retrieve (id = users.pop_id, potype = users.#potype)
##		where users.users_id = @user
    if (!strcmp(strtrim(potype), "POP"))
      set_pop_usage(id, -1);

    if (!strcmp(argv[1], "POP")) {
##	repeat retrieve (id=machine.mach_id) where machine.name=uppercase(@box)
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	    return(SMS_MACHINE);
##	repeat replace users (#potype = "POP", pop_id = @id)
##		where users.users_id = @user
	set_pop_usage(id, 1);
    } else if (!strcmp(argv[1], "SMTP")) {
##      range of s is strings
##      repeat retrieve (id = s.string_id) where s.string = @box
##      inquire_equel (rowcount = "rowcount")
	if (rowcount == 0) {
##          range of v is values
##          repeat retrieve (id = v.value) where v.name = "strings_id"
	    id++;
##          repeat replace v (value = @id) where v.name = "strings_id"
##	    append to strings (string_id = id, string = box)
	}
##	repeat replace users (#potype = "SMTP", box_id = @id) 
##	       where users.users_id = @user
    } else /* argv[1] == "NONE" */ {
##	repeat replace users (#potype = "NONE") where users.users_id = @user
    }

    set_pobox_modtime(q, argv, cl);
##  repeat replace tblstats (updates = tblstats.updates + 1, modtime = "now")
##	where tblstats.#table = "users"
    return(SMS_SUCCESS);
##}


/* get_list_info:  passed a wildcard list name, returns lots of stuff about
 * each list.  This is tricky:  first build a queue of all requested
 * data.  Rest of processing consists of fixing gid, ace_name, and modby.
 */

get_list_info(q, aargv, cl, action, actarg)
    register struct query *q;
    char **aargv;
    client *cl;
    register int (*action)();
    int actarg;
##{
    char *argv[13], *malloc(), *realloc();
##  char *name, acl_type[9], listname[33], active[5], public[5], hidden[5];
##  char maillist[5], group[5], gid[6], acl_name[33], desc[256], modtime[27];
##  char modby[9], modwith[9];
##  int id, rowcount, acl_id, hid, modby_id;
    int returned;
    struct save_queue *sq, *sq_create();

    returned = rowcount = 0;
    name = aargv[0];

    sq = sq_create();
##  range of l is list
##  repeat retrieve (id = l.list_id) where l.#name = @name {
	sq_save_data(sq, id);
	rowcount++;
##  }
    if (rowcount == 0)
      return(SMS_NO_MATCH);

    argv[0] = listname; argv[1] = active; argv[2] = public; argv[3] = hidden;
    argv[4] = maillist; argv[5] = group; argv[6] = gid; argv[7] = acl_type;
    argv[8] = acl_name; argv[9] = desc; argv[10] = modtime; argv[11] = modby;
    argv[12] = modwith;

    while (sq_get_data(sq, &id)) {
	if (id == 0)
	  continue;
	argv[6] = gid;
##	repeat retrieve (listname = l.#name, active = text(l.#active), 
##		public = text(l.#public), hidden = text(l.#hidden),
##		hid = l.#hidden, maillist = text(l.#maillist),
##		group = text(l.#group), gid = text(l.#gid),
##		acl_type = trim(l.#acl_type), acl_id = l.#acl_id,
##		desc = l.#desc, modtime = l.#modtime, modby_id = l.#modby,
##		modwith =l.#modwith)
##	    where l.list_id = @id

	if (atoi(gid) == -1)
	    argv[6] = UNIQUE_GID;

	if (!strcmp(acl_type, "LIST")) {
##	    repeat retrieve (acl_name = l.#name) where l.list_id = @acl_id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(acl_name, "???");
	} else if (!strcmp(acl_type, "USER")) {
##	    repeat retrieve (acl_name = users.#login)
##		where users.users_id = @acl_id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
		strcpy(acl_name, "???");
	} else if (!strcmp(acl_type, "NONE")) {
	    strcpy(acl_name, "NONE");
	} else
	  strcpy(acl_name, "???");

##	repeat retrieve (modby = users.login) where users.users_id = @modby_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  sprintf(modby, "#%d", id);

	sms_trim_args(q->vcnt, argv);
	returned++;
	(*action)(q->vcnt, argv, actarg);
    }

    sq_destroy(sq);
##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = "list"

    return (SMS_SUCCESS);
##}


/* get_ace_use - given a type and a name, return a type and a name.
 * The ace_type is one of "LIST", "USER", "RLIST", or "RUSER" in argv[0],
 * and argv[1] will contain the ID of the entity in question.  The R*
 * types mean to recursively look at every containing list, not just
 * when the object in question is a direct member.  On return, the
 * usage type will be one of LIST, SERVICE, FILESYS, QUOTA, QUERY, or ZEPHYR.
 */

int get_ace_use(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
    int found = 0;
##  char *atype;
##  int aid, listid, id;
    struct save_queue *sq, *sq_create();

    atype = argv[0];
    aid = *(int *)argv[1];
    if (!strcmp(atype, "LIST") || !strcmp(atype, "USER")) {
	return(get_ace_internal(atype, aid, action, actarg));
    }

    sq = sq_create();
    if (!strcmp(atype, "RLIST")) {
	sq_save_data(sq, aid);
	/* get all the list_id's of containing lists */
##	range of m is members
	while (sq_get_data(sq, &id)) {
##	    repeat retrieve (listid = m.list_id)
##		where m.member_type = "LIST" and m.member_id = @id {
	      sq_save_unique_data(sq, listid);
##	    }
	  }
	/* now process each one */
	while (sq_get_data(sq, &id)) {
	    if (get_ace_internal("LIST", id, action, actarg) == SMS_SUCCESS)
	      found++;
	}
    }

    if (!strcmp(atype, "RUSER")) {
##	range of m is members
##	repeat retrieve (listid = m.list_id)
##		where m.member_type = "USER" and m.member_id = @aid {
	    sq_save_data(sq, listid);
##	}
	/* get all the list_id's of containing lists */
	while (sq_get_data(sq, &id)) {
##	    repeat retrieve (listid = m.list_id)
##		where m.member_type = "LIST" and m.member_id = @id {
	      sq_save_unique_data(sq, listid);
##	    }
	  }
	/* now process each one */
	while (sq_get_data(sq, &id)) {
	    if (get_ace_internal("LIST", id, action, actarg) == SMS_SUCCESS)
	      found++;
	}
	if (get_ace_internal("USER", aid, action, actarg) == SMS_SUCCESS)
	  found++;
    }

    sq_destroy(sq);	
    if (!found) return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* This looks up a single list or user for ace use.  atype must be "USER"
 * or "LIST", and aid is the ID of the corresponding object.  This is used
 * by get_ace_use above.
 */

##get_ace_internal(atype, aid, action, actarg)
##  char *atype;
##  int aid;
    int (*action)();
    int actarg;
##{
    char *rargv[2];
    int found = 0;
##  char name[33];

    rargv[1] = name;
    if (!strcmp(atype, "LIST")) {
	rargv[0] = "FILESYS";
##	repeat retrieve (name = filesys.label) 
##		where filesys.owners = @aid {
	    (*action)(2, rargv, actarg);
	    found++;
##	}

	rargv[0] = "QUERY";
##	repeat retrieve (name = capacls.capability)
##		where capacls.list_id = @aid {
	    (*action)(2, rargv, actarg);
	    found++;
##	}
    } else if (!strcmp(atype, "USER")) {
	rargv[0] = "FILESYS";
## 	repeat retrieve (name = filesys.label) 
##		where filesys.owner = @aid {
	    (*action)(2, rargv, actarg);
	    found++;
##	}
    }

    rargv[0] = "LIST";
##  repeat retrieve (name = list.#name) 
##		where list.acl_type = @atype and list.acl_id = @aid {
	 (*action)(2, rargv, actarg);
	 found++;
##  }

    rargv[0] = "SERVICE";
##  repeat retrieve (name = servers.#name) 
##		where servers.acl_type = @atype and servers.acl_id = @aid {
	 (*action)(2, rargv, actarg);
	 found++;
##  }

    rargv[0] = "HOSTACCESS";
##  repeat retrieve (name = machine.#name)
##		where machine.mach_id = hostaccess.mach_id and 
##		     hostaccess.acl_type = @atype and hostaccess.acl_id = @aid {
	(*action)(2, rargv, actarg);
	found++;
##  }
    rargv[0] = "ZEPHYR";
##  repeat retrieve (name = zephyr.class) 
##		where zephyr.xmt_type = @atype and zephyr.xmt_id = @aid or
##		      zephyr.sub_type = @atype and zephyr.sub_id = @aid or
##		      zephyr.iws_type = @atype and zephyr.iws_id = @aid or
##		      zephyr.iui_type = @atype and zephyr.iui_id = @aid {
	 (*action)(2, rargv, actarg);
	 found++;
##  }

    if (!found) return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* get_lists_of_member - given a type and a name, return the name and flags
 * of all of the lists of the given member.  The member_type is one of
 * "LIST", "USER", "STRING", "RLIST", "RUSER", or "RSTRING" in argv[0],
 * and argv[1] will contain the ID of the entity in question.  The R*
 * types mean to recursively look at every containing list, not just
 * when the object in question is a direct member.
 */

int get_lists_of_member(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
    int found = 0;
##  char *atype;
##  int aid, listid, id;
    struct save_queue *sq, *sq_create();

    atype = argv[0];
    aid = *(int *)argv[1];
    if (!strcmp(atype, "LIST") ||
	!strcmp(atype, "USER") ||
	!strcmp(atype, "STRING")) {
      return(glom_internal(atype, aid, action, actarg));
    }

    sq = sq_create();
    if (!strcmp(atype, "RLIST")) {
	sq_save_data(sq, aid);
	/* get all the list_id's of containing lists */
##	range of m is members
	while (sq_get_data(sq, &id)) {
##	    repeat retrieve (listid = m.list_id)
##		where m.member_type = "LIST" and m.member_id = @id {
	      sq_save_unique_data(sq, listid);
##	    }
	  }
	/* now process each one */
	while (sq_get_data(sq, &id)) {
	    if (glom_internal("LIST", id, action, actarg) == SMS_SUCCESS)
	      found++;
	}
    }

    if (!strcmp(atype, "RUSER")) {
##	range of m is members
##	repeat retrieve (listid = m.list_id)
##		where m.member_type = "USER" and m.member_id = @aid {
	    sq_save_data(sq, listid);
##	}
	/* get all the list_id's of containing lists */
	while (sq_get_data(sq, &id)) {
##	    repeat retrieve (listid = m.list_id)
##		where m.member_type = "LIST" and m.member_id = @id {
	      sq_save_unique_data(sq, listid);
##	    }
	  }
	/* now process each one */
	while (sq_get_data(sq, &id)) {
	    if (glom_internal("LIST", id, action, actarg) == SMS_SUCCESS)
	      found++;
	}
	if (glom_internal("USER", aid, action, actarg) == SMS_SUCCESS)
	  found++;
    }

    if (!strcmp(atype, "RSTRING")) {
##	range of m is members
##	repeat retrieve (listid = m.list_id)
##		where m.member_type = "STRING" and m.member_id = @aid {
	    sq_save_data(sq, listid);
##	}
	/* get all the list_id's of containing lists */
	while (sq_get_data(sq, &id)) {
##	    repeat retrieve (listid = m.list_id)
##		where m.member_type = "LIST" and m.member_id = @id {
	      sq_save_unique_data(sq, listid);
##	    }
	  }
	/* now process each one */
	while (sq_get_data(sq, &id)) {
	    if (glom_internal("LIST", id, action, actarg) == SMS_SUCCESS)
	      found++;
	}
	if (glom_internal("STRING", aid, action, actarg) == SMS_SUCCESS)
	  found++;
    }

##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = "members"
    sq_destroy(sq);	
    if (!found) return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* This looks up a single list, user, or string as a member.  atype must be
 * "USER", "LIST", or "STRING" and aid is the ID of the corresponding object.
 * This is used by get_lists_of_members above.
 */

##glom_internal(atype, aid, action, actarg)
##  char *atype;
##  int aid;
    int (*action)();
    int actarg;
##{
    char *rargv[6];
    int found = 0;
##  char name[33], active[5], public[5], hidden[5], maillist[5], group[5];

    rargv[0] = name;
    rargv[1] = active;
    rargv[2] = public;
    rargv[3] = hidden;
    rargv[4] = maillist;
    rargv[5] = group;
##  repeat retrieve (name = list.#name, active = text(list.#active), 
##		     public = text(list.#public), hidden = text(list.#hidden),
##		     maillist = text(list.#maillist), group = text(list.#group))
##		where list.list_id = m.list_id and
##		      m.member_type = @atype and m.member_id = @aid {
	 (*action)(6, rargv, actarg);
	 found++;
##  }

    if (!found) return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* qualified_get_lists: passed "TRUE", "FALSE", or "DONTCARE" for each of
 * the five flags associated with each list.  It will return the name of
 * each list that meets the quailifications.  It does this by building a
 * where clause based on the arguments, then doing a retrieve.
 */

static char *lflags[5] = { "active", "public", "hidden", "maillist", "group" };

int qualified_get_lists(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
{
    return(qualified_get(q, argv, action, actarg, "l.list_id != 0",
			 "l", "name", lflags));
}


/** get_members_of_list - optimized query for retrieval of list members
 **
 ** Inputs:
 **   argv[0] - list_id
 **
 ** Description:
 **   - retrieve USER members, then LIST members, then STRING members
 **/

get_members_of_list(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
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

##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = "members"
    return(SMS_SUCCESS);
##}


/* count_members_of_list: this is a simple query, but it cannot be done
 * through the dispatch table.
 */

int count_members_of_list(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  int  list, ct = 0;
    char *rargv[1], countbuf[5];

    list = *(int *)argv[0];
    rargv[0] = countbuf;
##  repeat retrieve (ct = count(members.list_id where members.list_id = @list))
    sprintf(countbuf, "%d", ct);
    (*action)(1, rargv, actarg);
##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = "members"
    return(SMS_SUCCESS);
##}


/* qualified_get_server: passed "TRUE", "FALSE", or "DONTCARE" for each of
 * the three flags associated with each service.  It will return the name of
 * each service that meets the quailifications.  It does this by building a
 * where clause based on the arguments, then doing a retrieve.
 */

static char *sflags[3] = { "enable", "inprogress", "harderror" };

int qualified_get_server(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
{
    return(qualified_get(q, argv, action, actarg, "s.name != \"\"",
			 "s", "name", sflags));
}


/* generic qualified get routine, used by qualified_get_lists,
 * qualified_get_server, and qualified_get_serverhost.
 *   Args:
 *	start - a simple where clause, must not be empty
 *	range - the name of the range variable
 *	field - the field to return
 *	flags - an array of strings, names of the flag variables
 */

int qualified_get(q, argv, action, actarg, start, range, field, flags)
    struct query *q;
    char *argv[];
    int (*action)();
    int actarg;
    char *start;
    char *range;
    char *field;
    char *flags[];
##{
##  char name[33], qual[256], *rvar, *rtbl, *rfield;
    char *rargv[1], buf[32];
##  int rowcount, i;

    strcpy(qual, start);
    for (i = 0; i < q->argc; i++) {
	if (!strcmp(argv[i], "TRUE")) {
	    sprintf(buf, " and %s.%s != 0", range, flags[i]);
	    (void) strcat(qual, buf);
	} else if (!strcmp(argv[i], "FALSE")) {
	    sprintf(buf, " and %s.%s = 0", range, flags[i]);
	    (void) strcat(qual, buf);
	}
    }
      
    rargv[0] = name;
    rvar = range;
    rtbl = q->rtable;
    rfield = field;
##  range of rvar is rtbl
##  retrieve (name = rvar.rfield) where qual {
	(*action)(1, rargv, actarg);
##  }
##  inquire_equel(rowcount = "rowcount")
##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = @rtbl
    if (rowcount == 0)
      return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* qualified_get_serverhost: passed "TRUE", "FALSE", or "DONTCARE" for each of
 * the five flags associated with each serverhost.  It will return the name of
 * each service and host that meets the quailifications.  It does this by 
 * building a where clause based on the arguments, then doing a retrieve.
 */

static char *shflags[6] = { "service", "enable", "override", "success",
			    "inprogress", "hosterror" };

int qualified_get_serverhost(q, argv, cl, action, actarg)
    struct query *q;
    char *argv[];
    client *cl;
    int (*action)();
    int actarg;
##{
##  char sname[33], mname[33], qual[256];
    char *rargv[2], buf[32];
##  int rowcount, i;

    sprintf(qual, "machine.mach_id = sh.mach_id and sh.service = uppercase(\"%s\")",
	    argv[0]);
    for (i = 1; i < q->argc; i++) {
	if (!strcmp(argv[i], "TRUE")) {
	    sprintf(buf, " and sh.%s != 0", shflags[i]);
	    strcat(qual, buf);
	} else if (!strcmp(argv[i], "FALSE")) {
	    sprintf(buf, " and sh.%s = 0", shflags[i]);
	    strcat(qual, buf);
	}
    }
      
    rargv[0] = sname;
    rargv[1] = mname;
##  range of sh is serverhosts
##  retrieve (sname = sh.service, mname = machine.name) where qual {
	(*action)(2, rargv, actarg);
##  }
##  inquire_equel(rowcount = "rowcount")
##  repeat replace tblstats (retrieves = tblstats.retrieves + 1)
##	where tblstats.#table = "serverhosts"
    if (rowcount == 0)
      return(SMS_NO_MATCH);
    return(SMS_SUCCESS);
##}


/* register_user - change user's login name and allocate a pobox, group,
 * filesystem, and quota for them.  The user's status must start out as 0,
 * and is left as 2.  Arguments are: user's UID, new login name, and user's
 * type for filesystem allocation (SMS_FS_STUDENT, SMS_FS_FACULTY, 
 * SMS_FS_STAFF, SMS_FS_MISC).
 */

register_user(q, argv, cl)
    struct query *q;
    char **argv;
    client *cl;
##{
##  char *login, dir[65], *entity, *directory, machname[33];
##  int who, rowcount, mid, uid, users_id, flag, utype, nid, list_id, quota;
##  int size, alloc, pid, m_id;
    int maxsize;

    entity = cl->entity;
    who = cl->users_id;

    uid = atoi(argv[0]);
    login = argv[1];
    utype = atoi(argv[2]);

##  range of u is users
##  range of l is list
##  range of sh is serverhosts
##  range of n is nfsphys
##  range of m is machine

    /* find user */
##  repeat retrieve (users_id = u.#users_id)
##	where u.#uid = @uid and u.status = 0
##  inquire_equel(rowcount = "rowcount");
    if (rowcount == 0)
      return(SMS_NO_MATCH);
    if (rowcount > 1)
      return(SMS_NOT_UNIQUE);

    /* check new login name */
##  repeat retrieve (flag = any(u.#login where u.#login = @login))
    if (flag)
      return(SMS_IN_USE);
##  repeat retrieve (flag = any(l.#name where l.#name = @login))
    if (flag)
      return(SMS_IN_USE);
##  repeat retrieve (flag = any(filesys.#name where filesys.#name = @login))
    if (flag)
      return(SMS_IN_USE);
    com_err(whoami, 0, "new login name OK");

    /* choose place for pobox, put in mid */
##  repeat retrieve (mid = sh.mach_id, machname = m.name)
##    where sh.service = "POP" and m.mach_id = sh.mach_id and
##	sh.value2 - sh.value1 = max(sh.value2-sh.value1 where sh.service="POP")
##  inquire_equel(rowcount = "rowcount");
    if (rowcount == 0)
      return(SMS_NO_POBOX);

    /* change login name, set pobox */
##  repeat replace u (#login = @login, status = 2, modtime = "now",
##		      modby = @who, modwith = @entity, potype="POP",
##		      pop_id = @mid, pmodtime="now", pmodby=@who,
##		      pmodwith=@entity)
##	where u.#users_id = @users_id
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
    set_pop_usage(mid, 1);
    com_err(whoami, 0, "set login name to %s and pobox to %s", login,
	    trim(machname));

    /* create group list */
    if (set_next_object_id("gid", "list"))
      return(SMS_NO_ID);
    if (set_next_object_id("list_id", "list"))
      return(SMS_NO_ID);
##  repeat retrieve (list_id = values.value) where values.name = "list_id"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
##  repeat append list (name = @login, #list_id = @list_id, active = 1,
##			public = 0, hidden = 0, maillist = 0, group = 1,
##			#gid = values.value, desc = "User Group",
##			acl_type = "USER", acl_id = @users_id, modtime = "now",
##			modby = @who, modwith = @entity)
##	where values.name = "gid"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
##  repeat append members (#list_id = @list_id, member_type = "USER",
##			   member_id = @users_id)
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
    com_err(whoami, 0, "group list created");

    /* decide where to put filesystem */
    maxsize = 0;
    directory = NULL;
##  repeat retrieve (mid = n.mach_id, dir = trim(n.#dir), nid = n.nfsphys_id,
##		     flag = n.status, size = n.#size, alloc = n.allocated) {
	if ((flag & utype) && (size != 0) && (size - alloc > maxsize)) {
	    maxsize = size - alloc;
	    if (directory)
	      free(directory);
	    directory = strsave(dir);
	    pid = nid;
	    m_id = mid;
	}
##  }
    if (maxsize == 0)
      return(SMS_NO_FILESYS);

    /* create filesystem */
    if (set_next_object_id("filsys_id", "filesys"))
      return(SMS_NO_ID);
##  repeat append filesys (filsys_id = values.value, phys_id = @pid,
##			   label = @login, type = "NFS", mach_id = @m_id,
##			   name = @directory + "/" + @login,
##			   mount = "/mit/" + @login,
##			   access = "w", comments = "User Locker",
##			   owner = @users_id, owners = @list_id, createflg = 1,
##			   lockertype = "HOMEDIR", modtime = "now",
##			   modby = @who, modwith = @entity)
##	where values.name = "filsys_id"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
    com_err(whoami, 0, "filesys created on mach %d in %s/%s", m_id,
	    directory, login);

    /* set quota */
##  repeat retrieve (quota = values.value) where values.name = "def_quota"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_NO_QUOTA);
##  repeat append nfsquota (#users_id = @users_id, filsys_id = values.value,
##			    #quota = @quota, phys_id = @pid, modtime = "now",
##			    modby = @who, modwith = @entity)
##	where values.name = "filsys_id"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
##  repeat replace nfsphys (allocated = nfsphys.allocated + @quota)
##	where nfsphys.nfsphys_id = filesys.#phys_id and
##	      filesys.filsys_id = values.value and values.name = "filsys_id"
##  inquire_equel(rowcount = "rowcount");
    if (rowcount != 1)
      return(SMS_INTERNAL);
    com_err(whoami, 0, "quota of %d assigned", quota);

##  repeat replace tblstats (updates = tblstats.updates + 1, modtime = "now")
##	where tblstats.table = "users"
##  repeat replace tblstats (appends = tblstats.appends + 1, modtime = "now")
##	where tblstats.table = "list" or tblstats.table = "filesys" or
##	      tblstats.table = "nfsquota"
    return(SMS_SUCCESS);
##}



/** set_pop_usage - incr/decr usage count for pop server in serverhosts talbe
 **
 ** Inputs:
 **   id of machine
 **   delta (will be +/- 1)
 **
 ** Description:
 **   - incr/decr value field in serverhosts table for pop/mach_id
 **
 **/

static int set_pop_usage(id, count)
int id;
int count;
##{
##  int mach_id = id;
##  int n = count;

##  range of sh is serverhosts
##  repeat replace sh (value1 = sh.value1 + @n)
##         where sh.service = "POP" and sh.#mach_id = @mach_id

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

    if (log_flags & LOG_VALID)
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
	    if (log_flags & LOG_VALID)
		com_err(whoami, 0, "validating %s in %s: %s", 
		    vo->namefield, vo->table, argv[vo->index]);
	    status = validate_name(argv, vo);
	    break;

	case V_ID:
	    if (log_flags & LOG_VALID)
		com_err(whoami, 0, "validating %s in %s: %s", 
		    vo->idfield, vo->table, argv[vo->index]);
	    status = validate_id(argv, vo);
	    break;

	case V_DATE:
	    if (log_flags & LOG_VALID)
		com_err(whoami, 0, "validating date: %s", argv[vo->index]);
	    status = validate_date(argv, vo);
	    break;

	case V_TYPE:
	    if (log_flags & LOG_VALID)
		com_err(whoami, 0, "validating %s type: %s",
		    vo->table, argv[vo->index]);
	    status = validate_type(argv, vo);
	    break;

	case V_TYPEDATA:
	    if (log_flags & LOG_VALID)
		com_err(whoami, 0, "validating typed data (%s): %s",
		    argv[vo->index - 1], argv[vo->index]);
	    status = validate_typedata(q, argv, vo);
	    break;

	case V_RENAME:
	    if (log_flags & LOG_VALID)
	        com_err(whoami, 0, "validating rename %s in %s",
			argv[vo->index], vo->table);
	    status = validate_rename(argv, vo);
	    break;

	case V_CHAR:
	    if (log_flags & LOG_VALID)
	      com_err(whoami, 0, "validating chars: %s", argv[vo->index]);
	    status = validate_chars(argv[vo->index]);
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


/* validate_chars: verify that there are no illegal characters in
 * the string.  Legal characters are printing chars other than 
 * ", *, ?, \, [ and ].
 */
static int illegalchars[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^@ - ^O */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^P - ^_ */
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, /* SPACE - / */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, /* 0 - ? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* @ - O */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, /* P - _ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ` - o */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, /* p - ^? */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

validate_chars(s)
register char *s;
{
    while (*s)
      if (illegalchars[*s++])
	return(SMS_BAD_CHAR);
    return(SMS_EXISTS);
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
    register char *c;

    name = argv[vo->index];
    table = vo->table;
    /* minor kludge to upcasify machine names */
    if (!strcmp(table, "machine"))
	for (c = name; *c; c++) if (islower(*c)) *c = toupper(*c);
    namefield = vo->namefield;
    idfield = vo->idfield;
    if (!strcmp(namefield, "uid")) {
##    retrieve (id = table.idfield) where table.namefield = int4(name)
##    inquire_equel (rowcount = "rowcount")
    } else {
##    retrieve (id = table.idfield) where table.namefield = name
##    inquire_equel (rowcount = "rowcount")
    }
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
    register char *c;

    name = argv[vo->index];
    table = vo->table;
    namefield = vo->namefield;
    if (!strcmp(table, "servers") && !strcmp(namefield, "name")) {
	for (c = name; *c; c++)
	  if (islower(*c))
	    *c = toupper(*c);
    }
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
    return(SMS_EXISTS);
##}


validate_rename(argv, vo)
char *argv[];
struct valobj *vo;
##{
##  char *name, *table, *namefield, *idfield;
##  int id;
    register char *c;

    c = name = argv[vo->index];
    while (*c)
      if (illegalchars[*c++])
	return(SMS_BAD_CHAR);
    table = vo->table;
    /* minor kludge to upcasify machine names */
    if (!strcmp(table, "machine"))
	for (c = name; *c; c++) if (islower(*c)) *c = toupper(*c);
    namefield = vo->namefield;
    idfield = vo->idfield;
    id = -1;
    if (idfield == 0) {
	if (!strcmp(argv[vo->index], argv[vo->index - 1]))
	  return(SMS_EXISTS);
##	retrieve (id = any(table.namefield where table.namefield = name))
	if (id)
	  return(vo->error);
	else
	  return(SMS_EXISTS);
    }
##  retrieve (id = table.idfield) where table.namefield = name
    if (id == -1 || id == *(int *)argv[vo->index - 1])
      return(SMS_EXISTS);
    else
      return(vo->error);
##}


validate_type(argv, vo)
    char *argv[];
    register struct valobj *vo;
##{
##  char *typename;
##  char *value;
##  int exists;
    register char *c;

    typename = vo->table;
    c = value = argv[vo->index];
    while (*c)
      if (illegalchars[*c++])
	return(SMS_BAD_CHAR);

    /* uppercase type fields */
    for (c = value; *c; c++) if (islower(*c)) *c = toupper(*c);

##  range of a is alias
##  repeat retrieve (exists = any(a.trans where a.name = @typename and
##				      a.type = "TYPE" and
##				      a.trans = @value))
    return (exists ? SMS_EXISTS : vo->error);
##}

/* validate member or type-specific data field */

validate_typedata(q, argv, vo)
    register struct query *q;
    register char *argv[];
    register struct valobj *vo;
##{
##  char *name;
##  char *field_type;
##  char data_type[129];
##  int id;
##  int rowcount;
    char *index();
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
    if (index(data_type, ' '))
	*index(data_type, ' ') = 0;
    if (!strcmp(data_type, "user")) {
	/* USER */
##	repeat retrieve (id = users.users_id) where users.login = @name
##	inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) return(SMS_USER);

    } else if (!strcmp(data_type, "list")) {
	/* LIST */
##	repeat retrieve (id = list.list_id) where list.#name = @name
##	inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) {
	    /* if idfield is non-zero, then if argv[0] matches the string
	     * that we're trying to resolve, we should get the value of
	     * values.[idfield] for the id.
	     */
	    if (vo->idfield && !strcmp(argv[0], argv[vo->index])) {
		set_next_object_id(q->validate->object_id, q->rtable);
		name = vo->idfield;
##		repeat retrieve (id = values.value) where values.#name = @name
##		inquire_equel(rowcount = "rowcount")
		if (rowcount != 1) return(SMS_LIST);
	    } else
	      return(SMS_LIST);
	}
    } else if (!strcmp(data_type, "machine")) {
	/* MACHINE */
	for (c = name; *c; c++) if (islower(*c)) *c = toupper(*c);
##      repeat retrieve (id = machine.mach_id) where machine.#name = @name
##      inquire_equel (rowcount = "rowcount")
	if (rowcount != 1) return(SMS_MACHINE);

    } else if (!strcmp(data_type, "string")) {
	/* STRING */
##      range of s is strings
##      repeat retrieve (id = s.string_id) where s.string = @name
##      inquire_equel (rowcount = "rowcount")
	if (rowcount == 0) {
	    if (q->type != APPEND) return(SMS_STRING);
##          range of v is values
##          retrieve (id = v.value) where v.#name = "strings_id"
	    id++;
##          replace v (value = id) where v.#name = "strings_id"
##	    append to strings (string_id = id, string = name)
	}
    } else if (!strcmp(data_type, "none")) {
	id = 0;
    } else {
	return(SMS_TYPE);
    }

    /* now set value in argv */
    *(int *)argv[vo->index] = id;
    
    return (SMS_EXISTS);
##}


/* This looks up a login name and returns the SMS internal ID.  It is used
 * by authenticate to put the users_id in the client structure.
 */

int get_users_id(name)
char *name;
##{
##  int id, rowcount;
##  char *login;

    login = name;

##  range of u is users
##  repeat retrieve (id = u.#users_id) where u.#login = @login
##  inquire_equel (rowcount = "rowcount")
    
    if (rowcount == 1)
	return(id);
    else
	return(0);
##}


/* Check the database at startup time.  For now this just resets the
 * inprogress flags that the DCM uses.
 */

sanity_check_database()
##{
##}
