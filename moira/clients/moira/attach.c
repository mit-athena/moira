#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v 1.1 1988-06-09 14:12:43 kit Exp $";
#endif lint

/*	This is the file attach.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: Functions for maintaining data used by Hesiod 
 *                   to map courses/projects/users to their file systems, 
 *                   and maintain filesys info. 
 *	
 *	Created: 	5/4/88
 *	By:		Chris D. Peterson
 *      Based Upon:     attach.c 87/07/24 marcus
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/attach.c,v 1.1 1988-06-09 14:12:43 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include "mit-copyright.h"
#include "allmaint.h"
#include "globals.h"
#include "infodefs.h"

#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#define FS_ALIAS_TYPE "FILESYS"

#define FS_NAME         0
#define FS_TYPE         1
#define FS_MACHINE      2
#define FS_PACK         3
#define FS_M_POINT      4
#define FS_ACCESS       5
#define FS_COMMENTS     6
#define FS_OWNER        7
#define FS_OWNERS       8
#define FS_CREATE       9
#define FS_L_TYPE       10
#define FS_MODTIME      11
#define FS_MODBY        12
#define FS_MODWITH      13

#define FS_LABEL        14
/* FS_MACHINE already defined, this is okay, as there is no overlap. */
#define FS_GROUP        15

/*	Function Name: GetFSInfo
 *	Description: Stores the info in a queue.
 *	Arguments: type - type of information to get.
 *                 name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue.
 */

