#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v 1.17 1988-12-07 18:49:03 mar Exp $";
#endif lint

/*	This is the file lists.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: All list manipulation functions, except delete.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/lists.c,v 1.17 1988-12-07 18:49:03 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <sms_app.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#define LIST    0
#define MEMBERS 1
#define GLOM    2
#define ACE_USE 3

#define DEFAULT_ACTIVE      DEFAULT_YES
#define DEFAULT_PUBLIC      DEFAULT_YES
#define DEFAULT_HIDDEN      DEFAULT_NO
#define DEFAULT_MAILLIST    DEFAULT_YES
#define DEFAULT_GROUP       DEFAULT_NO
#define DEFAULT_GID         UNIQUE_GID
#define DEFAULT_ACE_TYPE    "user"
#define DEFAULT_ACE_NAME    (user)
#define DEFAULT_DESCRIPTION DEFAULT_COMMENT

/* globals only for this file. */

static char current_list[BUFSIZ];

/*	Function Name: PrintListAce
 *	Description: This function prints the list ace information.
 *	Arguments: info - an info structure.
 *	Returns: none.
 */

static void
PrintListAce(info)
char ** info;
{
    char buf[BUFSIZ];

    sprintf(buf, "Item: %-20s Name: %s", info[ACE_TYPE], 
	    info[ACE_NAME]);
    Put_message(buf);
}

/*	Function Name: PrintListInfo
 *	Description: This function Prints out the List info in a coherent form.
 *	Arguments: info - the List info.
 *	Returns: none.
 */

