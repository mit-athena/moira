#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.8 1988-08-07 17:31:15 mar Exp $";
#endif lint

/*	This is the file delete.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: functions for deleting users and lists.
 *	
 *	Created: 	5/18/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.8 1988-08-07 17:31:15 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"
#include "infodefs.h"

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
    int status;
    char *args[2], buf[BUFSIZ], **info;

    if ( (status = sms_query("count_members_of_list", 1, &name, StoreInfo,
			     (char *) &elem)) != 0) {
	com_err(program_name, status, 
		" in DeleteList (count_members_of_list).");
	return;
    }
    info = (char **) elem->q_data;
    if ( strcmp(info[NAME],"0") == 0) {
	if (verbose) {
	    sprintf(buf, "Delete the empty list %s? ", name);
	    if (YesNoQuestion(buf, FALSE) != TRUE) {
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

/*	Function Name: CheckAce
 *	Description: Checks an ace to see of we should delete it.
 *	Arguments: type - the type of this ace.
 *                 name - the name of the ace.
 *                 verbose - query user?
 *	Returns: none.
 */

void
CheckAce(type, name, verbose)
char * type, *name;
Bool verbose;
{
    char *args[2], buf[BUFSIZ];
    int status;

    if ( strcmp(type, "LIST") != 0 ) 
	return;		/* If the ace is not a list the ignore it. */
 
    args[0] = type;
    args[1] = name;
    status = sms_query("get_ace_use", 2, args, NullFunc,  (char *) NULL);
    if (status != SMS_NO_MATCH)
	return;			/* If this query fails the ace will
				   not be deleted even if it is empty. */
    if (verbose) {
	sprintf(buf, "Delete the unused Access Control Entity (ACE) %s? ",
		name);
	if ( YesNoQuestion(buf, FALSE) != TRUE) {
	    Put_message("Aborting Deletion!");
	    return;
	}
    }
/*
 * Delete the ACE.
 *
 * NOTE: Delete list expects only the name of the list to delete in argv[1].
 *       since, 'args' already satisfies this, there is no need to create
 *       a special argument list.
 */
    DeleteList(2, args);
}
	

/*	Function Name: CheckIfAce
 *	Description: Checks to see if this is an ace of another data object.
 *	Arguments: name - name of the object.
 *	Returns: SUB_ERROR if this list is an ace, or if the query did not
 *               succeed.
 */

int
CheckIfAce(name, type, verbose)
char * name, * type;
Bool verbose;
{
    char * args[2], buf[BUFSIZ], **info;
    struct qelem *local, *elem;
    int status;
    elem = NULL;

    args[0] = type;
    args[1] = name;
    switch (status = sms_query("get_ace_use", 2, args,
			       StoreInfo, (char *) &elem)) {
    case SMS_NO_MATCH:
	return(DM_NORMAL);
    case SMS_SUCCESS:
	local = elem = QueueTop(elem);
	info = (char **) local->q_data;
	if (QueueCount(elem) == 1 &&
	    !strcmp(info[0], "LIST") &&
	    !strcmp(info[1], name)) {
	    FreeQueue(elem);
	    return(DM_NORMAL);
	}
	if (verbose) {
	    sprintf(buf, "%s %s %s", type, name,
		    "is the ACE for the following data objects:");
	    Put_message(buf);
	    Put_message("");
	    while (local != NULL) {
		info = (char **) local->q_data;
		if (!strcmp(info[0], "LIST") &&
		    !strcmp(info[1], name))
		    continue;
		print( countargs(info), info, null);
		local = local->q_forw;
	    }
	    put_message("");
	    put_message(
                 "the ace for each of these items must be changed before");
	    sprintf(buf,"the %s %s can be deleted.\n", type, name);
	    put_message(buf);
	}
	break;
    default:
	com_err(program_name, status, " in checkiface (get_ace_use).");
	return(sub_error);
    }
    freequeue(elem);
    return(sub_error);
}

/*	function name: removeitemfromlists
 *	description: this function removes a list from all other lists of
 *                   which it is a member.
 *	arguments: name - name of the item
 *                 elem - a pointer to a queue element. returned
 *                 verbose - verbose mode.
 *	returns: sub_error if there is an error.
 */

