#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v 1.13 1988-09-02 12:25:42 mar Exp $";
#endif

/*	This is the file attach.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: Functions for maintaining data used by Hesiod 
 *                   to map courses/projects/users to their file systems, 
 *                   and maintain filesys info. 
 *	
 *	Created: 	5/4/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v 1.13 1988-09-02 12:25:42 mar Exp $
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

#define FS_ALIAS_TYPE "FILESYS"

#define LABEL        0
#define MACHINE      1
#define GROUP        2
#define ALIAS        3

#define DEFAULT_TYPE     ("NFS")
#define DEFAULT_MACHINE  DEFAULT_NONE
#define DEFAULT_PACK     DEFAULT_NONE
#define DEFAULT_M_POINT  DEFAULT_NONE
#define DEFAULT_ACCESS   ("w")
#define DEFAULT_COMMENTS DEFAULT_COMMENT
#define DEFAULT_OWNER    (user)
#define DEFAULT_OWNERS   (user)
#define DEFAULT_CREATE   DEFAULT_YES
#define DEFAULT_L_TYPE   ("PROJECT")

/*	Function Name: SetDefaults
 *	Description: sets the default values for filesystem additions.
 *	Arguments: info - an array of char pointers to recieve defaults. 
 *	Returns: char ** (this array, now filled).
 */

static char ** 
SetDefaults(info, name)
char ** info;
char * name;
{
    info[FS_NAME] =     Strsave(name);
    info[FS_TYPE] =     Strsave(DEFAULT_TYPE);
    info[FS_MACHINE] =  Strsave(DEFAULT_MACHINE);
    info[FS_PACK] =     Strsave(DEFAULT_PACK);
    info[FS_M_POINT] =  Strsave(DEFAULT_M_POINT);
    info[FS_ACCESS] =   Strsave(DEFAULT_ACCESS);
    info[FS_COMMENTS] = Strsave(DEFAULT_COMMENTS);
    info[FS_OWNER] =    Strsave(DEFAULT_OWNER);
    info[FS_OWNERS] =   Strsave(DEFAULT_OWNERS);
    info[FS_CREATE] =   Strsave(DEFAULT_CREATE);
    info[FS_L_TYPE] =   Strsave(DEFAULT_L_TYPE);
    info[FS_MODTIME] = info[FS_MODBY] = info[FS_MODWITH] = info[FS_END] = NULL;
    return(info);
}

/*	Function Name: GetFSInfo
 *	Description: Stores the info in a queue.
 *	Arguments: type - type of information to get.
 *                 name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue.
 */

static struct qelem *
GetFSInfo(type, name)
int type;
char *name;
{
    int stat;
    struct qelem * elem = NULL;
    char * args[5];

    switch (type) {
    case LABEL:
	if ( (stat = sms_query("get_filesys_by_label", 1, &name,
			       StoreInfo, (char *)&elem)) != 0) {
	    com_err(program_name, stat, NULL);
	    return(NULL);
	}
	break;
    case MACHINE:
	if ( (stat = sms_query("get_filesys_by_machine", 1, &name,
			       StoreInfo, (char *)&elem)) != 0) {
	    com_err(program_name, stat, NULL);
	    return(NULL);
	}
	break;
    case GROUP:
	if ( (stat = sms_query("get_filesys_by_group", 1, &name,
			       StoreInfo, (char *)&elem)) != 0) {
	    com_err(program_name, stat, NULL);
	    return(NULL);
	}
	break;
    case ALIAS:
	args[ALIAS_NAME] = name;
	args[ALIAS_TYPE] = FS_ALIAS_TYPE;
	args[ALIAS_TRANS] = "*";
	if ( (stat = sms_query("get_alias", 3, args, StoreInfo, 
			       (char *) &elem)) != 0) {
	    com_err(program_name, stat, " in get_alias.");
	    return(NULL);
	}
    }

    return(QueueTop(elem));
}

/*	Function Name: PrintFSAlias
 *	Description: Prints a filesystem alias
 *	Arguments: info - an array contains the strings of info.
 *	Returns: the name of the filesys - used be QueryLoop().
 */

