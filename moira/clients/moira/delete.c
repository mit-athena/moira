#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.2 1988-06-10 18:36:30 kit Exp $";
#endif lint

/*	This is the file delete.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: functions for deleting users and lists.
 *	
 *	Created: 	5/18/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.2 1988-06-10 18:36:30 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

static int RealDeleteList(), RealDeleteUser();

/*	Function Name: CheckListForDeletion
 *	Description: Check one of the lists in which we just removed a member.
 *                   if the list is empty then it will delete it.
 *	Arguments: name - name of the list to check.
 *                 verbose - verbose mode?
 *	Returns: none.
 */

void
CheckListForDeletion(name, verbose)
char * name;
Bool verbose;
{
    struct qelem *elem = NULL;
    int status, ans;
    char *args[2], buf[BUFSIZ], **info;

    if ( (status = sms_query("count_members_of_list", 1, &name, StoreInfo,
			     (char *) &elem)) != 0) {
	com_err(program_name, status, 
		" in DeleteList (count_members_of_list).");
	return;
    }
    info = (char **) elem->q_data;
    if (info[0] == 0) {
	if (verbose) {
	    sprintf(buf, "Delete the empty list %s? ", name);
	    ans = YesNoQuestion(buf, TRUE);
	    if (ans != TRUE) {
		Put_message("Aborting Deletion!");
		FreeQueue(elem);
		return;	
	    }
	}
	args[0] = "foo";		/* not used. */
	args[1] = name;
	DeleteList(2, args);
    }
    FreeQueue(elem);
}

/*	Function Name: CheckAcl
 *	Description: Checks an acl to see of we should delete it.
 *	Arguments: type - the type of this acl.
 *                 name - the name of the acl.
 *                 verbose - query user?
 *	Returns: none.
 */

void
CheckAcl(type, name, verbose)
char * type, *name;
Bool verbose;
{
    char *args[2], buf[BUFSIZ];
    int status, ans;

    if ( strcmp(type, "LIST") != 0 ) 
	return;		/* If the acl is not a list the ignore it. */
 
    args[0] = type;
    args[1] = name;
    status = sms_query("get_acl_use", 2, args, NullFunc,  (char *) NULL);
    if (status != SMS_NO_MATCH)
	return;			/* If this query fails the acl will
				   not be deleted even if it is empty. */
    if (verbose) {
	sprintf(buf, "Delete the unused Access Control List (ACL) %s? ", name);
	ans = YesNoQuestion(buf, TRUE);
	if (ans != TRUE) {
	    Put_message("Aborting Deletion!");
	    return;
	}
    }
/*
 * Delete the ACL.
 *
 * NOTE: Delete list expects only the name of the list to delete in argv[1].
 *       since, 'args' already satisfies this, there is no need to create
 *       a special argument list.
 */
    DeleteList(2, args);
}
	

/*	Function Name: CheckIfAcl
 *	Description: Checks to see if this is an acl of another data object.
 *	Arguments: name - name of the object.
 *	Returns: SUB_ERROR if this list is an acl, or if the query did noe
 *               succeed.
 */

int
CheckIfAcl(name, type, verbose)
char * name, * type;
Bool verbose;
{
    char * args[2], buf[BUFSIZ];
    struct qelem *elem, *local;
    int status;
    elem = NULL;

    args[0] = type;
    args[1] = name;
    status = sms_query("get_acl_use", 2, args, StoreInfo, (char *) &elem);

    if (status == SMS_NO_MATCH) 
	return(SUB_NORMAL);

    if (status != 0) {
	com_err(program_name, status, " in CheckIfAcl (get_acl_use).");
	return(SUB_ERROR);
    }	
    if (verbose) {
	sprintf(buf, "%s %s %s %s", type, name,
		"is the Access Control List (ACL) for the following data",
		"objects:\n");
	Put_message(buf);
	local = elem;
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    Print( QueueCount(elem), info, NULL);
	    local = local->q_forw;
	}
	Put_message("The ACL for each of these items must be changed before");
	sprintf(buf,"the %s %s can be deleted.\n", type, name);
	Put_message(buf);
    }
    FreeQueue(elem);
    return(SUB_ERROR);
}

/*	Function Name: RemoveItemFromLists
 *	Description: this function removes a list from all other lists of
 *                   which it is a member.
 *	Arguments: name - name of the list.
 *                 elem - a pointer to a queue element. RETURNED
 *                 verbose - verbose mode.
 *	Returns: SUB_ERROR if there is an error.
 */

