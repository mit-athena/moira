#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.19 1997-01-29 23:06:22 danw Exp $";
#endif

/*	This is the file nfs.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains: All functions for manipulating NFS Physical directories.
 *	
 *	Created: 	5/6/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/nfs.c,v 1.19 1997-01-29 23:06:22 danw Exp $
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

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"


#define TYPE_NFS    "NFS"

#define DEFAULT_DIR    "/u1/lockers"
#define DEFAULT_DEVICE "/dev/ra1c"
#define DEFAULT_STATUS DEFAULT_YES /* active. */
#define DEFAULT_ALLOC  "0"
#define DEFAULT_SIZE   "0"

/*	Function Name: UpdatePrint
 *	Description: store a useful string for updates to print.
 *	Arguments: info - info about NFS service stored in array of strings.
 *	Returns: useful string.
 */

static char *
UpdatePrint(info)
char ** info;
{
    char temp_buf[BUFSIZ];
    sprintf(temp_buf, "Machine %s Directory %s", 
	    info[NFS_NAME], info[NFS_DIR]);
    return(Strsave(temp_buf));	/* Small memory leak here, but no good way
				   to avoid it that I see. */
}

/*	Function Name: PrintNFSInfo
 *	Description: Prints NFS Physical service information.
 *	Arguments: info - the information.
 *	Returns: directory of this nfs server, for DeleteNFSService().
 */

static char *
PrintNFSInfo(info)
char ** info;
{
    char buf[BUFSIZ], status_buf[BUFSIZ];
    int status = atoi(info[NFS_STATUS]);
    Bool is_one = FALSE;
    
    status_buf[0] = '\0';	/* clear string. */

    if (status & MR_FS_STUDENT) {
	strcat(status_buf, "Student");
	is_one = TRUE;
    }
    if (status & MR_FS_FACULTY) {
	if (is_one)
	    strcat(status_buf, " and ");
	strcat(status_buf, "Faculty");
	is_one = TRUE;
    }
    if (status & MR_FS_STAFF) {
	if (is_one)
	    strcat(status_buf, " and ");
	strcat(status_buf, "Staff");
	is_one = TRUE;
    }
    if (status & MR_FS_MISC) {
	if (is_one)
	    strcat(status_buf, " and ");
	strcat(status_buf, "Miscellaneous");
    }
    /* Add another type here. */
    if (status & MR_FS_GROUPQUOTA) {
	if (is_one)
	    strcat(status_buf, " / ");
	strcat(status_buf, "Group Quotas Enabled");
    }

    if (status_buf[0] == '\0')
	strcat(status_buf, "-- None --");
    
    Put_message("");
    sprintf(buf,"Machine: %-20s Directory: %-15s Device: %s",
	    info[NFS_NAME], info[NFS_DIR], info[NFS_DEVICE]);
    Put_message(buf);
    sprintf(buf, "Status: %s", status_buf);
    Put_message(buf);
    sprintf(buf, "Quota Allocated: %-17s Size: %s",
	    info[NFS_ALLOC], info[NFS_SIZE]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[NFS_MODBY], info[NFS_MODTIME],
	    info[NFS_MODWITH]);
    Put_message(buf);
    return(info[NFS_DIR]);
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

    if (GetValueFromUser("Device for this filsystem", &info[NFS_DEVICE]) ==
	SUB_ERROR)
      return(NULL);
    if (GetFSTypes(&info[NFS_STATUS], TRUE) == SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Allocated Space for this filsystem:",
			 &info[NFS_ALLOC]) == SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Size of this Filsystem:", &info[NFS_SIZE]) ==
	SUB_ERROR)
      return(NULL);

    FreeAndClear(&info[NFS_MODTIME], TRUE);
    FreeAndClear(&info[NFS_MODBY], TRUE);
    FreeAndClear(&info[NFS_MODWITH], TRUE);
    
    return(info);
}


