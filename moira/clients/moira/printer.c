/* $Id: printer.c,v 1.20 1998-03-10 21:09:42 danw Exp $
 *
 *	This is the file printer.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for handling the printers.
 *
 *	Created: 	8/16/88
 *	By:		Theodore Y. Ts'o
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.20 1998-03-10 21:09:42 danw Exp $");

void RealDeletePcap(char **info, Bool one_item);
void ChangePcap(char **info, Bool one_item);
void ChangePalladium(char **info, Bool one_item);
void RealDeletePalladium(char **info, Bool one_item);

#define DEFAULT_MACHINE "E40-PRINT-SERVER-1.MIT.EDU"

/*	Function Name: SetDefaults
 *	Description: sets the default values for filesystem additions.
 *	Arguments: info - an array of char pointers to recieve defaults.
 *	Returns: char ** (this array, now filled).
 */

static char **SetDefaults(char **info, char *name)
{
  char spool_dir[256];

  strcpy(spool_dir, "/usr/spool/printer/");
  strcat(spool_dir, name);

  info[PCAP_NAME] = strdup(name);
  info[PCAP_SPOOL_HOST] = strdup(DEFAULT_MACHINE);
  info[PCAP_SPOOL_DIR] = strdup(spool_dir);
  info[PCAP_RPRINTER] = strdup(name);
  info[PCAP_QSERVER] = strdup("[NONE]");
  info[PCAP_AUTH] = strdup("1");
  info[PCAP_PRICE] = strdup("10");
  info[PCAP_COMMENTS] = strdup("");
  info[PCAP_MODTIME] = info[PCAP_MODBY] = info[PCAP_MODWITH] = NULL;

  info[PCAP_END] = NULL;
  return info;
}

/*	Function Name: GetPcapInfo
 *	Description: Stores the info in a queue.
 *	Arguments: name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue or null
 * 		if printer not found.
 */

static struct mqelem *GetPcapInfo(char *name)
{
  int stat;
  struct mqelem *elem = NULL;

  if ((stat = do_mr_query("get_printcap_entry", 1, &name, StoreInfo, &elem)))
    {
      com_err(program_name, stat, " in GetPcapInfo");
      return NULL;
    }
  return QueueTop(elem);
}

/*	Function Name: PrintPcapInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this Printer.
 *	Returns: printer name
 */

