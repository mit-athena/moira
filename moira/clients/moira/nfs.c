#ifndef lint
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.1 1988-06-09 14:13:28 kit Exp $";
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
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.1 1988-06-09 14:13:28 kit Exp $
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
		   answer[U_MODBY], answer[U_MODTIME], answer[U_MODWITH]);
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
AskNFSInfo(info);
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
    
    return(info)
}

/*	Function Name: ShowNFSService
 *	Description: This function prints all exported partitions.
 *	Arguments: argc, argv - argv[1] - name of filesysytem.
 *                              argv[2] - name of filesystem device.
 *	Returns: DM_NORMAL.
 */

int 
ShowNFSService(argc, argv)
int argc;
char **argv;   
{
    register int stat;
    struct qelem * top, *elem = NULL;
    
    if ( (stat = sms_query("get_nfsphys", 2, argv + 1, 
			   StoreInfo, (char *)  &elem)) != 0)
	com_err(whoami, stat, " in ShowNFSServices.");

    top = elem;
    while (elem != NULL) {
	info = (char **) elem->q_data;
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
 *                             argv[2] - device
 *	Returns: DM_NORMAL.
 */

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
	    com_err(whoami, stat, " in get_nfsphys.");
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
	com_err(whoami, stat, " in AdsNFSService");
    
    FreeInfo(info);
    return (DM_NORMAL);
}

/*	Function Name: UpdateNFSService
 *	Description: Update the values for an nfsphys entry.
 *	Arguments: argc, argv -
 *                             argv[1] - machine name.
 *                             argv[2] - device
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
	com_err(whoami, stat, " in UpdateNFSService.");
	return (DM_NORMAL);
    }

    top = elem;
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
	    case QUIT:
		FreeQueue(top);
		Put_message("Aborting update.");
		Return(DM_NORMAL);
		break;
	    }
	}
	else
	    update = TRUE;

	if (update) {		/* actually perform update */
	    args = AskNFSInfo(info);
	    if ((stat = sms_query("update_nfsphys", num_args, args,
				  abort, (char *)NULL)) != 0) 
		com_err(whoami, stat, (char *) NULL);
	}
	elem = elem->q_next;
    }

    FreeQueue(top);
    return (DM_NORMAL);
}

/* stop CDP 6/7/88 */

/*	Function Name: DeleteCheck
 *	Description: This checks to see if we should delete this nfs
 *	Arguments: argc, argv - 
 *                              argv[0] - name of file system.
 *                              argv[1] - type of file system.
 *                              argv[3] - packname of fulsys
 *                data - a filled info structure.
 *	Returns: SMS_CONT.
 */

DeleteCheck(argc, argv, date)
int argc;
char **argv, *data;
{
    char buf[BUFSIZ];
    NFS_info *info = (NFS_info *) data;

    return(SMS_CONT);
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
    struct qelem *elem, *top;
    char * dir = argv[2];
    int length;
    Bool delete_ok = TRUE;

    elem = NULL;
    argv[1] = CanonicalizeHostname(argv[1]);

    stat = sms_query("get_nfsphys", 2, argv + 1, NullFunc, (char *) NULL);
    if (stat == SMS_NO_MATCH) {
	Put_message("This filsystem does not exist!");
	return(DM_NORMAL);
    }
    if (stat) {
	com_err(whoami, stat, " in DeleteNFSService");
	return(DM_NORMAL);
    }
    
    stat = sms_query("get_filesys_by_machine", 1, argv + 1, StoreInfo, 
		     &elem);
    if (stat && stat != SMS_NO_MATCH)
	com_err(whoami, stat, " while checking usage of partition");

    length = strlen( dir );
    top = elem;
    while (elem != NULL) {
	info = (char ** ) elem->q_data;
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
	    com_err(whoami, stat, " in DeleteNFSService");
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
