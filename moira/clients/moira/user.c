#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.36 1995-11-21 14:10:17 jweiss Exp $";
#endif lint

/*	This is the file user.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains: Functions for manipulating user information.
 *	
 *	Created: 	5/9/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v $
 *      $Author: jweiss $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.36 1995-11-21 14:10:17 jweiss Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <string.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>
#include <ctype.h>
#include <sys/time.h>
#ifdef GDSS
#include <des.h>
#include <krb.h>
#include <gdss.h>
#endif
#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#define LOGIN 0
#define UID   1
#define BY_NAME  2
#define CLASS 3
#define ID 4

#ifdef ATHENA
#define DEFAULT_SHELL "/bin/athena/tcsh"
#else
#define DEFAULT_SHELL "/bin/csh"
#endif
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
			  "Enrolled/Not Registerable (6)",
			  "Half Enrolled (7)" };

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
    int status;
#ifdef GDSS
    SigInfo si;
#endif

    sprintf(name, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
    sprintf(buf, "Login name: %-20s Full name: %s", info[U_NAME], name);
    Put_message(buf);
    sprintf(buf, "User id: %-23s Login shell %-10s Class: %s", 
	    info[U_UID], info[U_SHELL], info[U_CLASS]);
    Put_message(buf);

#ifdef GDSS
    sprintf(buf, "%s:%s", info[U_NAME], info[U_MITID]);
    si.rawsig = NULL;
    status = GDSS_Verify(buf, strlen(buf), info[U_SIGNATURE], &si);
#ifdef DEBUG
    hex_dump(info[U_SIGNATURE]);
    sprintf(buf, "GDSS_Verify => %d", status);
    Put_message(buf);
#endif /* DEBUG */
#else /* GDSS */
    status = 0;
#endif /* GDSS */

    sprintf(buf, "Account is: %-20s MIT ID number: %s Signed: %s",
	    UserState(atoi(info[U_STATE])), info[U_MITID],
	    *info[U_SIGNATURE] ? (status ? "Bad" : "Yes") : "No");
    Put_message(buf);
    if (atoi(info[U_SECURE]))
      sprintf(buf, "Secure password set on %s.", atot(info[U_SECURE]));
    else
      sprintf(buf, "No secure password set.");
    Put_message(buf);
    sprintf(buf, "Comments: %s", info[U_COMMENT]);
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
    info[U_COMMENT] = Strsave("");
    info[U_SIGNATURE] = Strsave("");
    info[U_SECURE] = Strsave("0");
    info[U_MODTIME] = info[U_MODBY] = info[U_MODWITH] = info[U_END] = NULL;
    return(info);
}


/* Check that the supplied name follows the capitalization rules, and 
 * offer to correct it if not.
 */