static char *PrintPcapInfo(char **info)
{
  char buf[BUFSIZ];

  if (!info)		/* If no informaion */
    {
      Put_message("PrintPcapInfo called with null info!");
      return NULL;
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

  return info[PCAP_NAME];
}

/*	Function Name: AskPcapInfo.
 *	Description: This function askes the user for information about a
 *                   printer and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *	Returns: none.
 */

static char **AskPcapInfo(char **info)
{
  char temp_buf[BUFSIZ];

  Put_message("");
  sprintf(temp_buf, "Printcap entry for %s.", info[PCAP_NAME]);
  Put_message(temp_buf);
  Put_message("");

  if (GetValueFromUser("Printer Server", &info[PCAP_SPOOL_HOST]) == SUB_ERROR)
    return NULL;
  info[PCAP_SPOOL_HOST] = canonicalize_hostname(info[PCAP_SPOOL_HOST]);
  if (GetValueFromUser("Spool Directory", &info[PCAP_SPOOL_DIR]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Remote Printer Name", &info[PCAP_RPRINTER]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Quotaserver for this printer", &info[PCAP_QSERVER]) ==
      SUB_ERROR)
    return NULL;
  info[PCAP_QSERVER] = canonicalize_hostname(info[PCAP_QSERVER]);
  if (GetYesNoValueFromUser("Authentication required", &info[PCAP_AUTH]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Price/page", &info[PCAP_PRICE]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Comments", &info[PCAP_COMMENTS]) == SUB_ERROR)
    return NULL;

  FreeAndClear(&info[PCAP_MODTIME], TRUE);
  FreeAndClear(&info[PCAP_MODBY], TRUE);
  FreeAndClear(&info[PCAP_MODWITH], TRUE);

  return info;
}

/* ---------------- Printer Menu ------------------ */

/*	Function Name: GetPcap
 *	Description: Get Printcap information
 *	Arguments: argc, argv - name of filsys in argv[1].
 *	Returns: DM_NORMAL.
 */

int GetPcap(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPcapInfo(argv[1]); /* get info. */
  Loop(top, (void *) PrintPcapInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

/*	Function Name: RealDeletePcap
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeletePcap(char **info, Bool one_item)
{
  int stat;

  if ((stat = do_mr_query("delete_printcap_entry", 1,
			  &info[PCAP_NAME], NULL, NULL)))
    com_err(program_name, stat, " printcap entry not deleted.");
  else
    Put_message("Printcap entry deleted.");
}

/*	Function Name: DeletePcap
 *	Description: Delete a printcap entry given its name.
 *	Arguments: argc, argv - argv[1] is the name of the printer.
 *	Returns: none.
 */

int DeletePcap(int argc, char **argv)
{
  struct mqelem *elem = GetPcapInfo(argv[1]);
  QueryLoop(elem, PrintPcapInfo, RealDeletePcap, "Delete Printer");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: AddPcap
 *	Description: Add a printcap entry
 *	Arguments: arc, argv - name of printer in argv[1].
 *	Returns: DM_NORMAL.
 */

int AddPcap(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args;
  int stat;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!(stat = do_mr_query("get_printcap_entry", 1, argv + 1,
			   NULL, NULL)))
    {
      Put_message ("A Printer by that name already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddPcap");
      return DM_NORMAL;
    }

  args = AskPcapInfo(SetDefaults(info, argv[1]));
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((stat = do_mr_query("add_printcap_entry", CountArgs(args), args,
			  NULL, NULL)))
    com_err(program_name, stat, " in AddPcap");

  FreeInfo(info);
  return DM_NORMAL;
}


/*	Function Name: ChangePcap
 *	Description: Do the work of changing a pcap
 *	Arguments: argc, argv - printcap info
 *	Returns:
 */

void ChangePcap(char **info, Bool one_item)
{
  int stat;
  char **oldinfo;

  oldinfo = CopyInfo(info);
  if (!AskPcapInfo(info))
    return;
  if ((stat = do_mr_query("delete_printcap_entry", 1, &info[PCAP_NAME],
			  NULL, NULL)))
    {
      com_err(program_name, stat, " printcap entry not deleted.");
      return;
    }
  if ((stat = do_mr_query("add_printcap_entry", CountArgs(info), info,
			  NULL, NULL)))
    {
      com_err(program_name, stat, " in ChngPcap");
      if ((stat = do_mr_query("add_printcap_entry", CountArgs(oldinfo) - 3,
			      oldinfo, NULL, NULL)))
	com_err(program_name, stat, " while attempting to put old info back");
    }
  FreeInfo(oldinfo);
  return;
}


/*	Function Name: ChngPcap
 *	Description:   Update the printcap information
 *	Arguments:     argc, argv - name of printer in argv[1].
 *	Returns:       DM_NORMAL.
 */

int ChngPcap(int argc, char **argv)
{
  struct mqelem *elem = GetPcapInfo(argv[1]);
  QueryLoop(elem, NullPrint, ChangePcap, "Change the printer");
  FreeQueue(elem);
  return DM_NORMAL;
}


/*	Function Name: SetPdDefaults
 *	Description: sets the default values for palladium additions.
 *	Arguments: info - an array of char pointers to recieve defaults.
 *	Returns: char ** (this array, now filled).
 */

static char **SetPdDefaults(char **info, char *name)
{
  info[PD_NAME] = strdup(name);
  info[PD_IDENT] = strdup("10000");
  info[PD_HOST] = strdup(DEFAULT_MACHINE);
  info[PD_MODTIME] = info[PD_MODBY] = info[PD_MODWITH] = NULL;

  info[PD_END] = NULL;
  return info;
}

/*	Function Name: AskPalladiumInfo.
 *	Description: This function askes the user for information about a
 *                   printer and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *	Returns: none.
 */

static char **AskPalladiumInfo(char **info)
{
  char temp_buf[BUFSIZ];

  Put_message("");
  sprintf(temp_buf, "Palladium Server/Supervisor entry for %s.",
	  info[PD_NAME]);
  Put_message(temp_buf);
  Put_message("");

  if (GetValueFromUser("RPC Program Number", &info[PD_IDENT]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Print Server/Supervisor Host", &info[PD_HOST]) ==
      SUB_ERROR)
    return NULL;
  info[PD_HOST] = canonicalize_hostname(info[PD_HOST]);

  FreeAndClear(&info[PD_MODTIME], TRUE);
  FreeAndClear(&info[PD_MODBY], TRUE);
  FreeAndClear(&info[PD_MODWITH], TRUE);

  return info;
}


/*	Function Name: PrintPalladiumInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this Printer.
 *	Returns: printer name
 */

static char *PrintPalladiumInfo(char **info)
{
  char buf[BUFSIZ];

  if (!info)		/* If no informaion */
    {
      Put_message("PrintPalladiumInfo called with null info!");
      return NULL;
    }

  sprintf(buf, "Name: %-24s Program #: %s  Host: %s",
	  info[PD_NAME], info[PD_IDENT], info[PD_HOST]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[PD_MODBY], info[PD_MODTIME],
	  info[PD_MODWITH]);
  Put_message(buf);
  return info[PD_NAME];
}


static struct mqelem *GetPalladiumInfo(char *name)
{
  int status;
  struct mqelem *elem = NULL;

  if ((status = do_mr_query("get_palladium", 1, &name, StoreInfo, &elem)))
    {
      com_err(program_name, status, " in GetPalladiumInfo");
      return NULL;
    }
  return QueueTop(elem);
}


void ChangePalladium(char **info, Bool one_item)
{
  int status;

  if (!AskPalladiumInfo(info))
    return;
  if ((status = do_mr_query("delete_palladium", 1, &info[PD_NAME],
			    NULL, NULL)))
    {
      com_err(program_name, status, " palladium entry not deleted.");
      return;
    }
  if ((status = do_mr_query("add_palladium", CountArgs(info), info,
			    NULL, NULL)))
    com_err(program_name, status, " in ChngPalladium");
  return;
}


/*	Function Name: RealDeletePalladium
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeletePalladium(char **info, Bool one_item)
{
  int stat;

  if ((stat = do_mr_query("delete_palladium", 1,
			  &info[PD_NAME], NULL, NULL)))
    com_err(program_name, stat, " palladium entry not deleted.");
  else
    Put_message("Palladium entry deleted.");
}


int GetPalladium(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPalladiumInfo(argv[1]);
  Loop(top, (void (*)(char **))PrintPalladiumInfo);
  FreeQueue(top);
  return DM_NORMAL;
}


int AddPalladium(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args;
  int status;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!(status = do_mr_query("get_palladium", 1, &argv[1], NULL, NULL)))
    {
      Put_message("A server or supervisor by that name already exists.");
      return DM_NORMAL;
    }
  else if (status != MR_NO_MATCH)
    {
      com_err(program_name, status, " in AddPalladium");
      return DM_NORMAL;
    }

  args = AskPalladiumInfo(SetPdDefaults(info, argv[1]));
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((status = do_mr_query("add_palladium", CountArgs(args), args,
			    NULL, NULL)))
    com_err(program_name, status, " in AddPalladium");

  FreeInfo(info);
  return DM_NORMAL;
}


int ChngPalladium(int argc, char **argv)
{
  struct mqelem *elem = GetPalladiumInfo(argv[1]);
  QueryLoop(elem, NullPrint, ChangePalladium, "Change the server/supervisor");
  FreeQueue(elem);
  return DM_NORMAL;
}


int DeletePalladium(int argc, char **argv)
{
  struct mqelem *elem = GetPalladiumInfo(argv[1]);
  QueryLoop(elem, PrintPalladiumInfo, RealDeletePalladium,
	    "Delete server/supervisor");
  FreeQueue(elem);
  return DM_NORMAL;
}

int ShowPalladiumAlias(int argc, char **argv)
{
  struct mqelem *elem = NULL;
  char *qargv[3], buf[BUFSIZ];
  int status;

  qargv[0] = argv[1];
  qargv[1] = "PALLADIUM";
  qargv[2] = argv[2];
  if ((status = do_mr_query("get_alias", 3, qargv, StoreInfo, &elem)))
    {
      com_err(program_name, status, " in ShowPalladiumAlias");
      return DM_NORMAL;
    }
  elem = QueueTop(elem);
  Put_message("");
  while (elem)
    {
      char **info = elem->q_data;
      sprintf(buf, "Printer: %-16s Server/Supervisor: %s", info[0], info[2]);
      Put_message(buf);
      elem = elem->q_forw;
    }

  FreeQueue(QueueTop(elem));
  return DM_NORMAL;
}

int AddPalladiumAlias(int argc, char **argv)
{
  int status;
  char *qargv[3];

  qargv[0] = argv[1];
  qargv[1] = "PALLADIUM";
  qargv[2] = argv[2];
  if ((status = do_mr_query("add_alias", 3, qargv, NULL, NULL)))
    com_err(program_name, status, " in AddPalladiumAlias");
  return DM_NORMAL;
}

int DeletePalladiumAlias(int argc, char **argv)
{
  int status;
  char *qargv[3];

  qargv[0] = argv[1];
  qargv[1] = "PALLADIUM";
  qargv[2] = argv[2];
  if ((status = do_mr_query("delete_alias", 3, qargv, NULL, NULL)))
    com_err(program_name, status, " in DeletePalladiumAlias");
  return DM_NORMAL;
}
