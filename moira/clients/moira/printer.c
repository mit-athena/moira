#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.3 1988-10-03 13:05:55 mar Exp $";
#endif lint

/*	This is the file printer.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: Functions for handling the printers.
 *	
 *	Created: 	8/16/88
 *	By:		Theodore Y. Ts'o
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.3 1988-10-03 13:05:55 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
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
#include "defs.h"
#include "f_defs.h"
#include "globals.h"
#include "infodefs.h"

#define PCAP_NAME      	0
#define PCAP_SPOOL_HOST	1
#define PCAP_SPOOL_DIR	2
#define PCAP_RPRINTER	3
#define PCAP_COMMENTS	4
#define PCAP_MODTIME	5
#define PCAP_MODBY	6
#define PCAP_MODWITH	7
#define PCAP_END	8

#define DEFAULT_MACHINE "E40-PRINT-SERVER-1.MIT.EDU"

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
    char spool_dir[256];

    strcpy(spool_dir, "/usr/spool/printer/");
    strcat(spool_dir, name);
    
    info[PCAP_NAME] =   	Strsave(name);
    info[PCAP_SPOOL_HOST] =	Strsave(DEFAULT_MACHINE);
    info[PCAP_SPOOL_DIR] =	Strsave(spool_dir);
    info[PCAP_RPRINTER] =	Strsave(name);
    info[PCAP_COMMENTS] = 	Strsave("");
    info[PCAP_MODTIME] = info[PCAP_MODBY] = info[PCAP_MODWITH] = NULL;
    
    info[PCAP_END] = NULL;
    return(info);
}

/*	Function Name: GetPcapInfo
 *	Description: Stores the info in a queue.
 *	Arguments: name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue or null
 * 		if printer not found.
 */

static struct qelem *
GetPcapInfo(name)
char *name;

{
    int stat;
    struct qelem *elem = NULL;

    if ( (stat = do_sms_query("get_printcap", 1, &name,
			      StoreInfo, (char *)&elem)) != 0) {
	    com_err(program_name, stat, NULL);
	    return(NULL);
    }
    return(QueueTop(elem));
}

/*	Function Name: PrintPcapInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this Printer.
 *	Returns: none
 */

static void
PrintPcapInfo(info)
char ** info;
{
    char buf[BUFSIZ];
    
    if (!info)	{		/* If no informaion */
	    Put_message("PrintPcapInfo called with null info!");
	    return;
    }
    sprintf(buf, "Printer: %-35s Spool host: %s", info[PCAP_NAME],
	    info[PCAP_SPOOL_HOST]);
    Put_message(buf);
    sprintf(buf, "Spool directory: %-27s Remote Printer Name: %s",
	    info[PCAP_SPOOL_DIR], info[PCAP_RPRINTER]);
    Put_message(buf);
    sprintf(buf, "Comments: ", info[PCAP_COMMENTS]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[PCAP_MODBY], info[PCAP_MODTIME], 
	    info[PCAP_MODWITH]);
    Put_message(buf);
}

/*	Function Name: AskPcapInfo.
 *	Description: This function askes the user for information about a 
 *                   printer and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *	Returns: none.
 */

static char **
AskPcapInfo(info)
char ** info;
{
    char temp_buf[BUFSIZ], *newname;

    Put_message("");
    sprintf(temp_buf, "Printcap entry for %s.", 
	    info[FS_NAME]);
    Put_message(temp_buf);
    Put_message("");

    GetValueFromUser("Printer Server", &info[PCAP_SPOOL_HOST]);
    strcpy(temp_buf, CanonicalizeHostname(info[PCAP_SPOOL_HOST]));
    free(info[PCAP_SPOOL_HOST]);
    info[PCAP_SPOOL_HOST] = Strsave(temp_buf);
    GetValueFromUser("Spool Directory", &info[PCAP_SPOOL_DIR]);
    GetValueFromUser("Remote Printer Name", &info[PCAP_RPRINTER]);
    GetValueFromUser("Comments", &info[PCAP_COMMENTS]);
    
    FreeAndClear(&info[PCAP_MODTIME], TRUE);
    FreeAndClear(&info[PCAP_MODBY], TRUE);
    FreeAndClear(&info[PCAP_MODWITH], TRUE);
    
    return(info);
}