struct qelem *
GetFSInfo(type, name)
int type;
char *name;
{
    int stat;
    struct qelem * elem = NULL;
    char * args[2];

    switch (type) {
    case FS_LABEL:
	if ( (stat = sms_query("get_filesys_by_label", 1, &name,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, NULL);
	    return(NULL);
	}
	break;
    case FS_MACHINE:
	if ( (stat = sms_query("get_filesys_by_label", 1, &name,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, NULL);
	    return(NULL);
	}
	break;
    case FS_GROUP:
	if ( (stat = sms_query("get_filesys_by_label", 1, &name,
			       StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, NULL);
	    return(NULL);
	}
	break;
    case FS_ALIAS:
	args[0] = name;
	args[1] = FS_ALIAS_TYPE;
	if ( (stat = sms_query("get_alias", 2, args, StoreInfo, &elem)) != 0) {
	    com_err(whoami, stat, " in get_alias.");
	    return(NULL);
	}
    }

    return(QueueTop(elem));
}

/*	Function Name: PrintFSInfo
 *	Description: Prints the filesystem information.
 *	Arguments: info - a pointer to the filesystem information.
 *	Returns: none.
 */

PrintFSInfo(info);
char ** info;
{
    char print_buf[BUFSIZ];
    sprintf(print_buf,"Information about filesystem: %s", info[FS_NAME]);
    Put_message(print_buf);
    sprintf(print_buf,"Type: %s\t\tMachine: %s\t\tPackname: %s",info[FS_TYPE],
	    info[FS_MACHINE], info[FS_PACK]);
    Put_message(print_buf);
    sprintf(print_buf,"Mountpoint %s\t\t Default Access: %s",info[FS_M_POINT],
	    info[FS_ACCESS]);
    Put_message(print_buf);
    sprintf(print_buf,"Comments; %s",info[FS_COMMENTS]);
    Put_message(print_buf);
    sprintf(print_buf, "User Ownership: %s,\t\tGroup Ownership:\t\t%s",
	    info[FS_OWNER], info[FS_OWNERS]);
    Put_message(print_buf);
    sprintf(print_buf, "Auto Create %s\t\tLocker Type: %s",info[FS_CREATE], 
	    info[FS_L_TYPE]);
    Put_message(print_buf);
    sprintf(print_buf, "Last Modified at %s, by %s with %s",info[FS_MODTIME],
	    info[FS_MODBY], info[FS_MODWITH]);
    Put_message(print_buf);
}

/*	Function Name: PrintAllFSInfo
 *	Description: Prints all infomation about a filesystem, stored in
 *                   a queue.
 *	Arguments: elem - a pointer to the first elem in the queue.
 *	Returns: none.
 */

void
PrintAllFSInfo(elem)
struct qelem * elem;
{
/* 
 * Not nescessary, but makes it clear that we are not changing this value.
 */
    struct qelem * local = elem;	
			        
    while(local != NULL) {
	char ** info = (char **) local->q_date;
	PrintFSInfo(info);
	local = local->next;
    }
}

/*	Function Name: AskFSInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *                 name - add a newname field? (T/F)
 *	Returns: none.
 */

void
AskFSInfo(info, flags);
char ** info;
Bool name;
{
    char * temp_buf, *newname;
    char *ret_args[100];
    int counter;

    sprintf(temp_buf,"\nChanging Attributes of user %s.\n",info[NAME]);
    Put_message(temp_buf);

    if (name) {
	newname = Strsave(info[FS_NAME]);
	GetValueFromUser("The new login name for this filesystem.",
			 &newname);
    }

    GetValueFromUser("Filesystem's Type:", &info[FS_TYPE]);
    GetValueFromUser("Filesystem's Machine:", &info[FS_MACHINE]);
    GetValueFromUser("Filesystem's Pack Name:", &info[FS_PACK]);
    GetValueFromUser("Filesystem's Mount Point:", &info[FS_M_POINT]);
    GetValueFromUser("Filesystem's Default Access:", &info[FS_ACCESS]);
    GetValueFromUser("Commants about this Filesystem:", &info[FS_COMMENTS]);
    GetValueFromUser("Filesystem's owner (user):", &info[FS_OWNER]);
    GetValueFromUser("Filesystem's owners (group):", &info[FS_OWNERS]);
    GetValueFromUser("Automatically create this filsystem (0/1):",
		     &info[FS_COMMENTS]);
    GetValueFromUser("Filesystem's lockertype:", &info[FS_L_TYPE]);

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
    struct qelem *elem;

    elem = GetFSInfo();	/* get info. */
    PrintAllFSInfo(elem);	/* print it all. */
    FreeQueue(elem);		/* clean the queue. */
    return (DM_NORMAL);
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
    int stat, answer, delete, number;
    struct qelem *elem, *temp_elem;
    
    if ( (temp_elem = elem = GetFSInfo(FS_LABEL, argv[1],
			     (char *) NULL) ) == NULL ) /* get info. */
	return(DM_NORMAL);
/* 
 * 1) If there is no (zero) match then we exit immediately.
 * 2) If there is exactly 1 element then we ask for confirmation only if in
 *    verbose mode, via the Confirm function.  
 * 3) If there is more than 1 filesystem to be deleted then we ask
 *    about each one, and delete on yes only, and about if the user hits
 *    quit.
 */
    number = QueueCount(elem);
    while (temp_elem != NULL) {
	char **info = (char **) temp_elem->q_data;
	
	if (number != 1) {
	    PrintFSInfo(info);
	
	    answer = YesNoQuitQuestion("\nDelete this filesys?", FALSE); 
	    switch(answer) {
	    case TRUE:
		delete = TRUE;
		break;
	    case FALSE:
		delete = FALSE;
		break;
	    default:		/* Quit. */
		Put_message("Aborting Delete Operation.");
		FreeQueue(elem);
		return(DM_NORMAL);
	    }
	}
	else
	    delete = 
	     Confirm("Are you sure that you want to delete this filsystem.")) 
/* 
 * Deletetions are  performed if the user hits 'y' on a list of multiple 
 * filesystem, or if the user confirms on a unique alias.
 */
	if (delete) {
	    if ( (status = sms_query("delete_filesys", 1,
				     &name, NullFunc, NULL)) != 0)
		com_err(whoami, stat, " filesystem not deleted.");
	    else
		Put_message("Filesystem deleted.");
	}
	else 
	    Put_message("Filesystem not deleted.");
	temp_elem = temp_elem->next;
    }

    FreeQueue(elem);		/* free all members of the queue. */
    return (DM_NORMAL);
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
    FS_info *info;
    struct qelem * elem, temp_elem;
    int update, stat, answer;
    
    elem = temp_elem = GetFSInfo(FS_LABEL, argv[1], (char *) NULL);

/* 
 * This uses the same basic method as the deletion routine above.
 */

    number = QueueCount(elem);
    while (temp_elem != NULL) {
	char ** info = (char **) temp_elem->q_data;
	if (number != 1) {
	    sprintf(buf, "%s %s %s (y/n/q)? ", "Would you like to change the",
		    "information about the filesystem:", 
		    info[FS_NAME]);
	    info = (char **) temp_elem->q_data;
	    answer = YesNoQuitQuestion(buf, FALSE);
	    switch(answer) {
	    case TRUE:
		update = TRUE
		break;
	    case FALSE:
		upadate = FALSE;
		break;
	    default:
		Put_message("Aborting Operation.");
		FreeQueue(elem);
		return(DM_NORMAL);
	    }
	}
	else
	    update = TRUE;

	if (update) {
	    char * args;
	    args = AskFSInfo(info, TRUE);
	    if ( (stat = sms_query("update_filesys", num_args, 
				   args, NullFunc, NULL)) != 0)
		com_err(whoami, stat, " in filesystem not updated");
	    else
		Put_message("filesystem sucessfully updated.");
	}
	temp_elem = temp_elem->q_forw;
    }
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
    int stat, count;

    if ( !ValidName(argv[1]) )
	return(DM_NORMAL);

    if ( (stat = sms_query("get_filesys_by_label", 1, argv + 1,
			   NullFunc, NULL)) == 0) {
	Put_message ("A Filesystem by that name already exists.");
	return(DM_NORMAL);
    } else if (status != SMS_NOMATCH) {
	com_err(whoami, stat, " in AddFS");
	return(DM_NORMAL);
    } 

    while (count = 0; count < 100; count++)
	info[count] = NULL;
    args = AskFSInfo(info, FALSE );
    
    if (stat = sms_query("add_filesys", CountArgs(args), args, 
			 NullFunc, NULL) != 0)
	com_err(whoami, stat, " in AddFS");

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
    register int stat = 0;
    char **info, buf[BUFSIZ];
    struct qelem *top, *elem;

    top = elem = GetFSInfo(FS_ALIAS, argv[1]);

    while (elem != NULL) {
	info = (char **) elem->q_data;
	sprintf(buf,"Alias: %s\tFilesystem: %s",info[NAME], info[ALIAS_TRANS]);
	putmessage(buf);
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return(DM_NORMAL);
}

/*	Function Name: CreateFSAlias
 *	Description: Create an alias name for a filsystem
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
    register int stat = 0;
    struct qelem *elem, *top;
    char *args[3], buf[BUFSIZ], **info;
    int num_args;

    elem = NULL;

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    args[0] = argv[1];
    args[1] = FS_ALIAS_TYPE;

/*
 * Check to see if this alias already exists in the database, if so then
 * print out values, free memory used and then exit.
 */

    if ( (stat = sms_query("get_alias", 2, args, StoreInfo, &elem)) == 0) {
	top = elem;
	while (elem != NULL) {
	    info = (char **) elem->q_data;	    
	    sprintf(buf,"The alias: %s\tcurrently describes the filesystem %s",
		    info[NAME], info[ALIAS_TRANS]);
	    Put_message(buf);
	    elem = elem->q_forw;
	}
	FreeQueue(top);
	return(DM_NORMAL);
    }
    else if ( stat != SMS_NO_MATCH) {
	com_err(whoami, stat, " in CreateFSAlias.");
        return(DM_NORMAL);
    }

    args[ALIAS_TRANS] = NULL;	/* set to zero */
    GetValueFromUser("Which filesystem will this alias point to?",
		     args[ALIAS_TRANS]);

    if ( (stat = sms_query("add_alias", 3, args, NullFunc, NULL)) != 0)
	com_err(whoami, stat, " in CreateFSAlias.");

    free(args[ALIAS_TRANS]);
    return (DM_NORMAL);
}

/*	Function Name: DeleteFSAlias
 *	Description: Delete an alias name for a filsystem
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
    register int stat;
    char *args[3], buf[BUFSIZ], **info;
    int num_args, delete;
    struct qelem *elem, *top;

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    top = elem = GetFSInfo(FS_ALIAS, argv[1]);

/* 
 * 1) If there are no (zero) match in elements then we exit immediately.
 * 2) If there is exactly 1 element then we ask for confirmation only if in
 *    verbose mode, via the Confirm function.  
 * 3) If there is more than 1 filesystem alias to be deleted then we ask
 *    about each one, and delete on yes only, and about if the user hits
 *    quit.
 */
    number = QueueCount(top);
    while (elem != NULL) {
	char **info = (char **) elem->q_data;

	if (number != 1) {
	    sprintf(buf, "%s %s for the filesystem %s.",
		    "Confirm that you want to delete\n the alias",
		    info[NAME], info[ALIAS_TRANS]);
	    Put_message(buf);
	    
	    answer = YesNoQuitQuestion("\nDelete this filesystem alias?", 
				       FALSE); 
	    switch(answer) {
	    case TRUE:
		delete = TRUE;
		break;
	    case FALSE:
		delete = FALSE;
		break;
	    default:		/* Quit. */
		Put_message("Aborting Delete Operation.");
		FreeQueue(top);
		return(DM_NORMAL);
	    }
	}
	else {
	    sprintf(buf, "Are you sure that you want to delete %s",
		    "this filesystem alias?");
	    delete = Confirm(buf);
	}
/* 
 * Deletetions are  performed if the user hits 'y' on a list of multiple 
 * filesystem aliases, or if the user confirms on a unique alias.
 */
	if (delete) {
	    if ( (status = sms_query("delete_filesys", 1,
				     &name, NullFunc, NULL)) != 0)
		com_err(whoami, stat, " filesystem alias not deleted.");
	    else
		Put_message("Filesystem alias deleted.");
	}
	else 
	    Put_message("Filesystem alias not deleted.");
	elem = elem->q_forw;
    }

    FreeQueue(Top);
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
	"These are the options:\n",
	"get - get information about a filesystem.",
	"add - add a new filesystem to the data base.",
	"update - update the information in the database on a filesystem.",
	"delete - delete a filesystem from the database.\n",
	"check - check information about association of a name and a filesys.",
	"alias - associate a name with a filsystem.",
	"unalias - disassociate a name with a filesystem.",
	"\twhere a name is: user, group, or project\n",
	"verbose - toggle the request for delete confirmation\n",
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