static char *
PrintFSAlias(info)
char ** info;
{
    char buf[BUFSIZ];

    sprintf(buf,"Alias: %-25s Filesystem: %s",info[ALIAS_NAME], 
	    info[ALIAS_TRANS]);
    Put_message(buf);
    return(info[ALIAS_NAME]);
}

/*	Function Name: PrintFSInfo
 *	Description: Prints the filesystem information.
 *	Arguments: info - a pointer to the filesystem information.
 *	Returns: none.
 */

static char *
PrintFSInfo(info)
char ** info;
{
    char print_buf[BUFSIZ];
    FORMFEED;
    sprintf(print_buf,"%20s Filesystem: %s", 
	    " ",info[FS_NAME]);
    Put_message(print_buf);
    sprintf(print_buf,"Type: %-40s Machine: %-15s",
	    info[FS_TYPE], info[FS_MACHINE]);
    Put_message(print_buf);
    sprintf(print_buf,"Default Access: %-2s Packname: %-17s Mountpoint %s ",
	    info[FS_ACCESS], info[FS_PACK], info[FS_M_POINT]);
    Put_message(print_buf);
    sprintf(print_buf,"Comments; %s",info[FS_COMMENTS]);
    Put_message(print_buf);
    sprintf(print_buf, "User Ownership: %-30s Group Ownership: %s",
	    info[FS_OWNER], info[FS_OWNERS]);
    Put_message(print_buf);
    sprintf(print_buf, "Auto Create: %-34s Locker Type: %s",
	    atoi(info[FS_CREATE]) ? "ON" : "OFF", 
	    info[FS_L_TYPE]);
    Put_message(print_buf);
    sprintf(print_buf, MOD_FORMAT, info[FS_MODBY], info[FS_MODTIME], 
	    info[FS_MODWITH]);
    Put_message(print_buf);
    return(info[FS_NAME]);
}

/*	Function Name: AskFSInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *                 name - add a newname field? (T/F)
 *	Returns: none.
 */

static char **
AskFSInfo(info, name)
char ** info;
Bool name;
{
    char temp_buf[BUFSIZ], *newname;

    Put_message("");
    sprintf(temp_buf, "Changing Attributes of filesystem %s.", 
	    info[FS_NAME]);
    Put_message(temp_buf);
    Put_message("");

    if (name) {
	newname = Strsave(info[FS_NAME]);
	GetValueFromUser("The new name for this filesystem",
			 &newname);
    }

    GetTypeFromUser("Filesystem's Type", "filesys", &info[FS_TYPE]);
    GetValueFromUser("Filesystem's Machine", &info[FS_MACHINE]);
    strcpy(temp_buf, CanonicalizeHostname(info[FS_MACHINE]));
    free(info[FS_MACHINE]);
    info[FS_MACHINE] = Strsave(temp_buf);
    GetValueFromUser("Filesystem's Pack Name", &info[FS_PACK]);
    GetValueFromUser("Filesystem's Mount Point", &info[FS_M_POINT]);
    GetValueFromUser("Filesystem's Default Access", &info[FS_ACCESS]);
    GetValueFromUser("Comments about this Filesystem", &info[FS_COMMENTS]);
    GetValueFromUser("Filesystem's owner (user)", &info[FS_OWNER]);
    GetValueFromUser("Filesystem's owners (group)", &info[FS_OWNERS]);
    GetYesNoValueFromUser("Automatically create this filesystem",
		     &info[FS_CREATE]);
    GetTypeFromUser("Filesystem's lockertype", "lockertype", &info[FS_L_TYPE]);

    FreeAndClear(&info[FS_MODTIME], TRUE);
    FreeAndClear(&info[FS_MODBY], TRUE);
    FreeAndClear(&info[FS_MODWITH], TRUE);

    if (name)			/* slide the newname into the #2 slot. */
	SlipInNewName(info, newname);

    return(info);
}

/* --------------- Filesystem Menu ------------- */

/*	Function Name: GetFS
 *	Description: Get Filesystem information by name.
 *	Arguments: argc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
GetFS(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetFSInfo(LABEL, argv[1]); /* get info. */
    Loop(top, (void *) PrintFSInfo);
    FreeQueue(top);		/* clean the queue. */
    return (DM_NORMAL);
}

