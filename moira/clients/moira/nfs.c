#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.3 1988-06-27 16:12:38 kit Exp $";
#endif lint

/*	This is the file nfs.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: The nfs maintanance code.
 *	
 *	Created: 	5/6/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.3 1988-06-27 16:12:38 kit Exp $
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

/* #include <sys/types.h> */

#define TYPE_NFS    "NFS"

#define DEFAULT_DIR    "/mit"
#define DEFAULT_STATUS "1"
#define DEFAULT_ALLOC  "0"
#define DEFAULT_SIZE   "400000"

/*	Function Name: PrintNFSInfo
 *	Description: Prints NFS Physical service information.
 *	Arguments: info - the information.
 *	Returns: none.
 */

void
PrintNFSInfo(info)
char ** info;
{
    char temp_buf[BUFSIZ];
    int status = atoi(info[NFS_STATUS]);
    
    sprintf(temp_buf,"Machine: %s,\tDirectory: %s,\tDevice: %s",
	    info[NFS_NAME], info[NFS_DIR], info[NFS_DEVICE]);
    Put_message(temp_buf);
    sprintf(temp_buf, "Status: %s,\tQuota Allocated: %s.\tSize: %s",
	    status ? "Active" : "Inactive", info[NFS_ALLOC], info[NFS_SIZE]);
    Put_message(temp_buf);
    sprintf(temp_buf, "Last Modification by %s at %s with %s.",
		   info[U_MODBY], info[U_MODTIME], info[U_MODWITH]);
    Put_message(temp_buf);
}
/*	Function Name: AskNFSInfo.
 *	Description: This function askes the user for information about a 
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *	Returns: the arglist to make the update call with.
 */

char **
AskNFSInfo(info)
char ** info;
{
    /* Also need name of the machine in this structure. */

    GetValueFromUser("Directory for filesystem:", &info[NFS_DIR]);
    GetValueFromUser("Device for this filsystem", &info[NFS_DEVICE]); 

    Put_message("\nTypes: Student, Faculty, Project, Staff, and Other.\n");
    GetValueFromUser("Please enter one or more of the above types:"
		     ,&info[NFS_STATUS]);

    GetValueFromUser("Allocated Space for this filsystem:",&info[NFS_ALLOC]);
    GetValueFromUser("Size of this Filsystem:",&info[NFS_SIZE]);

    FreeAndClear(&info[NFS_MODTIME], TRUE);
    FreeAndClear(&info[NFS_MODBY], TRUE);
    FreeAndClear(&info[NFS_MODWITH], TRUE);
    
    return(info);
}

/*	Function Name: ShowNFSService
 *	Description: This function prints all exported partitions.
 *	Arguments: argc, argv - argv[1] - name of machine.
 *                              argv[2] - name of directroy.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int 
ShowNFSService(argc, argv)
int argc;
char **argv;   
{
    register int stat;
    struct qelem * top, *elem = NULL;
    
    if ( (stat = sms_query("get_nfsphys", 2, argv + 1, 
			   StoreInfo, (char *)  &elem)) != 0)
	com_err(program_name, stat, " in ShowNFSServices.");

    top = elem;
    while (elem != NULL) {
	char ** info = (char **) elem->q_data;
	PrintNFSInfo(info);
	elem = elem->q_forw;
    }
    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: AddNFSService
 *	Description: Adds a new partition to the nfsphys relation
 *	Arguments: arc, argv - 
 *                             argv[1] - machine name.
 *                             argv[2] - directory.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddNFSService(argc, argv)
char **argv;
int argc;
{
    char **args;
    static char *info[MAX_ARGS_SIZE];
    int stat;
    
    if ( (stat = sms_query("get_nfsphys", 2, argv + 1, 
			   NullFunc, (char *) NULL)) == 0) {
	Put_message("This service already exists.");
	if (stat != SMS_NO_MATCH) 
	    com_err(program_name, stat, " in get_nfsphys.");
    }
    
    if ( (info[NFS_NAME] = CanonicalizeHostname(argv[1])) == NULL) {
	Put_message("Unknown host, try again...");
	return(DM_NORMAL);
    }

    info[NFS_NAME]   = Strsave(info[NFS_NAME]);
    info[NFS_DEVICE] = Strsave(argv[2]);
    info[NFS_DIR]    = Strsave(DEFAULT_DIR);
    info[NFS_STATUS] = Strsave(DEFAULT_STATUS);
    info[NFS_ALLOC]  = Strsave(DEFAULT_ALLOC);
    info[NFS_SIZE]   = Strsave(DEFAULT_SIZE);
    info[NFS_SIZE + 1] = NULL;	/* NULL terminate. */

    args = AskNFSInfo(info);
    
    if ((stat = sms_query("add_nfsphys", CountArgs(args), args,
			   Scream, (char *) NULL)) != 0) 
	com_err(program_name, stat, " in AdsNFSService");
    
    FreeInfo(info);
    return (DM_NORMAL);
}