CorrectCapitalization(name)
char **name;
{
    char temp_buf[BUFSIZ], fixname[BUFSIZ];

    strcpy(fixname, *name);
    FixCase(fixname);
    if (strcmp(fixname, *name)) {
	Put_message("You entered a name which does not follow the capitalization conventions.");
	sprintf(temp_buf, "Correct it to \"%s\"", fixname);
	if (YesNoQuestion(temp_buf, 1) == TRUE) {
	    free(*name);
	    *name = strsave(fixname);
	}
    }
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
    int siglen, i;
#ifdef GDSS
    SigInfo si;
#endif
    char temp_buf[BUFSIZ], *newname, *temp_ptr, *sig, sig_buf[BUFSIZ];

    if (name) {
	sprintf(temp_buf,"\nChanging Attributes of user %s.\n",info[U_NAME]);
	Put_message(temp_buf);
    } else {
	struct qelem *elem = NULL;
	char *argv[3];

	if (GetValueFromUser("User's last name", &info[U_LAST]) == SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_LAST]);
	if (GetValueFromUser("User's first name", &info[U_FIRST]) == SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_FIRST]);
	if (GetValueFromUser("User's middle name", &info[U_MIDDLE]) ==
	    SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_MIDDLE]);
	argv[0] = info[U_FIRST];
	argv[1] = info[U_LAST];
	if (do_mr_query("get_user_account_by_name", 2, argv,
			 StoreInfo, (char *) &elem) == 0) {
	    Put_message("A user by that name already exists in the database.");
	    Loop(QueueTop(elem), PrintUserInfo);
	    Loop(QueueTop(elem), FreeInfo);
	    FreeQueue(elem);
	    if (YesNoQuestion("Add new user anyway", TRUE) != TRUE)
	      return(NULL);
	}
    }
    if (name) {
	newname = Strsave(info[U_NAME]);
	if (GetValueFromUser("The new login name for this user", &newname) ==
	    SUB_ERROR)
	  return(NULL);
    } else if (GetValueFromUser("Login name for this user", &info[U_NAME]) ==
	       SUB_ERROR)
      return(NULL);

    if (GetValueFromUser("User's UID", &info[U_UID]) == SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("User's shell", &info[U_SHELL]) == SUB_ERROR)
      return(NULL);
    if (name) {
	if (GetValueFromUser("User's last name", &info[U_LAST]) == SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_LAST]);
	if (GetValueFromUser("User's first name", &info[U_FIRST]) == SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_FIRST]);
	if (GetValueFromUser("User's middle name", &info[U_MIDDLE]) ==
	    SUB_ERROR)
	  return(NULL);
	CorrectCapitalization(&info[U_MIDDLE]);
    }
    while (1) {
	int i;
	if (GetValueFromUser("User's status (? for help)", &info[U_STATE]) ==
	    SUB_ERROR)
	  return(NULL);
	if (isdigit(info[U_STATE][0]))
	  break;
	Put_message("Valid status numbers:");
	for (i = 0; i < US_END; i++) {
	    sprintf(temp_buf, "  %d: %s", i, states[i]);
	    Put_message(temp_buf);
	}
    }
    if (GetValueFromUser("User's MIT ID number", &info[U_MITID]) == SUB_ERROR)
      return(NULL);
    RemoveHyphens(info[U_MITID]);
    if (GetTypeFromUser("User's MIT Year (class)", "class", &info[U_CLASS]) ==
	SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Comments", &info[U_COMMENT]) == SUB_ERROR)
      return(NULL);

    if (YesNoQuestion("Secure password set",
		      atoi(info[U_SECURE]) ? TRUE : FALSE) == FALSE) {
	free(info[U_SECURE]);
	info[U_SECURE] = strsave("0");
    } else if (!strcmp(info[U_SECURE], "0")) {
	char buf[16];
	struct timeval tv;

	gettimeofday(&tv, (struct timezone *)NULL);
	sprintf(buf, "%d", tv.tv_sec);
	free(info[U_SECURE]);
	info[U_SECURE] = strsave(buf);
    }

    /* Sign record */
#ifdef GDSS
    if (strcmp(info[U_NAME], UNIQUE_LOGIN)) {
	if (name)
	  sprintf(temp_buf, "%s:%s", newname, info[U_MITID]);
	else
	  sprintf(temp_buf, "%s:%s", info[U_NAME], info[U_MITID]);
	si.rawsig = NULL;
	i = GDSS_Verify(temp_buf, strlen(temp_buf), info[U_SIGNATURE], &si);
	/* If it's already signed OK, don't resign it. */
	if (i != GDSS_SUCCESS) {
	    free(info[U_SIGNATURE]);
	    info[U_SIGNATURE] = malloc(GDSS_Sig_Size() * 2);
	sign_again:
	    i = GDSS_Sign(temp_buf, strlen(temp_buf), info[U_SIGNATURE]);
	    if (i != GDSS_SUCCESS)
	      com_err(program_name, gdss2et(i), "Failed to create signature");
	    else {
		unsigned char buf[256];
		si.rawsig = buf;
		i = GDSS_Verify(temp_buf, strlen(temp_buf),
				info[U_SIGNATURE], &si);
		if (strlen(buf) > 68) {
#ifdef DEBUG
		    Put_message("Signature too long, trying again");
#endif /* DEBUG */
		    goto sign_again;
		}
	    }
#ifdef DEBUG
	    Put_message("Made signature:");
	} else {
	    Put_message("Don't need to remake signature");
#endif /* DEBUG */
	}
#ifdef DEBUG
	hex_dump(info[U_SIGNATURE]);
#endif /* DEBUG */
    }
#else /* GDSS */
    info[U_SIGNATURE] = strsave("");
#endif /* GDSS */

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
 *
 * Note: if we are checking a login name, if the length is greater
 * than 8 characters, we immediately print a "no such user" error.
 * This gets around a bug in Ingres, where a non-existent 8 character
 * username returns a "no such user" error instantaneously, but a 9
 * character username takes 5-6 minutes.  :-(  We will need to change
 * this if we ever make a username longer than 8 characters.
 * Unlikely, but....
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
	if (strlen(name1) > 8) {
	    com_err(program_name, MR_USER,
		    " when attempting to get_user_acount_by_login.");
	    return (NULL);
        }
	args[0] = name1;
	if ( (status = do_mr_query("get_user_account_by_login", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_account_by_login.");
      	    return (NULL);		 
	}
	break;
    case UID:
	args[0] = name1;
	if ( (status = do_mr_query("get_user_account_by_uid", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_account_by_uid.");
	    return (NULL);	
	}
	break;
    case BY_NAME:
	args[0] = name1;
	args[1] = name2;    
	if ( (status = do_mr_query("get_user_account_by_name", 2, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_account_by_name.");
	    return (NULL);	
	}
	break;
    case CLASS:
	args[0] = name1;
	if ( (status = do_mr_query("get_user_account_by_class", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_account_by_class.");
	    return (NULL);	
	}
	break;
    case ID:
	args[0] = name1;
	if ( (status = do_mr_query("get_user_account_by_id", 1, args,
				    StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_user_account_by_id.");
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

    if ((args = AskUserInfo(SetUserDefaults(info), FALSE)) == NULL) {
	Put_message("Aborted.");
	return(DM_NORMAL);
    }
    if (args == NULL)
      return(DM_NORMAL);
    if ( (status = do_mr_query("add_user_account", CountArgs(args), 
				args, Scream, (char *) NULL)) != MR_SUCCESS)
	com_err(program_name, status, " in add_user_account");
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
    char *name;

    name = strsave("");
    if (GetValueFromUser("Login name for this user? ", &name) == SUB_ERROR)
      return(NULL);
    Put_message("KERBEROS code not added, did not reserve name with kerberos.");
    return(name);
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
    
    if (!Prompt_input("First Name: ", first, BUFSIZ))
      return(NULL);
    if (!Prompt_input("Last  Name: ", last, BUFSIZ))
      return(NULL);
    FixCase(first);
    FixCase(last);

    args[0] = first;
    args[1] = last;
    
    switch (status = do_mr_query("get_user_account_by_name", 2, args,
				  StoreInfo, (char *) &top)) {
    case MR_SUCCESS:
	break;
    case MR_NO_MATCH:
	Put_message("There is no user in the database with that name.");
	return(NULL);
    default:
	com_err(program_name, status, " in get_account_user_by_name.");
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
    
    Put_message("This function has NO kerberos support, so strange things");
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

    sprintf(temp_buf, "u%s", args[0]);
    login = strsave(temp_buf);
    if ( (GetValueFromUser("Login name for this user? ", &login) == SUB_ERROR) ||
	( GetFSTypes(&fstype, FALSE) == SUB_ERROR ) ) {
        args[1] = login;
	FreeInfo(args);	   /* This work because the NULL temination is ok. */
	return(DM_NORMAL);
    }
    Put_message("KERBEROS code not added, did not reserve name with kerberos.");
    args[1] = login;
    args[2] = fstype;
    args[3] = NULL;
    
    switch (status = do_mr_query("register_user", CountArgs(args),
				  args, Scream, (char *) NULL)) {
    case MR_SUCCESS:
	sprintf(temp_buf, "User %s successfully registered.", login);
	Put_message(temp_buf);
	SetUserPassword(login);
	break;
    case MR_IN_USE:
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

    if (args == NULL) {
	Put_message("Aborted.");
	return;
    }
    if ( (status = do_mr_query("update_user_account", CountArgs(args), 
				args, Scream, (char *) NULL)) != MR_SUCCESS) {
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
    char * qargs[2], **args;
    struct qelem *elem = NULL;

    if (one_item) {
	sprintf(txt_buf, "Deactivate user %s (y/n)", info[NAME]);
	if (YesNoQuestion(txt_buf, FALSE) != TRUE)
	    return;
    }

    qargs[0] = info[NAME];
    qargs[1] = "3";
    if ((status = do_mr_query("update_user_status", 2, qargs, Scream,
			       (char *) NULL)) != MR_SUCCESS) {
	com_err(program_name, status, " in update_user_status");
	sprintf(txt_buf, "User %s not deactivated due to errors.", info[NAME]);
	Put_message(txt_buf);
    } else if (YesNoQuestion("Also deactivate matching list and filesystem (y/n)",
			     FALSE) == TRUE) {
	if (status = do_mr_query("get_list_info", 1, &(info[NAME]),
				 StoreInfo, (char *) &elem)) {
	    com_err(program_name, status, " getting list info, not deactivating list or filesystem");
	    return;
	}
	args =(char **) (QueueTop(elem)->q_data);
	free(args[L_ACTIVE]);
	args[L_ACTIVE] = strsave("0");
	FreeAndClear(&args[L_MODTIME], TRUE);
	FreeAndClear(&args[L_MODBY], TRUE);
	FreeAndClear(&args[L_MODWITH], TRUE);
	SlipInNewName(args, args[L_NAME]);
	if (status = do_mr_query("update_list", CountArgs(args), args,
				 Scream, (char *) NULL)) {
	    com_err(program_name, status, " updating list, not deactivating list or filesystem");
	    FreeInfo(args);
	    FreeQueue(elem);
	    return;
	}
	FreeInfo(args);
	FreeQueue(elem);
	elem = (struct qelem *) NULL;
	if (status = do_mr_query("get_filesys_by_label", 1, &(info[NAME]),
				 StoreInfo, (char *) &elem)) {
	    com_err(program_name, status, " getting filsys info, not deactivating filesystem");
	    FreeInfo(args);
	    FreeQueue(elem);
	    return;
	}
	args = (char **) (QueueTop(elem)->q_data);
	free(args[FS_TYPE]);
	args[FS_TYPE] = strsave("ERR");
	free(args[FS_COMMENTS]);
	args[FS_COMMENTS] = strsave("Locker disabled; call 3-1325 for help");
	FreeAndClear(&args[FS_MODTIME], TRUE);
	FreeAndClear(&args[FS_MODBY], TRUE);
	FreeAndClear(&args[FS_MODWITH], TRUE);
	SlipInNewName(args, args[FS_NAME]);
	if (status = do_mr_query("update_filesys", CountArgs(args), args,
				 Scream, (char *) NULL)) {
	    com_err(program_name, status, " updating filesystem, not deactivating filesystem");
	    FreeInfo(args);
	    FreeQueue(elem);
	    return;
	}
	FreeInfo(args);
	FreeQueue(elem);
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
 *      NOTES: This just gets the username from the mr server 
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
    
    if ( (status = do_mr_query("get_user_account_by_uid", 1, argv+1, StoreInfo,
				(char * ) &elem)) != MR_SUCCESS)
	com_err(program_name, status, " in get_user_account_by_uid");
    
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

    if (YesNoQuestion("This will take a long time.  Are you sure", 0) == FALSE)
      return (DM_NORMAL);
    top = GetUserInfo(CLASS, argv[1], (char *) NULL);
    Loop(top, PrintUserName);

    FreeQueue(top);
    return (DM_NORMAL);
}


/*	Function Name: ShowUserById
 *	Description: Shows user information given an ID number.
 *	Arguments: argc, argv - ID number in argv[1].
 *	Returns: DM_NORMAL
 */

/* ARGSUSED */
int
ShowUserById(argc, argv)
int argc;
char *argv[];
{
    struct qelem *top, *elem;

    elem = top = GetUserInfo(ID, argv[1], (char *) NULL);
    Loop(elem, PrintUserInfo);

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

    if ((stat = do_mr_query("get_kerberos_user_map", 2, &argv[1],
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

    if (!strchr(argv[KMAP_PRINCIPAL + 1], '@')) {
	Put_message("Please specify a realm for the kerberos principal.");
	return(DM_NORMAL);
    }
    if ((stat = do_mr_query("add_kerberos_user_map", 2, &argv[1],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " in AddKrbMap.");
	if (stat == MR_EXISTS)
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

    if ((stat = do_mr_query("delete_kerberos_user_map", 2, &argv[1],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " in DeleteKrbMap.");
    }
    return(DM_NORMAL);
}


hex_dump(p)
unsigned  char *p;
{
    char buf[BUFSIZ];
    int i;

    sprintf(buf, "Size: %d", strlen(p));
    Put_message(buf);
    while (strlen(p) >= 8) {
	sprintf(buf, "%02x %02x %02x %02x %02x %02x %02x %02x",
		p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
	Put_message(buf);
	p += 8;
    }
    switch (strlen(p)) {
    case 7:
	sprintf(buf, "%02x %02x %02x %02x %02x %02x %02x",
		p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
	break;
    case 6:
	sprintf(buf, "%02x %02x %02x %02x %02x %02x",
		p[0], p[1], p[2], p[3], p[4], p[5]);
	break;
    case 5:
	sprintf(buf, "%02x %02x %02x %02x %02x",
		p[0], p[1], p[2], p[3], p[4]);
	break;
    case 4:
	sprintf(buf, "%02x %02x %02x %02x",
		p[0], p[1], p[2], p[3]);
	break;
    case 3:
	sprintf(buf, "%02x %02x %02x",
		p[0], p[1], p[2]);
	break;
    case 2:
	sprintf(buf, "%02x %02x",
		p[0], p[1]);
	break;
    case 1:
	sprintf(buf, "%02x",
		p[0]);
	break;
    default:
	return;
    }
    Put_message(buf);
}
