/* $Id: printer.c,v 1.25 1999-05-25 22:06:11 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/printer.c,v 1.25 1999-05-25 22:06:11 danw Exp $");

void RealDeletePrn(char **info, Bool one_item);
void ChangePrn(char **info, Bool one_item);
void ChangePrintSrvLoop(char **info, Bool one);
extern int GetAliasValue(int argc, char **argv, void *retval);
int StoreHWAddr(int argc, char **argv, void *retval);

static char *PrintPrintSrvInfo(char **info);
static char **SetPrintSrvDefaults(char **info, char *name);
static char **AskPrintSrvInfo(char **info);

#define BY_NAME 0
#define BY_ETHERNET 1
#define BY_HOSTNAME 2
#define BY_RM 3
#define BY_LOCATION 4
#define BY_CONTACT 5

#define DEFAULT_LOGHOST "WSLOGGER.MIT.EDU"

int StoreHWAddr(int argc, char **argv, void *retval)
{
  char **p = retval;
  *p = strdup(argv[0]);
  return MR_CONT;
}

/*	Function Name: SetDefaults
 *	Description: sets the default values for filesystem additions.
 *	Arguments: info - an array of char pointers to recieve defaults.
 *	Returns: char ** (this array, now filled).
 */

static char **SetDefaults(char **info, char *name)
{
  info[PRN_NAME] = strdup(name);
  info[PRN_TYPE] = strdup("PRIVATE");
  info[PRN_HWTYPE] = strdup("HP");
  info[PRN_DUPLEXNAME] = strdup("");
  info[PRN_HOSTNAME] = strdup(name);
  info[PRN_LOGHOST] = strdup(DEFAULT_LOGHOST);
  info[PRN_RM] = strdup("[ANY]");
  info[PRN_RP] = strdup(name);
  info[PRN_RQ] = strdup("[NONE]");
  info[PRN_KA] = strdup("1");
  info[PRN_PC] = strdup("10");
  info[PRN_AC] = strdup("[none]");
  info[PRN_LPC_ACL] = strdup("[none]");
  info[PRN_BANNER] = strdup("1");
  info[PRN_LOCATION] = strdup("");
  info[PRN_CONTACT] = strdup("");
  info[PRN_MODTIME] = info[PRN_MODBY] = info[PRN_MODWITH] = NULL;

  info[PRN_END] = NULL;
  return info;
}

/*	Function Name: GetPrnInfo
 *	Description: Stores the info in a queue.
 *	Arguments: name - name of the item to get information on.
 *	Returns: a pointer to the first element in the queue or null
 * 		if printer not found.
 */

static struct mqelem *GetPrnInfo(char *name, int how)
{
  int stat;
  struct mqelem *elem = NULL;

  switch (how)
    {
    case BY_NAME:
      stat = do_mr_query("get_printer", 1, &name, StoreInfo, &elem);
      if (stat == MR_NO_MATCH)
	{
	  stat = do_mr_query("get_printer_by_duplexname", 1, &name,
			     StoreInfo, &elem);
	}
      break;
    case BY_ETHERNET:
      stat = do_mr_query("get_printer_by_ethernet", 1, &name,
			 StoreInfo, &elem);
      break;
    case BY_HOSTNAME:
      name = canonicalize_hostname(strdup(name));
      stat = do_mr_query("get_printer_by_hostname", 1, &name,
			 StoreInfo, &elem);
      free(name);
      break;
    case BY_RM:
      name = canonicalize_hostname(strdup(name));
      stat = do_mr_query("get_printer_by_rm", 1, &name,
			 StoreInfo, &elem);
      free(name);
      break;
    case BY_LOCATION:
      stat = do_mr_query("get_printer_by_location", 1, &name,
			 StoreInfo, &elem);
      break;
    case BY_CONTACT:
      stat = do_mr_query("get_printer_by_contact", 1, &name,
			 StoreInfo, &elem);
      break;
    }

  if (stat)
    {
      com_err(program_name, stat, " in GetPrnInfo");
      return NULL;
    }
  return QueueTop(elem);
}

/*	Function Name: PrintPrnInfo
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this Printer.
 *	Returns: printer name
 */

