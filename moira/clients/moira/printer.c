#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.12 1991-01-04 16:58:21 mar Exp $";
#endif lint

/*	This is the file printer.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains: Functions for handling the printers.
 *	
 *	Created: 	8/16/88
 *	By:		Theodore Y. Ts'o
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.12 1991-01-04 16:58:21 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

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
    info[PCAP_QSERVER] =	Strsave("\\[NONE\\]");
    info[PCAP_AUTH] =		Strsave("1");
    info[PCAP_PRICE] =		Strsave("10");
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

    if ( (stat = do_mr_query("get_printcap_entry", 1, &name,
			      StoreInfo, (char *)&elem)) != 0) {
	    com_err(program_name, stat, " in GetPcapInfo");
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
    sprintf(buf, "Authentication: %-3s Price/page: %-3s  Quota Server: %s",
	    atoi(info[PCAP_AUTH]) ? "yes" : "no",
	    info[PCAP_PRICE], info[PCAP_QSERVER]);
    Put_message(buf);
    sprintf(buf, "Comments: %s", info[PCAP_COMMENTS]);
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
    char temp_buf[BUFSIZ];

    Put_message("");
    sprintf(temp_buf, "Printcap entry for %s.", 
	    info[PCAP_NAME]);
    Put_message(temp_buf);
    Put_message("");

    if (GetValueFromUser("Printer Server", &info[PCAP_SPOOL_HOST]) == SUB_ERROR)
      return(NULL);
    info[PCAP_SPOOL_HOST] = canonicalize_hostname(info[PCAP_SPOOL_HOST]);
    if (GetValueFromUser("Spool Directory", &info[PCAP_SPOOL_DIR]) ==
	SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Remote Printer Name", &info[PCAP_RPRINTER]) ==
	SUB_ERROR)
      return(NULL);
    if (!strcmp(info[PCAP_QSERVER], "[NONE]")) {
	free(info[PCAP_QSERVER]);
	info[PCAP_QSERVER] = strsave("\\[NONE\\]");
    }
    if (GetValueFromUser("Quotaserver for this printer", &info[PCAP_QSERVER]) ==
	SUB_ERROR)
      return(NULL);
    info[PCAP_QSERVER] = canonicalize_hostname(info[PCAP_QSERVER]);
    if (GetYesNoValueFromUser("Authentication required", &info[PCAP_AUTH]) ==
	SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Price/page", &info[PCAP_PRICE]) == SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Comments", &info[PCAP_COMMENTS]) == SUB_ERROR)
      return(NULL);
    
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

    if ( (stat = do_mr_query("delete_printcap_entry", 1,
			      &info[PCAP_NAME], Scream, NULL)) != 0)
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

    if ( (stat = do_mr_query("get_printcap_entry", 1, argv + 1,
			      NullFunc, NULL)) == 0) {
	Put_message ("A Printer by that name already exists.");
	return(DM_NORMAL);
    } else if (stat != MR_NO_MATCH) {
	com_err(program_name, stat, " in AddPcap");
	return(DM_NORMAL);
    } 

    args = AskPcapInfo(SetDefaults(info, argv[1]));
    if (args == NULL) {
	Put_message("Aborted.");
	return(DM_NORMAL);
    }

    if ( (stat = do_mr_query("add_printcap_entry", CountArgs(args), args, 
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
    char **oldinfo;

    oldinfo = CopyInfo(info);
    if (AskPcapInfo(info) == NULL)
      return(DM_QUIT);
    if ((stat = do_mr_query("delete_printcap_entry", 1, &info[PCAP_NAME],
			     Scream, NULL)) != 0) {
	com_err(program_name, stat, " printcap entry not deleted.");
	return(DM_NORMAL);
    }
    if ((stat = do_mr_query("add_printcap_entry", CountArgs(info), info,
			     NullFunc, NULL)) != 0) {
	com_err(program_name, stat, " in ChngPcap");
	if ((stat = do_mr_query("add_printcap_entry", CountArgs(oldinfo) - 3,
				oldinfo, NullFunc, NULL)) != 0)
	  com_err(program_name, stat, " while attempting to put old info back");
    }
    FreeInfo(oldinfo);
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


/*	Function Name: SetPdDefaults
 *	Description: sets the default values for palladium additions.
 *	Arguments: info - an array of char pointers to recieve defaults. 
 *	Returns: char ** (this array, now filled).
 */

static char ** 
SetPdDefaults(info, name)
char ** info;
char * name;
{
    info[PD_NAME] =		Strsave(name);
    info[PD_IDENT] =		Strsave("10000");
    info[PD_HOST] =		Strsave(DEFAULT_MACHINE);
    info[PD_MODTIME] = info[PD_MODBY] = info[PD_MODWITH] = NULL;

    info[PD_END] = NULL;
    return(info);
}

/*	Function Name: AskPalladiumInfo.
 *	Description: This function askes the user for information about a 
 *                   printer and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *	Returns: none.
 */

static char **
AskPalladiumInfo(info)
char ** info;
{
    char temp_buf[BUFSIZ];

    Put_message("");
    sprintf(temp_buf, "Palladium Server/Supervisor entry for %s.", 
	    info[PD_NAME]);
    Put_message(temp_buf);
    Put_message("");

    if (GetValueFromUser("RPC Program Number", &info[PD_IDENT]) == SUB_ERROR)
      return(NULL);
    if (GetValueFromUser("Print Server/Supervisor Host", &info[PD_HOST]) ==
	SUB_ERROR)
      return(NULL);
    info[PD_HOST] = canonicalize_hostname(info[PD_HOST]);
    
    FreeAndClear(&info[PD_MODTIME], TRUE);
    FreeAndClear(&info[PD_MODBY], TRUE);
    FreeAndClear(&info[PD_MODWITH], TRUE);
    
    return(info);
}