int
RemoveItemFromLists(name, type, elem, verbose)
char * name, *type;
struct qelem ** elem;
int verbose;
{
    struct qelem *local;
    char *args[2], temp_buf[BUFSIZ];
    int status, ans;
    
    args[0] = type;
    args[1] = name;
    *elem = NULL;

/* 
 * Get all list of which this list is a member, and store them in a queue.
 */

    status = sms_query("get_list_of_member", 2, args, StoreInfo,
		       (char *) elem);

    if (status == SMS_NO_MATCH)
	return(SUB_NORMAL);

    if (status != 0) {	
	com_err(program_name, status, " in DeleteList (get_lists_of_member).");
	return(SUB_ERROR);
    }

/*
 * If verbose mode, ask user of we should remove our list from 
 * all these lists.
 */

    if (verbose) {
	sprintf(temp_buf, "%s %s is a member of %d other list(s).\n", type,
		name, QueueCount(*elem) );
	Put_message(temp_buf);
	local = *elem;
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    Print( CountArgs(info), info, NULL);
	    local = local->q_forw;
	}
	sprintf(temp_buf,"Remove %s %s from these lists? ", type, name);
	ans = YesNoQuestion(temp_buf, TRUE);
	if (ans < 0 || ans == FALSE) {
	    Put_message("Aborting...");
	    FreeQueue(*elem);
	    return(SUB_ERROR);
	}
    }

/*
 * Remove this list from all lists that it is a member of.
 */

    local = *elem;
    args[1] = name;
    args[2] = type;
    while (local != NULL) {
	char ** info = (char **) local->q_data;
	args[0] = info[0];
	if ( (status = sms_query("delete_member_from_list",
				 3, args, Scream, NULL)) != 0) {
	    com_err(program_name, status, " in delete_member\nAborting\n");
	    FreeQueue(*elem);
	    return(SUB_ERROR);
	}
	local = local->q_forw;
    }
    return(SUB_NORMAL);
}

/*	Function Name: RemoveMembersOfList
 *	Description: Deletes the members of the list.
 *	Arguments: name - name of the list.
 *                 verbose - query user, about deletion?
 *	Returns: SUB_ERROR - if we could not delete, or the user abouted.
 */

int
RemoveMembersOfList(name, verbose)
char * name;
Bool verbose;
{
    char buf[BUFSIZ], *args[10];
    struct qelem *local, *elem = NULL;
    int status, ans;

/* 
 * Get the members of this list.
 */

    status = sms_query("get_members_of_list", 1, &name, StoreInfo,
		       (char *) &elem);
    if (status == SMS_NO_MATCH) 
	return(SUB_NORMAL);

    if (status != 0) {	
	com_err(program_name, status, " in DeleteList (get_members_of_list).");
	return(SUB_ERROR);
    }

/*
 * If verbose mode, then ask the user if we should delete.
 */

    if (verbose) {
	sprintf(buf, "List %s has %d member(s):", name, QueueCount(elem) );
	Put_message(buf);
	local = elem;
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    Print( CountArgs(info), info, NULL);
	    local = local->q_forw;
	}
	sprintf(buf, "Remove these member(s) from list %s? ", name);
	ans = YesNoQuestion(buf, TRUE);
	if (ans < 0 || ans == FALSE) {
	    Put_message("Aborting...");
	    FreeQueue(elem);
	    return(SUB_ERROR);
	}
    }

/*
 * Perform The Removal.
 */

    local = elem;
    args[0] = name; 
    while (local != NULL) {
	char ** info = (char **) local->q_data;
	args[1] = info[0];
	args[2] = info[1];
	if ( (status = sms_query("delete_member_from_list",
				 3, args, Scream, NULL)) != 0) {
	    com_err(program_name, status, " in delete_member\nAborting\n");
	    FreeQueue(elem);
	    return(SUB_ERROR);
	}
	local = local->q_forw;
    }
    return(SUB_NORMAL);
}


/*	Function Name: DeleteUserGroup
 *	Description: Deletes the list given by name if it exists.
 *                   intended to be used to delete user groups
 *	Arguments: name - the name of the list to delete.
 *                 verbose - flag that if TRUE queries the user to
 *                           ask if list should be deleted.
 *	Returns: SMS_ERROR if there is an error.
 */

int
DeleteUserGroup(name, verbose)
char * name;
Bool verbose;
{
    int status, ans;
    char buf[BUFSIZ], *args[10];

    status = sms_query("get_list_info", 1, &name, NullFunc, (char *) NULL);
    if (status == 0) {
	if (verbose) {
	    sprintf(buf, "There is also a list named %s, delete it?", name);
	    ans = YesNoQuestion(buf, TRUE);
	    if (ans == FALSE) {
		Put_message("Leaving group alone.");
		return(SUB_NORMAL);
	    }
	    if (ans < 0) {
		Put_message("Aborting...\n");
		return(SUB_ERROR);
	    }
	}
	/* ans == TRUE  || ~verbose */
	    args[0] = "foo";	/* not used. */
	    args[1] = name;
	    DeleteList(2, args);
    }
    else if (status != SMS_NO_MATCH) {
	com_err(program_name, status, " Aborting Delete User.");	
	return(SUB_ERROR);
    }
    return(SUB_NORMAL);
}