int
removeitemfromlists(name, type, elem, verbose)
char * name, *type;
struct qelem ** elem;
int verbose;
{
    struct qelem *local;
    char *args[10], temp_buf[bufsiz];
    int lists;
    register int status;
    
    args[0] = type;
    args[1] = name;
    *elem = null;

/* 
 * get all list of which this item is a member, and store them in a queue.
 */

    status = sms_query("get_lists_of_member", 2, args, storeinfo,
		       (char *) elem);

    if (status == sms_no_match)
	return(sub_normal);

    if (status != sms_success) {	
	com_err(program_name, status, " in deletelist (get_lists_of_member).");
	return(sub_error);
    }

/*
 * if verbose mode, ask user of we should remove our list from 
 * all these lists.
 */

    local = *elem = queuetop(*elem);
    lists = queuecount(*elem);
    if (lists == 0)
	return(sub_normal);
    if (verbose) {
	sprintf(temp_buf, "%s %s is a member of %d other list%s.\n", type,
		name, lists, ((lists == 1) ? "" : "s") );
	put_message(temp_buf);
	while (local != null) {
	    char ** info = (char **) local->q_data;
	    print( 1, &info[glom_name], (char *) null);
	    local = local->q_forw;
	}
	put_message(" ");	/* blank line. */
	sprintf(temp_buf,"remove %s %s from these lists? ", type, name);
	if (yesnoquestion(temp_buf, false) != true) {
	    put_message("aborting...");
	    freequeue(*elem);
	    *elem = null;
	    return(sub_error);
	}
    }

/*
 * remove this list from all lists that it is a member of.
 */

    local = *elem;
    args[dm_member] = name;
    args[dm_type] = type;
    while (local != null) {
	char ** info = (char **) local->q_data;
	args[dm_list] = info[glom_name];
	if ( (status = sms_query("delete_member_from_list",
				 3, args, scream, null)) != 0) {
	    com_err(program_name, status, " in delete_member\naborting\n");
	    freequeue(*elem);
	    return(sub_error);
	}
	local = local->q_forw;
    }
    return(sub_normal);
}

/*	function name: removemembersoflist
 *	description: deletes the members of the list.
 *	arguments: name - name of the list.
 *                 verbose - query user, about deletion?
 *	returns: sub_error - if we could not delete, or the user abouted.
 */

int
removemembersoflist(name, verbose)
char * name;
bool verbose;
{
    char buf[bufsiz], *args[10];
    struct qelem *local, *elem = null;
    int status, members;
/* 
 * get the members of this list.
 */
    status = sms_query("get_members_of_list", 1, &name, storeinfo,
		       (char *) &elem);
    if (status == sms_no_match) 
	return(sub_normal);

    if (status != 0) {	
	com_err(program_name, status, " in deletelist (get_members_of_list).");
	return(sub_error);
    }
/*
 * if verbose mode, then ask the user if we should delete.
 */
    local = elem = queuetop(elem);
    if ( (members = queuecount(elem)) == 0)
	return(sub_normal);
    if (verbose) {
	sprintf(buf, "list %s has %d member%s:", name, queuecount(elem),
		((members == 1) ? "" : "s") );
	put_message(buf);
	put_message(" ");	/* blank line. */
	while (local != null) {
	    char ** info = (char **) local->q_data;
	    print( countargs(info), info, null);
	    local = local->q_forw;
	}
	put_message(" ");	/* blank line. */
	sprintf(buf, "remove th%s member%s from list %s? ", 
		((members == 1) ? "is" : "ese"), 
		((members == 1) ? "" : "s"), name );
	if ( yesnoquestion(buf, false) != true) {
	    put_message("aborting...");
	    freequeue(elem);
	    return(sub_error);
	}
    }
/*
 * perform the removal.
 */
    local = elem;
    args[0] = name; 
    while (local != null) {
	char ** info = (char **) local->q_data;
	args[1] = info[0];
	args[2] = info[1];
	if ( (status = sms_query("delete_member_from_list",
				 3, args, scream, null)) != 0) {
	    com_err(program_name, status, " in delete_member\naborting\n");
	    freequeue(elem);
	    return(sub_error);
	}
	local = local->q_forw;
    }
    return(sub_normal);
}