static void
PrintListInfo(info)
char ** info;
{
    char buf[BUFSIZ];

    Put_message(" ");
    (void) sprintf(buf, "%20sList: %s", "", info[L_NAME]);
    (void) Put_message(buf);
    (void) sprintf(buf, "Description: %s", info[L_DESC]);
    (void) Put_message(buf);
    if ( atoi(info[L_MAILLIST]))
	Put_message("This list is a mailing list.");
    else
	Put_message("This list is NOT a mailing list.");
    if ( atoi(info[L_GROUP]) ) {
	(void) sprintf(buf,"This list is a Group and its ID number is %s",
		       info[L_GID]);
	Put_message(buf);
    }
    else
	Put_message("This list is NOT a Group.");

    if (strcmp(info[L_ACE_TYPE],"NONE") == 0)
	Put_message("This list has no Administrator, how strange?!");
    else {
	sprintf(buf, "The Administrator of this list is the %s: %s",
		info[L_ACE_TYPE], info[L_ACE_NAME]);
	Put_message(buf);
    }

    (void) sprintf(buf, "This list is: %s, %s, and %s",
		   atoi(info[L_ACTIVE]) ? "active" : "inactive",
		   atoi(info[L_PUBLIC]) ? "public" : "private",
		   atoi(info[L_HIDDEN]) ? "hidden" : "visible");
    (void) Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[L_MODBY], info[L_MODTIME], info[L_MODWITH]);
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
char * name1, *name2;
{
    char *args[2];
    struct qelem * elem = NULL;
    register int status;

    switch(type) {
    case LIST:
	args[0] = name1;
	if ( (status = do_sms_query("get_list_info", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, " in get_list_info");
	    return (NULL);
	}
	break;
    case MEMBERS:
	args[0] = name1;
	if ( (status = do_sms_query("get_members_of_list", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, " in get_members_of_list");
	    return (NULL);
	}
	break;
    case GLOM:
	args[0] = name1; 	
	args[1] = name2; 	
	if ( (status =  do_sms_query("get_lists_of_member", 2, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, " in get_list_of_members");
	    return (NULL);
	}
	break;
    case ACE_USE:
	args[0] = name1; 	
	args[1] = name2; 	
	if ( (status =  do_sms_query("get_ace_use", 2, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, " in get_ace_use");
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
AskListInfo(info, name)
char ** info;
Bool name;
{
    char temp_buf[BUFSIZ], *newname;

    Put_message(" ");
    sprintf(temp_buf,"Setting information of list %s.",info[L_NAME]);
    Put_message(temp_buf);
    Put_message(" ");

    if (name) {
	newname = Strsave(info[L_NAME]);
	GetValueFromUser("The new name for this list", &newname);
    }
    GetYesNoValueFromUser("Is this an active list", &info[L_ACTIVE]);
    GetYesNoValueFromUser("Is this a public list", &info[L_PUBLIC]);
    GetYesNoValueFromUser("Is this a hidden list", &info[L_HIDDEN]);
    GetYesNoValueFromUser("Is this a maillist", &info[L_MAILLIST]);
    GetYesNoValueFromUser("Is this a group", &info[L_GROUP]);
    if (atoi(info[L_GROUP]))
	GetValueFromUser("What is the GID for this group.", &info[L_GID]);

    GetTypeFromUser("What Type of Administrator", "ace_type",&info[L_ACE_TYPE]);
    if ( (strcmp(info[L_ACE_TYPE], "USER") == 0) || 
	(strcmp(info[L_ACE_TYPE], "user") == 0) )
	GetValueFromUser("Who will be the administrator of this list: ",
			 &info[L_ACE_NAME]);
    if ( (strcmp(info[L_ACE_TYPE], "LIST") == 0) ||
	(strcmp(info[L_ACE_TYPE], "list") == 0) )
       GetValueFromUser("Which group will be the administrator of this list: ",
			 &info[L_ACE_NAME]);
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
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM status code.
 */

/* ARGSUSED */
int
ShowListInfo(argc, argv)
int argc;
char **argv;
{
    struct qelem *top, *list;

    top = list = GetListInfo(LIST, argv[1], (char *) NULL);
    while (list != NULL) {
	PrintListInfo( (char **) list->q_data);
	list = list->q_forw;
    }
    
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: RealUpdateList
 *	Description: performs the actual update of the list.
 *	Arguments: info - all information needed to update the list.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */
 
/* ARGSUSED */   
static void
RealUpdateList(info, junk)
char ** info;
Bool junk;
{
    register int stat;
    char ** args;
    
    args = AskListInfo(info, TRUE);
    if ( (stat = do_sms_query("update_list", CountArgs(args), args, 
			   Scream, (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, stat, " in UpdateList.");	
	Put_message("List ** NOT ** Updated.");
    }
    else
	Put_message("List successfully updated.");
}

/*	Function Name: UpdateList
 *	Description: updates the information on a list.
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM Status code.
 */

/* ARGSUSED */
int
UpdateList(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetListInfo(LIST, argv[1], (char *) NULL);
    QueryLoop(top, NullPrint, RealUpdateList, "Update the list");

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: SetDefaults
 *	Description: sets defaults for AddList function
 *	Arguments: info - the array to add them to.
 *                 name - name of the program to add.
 *	Returns: defaults - the default information.
 */

static char **
SetDefaults(info, name)
char ** info;
char * name;
{
   info[L_NAME] =     Strsave(name);
   info[L_ACTIVE] =   Strsave(DEFAULT_ACTIVE);
   info[L_PUBLIC] =   Strsave(DEFAULT_PUBLIC);
   info[L_HIDDEN] =   Strsave(DEFAULT_HIDDEN);
   info[L_MAILLIST] = Strsave(DEFAULT_MAILLIST);
   info[L_GROUP] =    Strsave(DEFAULT_GROUP);
   info[L_GID] =      Strsave(DEFAULT_GID);
   info[L_ACE_TYPE] = Strsave(DEFAULT_ACE_TYPE);
   info[L_ACE_NAME] = Strsave(DEFAULT_ACE_NAME);
   info[L_DESC] =     Strsave(DEFAULT_DESCRIPTION);
   info[L_MODTIME] = info[L_MODBY] = info[L_MODWITH] = info[L_END] = NULL;
   return(info);
}

/*	Function Name: AddList
 *	Description: 
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: SUB_ERROR if list not created.
 */

/* ARGSUSED */
int
AddList(argc, argv)
int argc;
char **argv;
{
    static char *info[MAX_ARGS_SIZE], **add_args;
    int status, ret_code = SUB_NORMAL;

    status = do_sms_query("get_list_info", 1, argv + 1, NullFunc, 
		       (char *) NULL);
    if (status != SMS_NO_MATCH) {
	if (status == SMS_SUCCESS)
	    Put_message("This list already exists.");
	else
	    com_err(program_name, status, " in AddList.");	
	return(SUB_ERROR);
    }

    add_args = AskListInfo(SetDefaults(info,argv[1]), FALSE);

    if ( (status = do_sms_query("add_list", CountArgs(add_args), add_args,
			     Scream, (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, status, " in AddList.");	
	Put_message("List Not Created.");
	ret_code = SUB_ERROR;
    }

    FreeInfo(info);
    return(ret_code);
}

/*	Function Name: Instructions
 *	Description: This func prints out instruction on manipulating lists.
 *	Arguments: none
 *	Returns: DM Status Code.
 */

int
ListHelp()
{
    static char * message[] = {
	"Listmaint handles the creation, deletion, and updating of lists.",
	"A list can be a mailing list, a group list, or both.",
	"The concept behind lists is that a list has an owner",
	"- administrator -  and members.",
	"The administrator of a list may be another list.",
	"The members of a list can be users (login names), other lists,",
	"or address strings.",
	"You can use certain keys to do the following:",
	"    Refresh the screen - Type ctrl-L.",
	"    Escape from a function - Type ctrl-C.",
	"    Suspend the program (temporarily) - Type ctrl-Z.",
	NULL,
    };

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
    char temp_buf[BUFSIZ];
    char *list_name = argv[1];
    register int stat;    

    if (!ValidName(list_name))
	return(DM_QUIT);

    if (*argv[0] == 'a') {	/* add_list */
	if (AddList(argc, argv) == SUB_ERROR)
	    return(DM_QUIT);
	(void) sprintf(temp_buf, "List '%s' created. Do you want to %s",
		       list_name, "change its membership (y/n)? ");
	if (YesNoQuestion(temp_buf, TRUE) != TRUE )
	    return(DM_QUIT);
    }
    else 
	/* All we want to know is if it exists. */
	switch( (stat = do_sms_query("count_members_of_list", 1, argv + 1,
				   NullFunc, (char *) NULL))) {
	case SMS_SUCCESS:
	    break;
	case SMS_LIST:
	    Put_message("This list does not exist.");
	    return(DM_QUIT);
	case SMS_PERM:
	    Put_message("You are not allowed to view this list.");
	    break;
	default:
	    com_err(program_name, stat, " in get_list_info");
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

int
ListMembersByType(type)
char * type;
{
    char temp_buf[BUFSIZ];
    register int status;
    char * args[10];

    args[0] = current_list;
    args[1] = NULL;

    found_some = FALSE;
    if ( (status = do_sms_query("get_members_of_list", CountArgs(args), args, 
			     PrintByType, type)) != 0) {
	com_err(program_name, status, " in ListMembersByType");
	return(DM_NORMAL);
    }
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

    ret_argv[LM_TYPE] = Strsave("user");
    GetTypeFromUser("Type of member", "member", &ret_argv[LM_TYPE]);

    sprintf(temp_buf,"Name of %s to %s", ret_argv[LM_TYPE], action);
    ret_argv[LM_MEMBER] = Strsave(user);
    GetValueFromUser(temp_buf, &ret_argv[LM_MEMBER]);
    ret_argv[LM_END] = NULL;		/* NULL terminate this list. */

    if (!ValidName( ret_argv[LM_MEMBER] ) ) {
	FreeInfo(ret_argv);
	return(SUB_ERROR);
    }
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
    char *args[10], temp_buf[BUFSIZ];
    register int status;

    if ( GetMemberInfo("add", args) == SUB_ERROR )
	return(DM_NORMAL);

    if ( (status = do_sms_query("add_member_to_list", CountArgs(args), args,
			   Scream, NULL)) != SMS_SUCCESS) {
	if (status == SMS_EXISTS) {
	    sprintf(temp_buf, "The %s %s is already a member of LIST %s.",
		    args[LM_TYPE], args[LM_MEMBER], args[LM_LIST]);
	    Put_message(temp_buf);
	}
	else 
	    com_err(program_name, status, " in AddMember");
    }

    FreeInfo(args);
    return(DM_NORMAL);
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
    register int status;

    if( GetMemberInfo("delete", args) == SUB_ERROR )
	return(DM_NORMAL);

    if (Confirm("Are you sure you want to delete this member?") ) {
	if (status = do_sms_query("delete_member_from_list", CountArgs(args),
			       args, Scream, NULL))
	    com_err(program_name, status, " in DeleteMember");
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
 *      NOTES: QueryLoop() does not work here because info does not have
 *             enough information in it to delete the member from the list.
 */

int
InterRemoveItemFromLists()
{
    register int status;
    char type[BUFSIZ], name[BUFSIZ], *args[10], buf[BUFSIZ];
    struct qelem *top, *elem;

    if ( !(PromptWithDefault("Type of member (user, list, string)", type, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);
    
    sprintf(buf, "Name of %s", type);
    if ( !(PromptWithDefault(buf, name, BUFSIZ, user)) ) {
	return(DM_NORMAL);
    }

    if (!ValidName(name))
	return(DM_NORMAL);

    top = elem = GetListInfo(GLOM, type, name);

    while(elem != NULL) {
	char line[BUFSIZ];
	char ** info = (char **) elem->q_data;
	sprintf(line, "Delete %s %s from the list \"%s\" (y/n/q)? ", type,
		name, info[GLOM_NAME]);
	switch (YesNoQuitQuestion(line, FALSE)) {
	case TRUE:
	    Put_message("deleting...");
	    args[DM_LIST] = info[GLOM_NAME];
	    args[DM_TYPE] = type;
	    args[DM_MEMBER] = name;
	    if ( (status = do_sms_query("delete_member_from_list", 3, args,
			       Scream, (char *) NULL)) != 0)
		/* should probabally check to delete list. */
		com_err(program_name, status, " in delete_member");
	    break;
	case FALSE:
	    break;
	default:
	    Put_message("Aborting...");
	    FreeQueue(top);
	    return(DM_NORMAL);
	}
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return(DM_NORMAL);
}

/*-*-* LIST MENU *-*-*/

/*	Function Name: ListByMember
 *	Description: This gets all lists that a given member is a member of.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
ListByMember()
{
    char buf[BUFSIZ], temp_buf[BUFSIZ], *type, *name, **info;
    Bool maillist, group;
    struct qelem *top, *elem;

    if ( !(PromptWithDefault("Type of member (user, list, string)", buf, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);

    /* What we really want is a recursive search */
    sprintf(temp_buf, "R%s", buf); 
    type = Strsave(temp_buf);

    sprintf(temp_buf, "Name of %s", buf);
    if ( !(PromptWithDefault(temp_buf, buf, BUFSIZ, user)) ) {
	free(type);
	return(DM_NORMAL);
    }
    name = Strsave(buf);

    maillist = YesNoQuestion("Show Lists that are Maillists (y/n) ?",
			     TRUE);
    group = YesNoQuestion("Show Lists that are Groups (y/n) ?", TRUE);

    elem = top = GetListInfo(GLOM, type, name);

    while (elem != NULL) {
	info = (char**) elem->q_data;
	if ((maillist == TRUE && !strcmp(info[GLOM_MAILLIST], "1")) ||
	    (group == TRUE && !strcmp(info[GLOM_GROUP], "1")))
		Put_message(info[GLOM_NAME]);
    	elem = elem->q_forw;
    }
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: ListByAdministrator
 *	Description: This function prints all lists which a given user or
 *                   group administers.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
ListByAdministrator()
{
    char buf[BUFSIZ], temp_buf[BUFSIZ], *type, *name;
    struct qelem *top;

    if ( !(PromptWithDefault("Type of member (user, list, string)", buf, 
			    BUFSIZ, "user")) )
	return(DM_NORMAL);

    if ( YesNoQuestion("Do you want a recursive search (y/n)", FALSE) == 1 ) {
	sprintf(temp_buf, "R%s", buf);	/* "USER" to "RUSER" etc. */
	type = Strsave(temp_buf);
    }
    else 
	type = Strsave(buf);
    
    sprintf(temp_buf, "Name of %s", buf);
    if ( !(PromptWithDefault(temp_buf, buf, BUFSIZ, user)) ) {
	free(type);
	return(DM_NORMAL);
    }
    name = Strsave(buf);

    top = GetListInfo(ACE_USE, type, name);
    Loop(top, PrintListAce);

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
    register int status;
    static char * args[] = {
	"TRUE",			/* active */
	"DONTCARE",		/* public */
	"FALSE",		/* hidden */
	"DONTCARE",		/* maillist */
	"TRUE",			/* group. */
    };

    if (YesNoQuestion("This query will take a while, Do you wish to continue?",
		       TRUE) == TRUE )
	if (status = do_sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(program_name, status, " in ListAllGroups");
    return (DM_NORMAL);
}

/*	Function Name: ListAllMailLists
 *	Description: This function lists all visable maillists.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListAllMailLists()
{
    register int status;
    static char * args[] = {
	"TRUE",			/* active */
	"DONTCARE",		/* public */
	"FALSE",		/* hidden */
	"TRUE",			/* maillist */
	"DONTCARE",		/* group. */
    };

    if (YesNoQuestion("This query will take a while. Do you wish to continue?",
		       TRUE) == TRUE )
	if (status = do_sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(program_name, status, " in ListAllGroups");

    return (DM_NORMAL);	
}

/*	Function Name: ListAllPublicMailLists
 *	Description: This function lists all public mailing lists.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

ListAllPublicMailLists()
{
    register int status;
    static char * args[] = {
	"TRUE",			/* active */
	"TRUE",			/* public */
	"FALSE",		/* hidden */
	"TRUE",			/* maillist */
	"DONTCARE",		/* group. */
    };

    if (YesNoQuestion("This query will take a while. Do you wish to continue?",
		       TRUE) == TRUE )
	if (status = do_sms_query("qualified_get_lists", 5, args,
			       Print, (char *) NULL) != 0)
	    com_err(program_name, status, " in ListAllGroups");

    return (DM_NORMAL);	
}