/*	Function Name: ShowNFSService
 *	Description: This function prints all exported partitions.
 *	Arguments: argc, argv - argv[1] - name of machine.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int 
ShowNFSService(argc, argv)
int argc;
char **argv;   
{
    register int stat;
    struct qelem *elem = NULL;
    char *args[10];

    if (!ValidName(argv[1]))
	return(DM_NORMAL);
    
    args[0] = canonicalize_hostname(strsave(argv[1]));
    args[1] = strsave(DEFAULT_DIR);
    if (GetValueFromUser("Directory:", &args[1]) == SUB_ERROR)
      return(DM_NORMAL);
    
    if ( (stat = do_mr_query("get_nfsphys", 2, args,
			      StoreInfo, (char *)  &elem)) != MR_SUCCESS)
	com_err(program_name, stat, " in ShowNFSServices.");
    free(args[0]);
    free(args[1]);		/* prevents memory leaks. */

    elem = QueueTop(elem);
    Loop(elem, (void *) PrintNFSInfo);

    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: AddNFSService
 *	Description: Adds a new partition to the nfsphys relation
 *	Arguments: arc, argv - argv[1] - machine name.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddNFSService(argc, argv)
char **argv;
int argc;
{
    char **add_args, *args[10];
    char *info[MAX_ARGS_SIZE];
    int stat;

    args[0] = canonicalize_hostname(strsave(argv[1]));
    args[1] = strsave(DEFAULT_DIR);
    if (GetValueFromUser("Directory:", &args[1]) == SUB_ERROR)
      return(DM_NORMAL);
    
    if (!ValidName(args[0]) || !ValidName(args[1]))
	return(DM_NORMAL);
    
    if ( (stat = do_mr_query("get_nfsphys", 2, args,
			      NullFunc, (char *) NULL)) == MR_SUCCESS)
      stat = MR_EXISTS;
    if (stat != MR_NO_MATCH) {
	com_err(program_name, stat, " in get_nfsphys.");
	return(DM_NORMAL);
    }
    
    info[NFS_NAME]   = Strsave(args[0]);
    info[NFS_DIR]    = args[1];	/* already saved. */
    info[NFS_DEVICE] = Strsave(DEFAULT_DEVICE);
    info[NFS_STATUS] = Strsave(DEFAULT_STATUS);
    info[NFS_ALLOC]  = Strsave(DEFAULT_ALLOC);
    info[NFS_SIZE]   = Strsave(DEFAULT_SIZE);
    info[NFS_MODBY] = info[NFS_MODWITH] = info[NFS_MODTIME] = NULL;
    info[NFS_END] = NULL;	

    if ((add_args = AskNFSInfo(info)) == NULL) {
	Put_message("Aborted.");
	return(DM_NORMAL);
    }
    
    if ((stat = do_mr_query("add_nfsphys", CountArgs(add_args), add_args,
			     Scream, (char *) NULL)) != 0) 
	com_err(program_name, stat, " in AdsNFSService");
    
    FreeInfo(info);
    free(args[0]);
    return (DM_NORMAL);
}

/*	Function Name: RealUpdateNFSService
 *	Description: performs the actual update of the nfs service.
 *	Arguments: info - info about NFS service stored in array of strings.
 *                 junk - an unused boolean.
 *	Returns: none.
 */

/* ARGSUSED */
static void
RealUpdateNFSService(info, junk)
char ** info;
Bool junk;
{
    char ** args;
    register int stat;
    
    if ((args = AskNFSInfo(info)) == NULL) {
	Put_message("Aborted.");
	return;
    }

    if ((stat = do_mr_query("update_nfsphys", CountArgs(args), args,
			     Scream, (char *)NULL)) != MR_SUCCESS) 
	com_err(program_name, stat, " in RealUpdateNFSService");
}

