#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.16 1989-08-22 15:55:32 mar Exp $";
#endif lint

/*	This is the file user.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: Functions for manipulating user information.
 *	
 *	Created: 	5/9/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.16 1989-08-22 15:55:32 mar Exp $
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
#include <ctype.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#define LOGIN 0
#define UID   1
#define BY_NAME  2
#define CLASS 3

#define DEFAULT_SHELL "/bin/csh"
#define DEFAULT_CLASS "?"


/*	Function Name: UserState
 *	Description: Convert a numeric state into a descriptive string.
 *	Arguments: state value
 *	Returns: pointer to statically allocated string.
 */

static char *states[] = { "Registerable (0)",
			  "Active (1)",
			  "Half Registered (2)",
			  "Deleted (3)",
			  "Not registerable (4)",
			  "Enrolled/Registerable (5)",
			  "Enrolled/Not Registerable (6)" };

static char *UserState(state)
int state;
{
    char buf[BUFSIZ];

    if (state < 0 || state >= US_END) {
	sprintf(buf, "Unknown (%d)", state);
	return(buf);
    }
    return(states[state]);
}


/*	Function Name: PrintUserName
 *	Description: Print name of a user.
 *	Arguments: info - the information about a user.
 *	Returns: none.
 */

static void
PrintUserName(info)
char ** info;
{
    char buf[BUFSIZ], print_buf[BUFSIZ];
    sprintf(buf, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
    sprintf(print_buf, "%-40s User Name: %s", buf, info[U_NAME]);
    Put_message(print_buf);
}

/*	Function Name: PrintUserInfo
 *	Description: Prints Information about a user.
 *	Arguments: info - an argument list with the user information
 *                          in it.
 *	Returns: none
 */

static void
PrintUserInfo(info)
char ** info;
{
    char name[BUFSIZ], buf[BUFSIZ];

    sprintf(name, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
    sprintf(buf, "Login name: %-20s Full name: %s", info[U_NAME], name);
    Put_message(buf);
    sprintf(buf, "User id: %-23s Login shell %-10s Class: %s", 
	    info[U_UID], info[U_SHELL], info[U_CLASS]);
    Put_message(buf);
    sprintf(buf, "Account is: %-20s Encrypted MIT ID number: %s",
	    UserState(atoi(info[U_STATE])), info[U_MITID]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[U_MODBY], info[U_MODTIME],info[U_MODWITH]);
    Put_message(buf);
}

/*	Function Name: SetUserDefaults
 *	Description: Sets the default values for add user.
 *	Arguments: info - a blank user info array of char *'s.
 *	Returns: args - the filled info structure.
 */

static char **
SetUserDefaults(info)
char ** info;
{
    info[U_NAME] = Strsave(UNIQUE_LOGIN);
    info[U_UID] = Strsave(UNIQUE_UID);
    info[U_SHELL] = Strsave(DEFAULT_SHELL);
    info[U_LAST] = Strsave(DEFAULT_NONE);
    info[U_FIRST] = Strsave(DEFAULT_NONE);
    info[U_MIDDLE] = Strsave(DEFAULT_NONE);
    info[U_STATE] = Strsave(DEFAULT_NO);
    info[U_MITID] = Strsave(DEFAULT_NONE);
    info[U_CLASS] = Strsave(DEFAULT_CLASS);
    info[U_MODTIME] = info[U_MODBY] = info[U_MODWITH] = info[U_END] = NULL;
    return(info);
}

/*	Function Name: AskUserInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the info into.
 *                 flags - Flags asking us which info we want.
 *	Returns: the args to pass to the query.
 *      NOTES: the return args are not necessarily in the correct order to
 *             use the #defined names (e.g args[UID] is not the uid anymore).
 */

char **
AskUserInfo(info, name)
char ** info;
Bool name;
{
    char temp_buf[BUFSIZ], *newname, *temp_ptr;

    if (name) {
	sprintf(temp_buf,"\nChanging Attributes of user %s.\n",info[U_NAME]);
	Put_message(temp_buf);
    } else {
	struct qelem *elem = NULL;
	char *argv[3];

	GetValueFromUser("User's last name", &info[U_LAST]);
	GetValueFromUser("User's first name", &info[U_FIRST]);
	GetValueFromUser("User's middle name", &info[U_MIDDLE]);
	argv[0] = info[U_FIRST];
	argv[1] = info[U_LAST];
	if (do_sms_query("get_user_by_name", 2, argv,
			 StoreInfo, (char *) &elem) == 0) {
	    Put_message("A user by that name already exists in the database.");
	    Loop(QueueTop(elem), PrintUserInfo);
	    Loop(QueueTop(elem), FreeInfo);
	    FreeQueue(elem);
	    if (YesNoQuestion("Add new user anyway", TRUE) == FALSE)
	      return(NULL);
	}
    }
    if (name) {
	newname = Strsave(info[U_NAME]);
	GetValueFromUser("The new login name for this user", &newname);
    }
    else
	GetValueFromUser("Login name for this user", &info[U_NAME]);

    GetValueFromUser("User's UID", &info[U_UID]);
    GetValueFromUser("User's shell", &info[U_SHELL]);
    if (name) {
	GetValueFromUser("User's last name", &info[U_LAST]);
	GetValueFromUser("User's first name", &info[U_FIRST]);
	GetValueFromUser("User's middle name", &info[U_MIDDLE]);
    }
    while (1) {
	int i;
	GetValueFromUser("User's status (? for help)", &info[U_STATE]);
	if (isdigit(info[U_STATE][0]))
	  break;
	Put_message("Valid status numbers:");
	for (i = 0; i < US_END; i++) {
	    sprintf(temp_buf, "  %d: %s", i, states[i]);
	    Put_message(temp_buf);
	}
    }
    temp_ptr = Strsave(info[U_MITID]);
    Put_message("User's MIT ID number (type a new unencrypted number, or keep same encryption)");
    GetValueFromUser("", &temp_ptr);
    if ( strcmp( temp_ptr, info[U_MITID] ) != 0) {
	EncryptID(temp_buf, temp_ptr, info[U_FIRST], info[U_LAST]);
	free(info[U_MITID]);
	info[U_MITID] = Strsave(temp_buf);
    }
    free(temp_ptr);
    GetTypeFromUser("User's MIT Year (class)", "class", &info[U_CLASS]);
    
    FreeAndClear(&info[U_MODTIME], TRUE);
    FreeAndClear(&info[U_MODBY], TRUE);
    FreeAndClear(&info[U_MODWITH], TRUE);

/* 
 * Slide the newname into the #2 slot, this screws up all future references 
 * to this list, since we slip the pointer into a info list it gets freed 
 * when the rest of the list gets freed.
 */
    if (name)			
	SlipInNewName(info, newname);

    return(info);
}

/*	Function Name: GetUserInfo
 *	Description: Stores the user information in a queue.
 *	Arguments: type - type of field given to get info, one of:
 *                        LOGIN, UID, BY_NAME, CLASS.
 *                 name1 - name of thing specified by type (wildcards okay)
 *                 name2 - other name, only used in get user by first and last.
 *                         (wildcards okay).
 *	Returns: the first element of the queue containing the user info.
 */

struct qelem *
GetUserInfo(type, name1, name2)
int type;
char *name1, *name2;
{
    char * args[2];
    register int status;
    struct qelem * elem = NULL;

    switch(type) {
    case LOGIN:
	args[0] = name1;
	if ( (status = do_sms_query("get_user_by_login", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_by_login.");
      	    return (NULL);		 
	}
	break;
    case UID:
	args[0] = name1;
	if ( (status = do_sms_query("get_user_by_uid", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_by_uid.");
	    return (NULL);	
	}
	break;
    case BY_NAME:
	args[0] = name1;
	args[1] = name2;    
	if ( (status = do_sms_query("get_user_by_name", 2, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_by_name.");
	    return (NULL);	
	}
	break;
    case CLASS:
	args[0] = name1;
	if ( (status = do_sms_query("get_user_by_class", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_by_class.");
	    return (NULL);	
	}
	break;
    }
    return( QueueTop(elem) );
}

/*	Function Name: AddNewUser
 *	Description: Adds a new user to the database.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddNewUser()
{
    register int status;
    char ** args, *info[MAX_ARGS_SIZE];

    args = AskUserInfo(SetUserDefaults(info), FALSE);
    if (args == NULL)
      return(DM_NORMAL);
    if ( (status = do_sms_query("add_user", CountArgs(args), 
				args, Scream, (char *) NULL)) != SMS_SUCCESS)
	com_err(program_name, status, " in add_user");
    else
	Put_message("New user added to database.");
    FreeInfo(args);
    return(DM_NORMAL);
}


/*	Function Name: GetLoginName
 *	Description: Asks the user for a login name and reserves
 *                   it with kerberous.
 *	Arguments: none.
 *	Returns: a malloced login name for the user.
 */

static char *
GetLoginName()
{
    char name[BUFSIZ];

    Prompt_input("Login name for this user? ", name, BUFSIZ);
    
    Put_message(
	      "KERBEROS code not added, did not reserve name with kerberos.");

    return(Strsave(name));
}


/*	Function Name: ChooseUser
 *	Description: Choose a user from a list and return the uid.
 *	Arguments: top - a queue of user information.
 *	Returns: uid - the malloced uid of the user that was chosen.
 */

static char *
ChooseUser(elem)
struct qelem * elem;
{
    while (elem != NULL) {
	char ** info = (char **)  elem->q_data;
	PrintUserInfo(info);
	switch(YesNoQuitQuestion("Is this the user you want (y/n/q)", FALSE)) {
	case TRUE:
	    return(Strsave(info[U_UID]));
	case FALSE:
	    break;
	default:		/* quit or ^C. */
	    return(NULL);
	}
	elem = elem->q_forw;
    }
    return(NULL);
}

/*	Function Name: GetUidNumberFromName
 *	Description: Gets the users uid number, from the name.
 *	Arguments: none.
 *	Returns: uid - a malloced string containing the uid.
 */

static char *
GetUidNumberFromName()
{
    char *args[5], *uid, first[BUFSIZ], last[BUFSIZ];
    register int status;
    struct qelem * top = NULL;
    
    Prompt_input("First Name: ", first, BUFSIZ);
    Prompt_input("Last  Name: ", last, BUFSIZ);

    args[0] = first;
    args[1] = last;
    
    switch (status = do_sms_query("get_user_by_name", 2, args,
				  StoreInfo, (char *) &top)) {
    case SMS_SUCCESS:
	break;
    case SMS_NO_MATCH:
	Put_message("There is no user in the database with that name.");
	return(NULL);
    default:
	com_err(program_name, status, " in get_user_by_name.");
	return(NULL);
    }
    
    top = QueueTop(top);
    if (QueueCount(top) == 1) /* This is a unique name. */ {
	char ** info = (char **) top->q_data;
	Put_message("User ID Number retrieved for the user: ");
	Put_message("");
	PrintUserName(info);
	uid = Strsave(info[U_UID]);
	FreeQueue(top);
	return(Strsave(uid));
    }

    Put_message("That name is not unique, choose the user that you want.");
    uid = ChooseUser(top);
    FreeQueue(top);
    return(uid);
}

/*	Function Name: SetUserPassword
 *	Description: Set the new kerberos password for this user.
 *	Arguments: name - kerberos principle name for this user, (login name).
 *	Returns: none.
 */

static void
SetUserPassword(name)
char * name;
{
    name = name;			/* make saber happy. */
    Put_message("Kerberos password not changed, code non-existant.");
    /* clever message to call account_admin, if this fails. */
}

/*	Function Name:  GiveBackLogin
 *	Description: Gives back previously reserved kerberous principle.
 *	Arguments: name - principle to give back.
 *	Returns: void.
 */

static void
GiveBackLogin(name)
char * name;
{
    name = name;			/* make saber happy. */
    Put_message("kerberos code not implimented, name not given back.");
    /* send mail to db maintainer if this fails. */
}

/*	Function Name: RegisterUser
 *	Description: This function registers a user.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int
RegisterUser()
{
    char * args[MAX_ARGS_SIZE];
    char *login, *fstype = NULL;
    char temp_buf[BUFSIZ];
    register int status;
    
    Put_message("This function has NO kerberos support, so stange things");
    Put_message("may happen if you use it to register a user.");

    switch (YesNoQuestion("Do you know the users UID Number (y/n)", FALSE)) {
    case TRUE:
	Prompt_input("What is the UID number of the user? ", temp_buf, BUFSIZ);
	args[0] = Strsave(temp_buf);
	break;
    case FALSE:
	if ( (args[0] = GetUidNumberFromName()) == NULL)
	    return(DM_NORMAL);
	break;
    default:
	return(DM_NORMAL);
    }

    if ( ((login = args[1] = GetLoginName()) == NULL) ||
	( GetFSTypes(&fstype) == SUB_ERROR ) ) {
	FreeInfo(args);	   /* This work because the NULL temination is ok. */
	return(DM_NORMAL);
    }
    args[2] = fstype;
    args[3] = NULL;
    
    switch (status = do_sms_query("register_user", CountArgs(args),
				  args, Scream, (char *) NULL)) {
    case SMS_SUCCESS:
	sprintf(temp_buf, "User %s successfully registered.", login);
	Put_message(temp_buf);
	SetUserPassword(login);
	break;
    case SMS_IN_USE:
	GiveBackLogin(login);
	sprintf(temp_buf, "The username %s is already in use.", login);
	Put_message(temp_buf);
	break;
    default:
	com_err(program_name, status, " in register_user");
	break;
    }
    FreeInfo(args);
    return(DM_NORMAL);
}

/*	Function Name: RealUpdateUser
 *	Description: actuall updates the user information.
 *	Arguments: info - all current information for the user fields.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

/* ARGSUSED */
static void
RealUpdateUser(info, junk)
char ** info;
Bool junk;
{
    register int status;
    char error_buf[BUFSIZ];
    char ** args = AskUserInfo(info, TRUE);
    
    if ( (status = do_sms_query("update_user", CountArgs(args), 
				args, Scream, (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, status, " in ModifyFields");
	sprintf(error_buf, "User %s not updated due to errors.", info[NAME]);
	Put_message(error_buf);
    }
}

/*	Function Name: UpdateUser
 *	Description: Modify some of the information about a user.
 *	Arguments: argc, argv - login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
UpdateUser(argc, argv)
int argc;
char **argv;
{
    struct qelem * elem;

    elem = GetUserInfo(LOGIN, argv[1], (char *) NULL);
    QueryLoop(elem, NullPrint, RealUpdateUser, "Update the user");
    
    FreeQueue(elem);
    return(DM_NORMAL);
}

/*	Function Name: RealDeactivateUser
 *	Description: sets the user's status to 3.
 *	Arguments: info - all current information for the user fields
 *		   one_item - indicates the user hasn't been queried yet
 *	Returns: none.
 */

static void
RealDeactivateUser(info, one_item)
char ** info;
Bool one_item;
{
    register int status;
    char txt_buf[BUFSIZ];
    char * qargs[2];

    if (one_item) {
	sprintf(txt_buf, "Deactivate user %s (y/n)", info[NAME]);
	if (!YesNoQuestion(txt_buf, 2))
	    return;
    }

    qargs[0] = info[NAME];
    qargs[1] = "3";
    if ((status = do_sms_query("update_user_status", 2, qargs, Scream,
			       (char *) NULL)) != SMS_SUCCESS) {
	com_err(program_name, status, " in update_user_status");
	sprintf(txt_buf, "User %s not deactivated due to errors.", info[NAME]);
	Put_message(txt_buf);
    }
}


/*	Function Name: DeactivateUser
 *	Description: sets the user's status to 3.
 *	Arguments: argc, argv - login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeactivateUser(argc, argv)
int argc;
char **argv;
{
    struct qelem * elem;

    elem = GetUserInfo(LOGIN, argv[1], (char *) NULL);
    QueryLoop(elem, NullPrint, RealDeactivateUser, "Deactivate user");
    
    FreeQueue(elem);
    return(DM_NORMAL);
}


/* ------------------------- Top Menu ------------------------- */

/* DeleteUser() in delete.c */

/*	Function Name: DeleteUserByUid
 *	Description: Deletes the user given a uid number.
 *	Arguments: argc, argv - uid if user in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES: This just gets the username from the sms server 
 *             and performs a DeleteUser().
 */

int
DeleteUserByUid(argc, argv)
int argc;
char **argv;
{
    int status;
    struct qelem *elem = NULL;
    char ** info;

    if(!ValidName(argv[1]))
	return(DM_NORMAL);
    
    if ( (status = do_sms_query("get_user_by_uid", 1, argv+1, StoreInfo,
				(char * ) &elem)) != SMS_SUCCESS)
	com_err(program_name, status, " in get_user_by_uid");
    
    info = (char **) elem->q_data;
    argv[1] = info[U_NAME];

    (void) DeleteUser(argc, argv);
    return(DM_NORMAL);
} 

/* ------------------------- Show User Information ------------------------- */

/*	Function Name: ShowUserByLogin
 *	Description: Shows user information given a login name.
 *	Arguments: argc, argv - login name in argv[1].
 *	Returns: DM_NORMAL
 */

/* ARGSUSED */
int
ShowUserByLogin(argc, argv)
int argc;
char *argv[];
{
    struct qelem *top, *elem;

    elem = top = GetUserInfo(LOGIN, argv[1], (char *) NULL);
    Loop(elem, PrintUserInfo);

    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: RetrieveUserByName
 *	Description: Show information on a user give fist and/or last name.
 *	Arguments: argc, argv - argv[1] - first name.
 *                              argv[2] - last name.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ShowUserByName(argc, argv)
int argc;
char *argv[];
{
    struct qelem *top;
    char buf[BUFSIZ];

    top = GetUserInfo(BY_NAME, argv[1], argv[2]);

    if (top == NULL)		/* if there was an error then return. */
	return(DM_NORMAL);

    if (!PromptWithDefault("Print full information, or just the names (f/n)?",
			   buf, 2, "f"))
	return(DM_NORMAL);

    switch(buf[0]) {
    case 'F':
    case 'f':
	Loop(top, PrintUserInfo);
	break;
    case 'N':
    case 'n':
	Loop(top, PrintUserName);
	break;
    }
    
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: ShowUserByClass
 *	Description: Shows real and login names of all users in class.
 *	Arguments: argc, argv - argv[1] contains the class.
 *	Returns: none.
 */

/* ARGSUSED */
int
ShowUserByClass(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetUserInfo(CLASS, argv[1], (char *) NULL);
    Loop(top, PrintUserName);

    FreeQueue(top);
    return (DM_NORMAL);
}


/*	Function Name: GetKrbmap
 *	Description: Shows user <-> Kerberos mappings
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

/* ARGSUSED */
int
GetKrbmap(argc, argv)
int argc;
char **argv;
{
    int stat;
    struct qelem *elem = NULL, *top;
    char buf[BUFSIZ];

    if ((stat = do_sms_query("get_kerberos_user_map", 2, &argv[1],
			     StoreInfo, (char *)&elem)) != 0) {
	com_err(program_name, stat, " in GetKrbMap.");
	return(DM_NORMAL);
    }

    top = elem = QueueTop(elem);
    Put_message("");
    while (elem != NULL) {
	char **info = (char **) elem->q_data;
	sprintf(buf, "User: %-9s Principal: %s",
		info[KMAP_USER], info[KMAP_PRINCIPAL]);
	Put_message(buf);
	elem = elem->q_forw;
    }

    FreeQueue(QueueTop(top));
    return(DM_NORMAL);
}


/*	Function Name: AddKrbmap
 *	Description: Add a new user <-> Kerberos mapping
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

/* ARGSUSED */
int
AddKrbmap(argc, argv)
int argc;
char **argv;
{
    int stat;

    if (!index(argv[KMAP_PRINCIPAL + 1], '@')) {
	Put_message("Please specify a realm for the kerberos principal.");
	return(DM_NORMAL);
    }
    if ((stat = do_sms_query("add_kerberos_user_map", 2, &argv[1],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " in AddKrbMap.");
	if (stat == SMS_EXISTS)
	  Put_message("No user or principal may have more than one mapping.");
    }
    return(DM_NORMAL);
}


/*	Function Name: DeleteKrbmap
 *	Description: Remove a user <-> Kerberos mapping
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

/* ARGSUSED */
int
DeleteKrbmap(argc, argv)
int argc;
char **argv;
{
    int stat;

    if ((stat = do_sms_query("delete_kerberos_user_map", 2, &argv[1],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " in DeleteKrbMap.");
    }
    return(DM_NORMAL);
}
