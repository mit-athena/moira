#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.6 1988-07-27 19:19:40 kit Exp $";
#endif lint

/*	This is the file delete.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: functions for deleting users and lists.
 *	
 *	Created: 	5/18/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/delete.c,v 1.6 1988-07-27 19:19:40 kit Exp $
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
 *	Returns: SUB_ERROR if this list is an ace, or if the query did noe
 *               succeed.
 */

int
CheckIfAce(name, type, verbose)
char * name, * type;
Bool verbose;
{
    char * args[2], buf[BUFSIZ];
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
	if (verbose) {
	    sprintf(buf, "%s %s %s %s", type, name,
		    "is the Access Control List (ACE) for the following data",
		    "objects:\n");
	    Put_message(buf);
	    while (local != NULL) {
		char ** info = (char **) local->q_data;
		Print( CountArgs(info), info, NULL);
		local = local->q_forw;
	    }
	    Put_message(
                 "The ACE for each of these items must be changed before");
	    sprintf(buf,"the %s %s can be deleted.\n", type, name);
	    Put_message(buf);
	}
	break;
    default:
	com_err(program_name, status, " in CheckIfAce (get_ace_use).");
	return(SUB_ERROR);
    }
    FreeQueue(elem);
    return(SUB_ERROR);
}

/*	Function Name: RemoveItemFromLists
 *	Description: this function removes a list from all other lists of
 *                   which it is a member.
 *	Arguments: name - name of the item
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
    char *args[10], temp_buf[BUFSIZ];
    int lists;
    register int status;
    
    args[0] = type;
    args[1] = name;
    *elem = NULL;

/* 
 * Get all list of which this item is a member, and store them in a queue.
 */

    status = sms_query("get_lists_of_member", 2, args, StoreInfo,
		       (char *) elem);

    if (status == SMS_NO_MATCH)
	return(SUB_NORMAL);

    if (status != SMS_SUCCESS) {	
	com_err(program_name, status, " in DeleteList (get_lists_of_member).");
	return(SUB_ERROR);
    }

/*
 * If verbose mode, ask user of we should remove our list from 
 * all these lists.
 */

    local = *elem = QueueTop(*elem);
    lists = QueueCount(*elem);
    if (lists == 0)
	return(SUB_NORMAL);
    if (verbose) {
	sprintf(temp_buf, "%s %s is a member of %d other list%s.\n", type,
		name, lists, ((lists == 1) ? "" : "s") );
	Put_message(temp_buf);
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    Print( 1, &info[GLOM_NAME], (char *) NULL);
	    local = local->q_forw;
	}
	Put_message(" ");	/* Blank Line. */
	sprintf(temp_buf,"Remove %s %s from these lists? ", type, name);
	if (YesNoQuestion(temp_buf, FALSE) != TRUE) {
	    Put_message("Aborting...");
	    FreeQueue(*elem);
	    *elem = NULL;
	    return(SUB_ERROR);
	}
    }

/*
 * Remove this list from all lists that it is a member of.
 */

    local = *elem;
    args[DM_MEMBER] = name;
    args[DM_TYPE] = type;
    while (local != NULL) {
	char ** info = (char **) local->q_data;
	args[DM_LIST] = info[GLOM_NAME];
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
    int status, members;
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
    local = elem = QueueTop(elem);
    if ( (members = QueueCount(elem)) == 0)
	return(SUB_NORMAL);
    if (verbose) {
	sprintf(buf, "List %s has %d member%s:", name, QueueCount(elem),
		((members == 1) ? "" : "s") );
	Put_message(buf);
	Put_message(" ");	/* Blank Line. */
	while (local != NULL) {
	    char ** info = (char **) local->q_data;
	    Print( CountArgs(info), info, NULL);
	    local = local->q_forw;
	}
	Put_message(" ");	/* Blank Line. */
	sprintf(buf, "Remove th%s member%s from list %s? ", 
		((members == 1) ? "is" : "ese"), 
		((members == 1) ? "" : "s", name) );
	if ( YesNoQuestion(buf, FALSE) != TRUE) {
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
    
    switch(status = sms_query("get_list_info", 1, argv + 1, StoreInfo, &list)){
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