/*	Function Name: RealDeleteFS
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item 
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void
RealDeleteFS(info, one_item)
char ** info;
Bool one_item;
{
    int stat;
    char temp_buf[BUFSIZ];

/* 
 * Deletetions are  performed if the user hits 'y' on a list of multiple 
 * filesystem, or if the user confirms on a unique alias.
 */
    sprintf(temp_buf, "Are you sure that you want to delete filesystem %s",
	    info[FS_NAME]);
    if(!one_item || Confirm(temp_buf)) {
	if ( (stat = sms_query("delete_filesys", 1,
			       &info[FS_NAME], Scream, NULL)) != 0)
	    com_err(program_name, stat, " filesystem not deleted.");
	else
	    Put_message("Filesystem deleted.");
    }
    else 
	Put_message("Filesystem not deleted.");
}

/*	Function Name: DeleteFS
 *	Description: Delete a filesystem give its name.
 *	Arguments: argc, argv - argv[1] is the name of the filesystem.
 *	Returns: none.
 */

/* ARGSUSED */
 
int
DeleteFS(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetFSInfo(LABEL, argv[1]);
    QueryLoop(elem, PrintFSInfo, RealDeleteFS, "Delete the Filesystem");

    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: RealChangeFS
 *	Description: performs the actual change to the filesys.
 *	Arguments: info - the information 
 *                 junk - an unused boolean.
 *	Returns: none.
 */

/* ARGSUSED. */
static void
RealChangeFS(info, junk)
char ** info;
Bool junk;
{
    int stat;
    char ** args = AskFSInfo(info, TRUE);
    extern Menu nfsphys_menu;

    stat = sms_query("update_filesys", CountArgs(args), args, NullFunc, NULL);
    switch (stat) {
    case SMS_NFS:
	Put_message("That NFS filesystem is not exported.");
	if (YesNoQuestion("Fix this now (Y/N)")) {
	    Do_menu(&nfsphys_menu, 0, NULL);
	    if (YesNoQuestion("Retry filesystem update now (Y/N)")) {
		if (stat = sms_query("update_filesys", CountArgs(args), args,
				     NullFunc, NULL))
		    com_err(program_name, stat, " filesystem not updated");
		else
		    Put_message("filesystem sucessfully updated.");
	    }
	}
	break;
    case SMS_SUCCESS:
	break;
    default:
	com_err(program_name, stat, " in UpdateFS");
    }
}

/*	Function Name: ChangeFS
 *	Description: change the information in a filesys record.
 *	Arguments: arc, argv - value of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
ChangeFS(argc, argv)
char **argv;
int argc;
{
    struct qelem *elem = GetFSInfo(LABEL, argv[1]);
    QueryLoop(elem, NullPrint, RealChangeFS, "Update the Filesystem");

    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: AddFS
 *	Description: change the information in a filesys record.
 *	Arguments: arc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddFS(argc, argv)
char **argv;
int argc;
{
    char *info[MAX_ARGS_SIZE], **args;
    int stat;
    extern Menu nfsphys_menu;

    if ( !ValidName(argv[1]) )
	return(DM_NORMAL);

    if ( (stat = sms_query("get_filesys_by_label", 1, argv + 1,
			   NullFunc, NULL)) == 0) {
	Put_message ("A Filesystem by that name already exists.");
	return(DM_NORMAL);
    } else if (stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in AddFS");
	return(DM_NORMAL);
    } 

    args = AskFSInfo(SetDefaults(info, argv[1]), FALSE );

    stat = sms_query("add_filesys", CountArgs(args), args, NullFunc, NULL);
    switch (stat) {
    case SMS_NFS:
	Put_message("That NFS filesystem is not exported.");
	if (YesNoQuestion("Fix this now (Y/N)")) {
	    Do_menu(&nfsphys_menu, 0, NULL);
	    if (YesNoQuestion("Retry filesystem creation now (Y/N)")) {
		if (stat = sms_query("add_filesys", CountArgs(args), args,
				     NullFunc, NULL))
		    com_err(program_name, stat, " in AddFS");
		else
		    Put_message("Created.");
	    }
	}
	break;
    case SMS_SUCCESS:
	break;
    default:
	com_err(program_name, stat, " in AddFS");
    }

    FreeInfo(info);
    return (DM_NORMAL);
}

/* -------------- Top Level Menu ---------------- */

/*	Function Name: GetFSAlias
 *	Description: Gets the value for a Filesystem Alias.
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES: There should only be one filesystem per alias, thus
 *             this will work correctly.
 */

/* ARGSUSED */
int
GetFSAlias(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetFSInfo(ALIAS, argv[1]);
    Put_message(" ");		/* blank line. */
    Loop(top, (void *) PrintFSAlias);
    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: CreateFSAlias
 *	Description: Create an alias name for a filesystem
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES:  This requires (name, type, transl)  I get {name, translation}
 *              from the user.  I provide type, which is well-known. 
 */

/* ARGSUSED */
int
CreateFSAlias(argc, argv)
int argc;
char **argv;
{
    register int stat;
    struct qelem *elem, *top;
    char *args[MAX_ARGS_SIZE], buf[BUFSIZ], **info;

    elem = NULL;

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    args[ALIAS_NAME] = Strsave(argv[1]);
    args[ALIAS_TYPE] = Strsave(FS_ALIAS_TYPE);
    args[ALIAS_TRANS] = Strsave("*");

/*
 * Check to see if this alias already exists in the database, if so then
 * print out values, free memory used and then exit.
 */

    if ( (stat = sms_query("get_alias", 3, args, StoreInfo, 
			   (char *)&elem)) == 0) {
	top = elem = QueueTop(elem);
	while (elem != NULL) {
	    info = (char **) elem->q_data;	    
	    sprintf(buf,"The alias: %s currently describes the filesystem %s",
		    info[ALIAS_NAME], info[ALIAS_TRANS]);
	    Put_message(buf);
	    elem = elem->q_forw;
	}
	FreeQueue(top);
	return(DM_NORMAL);
    }
    else if ( stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in CreateFSAlias.");
        return(DM_NORMAL);
    }

    args[ALIAS_TRANS]= args[ALIAS_END] = NULL;	/* set to NULL initially. */
    GetValueFromUser("Which filesystem will this alias point to?",
		     &args[ALIAS_TRANS]);

    if ( (stat = sms_query("add_alias", 3, args, NullFunc, NULL)) != 0)
	com_err(program_name, stat, " in CreateFSAlias.");

    FreeInfo(args);
    return (DM_NORMAL);
}
    
/*	Function Name: RealDeleteFSAlias
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item 
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void
RealDeleteFSAlias(info, one_item)
char ** info;
Bool one_item;
{
    int stat;
    char temp_buf[BUFSIZ];

/* 
 * Deletetions are  performed if the user hits 'y' on a list of multiple 
 * filesystem, or if the user confirms on a unique alias.
 */
    sprintf(temp_buf, 
	    "Are you sure that you want to delete the filesystem alias %s",
	    info[ALIAS_NAME]);
    if(!one_item || Confirm(temp_buf)) {
	if ( (stat = sms_query("delete_alias", CountArgs(info),
			       info, Scream, NULL)) != 0 )
	    com_err(program_name, stat, " filesystem alias not deleted.");
	else
	    Put_message("Filesystem alias deleted.");
    }
    else 
	Put_message("Filesystem alias not deleted.");
}

/*	Function Name: DeleteFSAlias
 *	Description: Delete an alias name for a filesystem
 *	Arguments: argc, argv - name of alias in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES:  This requires (name, type, transl)  I get {name, translation}
 *              from the user.  I provide type, which is well-known. 
 */

/* ARGSUSED */
int
DeleteFSAlias(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetFSInfo(ALIAS, argv[1]);
    QueryLoop(elem, PrintFSAlias, RealDeleteFSAlias,
	      "Delete the Filesystem Alias");
    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: AttachHelp
 *	Description: Print help info on attachmaint.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int
AttachHelp()
{
    static char *message[] = {
      "These are the options:\n\n",
      "get - get information about a filesystem.\n",
      "add - add a new filesystem to the data base.\n",
      "update - update the information in the database on a filesystem.\n",
      "delete - delete a filesystem from the database.\n",
      "check - check information about association of a name and a filesys.\n",
      "alias - associate a name with a filesystem.\n",
      "unalias - disassociate a name with a filesystem.\n",
      "verbose - toggle the request for delete confirmation.\n",
	NULL,
    };

    return(PrintHelp(message));
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