/*	Function Name: PrintPalladiumInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this Printer.
 *	Returns: none
 */

static void
PrintPalladiumInfo(info)
char ** info;
{
    char buf[BUFSIZ];
    
    if (!info)	{		/* If no informaion */
	    Put_message("PrintPalladiumInfo called with null info!");
	    return;
    }

    sprintf(buf, "Name: %-24s Program #: %s  Host: %s",
	    info[PD_NAME], info[PD_IDENT], info[PD_HOST]);
    Put_message(buf);
    sprintf(buf, MOD_FORMAT, info[PD_MODBY], info[PD_MODTIME],
	    info[PD_MODWITH]);
    Put_message(buf);
}


static struct qelem *
GetPalladiumInfo(name)
char *name;
{
    int status;
    struct qelem *elem = NULL;

    if ((status = do_mr_query("get_palladium", 1, &name, StoreInfo, &elem))
	!= 0) {
	com_err(program_name, status, " in GetPalladiumInfo");
	return(NULL);
    }
    return(QueueTop(elem));
}


int ChangePalladium(info, one_item)
char **info;
Bool one_item;
{
    int status;

    if (AskPalladiumInfo(info) == NULL)
      return(DM_QUIT);
    if ((status = do_mr_query("delete_palladium", 1, &info[PD_NAME],
			       Scream, NULL)) != 0) {
	com_err(program_name, status, " palladium entry not deleted.");
	return(DM_NORMAL);
    }
    if ((status = do_mr_query("add_palladium", CountArgs(info), info,
			       NullFunc, NULL)) != 0)
      com_err(program_name, status, " in ChngPalladium");
    return(DM_NORMAL);
}


/*	Function Name: RealDeletePalladium
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item 
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void
RealDeletePalladium(info, one_item)
char ** info;
Bool one_item;
{
    int stat;

    if ( (stat = do_mr_query("delete_palladium", 1,
			      &info[PD_NAME], Scream, NULL)) != 0)
	    com_err(program_name, stat, " palladium entry not deleted.");
    else
	    Put_message("Palladium entry deleted.");
}


int GetPalladium(argc, argv)
int argc;
char **argv;
{
    struct qelem *top;

    top = GetPalladiumInfo(argv[1]);
    Loop(top, PrintPalladiumInfo);
    FreeQueue(top);
    return(DM_NORMAL);
}


int AddPalladium(argc, argv)
int argc;
char **argv;
{
    char *info[MAX_ARGS_SIZE], **args;
    int status;

    if (!ValidName(argv[1]))
      return(DM_NORMAL);

    if ((status = do_mr_query("get_palladium", 1, &argv[1], NullFunc, NULL))
	== 0) {
	Put_message("A server or supervisor by that name already exists.");
	return(DM_NORMAL);
    } else if (status != MR_NO_MATCH) {
	com_err(program_name, status, " in AddPalladium");
	return(DM_NORMAL);
    }

    args = AskPalladiumInfo(SetPdDefaults(info, argv[1]));
    if (args == NULL) {
	Put_message("Aborted.");
	return(DM_NORMAL);
    }

    if ((status = do_mr_query("add_palladium", CountArgs(args), args,
			       Scream, NULL)) != 0)
      com_err(program_name, status, " in AddPalladium");

    FreeInfo(info);
    return(DM_NORMAL);
}


int ChngPalladium(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetPalladiumInfo(argv[1]);
    QueryLoop(elem, NullPrint, ChangePalladium, "Change the server/supervisor");
    FreeQueue(elem);
    return(DM_NORMAL);
}


int DeletePalladium(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = GetPalladiumInfo(argv[1]);
    QueryLoop(elem, PrintPalladiumInfo, RealDeletePalladium, "Delete server/supervisor");
    FreeQueue(elem);
    return(DM_NORMAL);
}

int ShowPalladiumAlias(argc, argv)
int argc;
char **argv;
{
    struct qelem *elem = NULL;
    char *qargv[3], buf[BUFSIZ];
    int status;

    qargv[0] = argv[1];
    qargv[1] = "PALLADIUM";
    qargv[2] = argv[2];
    if ((status = do_mr_query("get_alias", 3, qargv, StoreInfo, &elem)) != 0) {
	com_err(program_name, status, " in ShowPalladiumAlias");
	return(DM_NORMAL);
    }
    elem = QueueTop(elem);
    Put_message("");
    while (elem != NULL) {
	char **info = (char **) elem->q_data;
	sprintf(buf, "Printer: %-16s Server/Supervisor: %s", info[0], info[2]);
	Put_message(buf);
	elem = elem->q_forw;
    }

    FreeQueue(QueueTop(elem));
    return(DM_NORMAL);
}

int AddPalladiumAlias(argc, argv)
int argc;
char **argv;
{
    int status;
    char *qargv[3];

    qargv[0] = argv[1];
    qargv[1] = "PALLADIUM";
    qargv[2] = argv[2];
    if ((status = do_mr_query("add_alias", 3, qargv, Scream, NULL)) != 0)
      com_err(program_name, status, " in AddPalladiumAlias");
    return(DM_NORMAL);
}

int DeletePalladiumAlias(argc, argv)
int argc;
char **argv;
{
    int status;
    char *qargv[3];

    qargv[0] = argv[1];
    qargv[1] = "PALLADIUM";
    qargv[2] = argv[2];
    if ((status = do_mr_query("delete_alias", 3, qargv, Scream, NULL)) != 0)
      com_err(program_name, status, " in DeletePalladiumAlias");
    return(DM_NORMAL);
}