/* ---------------- Printer Menu ------------------ */

/*	Function Name: GetPcap
 *	Description: Get Printcap information
 *	Arguments: argc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
GetPcap(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetPcapInfo(argv[1]); /* get info. */
    Loop(top, (void *) PrintPcapInfo);
    FreeQueue(top);		/* clean the queue. */
    return (DM_NORMAL);
}

/*	Function Name: RealDeletePcap
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item 
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void
RealDeletePcap(info, one_item)
char ** info;
Bool one_item;
{
    int stat;
    char temp_buf[BUFSIZ];

    if ( (stat = do_sms_query("delete_printcap", 1,
			      &info[FS_NAME], Scream, NULL)) != 0)
	    com_err(program_name, stat, " printcap entry not deleted.");
    else
	    Put_message("Printcap entry deleted.");
}

/*	Function Name: DeletePcap
 *	Description: Delete a printcap entry given its name.
 *	Arguments: argc, argv - argv[1] is the name of the printer.
 *	Returns: none.
 */

/* ARGSUSED */
 
int
DeletePcap(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetPcapInfo(argv[1]);
    QueryLoop(elem, PrintPcapInfo, RealDeletePcap, "Delete Printer");

    FreeQueue(elem);
    return (DM_NORMAL);
}

/*	Function Name: AddPcap
 *	Description: Add a printcap entry
 *	Arguments: arc, argv - name of printer in argv[1].
 *	Returns: DM_NORMAL.
 */

/* ARGSUSED */
int
AddPcap(argc, argv)
char **argv;
int argc;
{
    char *info[MAX_ARGS_SIZE], **args;
    int stat;

    if ( !ValidName(argv[1]) )
	return(DM_NORMAL);

    if ( (stat = do_sms_query("get_printcap", 1, argv + 1,
			      NullFunc, NULL)) == 0) {
	Put_message ("A Printer by that name already exists.");
	return(DM_NORMAL);
    } else if (stat != SMS_NO_MATCH) {
	com_err(program_name, stat, " in AddPcap");
	return(DM_NORMAL);
    } 

    args = AskPcapInfo(SetDefaults(info, argv[1]));

    if ( (stat = do_sms_query("add_printcap", CountArgs(args), args, 
			      NullFunc, NULL)) != 0)
	com_err(program_name, stat, " in AddPcap");

    FreeInfo(info);
    return (DM_NORMAL);
}


/*	Function Name: ChangePcap
 *	Description: Do the work of changing a pcap
 *	Arguments: argc, argv - printcap info
 *	Returns: 
 */

int
ChangePcap(info, one_item)
char **info;
Bool one_item;
{
    int stat;

    if ((stat = do_sms_query("delete_printcap", 1, &info[FS_NAME],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " printcap entry not deleted.");
	return(DM_NORMAL);
    }
    AskPcapInfo(info);
    if ((stat = do_sms_query("add_printcap", CountArgs(info), info,
			     NullFunc, NULL)) != 0)
	com_err(program_name, stat, " in ChngPcap");
    return(DM_NORMAL);
}


/*	Function Name: ChngPcap
 *	Description:   Update the printcap information
 *	Arguments:     argc, argv - name of printer in argv[1].
 *	Returns:       DM_NORMAL.
 */

int
ChngPcap(argc, argv)
    int argc;
    char **argv;
{
    struct qelem *elem = GetPcapInfo(argv[1]);
    QueryLoop(elem, NullPrint, ChangePcap, "Change the printer");
    FreeQueue(elem);
    return(DM_NORMAL);
}