/*	Function Name: DeleteHomeFilesys
 *	Description: Delete the home filesystem for the named user.
 *	Arguments: name - name of the user (and filsystem) to delete.
 *                 verbose - if TRUE query user.
 *	Returns: SUB_NORMAL if home filesystem deleted, or nonexistant.
 */

int
DeleteHomeFilesys(name, verbose)
char * name;
Bool verbose;
{
    int status, ans;
    char buf[BUFSIZ];
    
    status = sms_query("get_filesystem_by_label", 1, &name, NullFunc, 
		       (char *) NULL);
    if (status == SMS_NO_MATCH)
	return(SUB_NORMAL);
    if (status == 0) {
	if (verbose) {
	    sprintf(buf, "Delete the filesystem named %s (y/n)?", name);
	    ans = YesNoQuestion(buf, TRUE);
	    if (ans != TRUE) {
		Put_message("Filesystem Not Deleted, aborting...\n\n");
		return(SUB_ERROR);
	    }
	}
	if ( (status = sms_query("delete_filesystem", 1, &name, Scream,
				 (char *) NULL) ) != 0) {
	    com_err(program_name, status, " in delete_filesystem).");
	    return(SUB_ERROR);
	}
	return(SUB_NORMAL);
    }
    com_err(program_name, status, " in get_filesystem_by_label).");
    return(SUB_ERROR);
}

/*	Function Name: DeleteAllUserQuotas
 *	Description: Deletes all quotas for a given user.
 *	Arguments: name - name of the user.
 *                 verbose - ask before performing deletion.
 *	Returns: SUB_NORMAL if no quotas, or all quotas removed.
 */

int
DeleteAllUserQuotas(name, verbose)
char * name;
int verbose;
{
    int status, ans;
    char buf[BUFSIZ], *args[10];
    struct qelem *local, *elem = NULL;
    
    status = sms_query("get_nfs_quotas_by_user", 1, &name, StoreInfo, 
		       (char *) &elem);
    if (status == SMS_NO_MATCH)
	return(SUB_NORMAL);
    if (status != 0) {
	com_err(program_name, status, " in delete_filesystem.");
	return(SUB_ERROR);
    }
    if (verbose) {
	sprintf(buf, "User %s has quotas on the following filsystems:");
	Put_message(buf);
	local = elem;
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    sprintf(buf, "Filesystem:\t%s\t\tQuota(in Kb):\t%s",
		    info[0], info[4]);
	    Put_message(buf);
	    local = local->q_forw;
	}
	ans = YesNoQuestion("Remove ** ALL ** these quota entries (y/n)?",
			    TRUE);
	if (ans != TRUE) {
	    Put_message("Aborting..\n\n");
	    return(SUB_ERROR);
	}
    }
    local = elem;
    args[1] = name;
    while (local != elem) {
	char ** info = (char **) local->q_data;
	args[0] = info[0];
	status = sms_query("delete_nfs_quota", 2, args, Scream, (char *) NULL);
	if (status != 0 && status != SMS_NO_MATCH) {
	    sprintf(buf,
		    "Could not remove quota on filesystem %s. ** ABORTING **",
		    args[0]);
	    Put_message(buf);
	    com_err(program_name, status, (char *) NULL);
	    FreeQueue(elem);
	    return(SUB_ERROR);
	}
	local = local->q_forw;
    }
    FreeQueue(elem);
    return(SUB_NORMAL);
}

/*	Function Name: DeleteList
 *	Description: deletes a list
 *	Arguments: argc, argv - standard SMS argc and argv.
 *	Returns: DM Status Code.
 */