/*	Function Name: UpdateNFSService
 *	Description: Update the values for an nfsphys entry.
 *	Arguments: argc, argv -
 *                             argv[1] - machine name.
 *                             argv[2] - directory.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED. */
int
UpdateNFSService(argc, argv)
char **argv;
int argc;
{
    register int stat;
    Bool update, one_service;
    char **args, **info, buf[BUFSIZ];
    struct qelem *elem, *top;
    elem = NULL;

    if ( (argv[1] = CanonicalizeHostname(argv[1])) == NULL) {
	Put_message("Unknown host, try again...");
	return(DM_NORMAL);
    }

    if ( (stat = sms_query("get_nfsphys", 2, argv + 1,
			   StoreInfo, (char *) &elem)) != 0) {
	com_err(program_name, stat, " in UpdateNFSService.");
	return (DM_NORMAL);
    }

    top = elem = QueueTop(elem);
    one_service = ( QueueCount(top) == 1 );
    while (elem != NULL) {
	info = (char **) elem->q_data;
	if (!one_service) {	/* If more than one then query through them. */
	    sprintf(buf,"Update - %s\tDirectory: %s? (y/n/q)", info[NFS_NAME],
		    info[NFS_DIR]);
	    switch( YesNoQuitQuestion(buf, FALSE)) {
	    case TRUE:
		update = TRUE;
		break;
	    case FALSE:
		update = FALSE;
		break;
	    default:
		FreeQueue(top);
		Put_message("Aborting update.");
		return(DM_NORMAL);
	    }
	}
	else
	    update = TRUE;

	if (update) {		/* actually perform update */
	    args = AskNFSInfo(info);
	    if ((stat = sms_query("update_nfsphys", CountArgs(args), args,
				  Scream, (char *)NULL)) != 0) 
		com_err(program_name, stat, (char *) NULL);
	}
	elem = elem->q_forw;
    }

    FreeQueue(top);
    return (DM_NORMAL);
}

/*	Function Name: DeleteNFSService
 *	Description: Delete an nfsphys entry.
 *	Arguments: argc, argv - name of file system in argv[1].
 *                              directory of file system in argv[2].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteNFSService(argc, argv)
int argc;
char **argv;
{
    register int stat;
    struct qelem *top, *elem = NULL;
    char * dir = argv[2];
    int length;
    Bool delete_ok = TRUE;

    argv[1] = CanonicalizeHostname(argv[1]);

    stat = sms_query("get_nfsphys", 2, argv + 1, NullFunc, (char *) NULL);
    if (stat == SMS_NO_MATCH) {
	Put_message("This filsystem does not exist!");
	return(DM_NORMAL);
    }
    if (stat) {
	com_err(program_name, stat, " in DeleteNFSService");
	return(DM_NORMAL);
    }
    
    stat = sms_query("get_filesys_by_machine", 1, argv + 1, StoreInfo, 
		     &elem);
    if (stat && stat != SMS_NO_MATCH)
	com_err(program_name, stat, " while checking usage of partition");

    length = strlen( dir );
    top = elem = QueueTop(elem);
    while (elem != NULL) {
	char buf[BUFSIZ];
	char ** info = (char ** ) elem->q_data;
	if ( (strcmp(info[FS_TYPE], TYPE_NFS) == 0) && 
	     (strcmp(info[FS_PACK], dir, length) == 0) ) {
	    sprintf(buf, "Filesystem %s uses that partition", info[FS_NAME]);
	    Put_message(buf);
	    delete_ok = FALSE;
	}
	elem = elem->q_forw;
    }

    if ( delete_ok && 
	Confirm("Do you really want to delete this Filesystem? (y/n) ")) {
	if ( (stat = sms_query("delete_nfsphys", 2, argv + 1,
			       Scream, (char *) NULL )) !=0 )
	    com_err(program_name, stat, " in DeleteNFSService");
    }
    else
	Put_message("Operation Aborted.\n");

    FreeQueue(top);
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