/*	function name: deleteusergroup
 *	description: deletes the list given by name if it exists.
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
	    ans = YesNoQuestion(buf, FALSE);
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
    int status;
    char buf[BUFSIZ];
    
    switch (status = sms_query("get_filesys_by_label", 1, &name, NullFunc, 
		       (char *) NULL)) {
    case SMS_NO_MATCH:
	break;
    case SMS_SUCCESS:
	if (verbose) {
	    sprintf(buf, "Delete the filesystem named %s (y/n)?", name);
	    switch (YesNoQuestion(buf, FALSE)) {
	    case FALSE:
		Put_message("Filesystem Not Deleted, continuing...\n");
		return(SUB_NORMAL);
	    case TRUE:
		break;
	    default:
		Put_message("Filesystem Not Deleted, aborting...\n\n");
		return(SUB_ERROR);
	    }
	}
	if ( (status = sms_query("delete_filesys", 1, &name, Scream,
				 (char *) NULL) ) != SMS_SUCCESS) {
	    com_err(program_name, status, " in delete_filesys.");
	    return(SUB_ERROR);
	}
	else 
	    Put_message("Filesystem Successfully Deleted.");
	break;
    default:
	com_err(program_name, status, " in get_filesystem_by_label).");
	return(SUB_ERROR);
    }
    return(SUB_NORMAL);
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
			     (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, status, ": user not deleted");
	return(SUB_ERROR);
    }
    (void) sprintf(buf, "User %s deleted.", name);
    Put_message(buf);
    return(SUB_NORMAL);
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
			     (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, status, ": list not deleted");
	return(SUB_ERROR);
    }
    (void) sprintf(buf, "List %s deleted.", name);
    Put_message(buf);
    Put_message("");
    return(SUB_NORMAL);
}

/*	Function Name: AttemptToDeleteList
 *	Description: Atempts to delete list, in the following manner:
 *                   1) try to delet it, if this fails in a known error then
 *                      a) try to clean up each of those known methods, or
 *                         at least explain why we failed.
 *	Arguments: list_info - info about this list.
 *                 ask_first - (T/F) query user before preparing for deletion,
 *                             and cleaning up?
 *	Returns: none - all is taken care of and error messages printed
 *                      one way or the other.
 */

void
AttemptToDeleteList(list_info, ask_first)
char ** list_info;
Bool ask_first;
{
    int status;
    struct qelem *local, *member_of;
    char *name = list_info[L_NAME];
    member_of = NULL;
    
    /*
     * Attempt delete. - will only work if:
     * 1) This list has no members.
     * 2) This list in a member of no other lists.
     * 3) This list is not an ace of another object.
     */
    
    switch (status = sms_query("delete_list", 1, &name,
			       Scream, (char *) NULL)) {
    case SMS_SUCCESS:
	Put_message("List Sucessfully Deleted.");
	CheckAce(list_info[L_ACE_TYPE], list_info[L_ACE_NAME], ask_first);
	break;
    case SMS_IN_USE:
	/* 
	 * This list is in use.  Try to find out why, 
	 * and for the cases where we have a good idea of 
	 * what to do we will query and then do it.
	 */
	
	if ( (CheckIfAce(name, "list", ask_first) == SUB_NORMAL) &&
	    (RemoveMembersOfList(name, ask_first) == SUB_NORMAL) &&
	    (RemoveItemFromLists(name, "list",
				 &member_of, ask_first) == SUB_NORMAL) &&
	    (RealDeleteList(name) == SUB_NORMAL) ) 
	{		/* if... */
	    CheckAce(list_info[L_ACE_TYPE], list_info[L_ACE_NAME], ask_first);
	    
	    local = QueueTop(member_of);
	    while (local != NULL) {
		char ** info = (char **) local->q_data;
		CheckListForDeletion(info[LM_LIST], ask_first);
		local = local->q_forw;
	    }
	    FreeQueue(member_of);
	}
	break;
    default:
	com_err(program_name, status, " in DeleteList (delete_list).");
	break;
    }
}