static char *PrintPrnInfo(char **info)
{
  char buf[BUFSIZ], *hwaddr;
  int status, banner = atoi(info[PRN_BANNER]);

  if (!info)		/* If no informaion */
    {
      Put_message("PrintPrnInfo called with null info!");
      return NULL;
    }
  Put_message("");
  sprintf(buf, "Printer: %-18s Duplex queue: %-18s", info[PRN_NAME],
	  *info[PRN_DUPLEXNAME] ? info[PRN_DUPLEXNAME] : "[none]");
  Put_message(buf);
  sprintf(buf, "Type: %-10s Hardware type: %-10s Hardware address: ",
	  info[PRN_TYPE], info[PRN_HWTYPE]);
  status = do_mr_query("get_host_hwaddr", 1, &info[PRN_HOSTNAME],
		       StoreHWAddr, &hwaddr);
  if (status == MR_SUCCESS)
    {
      strcat(buf, hwaddr);
      free(hwaddr);
    }
  else
    strcat(buf, "none");
  Put_message(buf);
  sprintf(buf, "Printer hostname: %s", info[PRN_HOSTNAME]);
  Put_message(buf);
  sprintf(buf, "Printer log host: %s", info[PRN_LOGHOST]);
  Put_message(buf);
  sprintf(buf, "Spool host: %s", info[PRN_RM]);
  Put_message(buf);
  sprintf(buf, "Remote Printer Name: %-38s Banner page: %s", info[PRN_RP],
	  banner ? ( banner == PRN_BANNER_FIRST ? "Yes" : "Last" ) : "No");
  Put_message(buf);
  sprintf(buf, "Authentication: %-3s Price/page: %-3s  Quota Server: %s",
	  atoi(info[PRN_KA]) ? "yes" : "no", info[PRN_PC], info[PRN_RQ]);
  Put_message(buf);
  sprintf(buf, "Restrict list: %-23s  LPC ACL: %-23s",
	  info[PRN_AC], info[PRN_LPC_ACL]);
  Put_message(buf);
  sprintf(buf, "Location: %s", info[PRN_LOCATION]);
  Put_message(buf);
  sprintf(buf, "Contact: %s", info[PRN_CONTACT]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[PRN_MODBY], info[PRN_MODTIME],
	  info[PRN_MODWITH]);
  Put_message(buf);

  return info[PRN_NAME];
}

/*	Function Name: AskPrnInfo.
 *	Description: This function askes the user for information about a
 *                   printer and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                        info into.
 *	Returns: none.
 */

