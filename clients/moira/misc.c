/* $Id: misc.c 3956 2010-01-05 20:56:56Z zacheiss $
 *
 *	This is the file misc.c for the Moira Client, which allows a naieve
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains:
 *		TableStats
 *		ShowClients
 *		ShowValue
 *
 *	Created: 	5 October 1988
 *	By:		Mark A. Rosenstein
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

#include <sys/types.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/clients/moira/misc.c $ $Id: misc.c 3956 2010-01-05 20:56:56Z zacheiss $");

void PrintStats(char **info);
void PrintClients(char **info);
void PrintValue(char **info);
void PrintAlias(char **info);

/*	Function Name: PrintStats
 *	Description: print statistics from argv
 *	Arguments: info: statistics tuple
 *	Returns: DM_NORMAL
 */

void PrintStats(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Table: %-30s Modified: %s", info[0], info[4]);
  Put_message(buf);
  sprintf(buf, "    %s appends, %s updates, %s deletes",
	  info[1], info[2], info[3]);
  Put_message(buf);
}


/*	Function Name: TableStats
 *	Description: display the Moira table statistics
 *	Arguments: NONE
 *	Returns: DM_NORMAL
 */

int TableStats(int argc, char **argv)
{
  int status;
  struct mqelem *elem = NULL;

  if ((status = do_mr_query("get_all_table_stats", 0, NULL, StoreInfo, &elem)))
    {
      com_err(program_name, status, " in TableStats");
      return DM_NORMAL;
    }
  Loop(QueueTop(elem), PrintStats);
  FreeQueue(elem);
  return DM_NORMAL;
}


/*	Function Name: PrintClients
 *	Description: print info from client tuple
 *	Arguments: argv
 */

void PrintClients(char **info)
{
  char buf[BUFSIZ];
  unsigned long host_address;
  struct hostent *host_entry;

  host_address = inet_addr(info[1]);
  if (host_address)
    {
      host_entry = gethostbyaddr((char *) &host_address, 4, AF_INET);
      if (host_entry)
	{
	  free(info[1]);
	  info[1] = strdup(host_entry->h_name);
	}
    }
  sprintf(buf, "Principal %s on %s (%s)", info[0], info[1], info[2]);
  Put_message(buf);
  sprintf(buf, "    Connected at %s, client %s", info[3], info[4]);
  Put_message(buf);
}


/*	Function Name: ShowClients
 *	Description: show clients actively using MR
 *	Arguments: NONE
 *	Returns: DM_NORMAL
 */

int ShowClients(int argc, char **argv)
{
  int status;
  struct mqelem *elem = NULL;

  if ((status = do_mr_query("_list_users", 0, NULL, StoreInfo, &elem)))
    {
      com_err(program_name, status, " in ShowClients");
      return DM_NORMAL;
    }
  Loop(QueueTop(elem), PrintClients);
  FreeQueue(elem);
  return DM_NORMAL;
}


/*	Function Name: PrintValue
 *	Description: displays variable values
 *	Arguments: argv
 */

void PrintValue(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Value: %s", info[0]);
  Put_message(buf);
}


/*	Function Name: ShowValue
 *	Description: get a variable value from MR
 *	Arguments: variable name
 *	Returns: DM_NORMAL
 */

int ShowValue(int argc, char **argv)
{
  int status;
  struct mqelem *elem = NULL;

  if ((status = do_mr_query("get_value", 1, &argv[1], StoreInfo, &elem)))
    {
      com_err(program_name, status, " in ShowValue");
      return DM_NORMAL;
    }
  Loop(elem, PrintValue);
  FreeQueue(elem);
  return DM_NORMAL;
}


/*	Function Name: PrintAlias
 *	Description: print an alias relation
 *	Arguments: argv
 */

void PrintAlias(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Name: %-20s Type: %-12s Value: %s",
	  info[0], info[1], info[2]);
  Put_message(buf);
}


/*	Function Name: ShowAlias
 *	Description: display an alias relation
 *	Arguments: name & type of alias
 *	Returns: DM_NORMAL
 */

int ShowAlias(int argc, char **argv)
{
  int status;
  char *info[4];
  struct mqelem *elem = NULL;

  info[0] = argv[1];
  info[1] = argv[2];
  info[2] = "*";
  if ((status = do_mr_query("get_alias", 3, info, StoreInfo, &elem)))
    {
      com_err(program_name, status, " in ShowAlias");
      return DM_NORMAL;
    }
  Loop(QueueTop(elem), PrintAlias);
  FreeQueue(elem);
  return DM_NORMAL;
}
