#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v 1.1 1988-06-09 14:13:11 kit Exp $";
#endif lint

/*	This is the file lists.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: 
 *	
 *	Borrowed from listmaint code: 	4/12/88
 *	By:		                Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v 1.1 1988-06-09 14:13:11 kit Exp $
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
#include "globals.h"
#include "infodefs.h"

#define LIST    0
#define MEMBERS 1
#define GLOM    2
#define ACL_USE 3

/*	Function Name: PrintListInfo
 *	Description: This function Prints out the List info in a coherent form.
 *	Arguments: info - the List info.
 *	Returns: none.
 */

void
PrintListInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    (void) sprintf(buf, "List: %s",info[L_NAME]);
    (void) Put_message(buf);
    (void) sprintf(buf, "\nDescription: %s", info[L_DESC]);
    (void) Put_message(buf);
    if ( atoi(info[L_MAILLIST]))
	Put_message("This list is a mailing list.");
    else
	Put_message("This list is NOT a mailing list.");
    if ( atoi(info[L_GROUP]) ) {
	(void) sprintf(buf,"This list is Group and its ID number is %s",
		       info[L_GID]);
	Put_message(buf);
    }
    else
	Put_message("This is NOT a Group.");

    if (strcmp(info[L_ACL_TYPE],"NONE") == 0)
	Put_message("This list has no Administrator, how strange?!");
    else {
	sprintf(buf, "The List Administrator of this list is the %s: %s",
		info[L_ACL_TYPE], info[ACL_NAME]);
	Put_message(buf);
    }

    (void) sprintf(buf, "This list is: %s, %s, and %s",
		   info[L_ACTIVE] ? "active" : "inactive",
		   info[L_PUBLIC] ? "public" : "private",
		   info[L_HIDDEN] ? "hidden" : "visible");
    (void) Put_message(buf);
    (void) sprintf(buf, "Last modification at %s, by %s using the program %s",
		   info[L_MODTIME], info[L_MODBY], info[L_MODWITH]);
    (void) Put_message(buf);
}

/*	Function Name: GetListInfo
 *	Description: Stores all info about a group of lists in a queue.
 *	Arguments: type - type of info to store.
 *                 name - name of the info.
 *	Returns: the first element in the queue.
 */