/*	Function Name: DeleteList
 *	Description: deletes a list
 *	Arguments: argc, argv - standard SMS argc and argv.
 *	Returns: DM Status Code.
 */

/* ARGSUSED */
int
DeleteList(argc, argv)
int argc;
char *argv[];
{
    char buf[BUFSIZ];
    struct qelem *top, *list;
    register int status;
    Bool one_list;

    list = NULL;
    
    switch(status = sms_query("get_list_info", 1, argv + 1, StoreInfo,
			      (char *) &list)){
    case SMS_SUCCESS:
	break;
/*    case SMS_NO_WILDCARD:
	Put_message("Wildcards are not accepted here.");
	return(DM_NORMAL);
*/  case SMS_NO_MATCH:
    case SMS_LIST:
	Put_message("There is no list that matches that name.");
	return(DM_NORMAL);
    default:
	com_err(program_name, status,	" in DeleteList (get_list_info).");
	return(DM_NORMAL);
    }
    
    top = list = QueueTop(list);
    one_list = (QueueCount(list) == 1);
    while (list != NULL) {
	char ** info = (char**) list->q_data;
	if (one_list) {
	    sprintf( buf, "Are you sure that you want to delete the list %s",
		    info[L_NAME]);
	    if ( Confirm(buf) ) AttemptToDeleteList(info, TRUE);
	}
	else {
	    sprintf(buf, "Delete the list %s", info[L_NAME]);
	    switch( YesNoQuestion( buf, FALSE ) ) {
	    case TRUE:
		AttemptToDeleteList(info, TRUE);
		break;
	    case FALSE:
		break;
	    default:
		Put_message ("Aborting...");
		FreeQueue(top);
		return(DM_NORMAL);
	    }
	}
	list = list->q_forw;
    }
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: DeleteUser
 *	Description: Deletes a user from the database.
 *	Arguments: argc, argv - name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteUser(argc, argv)
int argc;
char ** argv;
{
    int status;
    char buf[BUFSIZ];
    char * name = argv[1];	/* name of the user we are deleting. */
    struct qelem *local, *member_of = NULL;

    if (!ValidName(name))
	return(DM_NORMAL);

    if (!Confirm("Are you sure that you want to delete this user?"))
	return(DM_NORMAL);

    status = sms_query("delete_user", 1, &name, Scream, (char *) NULL);
    if (status != SMS_IN_USE && status != 0) {
	com_err(program_name, status, ": user not deleted");
	return(DM_NORMAL);
    }
    if (status == 0) {
	sprintf(buf,"User %s deleted.", name);
	Put_message(buf);
    }

/* By design decision, if a user has been registered, we will not
 * delete them.  So if the simple delete fails, give up.
 */
    Put_message("Sorry, registered users cannot be deleted.");

#ifdef notdef
    else if (status == SMS_IN_USE) {

/*
 * Check:
 * 1) Query - Delete home filesytem.
 * 2) Query - Delete user Group.
 * 2) Is the user an ACE of any object in the database?
 * 3) Query - Remove user from all list of which he is a member.
 *
 * If all these have been accomplished, then attempt to delete the user again.
 */
	if ( (DeleteHomeFilesys(name, TRUE) == SUB_ERROR) ||
	     (DeleteUserGroup(name, TRUE) == SUB_ERROR)  ||
	     (CheckIfAce(name, "user", TRUE) == SUB_ERROR) ||
	     (RemoveItemFromLists(name, "user",
				  &member_of, TRUE) == SUB_ERROR) ||
	     (RealDeleteUser(name) == SUB_ERROR) ) {
	    return(DM_NORMAL);
	}
    }

/*
 * Query - Delete all empty lists created by removing this user from them.
 */

    local = member_of;
    while (local != NULL) {
	char ** info = (char **) local->q_data;
	CheckListForDeletion(info[0], TRUE);
	local = local->q_forw;
    }

    FreeQueue(member_of);	/* Free memory and return. */
#endif
    return(DM_NORMAL);
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