int
DeleteList(argc, argv)
int argc;
char *argv[];
{
    char *list, **info;
    struct qelem *local, *member_of, *list_info;
    int status;

    member_of = list_info = NULL;

    list = argv[1];
    if (!Confirm("Are you sure that you want to delete this list?"))
	return(DM_NORMAL);
/*
 * Get Information about a list.
 */
    if ( (status = sms_query("get_list_info", 1, &list, StoreInfo, 
		       (char *) &list_info)) != 0) {
	com_err(program_name, status,	" in DeleteList (delete_list).");
	return(DM_NORMAL);
    }

/*
 * Attempt delete. - will only work if:
 * 1) This list has no members.
 * 2) This list in a member of no other lists.
 * 3) This list is not an acl of another object.
 */

    status = sms_query("delete_list", 1, &list, Scream, (char *) NULL);
    if (status != SMS_IN_USE && status != 0) {
	com_err(program_name, status,	" in DeleteList (delete_list).");
	return(DM_NORMAL);
    }
    if (status == 0) {
	Put_message("List Sucessfully Deleted.");
    }
    else  {			/* status = SMS_IN_USE. */

/* 
 * This list is in use.  Try to find out why, and for the cases where we
 * have a good idea of what to do we will query and then do it.
 */

	if ( (CheckIfAcl(list, "list", TRUE) == SUB_ERROR) ||
	    (RemoveItemFromLists(list, "list",
				 &member_of, TRUE) == SUB_ERROR) ||
	    (RemoveMembersOfList(list, TRUE) == SUB_ERROR) ||
	    (RealDeleteList(list) == SUB_ERROR) ) {
	    
	    FreeQueue(list_info);
	    FreeQueue(member_of);
	    return(DM_NORMAL);	
	}
    }

    info = (char **) list_info->q_data;
    CheckAcl(info[L_ACL_TYPE], info[L_ACL_NAME], TRUE);

    local = member_of;
    while (local != NULL) {
	info = (char **) local->q_data;
	CheckListForDeletion(info[LM_LIST], TRUE);
	local = local->q_forw;
    }

    FreeQueue(list_info);
    FreeQueue(member_of);
    return(DM_NORMAL);
}

/*	Function Name: RealDeleteList
 *	Description: Just Deletes the list.
 *	Arguments: name - name of list to delete
 *	Returns: SUB_ERROR if the deletion failed.
 */

static int
RealDeleteList(name)
char * name;
{
    char buf[BUFSIZ];
    int status;

    if ( (status = sms_query("delete_list", 1, &name, Scream, 
			     (char *) NULL)) == NULL) {
	com_err(program_name, status, ": list not deleted");
	return(SUB_ERROR);
    }
    (void) sprintf(buf, "List %s deleted./n", name);
    Put_message(buf);
    return(SUB_NORMAL);
}

/*	Function Name: DeleteUser
 *	Description: Deletes a user from the database.
 *	Arguments: argc, argv - name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int
DeleteUser(argc, argv)
int argc;
char ** argv;
{
    int status;
    char buf[BUFSIZ];
    char * name = argv[1];	/* name of the user we are deleting. */
    struct qelem *local, *member_of = NULL;

    if (!Confirm("Are you sure that you want to delete this user?"))
	return(DM_NORMAL);

    status = sms_query("delete_user", 1, &name, Scream, (char *) NULL);
    if (status != SMS_IN_USE && status != 0) {
	com_err(program_name, status, ": list not deleted");	
	return(DM_NORMAL);
    }
    if (status == 0) {
	sprintf(buf,"User %s deleted.", name);
	Put_message(buf);
    }
    else if (status == SMS_IN_USE) {

/*
 * Check:
 * 1) Is the user an ACL of any object in the database?
 * 2) Query - Delete home filesytem.
 * 3) Query - Delete users quota on all machines.	
 * 4) Query - Remove user from all list of which he is a member.
 *
 * If all these have been accomplished, then attempt to delete the user again.
 */
	if ( (CheckIfAcl(name, "user", TRUE) == SUB_ERROR) ||
	     (DeleteHomeFilesys(name, TRUE) == SUB_ERROR) ||
	     (DeleteAllUserQuotas(name, TRUE) == SUB_ERROR) ||
	     (RemoveItemFromLists(name, "user",
				  &member_of, TRUE) == SUB_ERROR) ||
	     (RealDeleteUser(name) == SUB_ERROR) ) {
	    FreeQueue(member_of);
	    return(DM_NORMAL);
	}
    }

/*
 * Query - Delete user group.
 * Query - Delete all empty lists created by removing this user from them.
 */

    if (DeleteUserGroup(name, TRUE) == SUB_ERROR) {
	FreeQueue(member_of);
	return(DM_NORMAL);
    }

    local = member_of;
    while (local != NULL) {
	char ** info = (char **) local->q_data;
	CheckListForDeletion(info[0], TRUE);
	local = local->q_forw;
    }

    FreeQueue(member_of);	/* Free memory and return. */
    return(DM_NORMAL);
}

/*	Function Name: RealDeleteUser
 *	Description: Just Deletes the user.
 *	Arguments: name - name of User to delete
 *	Returns: SUB_ERROR if the deletion failed.
 */

static int
RealDeleteUser(name)
char * name;
{
    char buf[BUFSIZ];
    int status;

    if ( (status = sms_query("delete_user", 1, &name, Scream, 
			     (char *) NULL)) == NULL) {
	com_err(program_name, status, ": user not deleted");
	return(SUB_ERROR);
    }
    (void) sprintf(buf, "User %s deleted./n", name);
    Put_message(buf);
    return(SUB_NORMAL);
}

    
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