static char **AskPrnInfo(char **info)
{
  char temp_buf[BUFSIZ];
  char *args[3], *lpc_acl;
  char *s, *d;
  int status;

  Put_message("");
  sprintf(temp_buf, "Printer entry for %s.", info[PRN_NAME]);
  Put_message(temp_buf);
  Put_message("");

  if (GetTypeFromUser("Type of Printer", "printertype", &info[PRN_TYPE]) ==
      SUB_ERROR)
    return NULL;
  if (GetTypeFromUser("Hardware Type", "printerhwtype", &info[PRN_HWTYPE]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Duplex spool name", &info[PRN_DUPLEXNAME]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Printer hostname (or [none])", &info[PRN_HOSTNAME]) ==
      SUB_ERROR)
    return NULL;
  info[PRN_HOSTNAME] = canonicalize_hostname(info[PRN_HOSTNAME]);
  if (GetValueFromUser("Log host", &info[PRN_LOGHOST]) == SUB_ERROR)
    return NULL;
  info[PRN_LOGHOST] = canonicalize_hostname(info[PRN_LOGHOST]);
  if (GetValueFromUser("Spool host (or [any])", &info[PRN_RM]) == SUB_ERROR)
    return NULL;
  info[PRN_RM] = canonicalize_hostname(info[PRN_RM]);
  if (GetValueFromUser("Remote printer name", &info[PRN_RP]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Quota server", &info[PRN_RQ]) == SUB_ERROR)
    return NULL;
  info[PRN_RQ] = canonicalize_hostname(info[PRN_RQ]);
  if (GetYesNoValueFromUser("Kerberos authenticated", &info[PRN_KA]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Price per page", &info[PRN_PC]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Restrict list", &info[PRN_AC]) == SUB_ERROR)
    return NULL;
  args[0] = info[PRN_TYPE];
  args[1] = "LPC_ACL";
  args[2] = "*";
  status = do_mr_query("get_alias", 3, args, GetAliasValue, &lpc_acl);
  if (status == MR_SUCCESS)
    {
      free(info[PRN_LPC_ACL]);
      info[PRN_LPC_ACL] = lpc_acl;
    }
  if (GetValueFromUser("LPC ACL", &info[PRN_LPC_ACL]) == SUB_ERROR)
    return NULL;
  if (GetYesNoValueFromUser("Banner page", &info[PRN_BANNER]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Location", &info[PRN_LOCATION]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Contact", &info[PRN_CONTACT]) == SUB_ERROR)
    return NULL;

  FreeAndClear(&info[PRN_MODTIME], TRUE);
  FreeAndClear(&info[PRN_MODBY], TRUE);
  FreeAndClear(&info[PRN_MODWITH], TRUE);

  return info;
}

/* ---------------- Printer Menu ------------------ */

/*	Function Name: GetPrn
 *	Description: Get Printer information
 *	Arguments: argc, argv - name of printer in argv[1].
 *	Returns: DM_NORMAL.
 */

int GetPrn(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_NAME); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

int GetPrnByEthernet(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_ETHERNET); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

int GetPrnByHostname(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_HOSTNAME); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

int GetPrnByRM(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_RM); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

int GetPrnByLocation(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_LOCATION); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

int GetPrnByContact(int argc, char **argv)
{
  struct mqelem *top;

  top = GetPrnInfo(argv[1], BY_CONTACT); /* get info. */
  Loop(top, (void (*)(char **)) PrintPrnInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}


/*	Function Name: RealDeletePrn
 *	Description: Does the real deletion work.
 *	Arguments: info - array of char *'s containing all useful info.
 *                 one_item - a Boolean that is true if only one item
 *                              in queue that dumped us here.
 *	Returns: none.
 */

void RealDeletePrn(char **info, Bool one_item)
{
  int stat;

  if ((stat = do_mr_query("delete_printer", 1, &info[PRN_NAME], NULL, NULL)))
    com_err(program_name, stat, " printer not deleted.");
  else
    Put_message("Printer deleted.");
}

/*	Function Name: DeletePrn
 *	Description: Delete a printer given its name.
 *	Arguments: argc, argv - argv[1] is the name of the printer.
 *	Returns: none.
 */

int DeletePrn(int argc, char **argv)
{
  struct mqelem *elem = GetPrnInfo(argv[1], BY_NAME);
  QueryLoop(elem, PrintPrnInfo, RealDeletePrn, "Delete Printer");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: AddPrn
 *	Description: Add a printer
 *	Arguments: arc, argv - name of printer in argv[1].
 *	Returns: DM_NORMAL.
 */

int AddPrn(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args;
  int stat;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!(stat = do_mr_query("get_printer", 1, argv + 1, NULL, NULL)) ||
      !(stat = do_mr_query("get_printer_by_duplexname", 1, argv + 1,
			   NULL, NULL)))
    {
      Put_message ("A Printer by that name already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddPrn");
      return DM_NORMAL;
    }

  args = AskPrnInfo(SetDefaults(info, argv[1]));
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((stat = do_mr_query("add_printer", CountArgs(args), args, NULL, NULL)))
    com_err(program_name, stat, " in AddPrn");

  if (stat == MR_SUCCESS && strcasecmp(info[PRN_HOSTNAME], "[NONE]"))
    UpdateHWAddr(2, &info[PRN_HOSTNAME - 1]);

  FreeInfo(info);
  return DM_NORMAL;
}


/*	Function Name: ChangePrn
 *	Description: Do the work of changing a Prn
 *	Arguments: argc, argv - printcap info
 *	Returns:
 */

void ChangePrn(char **info, Bool one_item)
{
  int stat;
  char **oldinfo;

  oldinfo = CopyInfo(info);
  if (!AskPrnInfo(info))
    return;
  if ((stat = do_mr_query("delete_printer", 1, &info[PRN_NAME], NULL, NULL)))
    {
      com_err(program_name, stat, " printer not updated.");
      return;
    }
  if ((stat = do_mr_query("add_printer", CountArgs(info), info, NULL, NULL)))
    {
      com_err(program_name, stat, " in ChngPrn");
      if ((stat = do_mr_query("add_printer", CountArgs(oldinfo) - 3,
			      oldinfo, NULL, NULL)))
	com_err(program_name, stat, " while attempting to put old info back");
    }
  FreeInfo(oldinfo);
  return;
}


/*	Function Name: ChngPrn
 *	Description:   Update the printcap information
 *	Arguments:     argc, argv - name of printer in argv[1].
 *	Returns:       DM_NORMAL.
 */

int ChngPrn(int argc, char **argv)
{
  struct mqelem *elem = GetPrnInfo(argv[1], BY_NAME);
  QueryLoop(elem, NullPrint, ChangePrn, "Change the printer");
  FreeQueue(elem);
  return DM_NORMAL;
}


int UpdateHWAddr(int argc, char **argv)
{
  int stat;
  char *name, *hwaddr, *s, *d, *uargv[2];

  name = canonicalize_hostname(strdup(argv[1]));
  stat = do_mr_query("get_host_hwaddr", 1, &name, StoreHWAddr, &hwaddr);
  if (stat != MR_SUCCESS)
    {
      free(name);
      com_err(program_name, stat, " checking host ethernet address");
      return DM_NORMAL;
    }

  if (GetValueFromUser("Hardware ethernet address", &hwaddr) == SUB_ERROR)
    {
      free(name);
      return DM_NORMAL;
    }

  s = d = hwaddr;
  do
    {
      if (*s != ':')
	*d++ = *s;
    }
  while (*s++);

  uargv[0] = name;
  uargv[1] = hwaddr;
  if ((stat = do_mr_query("update_host_hwaddr", 2, uargv, NULL, NULL)))
    com_err(program_name, stat, " updating ethernet address.");

  free(name);
  free(hwaddr);
  return DM_NORMAL;
}


int GetPrintSrv(int argc, char **argv)
{
  int stat;
  struct mqelem *elem = NULL, *top;
  char *name;

  name = canonicalize_hostname(strdup(argv[1]));
  stat = do_mr_query("get_print_server", 1, &name, StoreInfo, &elem);
  if (stat)
    {
      com_err(program_name, stat, " in GetPrintSrv");
      return DM_NORMAL;
    }

  top = QueueTop(elem);
  Loop(top, (void *) PrintPrintSrvInfo);
  FreeQueue(top);		/* clean the queue. */
  return DM_NORMAL;
}

static char *PrintPrintSrvInfo(char **info)
{
  char buf[BUFSIZ];
  int status;

  if (!info)		/* If no informaion */
    {
      Put_message("PrintPrintSrvInfo called with null info!");
      return NULL;
    }
  Put_message("");
  sprintf(buf, "Hostname: %s", info[PRINTSERVER_HOST]);
  Put_message(buf);
  sprintf(buf, "Kind: %-10s Printer Types: %s", info[PRINTSERVER_KIND],
	  info[PRINTSERVER_TYPES]);
  Put_message(buf);
  if (!strcmp(info[PRINTSERVER_OWNER_TYPE], "NONE"))
    sprintf(buf, "Owner: %-25s", info[PRINTSERVER_OWNER_TYPE]);
  else
    {
      sprintf(buf, "Owner: %s %-*s", info[PRINTSERVER_OWNER_TYPE],
	      24 - strlen(info[PRINTSERVER_OWNER_TYPE]),
	      info[PRINTSERVER_OWNER_NAME]);
    }
  strcat(buf, "LPC ACL: ");
  strcat(buf, info[PRINTSERVER_LPC_ACL]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[PRINTSERVER_MODBY], info[PRINTSERVER_MODTIME],
	  info[PRINTSERVER_MODWITH]);
  Put_message(buf);

  return info[PRINTSERVER_HOST];
}

static char **SetPrintSrvDefaults(char **info, char *name)
{
  info[PRINTSERVER_HOST] = strdup(name);
  info[PRINTSERVER_KIND] = strdup("ATHENA");
  info[PRINTSERVER_TYPES] = strdup("PRIVATE");
  info[PRINTSERVER_OWNER_TYPE] = strdup("NONE");
  info[PRINTSERVER_OWNER_NAME] = strdup("");
  info[PRINTSERVER_LPC_ACL] = strdup("[none]");
  info[PRINTSERVER_MODTIME] = info[PRINTSERVER_MODBY] =
    info[PRINTSERVER_MODWITH] = NULL;

  info[PRINTSERVER_END] = NULL;
  return info;
}

static char **AskPrintSrvInfo(char **info)
{
  char buf[BUFSIZ];
  char *args[3], *lpc_acl;
  char *s, *d;
  int status;

  Put_message("");
  sprintf(buf, "Print Server entry for %s.", info[PRINTSERVER_HOST]);
  Put_message(buf);
  Put_message("");

  if (GetTypeFromUser("Kind of LPD", "lpd_kind", &info[PRINTSERVER_KIND]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Printer types", &info[PRINTSERVER_TYPES]) == SUB_ERROR)
    return NULL;
  if (GetTypeFromUser("Owner type", "ace_type", &info[PRINTSERVER_OWNER_TYPE])
      == SUB_ERROR)
    return NULL;
  if (strcmp(info[PRINTSERVER_OWNER_TYPE], "NONE") &&
      GetValueFromUser("Owner Name", &info[PRINTSERVER_OWNER_NAME]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("LPC ACL", &info[PRINTSERVER_LPC_ACL]) == SUB_ERROR)
    return NULL;

  FreeAndClear(&info[PRINTSERVER_MODTIME], TRUE);
  FreeAndClear(&info[PRINTSERVER_MODBY], TRUE);
  FreeAndClear(&info[PRINTSERVER_MODWITH], TRUE);

  return info;
}

int AddPrintSrv(int argc, char **argv)
{
  char *info[MAX_ARGS_SIZE], **args, *name;
  int stat;

  name = canonicalize_hostname(strdup(argv[1]));

  if (!(stat = do_mr_query("get_print_server", 1, &name, NULL, NULL)))
    {
      Put_message ("A print server record for that host already exists.");
      free(name);
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddPrintSrv");
      free(name);
      return DM_NORMAL;
    }

  args = AskPrintSrvInfo(SetPrintSrvDefaults(info, name));
  free(name);
  if (!args)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  if ((stat = do_mr_query("add_print_server", CountArgs(args),
			  args, NULL, NULL)))
    com_err(program_name, stat, " in AddPrintSrv");

  FreeInfo(info);
  return DM_NORMAL;
}

int ChangePrintSrv(int argc, char **argv)
{
  char *name, **args;
  struct mqelem *elem = NULL;
  int stat;

  name = canonicalize_hostname(strdup(argv[1]));
  if ((stat = do_mr_query("get_print_server", 1, &name, StoreInfo, &elem)))
    {
      free(name);
      com_err(program_name, stat, " in ChangePrintSrv");
      return DM_NORMAL;
    }
  free(name);

  QueryLoop(elem, NullPrint, ChangePrintSrvLoop, "Change the print server");
  FreeQueue(elem);
  return DM_NORMAL;
}

void ChangePrintSrvLoop(char **info, Bool one)
{
  int stat;

  if (!AskPrintSrvInfo(info))
    return;

  if ((stat = do_mr_query("update_print_server", CountArgs(info),
			  info, NULL, NULL)))
    com_err(program_name, stat, " in ChangePrintSrv");

  FreeInfo(info);
  return;
}

int DelPrintSrv(int argc, char **argv)
{
  int stat;
  char *name;

  name = canonicalize_hostname(strdup(argv[1]));

  if ((stat = do_mr_query("delete_print_server", 1, &name, NULL, NULL)))
    com_err(program_name, stat, " while deleting print server");
  free(name);
  return DM_NORMAL;
}
