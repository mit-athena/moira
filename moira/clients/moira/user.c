#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.4 1988-06-29 20:13:04 kit Exp $";
#endif lint

/*	This is the file user.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: the functions for usermaint.
 *	
 *	Created: 	5/9/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/user.c,v 1.4 1988-06-29 20:13:04 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

#define LOGIN 0
#define UID   1
#define BY_NAME  2
#define QUOTA 3
#define CLASS 4

static void PrintQuota();	/* prints Quota info. */
static void PrintUserInfo();	/* prints User info. */
static int PrintPOBoxes();	/* prints PO Box information. */

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
    char temp_buf[BUFSIZ], *newname;

    sprintf(temp_buf,"\nChanging Attributes of user %s.\n",info[U_NAME]);
    Put_message(temp_buf);

    if (name) {
	newname = Strsave(info[U_NAME]);
	GetValueFromUser("The new login name for this user.", &newname);
    }
    GetValueFromUser("User's UID:", &info[U_UID]);
    GetValueFromUser("User's shell:", &info[U_SHELL]);
    GetValueFromUser("User's last name:", &info[U_LAST]);
    GetValueFromUser("User's first name:", &info[U_FIRST]);
    GetValueFromUser("User's middle name:", &info[U_MIDDLE]);
    GetValueFromUser("User's status:", &info[U_STATE]);
    GetValueFromUser("User's MIT ID number:", &info[U_MITID]);
    RemoveHyphens(info[U_MITID]); /* zap'em */
    GetValueFromUser("User's MIT Year (class):", &info[U_CLASS]);
    
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
	if ( (status = sms_query("get_user_by_login", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_use_by_login.");
      	    return (NULL);		 
	}
	break;
    case UID:
	args[0] = name1;
	if ( (status = sms_query("get_user_by_uid", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_use_by_uid.");
	    return (NULL);	
	}
	break;
    case BY_NAME:
	args[0] = name1;
	args[1] = name2;    
	if ( (status = sms_query("get_user_by_name", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_use_by_name.");
	    return (NULL);	
	}
	break;
    case CLASS:
	args[0] = name1;
	if ( (status = sms_query("get_user_by_class", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, 
		    " when attempting to get_use_by_class.");
	    return (NULL);	
	}
	break;
    case QUOTA:
	args[0] = name1;
	if ( (status = sms_query("get_nquotas_by_user", 1, args,
			       StoreInfo, (char *) &elem)) != 0) {
	    com_err(program_name, status, " in get_nfs_quotas_by_user");
	    return (DM_NORMAL);
	}
	break;
    }
    return( QueueTop(elem) );
}

/* -------------------------- Change Menu -------------------------- */

/*	Function Name: ModifyUser
 *	Description: Modify some of the information about a user.
 *	Arguments: argc, argv - login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ModifyUser(argc, argv)
int argc;
char **argv;
{
    int status;
    char *temp_buf, error_buf[BUFSIZ];
    struct qelem * elem, * local;

    local = elem = GetUserInfo(LOGIN, argv[1], (char *) NULL);

    while (local != NULL) {
	char ** info = (char **) local->q_data;
	char ** args = AskUserInfo(info, TRUE);

	if ( (status = sms_query("update_user", CountArgs(args), 
				 args, Scream, NULL)) != 0) {
	    com_err(program_name, status, " in ModifyFields");
	    if (local->q_forw == NULL)
		temp_buf = "";
	    else
		temp_buf = ", Continuing to next user";
	    sprintf(error_buf,"User %s not updated due to error%s.",
		    info[NAME], temp_buf);
	    Put_message(error_buf);
	}
	local = local->q_forw;
    }
    FreeQueue(elem);
    return(DM_NORMAL);
}


/*	Function Name: POType
 *	Description: Asks the SMS server if it is of type pop, of
 *                   of type local. If neither, we assume that it's 
 *                   of type foreign. 
 *	Arguments: machine - a canonicalized machine name, 
 *	Returns: the PO Type, one of: pop, local, foreign.
 */

char *
POType(machine)
char *machine;
{
    int status;
    char * type;
    struct qelem *top, *elem = NULL;

    type = "pop";
    status = sms_query("get_server_locations", 1, &type,
		       StoreInfo, &elem);
    if (status && (status != SMS_NO_MATCH)) {
	com_err(program_name, status, (char *) NULL);
	return( (char *) NULL);
    }

    top = elem = QueueTop(elem);
    while (elem != NULL) {
	char ** info = (char **) elem->q_data;
	if (strcmp (info[1], machine) == 0) {
	    FreeQueue(top);
	    return( Strsave("POP") );
	}
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return ( Strsave("SMTP") );
}

/*	Function Name: ChangeUserPOBox
 *	Description: Changes The P.O. Box for a user.
 *	Arguments: argc, argv - the login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
ChangeUserPOBox(argc, argv)
int argc;
char **argv;
{
    register int status;
    struct qelem * poqueue, *local;
    char *type, buf[BUFSIZ], *pohost;
    static char *po[4];
    poqueue = NULL;

    sprintf(buf,"Current pobox for user %s: \n", argv[1]); 
    Put_message(buf);
    status = sms_query("get_pobox", 1, argv + 1, PrintPOBoxes, NULL);
    if (status != SMS_NO_MATCH && status != 0) {
	com_err(program_name, status, "in ChangeUserPOBox.");
        return(DM_NORMAL);
    }
    else if (status == SMS_NO_MATCH) 
	Put_message("This user has no P.O. Box.");
    
    if (YesNoQuestion("Shall we use the least loaded Post Office?", TRUE)) {
	po[0] = "pop";
	po[1] = "*";
	if ( status = sms_query ("get_server_host_info", 2, po,
				 StoreInfo, (char *) &poqueue) != 0) {
	    com_err(program_name, status, 
		    " in ChangeUserPOBox (get_server_host_info).");
	    return(DM_NORMAL);
	}
	local = poqueue;
	while (local != NULL) {
	    char ** args = (char **) local->q_data;
	    int new_space, old_space = -1 ;
	    if ( !isdigit(*args[6]) || !isdigit(*args[7]) )
		Put_message(
		    "non-digit value in server_host_info, this is a bug.");
	    else {
		new_space = atoi(argv[7]) - atoi(args[6]);
		if ( (new_space < old_space) || (old_space == -1) ) {
		    old_space = new_space;
		    strcpy(buf, args[1]);
		}
	    }
	    local = local->q_forw;
	}
	pohost = Strsave(buf);
	FreeQueue(poqueue);
	type = "POP";
	(void) sprintf(buf, "The Post Office %s was chosen.",
		       pohost);
	Put_message(buf);
    }
    else if( (Prompt_input("Which Machine for Post Office?", 
			   buf, BUFSIZ)) ) {
	if ( (pohost = CanonicalizeHostname(buf)) == NULL) {
	    Put_message("\nNameserver doesn't know that machine, sorry.");
	    return (DM_NORMAL);
	}
	type = POType(pohost); 
    }
    
    sprintf(buf, "%s %s's %s %s?", "Are you sure that you want to replace all",
	    argv[1], "P.O. Boxes\n with ** ONE ** on the machine", pohost);

    if(Confirm(buf)) {
	po[0] = argv[1];
	po[1] = type;
	po[2] = pohost;
	po[3] = po[0];
	if (status = sms_query("set_pobox", 4, po, Scream, NULL) != 0 )
	    com_err(program_name, status, " in ChangeUserPOBox");
    } else 
	Put_message("Operation Aborted.");

    return (DM_NORMAL);
}

/* ------------------------- Top Menu ------------------------- */

/* delete user in delete.c */

/*	Function Name: DeleteUserByUid
 *	Description: Deletes the user given a uid number.
 *	Arguments: argc, argv - uid if user in argv[1].
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
DeleteUserByUid(argc, argv)
int argc;
char **argv;
{
    int status;

    if (Confirm("Are you sure you want to remove this user"))
	if ( (status = sms_query("delete_user_by_uid", 1, argv+1, Scream,
			       (char * ) NULL)) != 0)
	    com_err(program_name, status, " in DeleteUserByUid");

    return(DM_NORMAL);
} 

/* ------------------------- Show Quota Info ------------------------- */

/*	Function Name: ShowDefaultQuota
 *	Description: This prints out a default quota for the system.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int
ShowDefaultQuota()
{
    int status;
    static char *val[] = {"def_quota"};

    if (status = sms_query("get_value", 1, val, Print, (char *) NULL) != 0)
	com_err(program_name, status, " in ShowDefaultQuota");

    return (DM_NORMAL);
}

/*	Function Name: ShowUserQuota
 *	Description: Shows the quota of a user.
 *	Arguments: argc, argv - users login name is argv[1].
 *	Returns: DM_NORMAL
 */

/*ARGSUSED*/
int
ShowUserQuota(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem, *top;
    top = elem = GetUserInfo(QUOTA, argv[1], (char *) NULL);
    
    while (elem != NULL) {
	char ** info = (char **) elem->q_data;
	PrintQuota(info);
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: ChangeDefaultQuota
 *	Description: Changes the System Wide default quota.
 *	Arguments: argc, argv - New quota in argv[1].
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
ChangeDefaultQuota(argc, argv)
int argc;
char *argv[];
{
    char buf[BUFSIZ];
    int status;
    static char *newval[] = {
	"update_value", "def_quota", NULL,
    };

    sprintf(buf,"%s%s",
	    "Are you sure that you want to change the default quota\n",
	    "for all new users? (y/n) ");
    if(!Confirm(buf)) {
	newval[2] = argv[1];
	if (status = sms_query("update_value", 3, newval, Scream, NULL) != 0)
	    com_err(program_name, status, " in update_value");
    }
    else
	Put_message("Quota not changed.");

    return (DM_NORMAL);
}

/* ---------------------- User Locker Manipultation -------------------- */

/*	Function Name: AddUserLocker
 *	Description: Add a new locker for a user.
 *	Arguments: arc, argv - 
 *                             argv[1] login name of user.
 *                             argv[2] server host name.
 *                             argv[3] Directory on host.
 *                             argv[4] quota in Kb.
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
AddUserLocker(argc, argv)
int argc;
char **argv;
{
    int status;
    char *args[4];

    args[0] = argv[1];
    args[2] = argv[3];
    args[3] = argv[4];
    
    args[1] = CanonicalizeHostname(argv[2]);
    if (args[1] == (char *)NULL) {
	Put_message("Could not canonicalize hostname; continuing..");
	args[1] = argv[2];
    }
    
    if (status = sms_query("add_locker", 4, args, Scream, NULL) != 0)
	com_err(program_name, status, " in add_user_locker");

    return(DM_NORMAL);
}

/* needs to be fixed - CDP 6/10/88 */

/*	Function Name: DeleteUserLocker
 *	Description: Deletes a locker - BOOM.
 *	Arguments: arc, argv - the name of the locker in argv[1]/
 *	Returns: DM_NORMAL.
 */

/*ARGSUSED*/
int
DeleteUserLocker(argc, argv)
int argc;
char **argv;
{
    int status;
    if (status = sms_query("delete_locker", 1, argv + 1,
			   Scream, (char *)NULL) != 0) 
	    com_err(program_name, status, " in delete_locker");

    return(DM_NORMAL);
}

/*	Function Name: ChangeUserQuota
 *	Description: This function allows all quotas to be updated for a user.
 *	Arguments: arg, argv - the name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ChangeUserQuota(argc, argv)
int argc;
char *argv[];
{
    int status;
    char error_buf[BUFSIZ];
    struct qelem *elem, *local;
    
    elem = GetUserInfo(QUOTA, argv[1], (char *) NULL);

    local = elem;
    while (local != NULL) {
	char **info;
	info = (char **) local->q_data;
	PrintQuota(info);
	GetValueFromUser("New quota (in KB): ", &info[Q_QUOTA]);
	
	if (status = sms_query("update_nfs_quota", 3, info,
			       Scream, (char *) NULL) != 0) {
	    com_err(program_name, status, " in update_nfs_quota");
	    sprintf(error_buf,"Could not perform quota change on %s",
		    info[Q_FILESYS]); 
	    Put_message(error_buf);
	}
	local = local->q_forw;
    }
    FreeQueue(elem);
    return (DM_NORMAL);
}

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
    while (elem != NULL) {
	PrintUserInfo( (char **) elem->q_data, FALSE);
	elem = elem->q_forw;
    }

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
    struct qelem *top, *elem;
    char buf;

    elem = top = GetUserInfo(BY_NAME, argv[1], argv[2]);

    if (!PromptWithDefault("Print full information, or just the names (F/N)?",
			   &buf, 1, "F"))
	return(DM_NORMAL);

    while (elem != NULL) {
	switch(buf) {
	case 'F':
	case 'f':
	    PrintUserInfo( (char **) elem->q_data, FALSE);
	    break;
	case 'N':
	case 'n':
	    PrintUserInfo( (char **) elem->q_data, TRUE);
	    break;
	}
	elem = elem->q_forw;
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
    struct qelem *top, *elem;

    elem = top = GetUserInfo(CLASS, argv[1], (char *) NULL);

    while (elem != NULL) {
	PrintUserInfo( (char **) elem->q_data, TRUE);
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return (DM_NORMAL);
}


/*	Function Name: PrintQuota
 *	Description: Prints a users quota information.
 *	Arguments: info - a pointer to the quota information:
 *	Returns: none.
 */

static void
PrintQuota(info)
char ** info;
{
    char buf[BUFSIZ];

    sprintf(buf, "Machine: %s\t\tDirectory: %s\t\tQuota: %s",
		   info[Q_MACHINE], info[Q_DIRECTORY], info[Q_QUOTA]);
    Put_message(buf);
}

/*	Function Name: PrintPOBoxes
 *	Description: Yet another specialized print function.
 *	Arguments: argc, argv - 
 *                             argv[0] - login name.
 *                             argv[1] - type.
 *                             argv[2] - machine.
 *                             argv[3] - box name.
 *                 junk.  - NOT USED
 *	Returns: SMS_CONT
 */

/* ARGSUSED */
static int
PrintPOBoxes(argc, argv, junk)
int argc;
char **argv;
char * junk;
{
    char buf[BUFSIZ];
    /* no newline 'cause Put_message adds one */

    (void) sprintf(buf, "Address: %s@%s\t\tType: %s", argv[PO_BOX],
		   argv[PO_MACHINE], argv[PO_TYPE]);
    (void) Put_message(buf);

    return (SMS_CONT);
}

/*	Function Name: PrintUserInfo
 *	Description: Prints Information about a user.
 *	Arguments: info - an argument list with the user information
 *                          in it.
 *                 name_only - if TRUE then print only the users name.
 *	Returns: none
 */

static void
PrintUserInfo(info, name_only)
char ** info;
Bool name_only;
{
    char buf[BUFSIZ];

    if (name_only) {
	sprintf(buf, "%s, %s %s", info[U_LAST],
		info[U_FIRST], info[U_MIDDLE]);
	sprintf(buf, "%-40s/tUser Name: %s", buf, info[U_NAME]);
	Put_message(buf);
    }
    else {
	(void) sprintf(buf, 
		       "Login name: %-10s/tUser id: %-10s\tLogin shell %s",
		       info[U_NAME], info[U_UID], info[U_SHELL]);
	(void) Put_message(buf);
	(void) sprintf(buf, "Full name: %s %s %s\tClass: %s", 
		       info[U_FIRST], info[U_MIDDLE], 
		       info[U_LAST], info[U_CLASS]);
	(void) Put_message(buf);
	(void) sprintf(buf,
		       "Account status: %s\tEncrypted MIT ID number: %s",
		       atoi(info[U_STATE]) ? "active" : "inactive",
		       info[U_MITID]);
	(void) Put_message(buf);
	(void) sprintf(buf, "Last Modification by %s at %s with %s.",
		       info[U_MODBY], info[U_MODTIME], info[U_MODWITH]);
	(void) Put_message(buf);
    }
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