/* ARGSUSED */
struct qelem *
GetListInfo(type, name1, name2)
int type;
char * name;
{
    char *args[2];
    struct quelem * elem = NULL;

    switch(type) {
    case LIST:
	args[0] = name1;
	if ( (status = sms_query("get_list_info", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(whoami, status, " in get_nfs_quotas_by_user");
	    return (NULL);
	}
	break;
    case MEMBERS:
	args[0] = name1;
	if ( (status = sms_query("get_members_of_list", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(whoami, status, " in get_nfs_quotas_by_user");
	    return (NULL);
	}
	break;
    case GLOM:
	args[0] = name1; 	
	args[1] = name2; 	
	if ( (status =  sms_query("get_list_of_member", 2, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(whoami, status, " in get_nfs_quotas_by_user");
	    return (NULL);
	}
	break;
    case ACL_USE:
	args[0] = name1; 	
	args[1] = name2; 	
	if ( (status =  sms_query("get_acl_use", 2, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(whoami, status, " in get_nfs_quotas_by_user");
	    return (NULL);
	}
	break;
    }
    return( QueueTop(elem) );
}	

/*	Function Name: AskListInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *                 name - add a newname field? (T/F)
 *	Returns: SUB_ERROR or SUB_NORMAL.
 */

char **
AskListInfo(info, name);
char ** info;
Bool name;
{
    char * temp_buf, *newname;
    char *ret_args[100];
    int counter;

    sprintf(temp_buf,"\nChanging Attributes of list %s.\n",info[L_NAME]);
    Put_message(temp_buf);

    if (name) {
	newname = Strsave(info[L_NAME]);
	GetValueFromUser("The new name for this list.", &newname);
    }
    GetValueFromUser("Is this list active (1/0): ", &info[L_ACTIVE]);
    GetValueFromUser("Is this list public (1/0): ", &info[L_PUBLIC]);
    GetValueFromUser("Is this list hidden (1/0): ", &info[L_HIDDEN]);
    GetValueFromUser("Is this a maillist  (1/0): ", &info[L_MAILLIST]);
    GetValueFromUser("is this a group     (1/0): ", &info[L_GROUP]);
    if (atoi(info[L_GROUP])) 
	GetValueFromUser("What is the Gid for this group '#' get unique gid:".
			 &info[L_GID]);
    GetValueFromUser("What Type of Administrator (none, user, list): ",
		     &info[L_ACL_TYPE]);
    if ( strcmp(info[L_ACL_TYPE],"USER") == 0) 
	GetValueFromUser("Who will be the administrator of this list: ",
			 &info[L_ACL_NAME]);
    if ( strcmp(info[L_ACL_TYPE],"USER") == 0) 
       GetValueFromUser("Which group will be the administrator of this list: ",
			 &info[L_ACL_NAME]);
    GetValueFromUser("Description: ", &info[L_DESC]);

    FreeAndClear(&info[L_MODTIME], TRUE);
    FreeAndClear(&info[L_MODBY], TRUE);
    FreeAndClear(&info[L_MODWITH], TRUE);
/* 
 * Slide the newname into the #2 slot, this screws up all future references 
 * to this list.
 */
    if (name) 			/* slide the newname into the #2 slot. */
	SlipInNewName(info, newname);

    return(info);
}

/* -------------- List functions. -------------- */

/*	Function Name: ShowListInfo.
 *	Description: shows info on a list.
 *	Arguments: argc, argv - standard SMS argc and argv.
 *	Returns: DM status code.
 */

int
ShowListInfo(argc, argv)
int argc;
char **argv;
{
    struct qelem *top, *list;

    top = list = StoreListInfo(LIST, argv[1], (char *) NULL);
    while (list != NULL) {
	PrintListInfo( (char **) list->q_data);
	list = list->q_forw;
    }
    
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: UpdateList
 *	Description: 
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM Status code.
 */

int
UpdateList(argc, argv)
int argc;
char **argv;
{
    char **info, **up_args;
    struct qelem *top, *list;
    int status;

    list = top = GetListInfo(LIST, argv[1], (char *) NULL);
    
    while (list != NULL) {
	info = (char **) list->q_data;
	up_args = AskListInfo(info, TRUE);
	if ( (status = sms_query("update_list", CountArgs(up_args), up_args,
				 Scream, (char *) NULL)) != 0) {
	    com_err(whoami, status, " in UpdateList.");	
	    Put_message("List Not Updated.");
	    list = list->q_forw;
	}
    }

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: AddList
 *	Description: 
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM Status code.
 */

int
AddList(argc, argv)
int argc;
char **argv;
{
    static char *info[MAX_ARGS_SIZE], **add_args;
    struct qelem *top, *list;
    int status;

    if (!ValidName(argv[1])) /* Check if list name is valid. */
	return(DM_QUIT);
    
    status = sms_query("get_list_info", 1, argv + 1, NullFunc, 
		       (caddr_t) NULL);
    if (status != SMS_NO_MATCH) {
	if (status == 0)
	    Put_message("This list already exists.");
	else
	    com_err(whoami, status, " in AddList.");	
	return(DM_QUIT);
    }

    add_args = AskListInfo(info, FALSE);

    if ( (status = sms_query("add_list", CountArgs(add_args), add_args,
			     Scream, (char *) NULL)) != 0) {
	com_err(whoami, status, " in AddList.");	
	Put_message("List Not Created.");
	list = list->q_forw;
    }

    FreeInfo(info);
    return(DM_NORMAL);
}

/*	Function Name: Instructions
 *	Description: This func prints out instruction on manipulating lists.
 *	Arguments: none
 *	Returns: DM Status Code.
 */

int
Instructions()
{
    static char * message[] = {
	"Listmaint handles the creation, deletion, and updating of lists.\n",
	"A list can be a mailing list, a group list, or both.\n",
	"The concept behind lists is that a list has an owner",
	"- administrator -  and members.\n";
	"The administrator of a list may be another list.\n",
	"The members of a list can be users (login names), other lists,",
	"or address strings.\n",
	"You can use certain keys to do the following:",
	"    Refresh the screen - Type ctrl-L.\n",
	"    Escape from a function - Type ctrl-C.\n",
	"    Suspend the program (temporarily) - Type ctrl-Z.\n",
	NULL,
    }

    return(PrintHelp(message));
}

/*-*-* LISTMAINT UPDATE MENU *-*-*/

/*	Function Name: ListmaintMemberMenuEntry
 *	Description: entry routine into the listmaint member menu.
 *	Arguments: m - the member menu.
 *                 argc, argv - name of the list in argv[1].
 *	Returns: none.
 */

/* ARGSUSED */
int
ListmaintMemberMenuEntry(m, argc, argv)
Menu *m;
int argc;
char **argv;
{
    char buf[BUFSIZ], temp_buf[BUFSIZ];
    char *list_name = argv[1];

    if (*argv[0] == 'a') {	/* add_list */
	/* actually, AddList no longer returns DM_QUIT.
	 * Maybe later, when com_err no longer blows out there.
	 */
	if (AddList(argc, argv) == DM_QUIT)
	    return(DM_QUIT);
	(void) sprintf(temp_buf, "List '%s' created. Do you want to %s",
		       list_name, "change its membership?");
	if (YesNoQuestion(temp_buf, TRUE) != TRUE )
	    return(DM_QUIT);
    }
    else if (!ValidName(list_name)) {
	(void) sprintf(temp_buf, "Bad list: '%s'", list_name);
	Put_message(temp_buf);
    return(DM_QUIT);
    }

    (void) sprintf(temp_buf, 
		   "Change/Display membership of '%s'", list_name);
    m->m_title = Strsave(temp_buf);
    strcpy(current_list, list_name);
    return(DM_NORMAL);
}

/*	Function Name: ListmaintMemberMenuExit
 *	Description: This is the function called when the member menu is
 *                   exited, it frees the memory that is storing the name.
 *	Arguments: m - the menu
 *	Returns: DM_NORMAL 
 */

int
ListmaintMemberMenuExit(m)
Menu *m;
{
    free(m->m_title);
    strcpy(current_list, "");
    return(DM_NORMAL);
}

/*	Function Name: ListMembersByType
 *	Description: This function lists the users of a list by type.
 *	Arguments: type - the type of the list "USER", "LIST", or "STRING".
 *	Returns: none.
 *      NOTE: if type is NULL, all lists members are listed.
 */

void
ListMembersByType(type);
char * type;
{
    char temp_buf[BUFSIZ];

    found_some = FALSE;
    if (status = sms_query("get_members_of_list", 1, &current_list, 
			   PrintByType, type))
	com_err(whoami, status, " in ListMembersByType\n");
    if (!found_some) {
	if (type == NULL)
	    Put_message("List is empty (no members).");
	else {
	    sprintf(temp_buf,"No %s Members",type);
	    Put_message(temp_buf);
	}
    }
}

/*	Function Name: ListAllMembers
 *	Description: lists all members of the current list.
 *	Arguments: 
 *	Returns: DM_NORMAL
 */

int
ListAllMembers()
{
    ListMembersByType(NULL);
    return (DM_NORMAL);
}

/*	Function Name: ListUserMembers
 *	Description: This function lists all members of a list of type "USER".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListUserMembers()
{
    ListMembersByType("USER");
    return(DM_NORMAL);
}

/*	Function Name: ListListMembers
 *	Description: This function lists all members of a list of type "LIST".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListListMembers()
{
    ListMembersByType("LIST");
    return(DM_NORMAL);
}

/*	Function Name: ListStringMembers
 *	Description:This function lists all members of a list of type "STRING".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListStringMembers()
{
    ListMembersByType("STRING");
    return(DM_NORMAL);
}

/*	Function Name: GetMemberInfo
 *	Description: This function gets the information needed to
 *                   add or delete a user from a list.
 *	Arguments: argc, argv - standard.
 *                 action - name of the action to be performed either
 *                          "add" or "delete".
 *                 ret_argc, ret_argv - the returned value of argc and argv.
 *	Returns: SUB_ERROR or SUB_NORMAL.
 */

int
GetMemberInfo(action, ret_argv)
char *action, **ret_argv;
{
    char temp_buf[BUFSIZ], ret_buf[BUFSIZ];

    ret_argv[LM_LIST] = Strsave(current_list);
    ret_argv[LM_MEMBER] = "nobody";

    PromptWithDefault("Type of member (USER, LIST, or STRING)",
			ret_buf, BUFSIZ, "USER");
    ret_argv[LM_TYPE]= Strsave(ret_buf);

    status = sms_access("add_member_to_list", 3, ret_argv);
    if (status == SMS_TYPE) {
	Put_message("\"type\" must be one of 'STRING', 'LIST', or 'USER'.");
	return(SUB_ERROR);
    } else if (status) {
	com_err(whoami, status, NULL);
    }

    sprintf(temp_buf,"Name of member to %s", action);
    PromptWithDefault(temp_buf, ret_buf, BUFSIZ, user);
    ret_argv[LM_MEMBER] = Strsave(ret_buf);
    ret-args[LM_END] = NULL;		/* null terminat this list. */
    return(SUB_NORMAL);
}

/*	Function Name: AddMember
 *	Description: This function adds a member to a list.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
AddMember()
{
    char *args[10];

    if( GetMemberInfo("add", args) == SUB_ERROR )
	return(DM_NORMAL);

    if (status = sms_query("add_member_to_list", CountArgs(args), args,
			   Scream, NULL) != 0)
	com_err(whoami, status, " in AddMember\n");

    FreeInfo(args);
    return (DM_NORMAL);
}

/*	Function Name: DeleteMember
 *	Description: This function deletes a member from a list.
 *	Arguments: none.
 *	Returns: DM_NORMAL
 */

int
DeleteMember()
{
    char *args[10];

    if( GetMemberInfo("delete", args) == SUB_ERROR )
	return(DM_NORMAL);

    if (Confirm("Are you sure you want to delete this member?") ) {
	if (status = sms_query("delete_member_from_list", CountArgs(args),
			       args, Scream, NULL))
	    com_err(whoami, status, " in DeleteMember\n");
	else
	    Put_message("Deletion Completed.");
    }
    else 
	Put_message("Deletion has been Aborted.");

    FreeInfo(args);
    return(DM_NORMAL);
}


/*	Function Name: InterRemoveItemFromLists
 *	Description: This function allows interactive removal of an item
 *                   (user, string, list) for all list  that it is on.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
InterRemoveItemFromLists()
{
    char type[BUFSIZ], name[BUFSIZ], args[10];
    qelem *top, *elem;

    if ( !(PromptWithDefault("Type of member (user, list, string)? ", type, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);
    
    if ( !(PromptWithDefault("Name of member? ", name, BUFSIZ, user)) ) {
	return(DM_NORMAL);
    }

    top = elem = GetListInfo(GLOM, type, name);

    while(elem != NULL) {
	info = (char **) elem->q_data;
	sprintf(line, "Delete %s from the list \"%s\" [Y/N/Q]? ", name,
		info[0]);
	PromptWithDefault(line, buf, BUFSIZ, "N" );
	if (buf[0] == 'y' || buf[0] == 'Y') {
	    Put_message("deleting...");
	    args[DM_LIST] = info[0];
	    args[DM_TYPE] = type;
	    args[DM_MEMBER] = name;
	    if ( (status = sms_query("delete_member_from_list", 3, args,
			       Scream, (char *) NULL)) != 0)
		/* should check to delete list. */
		com_err(whoami, status, " in delete_member");
	} 
	else if (buf[0] == 'q' || buf[0] == 'Q') {
	    Put_message("Quitting.");
	    break;
	}
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return(DM_NORMAL);
}

/*-*-* LIST MENU *-*-*/

/*	Function Name: ListByMember
 *	Description: This gets all lists that a given member is a member of.
 *	Arguments: argc, argv - agrv[1] contains the type.
 *                              argv[2] contains the name of the member.
 *	Returns: DM_NORMAL.
 */

int
ListByMember(argc, argv)
int argc;
char **argv;
{
    char buf[BUFSIZ], *type, *name, **info;

    if ( !(PromptWithDefault("Type of member (user, list, string)? ", buf, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);

    sprintf(buf, "R%s", buf);	/* What we really want is a recursive search */
    type = Strsave(buf);
    
    if ( !(PromptWithDefault("Name of member? ", buf, BUFSIZ, user)) ) {
	free(type);
	return(DM_NORMAL);
    }
    name = Strsave(buf);

    maillist = YesNoQuestion("Show only Lists that are Maillists (y/n) ?",
			     TRUE);
    group = YesNoQuestion("Show only Lists that are Groups (y/n) ?", TRUE);

    elem = top = GetListInfo(GLOM, type, name);

    while (elem != NULL) {
	info = (char**) elem->q_data;
	if (maillist != TRUE || !strcmp(info[GLOM_MAILLIST], "1")) 
	    if (group != TRUE || !strcmp(info[GLOM_GROUP], "1")) 
		Put_message(info[GLOM_NAME]);
    	elem = elem->q_forw;
    }
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: ListByAdministrator
 *	Description: This function prints all lists which a given user or
 *                   group administers.
 *	Arguments: argc, argv - type of the acl in argc[1].
 *                              name of the acl in argc[2].
 *	Returns: DM_NORMAL.
 */

int
ListByAdministrator(argc, argv)
int argc;
char **argv;
{
    char buf[BUFSIZ], *type, *name, **info;

    if ( !(PromptWithDefault("Type of member (user, list, string)? ", buf, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);

    if ( YesNoQuestion("Do you want a recursive search?",TRUE) == 1 )
	sprintf(buf,"R%s",buf);	/* "USER" to "RUSER" etc. */
    type = Strsave(buf);
    
    if ( !(PromptWithDefault("Name of member? ", buf, BUFSIZ, user)) ) {
	free(type);
	return(DM_NORMAL);
    }
    name = Strsave(buf);

    top = elem = GetListInfo(ACL_USE, type, name);
    while (elem != NULL) {
	info = (char **) elem->q_data;
	sprintf(buf, "Type: %s,\tName: %s", info[0], info[1]);
	Put_message(buf);
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: ListAllGroups
 *	Description: This function lists all visable groups.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

ListAllGroups()
{
    static char * args[] = {
	"TRUE",			/* active */
	"DONTCARE",		/* public */
	"FALSE",		/* hidden */
	"DONTCARE",		/* maillist */
	"TRUE",			/* group. */
    }

    if (YesNoQuestion("This query will take a while, Do you wish to continue?",
		       TRUE) )
	if (status = sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(whoami, status, " in ListAllGroups\n");
    return (DM_NORMAL);
}

/*	Function Name: ListAllMailLists
 *	Description: This function lists all visable maillists.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListAllMailLists()
{
    static char * args[] = {
	"TRUE",			/* active */
	"DONTCARE",		/* public */
	"FALSE",		/* hidden */
	"TRUE",			/* maillist */
	"DONTCARE",		/* group. */
    }

    if (YesNoQuestion("This query will take a while. Do you wish to continue?",
		       TRUE) )
	if (status = sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(whoami, status, " in ListAllGroups\n");

    return (DM_NORMAL);	
}

/*	Function Name: ListAllPublicMailLists
 *	Description: This function lists all public mailing lists.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListAllPublicMailLists()
{
    static char * args[] = {
	"TRUE",			/* active */
	"TRUE",			/* public */
	"FALSE",		/* hidden */
	"TRUE",			/* maillist */
	"DONTCARE",		/* group. */
    }

    if (YesNoQuestion("This query will take a while. Do you wish to continue?",
		       TRUE) )
	if (status = sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(whoami, status, " in ListAllGroups\n");

    return (DM_NORMAL);	
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