/*	Function Name: UpdateNFSService
 *	Description: Update the values for an nfsphys entry.
 *	Arguments: argc, argv - argv[1] - machine name.
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED. */
int
UpdateNFSService(argc, argv)
char **argv;
int argc;
{
    register int stat;
    struct qelem *elem = NULL;
    char * args[10];

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    args[0] = canonicalize_hostname(strsave(argv[1]));
    args[1] = strsave(DEFAULT_DIR);
    if (GetValueFromUser("Directory:", &args[1]) == SUB_ERROR)
      return(DM_NORMAL);

    if ( (stat = do_mr_query("get_nfsphys", 2, args,
			      StoreInfo, (char *) &elem)) != MR_SUCCESS) {
	com_err(program_name, stat, " in UpdateNFSService.");
	return (DM_NORMAL);
    }
    free(args[0]);
    free(args[1]);		/* stop memory leaks. */

    elem = QueueTop(elem);
    QueryLoop(elem, UpdatePrint, RealUpdateNFSService, 
	      "Update NFS Service for");

    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: FSPartPrint
 *	Description: print filesystem partition usage.
 *	Arguments: info - the filesystem information.
 *	Returns: none.
 */

static void
FSPartPrint(info)
char ** info;
{
    char buf[BUFSIZ];
    sprintf(buf, "NFS Filesystem %s uses that partition.", info[FS_NAME]);
    Put_message(buf);
}

/*	Function Name: RealDeleteNFSService
 *	Description: Actually Deletes the filesystem (some checks are made).
 *	Arguments: info - info about NFS service stored in array of strings.
 *                 one_item - if TRUE then only one item on the queue, and
 *                            we should confirm.
 *	Returns: none.
 */

static void
RealDeleteNFSService(info, one_item)
char ** info;
Bool one_item;
{
    char temp_buf[BUFSIZ], *args[10];
    struct qelem *elem= NULL;
    register int stat;
    
    sprintf(temp_buf,
	    "Are you sure that you want to delete the %s directory on %s",
	    info[NFS_DIR],info[NFS_NAME]);

/* 
 * Check to be sure that it is not used by any of the nfs packs.
 */

    if (!one_item || Confirm(temp_buf)) {
	args[0] = info[NFS_NAME];
	args[1] = info[NFS_DIR];
	args[2] = NULL;
	switch(stat = do_mr_query("get_filesys_by_nfsphys", CountArgs(args), 
				   args, StoreInfo, (char *)&elem)) {
	case MR_NO_MATCH:	/* it is unused, delete it. */
	    if ( (stat = do_mr_query("delete_nfsphys", 2, info, Scream, 
				      (char *) NULL )) != MR_SUCCESS)
		com_err(program_name, stat, " in DeleteNFSService");
	    else
		Put_message("Physical Filesystem Deleted.");
	    break;
	case MR_SUCCESS:	/* it is used, print filesys's that use it. */
	    elem = QueueTop(elem);
	    Put_message("The following fileystems are using this partition,");
	    Put_message("and must be removed before it can be deleted.");
	    Put_message("");
	    Loop(elem, FSPartPrint);

	    FreeQueue(elem);
	    Put_message("");
	    break;
	default:
	    com_err(program_name, stat, " while checking usage of partition");
	}
    }
    else
	Put_message("Physical filesystem not deleted.");
}

/*	Function Name: DeleteNFSService
 *	Description: Delete an nfsphys entry.
 *	Arguments: argc, argv - name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
DeleteNFSService(argc, argv)
int argc;
char **argv;
{
    register int stat;
    struct qelem *elem = NULL;
    char * args[10];

    if (!ValidName(argv[1]))
	return(DM_NORMAL);

    args[0] = canonicalize_hostname(strsave(argv[1]));
    args[1] = strsave(DEFAULT_DIR);
    if (GetValueFromUser("Directory:", &args[1]) == SUB_ERROR)
      return(DM_NORMAL);

    switch(stat = do_mr_query("get_nfsphys", 2, args, 
			       StoreInfo, (char *) &elem)) {
    case MR_NO_MATCH:
	Put_message("This filsystem does not exist!");
	return(DM_NORMAL);
    case MR_SUCCESS:
	break;
    default:
	com_err(program_name, stat, " in DeleteNFSService");
	return(DM_NORMAL);
    }
    free(args[0]);
    free(args[1]);		/* stop memory leaks, in your neighborhood. */

    QueryLoop(elem, PrintNFSInfo, RealDeleteNFSService,
	      "Delete the Physical Filesystem on Directory");

    FreeQueue(elem);
    return(DM_NORMAL);
}    
