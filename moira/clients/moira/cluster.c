#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.31 1998-01-05 19:51:55 danw Exp $";
#endif

/*	This is the file cluster.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains:
 *
 *	Created: 	4/22/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/cluster.c,v 1.31 1998-01-05 19:51:55 danw Exp $
 *
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution
 *	see the file mit-copyright.h
 */

/* BTW: for anyone who cares MCD is short for Machine, Cluster, Data. */

#include <stdio.h>
#include <string.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <ctype.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#define MACHINE  0
#define CLUSTER  1
#define DATA     2
#define MAP      3
#define SUBNET	 4
#define CNAME	 5

#define M_DEFAULT_TYPE     DEFAULT_NONE

#define C_DEFAULT_DESCRIPT DEFAULT_NONE
#define C_DEFAULT_LOCATION DEFAULT_NONE

#define CD_DEFAULT_LABEL   DEFAULT_NONE
#define CD_DEFAULT_DATA    DEFAULT_NONE

#define S_DEFAULT_LOW	"18.0.0.20"
#define S_DEFAULT_HIGH	"18.0.2.249"

static char *states[] = {
  "Reserved (0)",
  "Active (1)",
  "None (2)",
  "Deleted (3)"
};

static char *MacState(int state)
{
  static char buf[BUFSIZ];

  if (state < 0 || state > 3)
    {
      sprintf(buf, "Unknown (%d)", state);
      return buf;
    }
  return states[state];
}



/* -------------------- Set Defaults -------------------- */

/*	Function Name: SetMachineDefaults
 *	Description: sets machine defaults.
 *	Arguments: info - an array to put the defaults into.
 *                 name - Canonacalized name of the machine.
 *	Returns: info - the array.
 */

static char **SetMachineDefaults(char **info, char *name)
{
  info[M_NAME] = Strsave(name);
  info[M_VENDOR] = Strsave(M_DEFAULT_TYPE);
  info[M_MODEL] = Strsave(M_DEFAULT_TYPE);
  info[M_OS] = Strsave(M_DEFAULT_TYPE);
  info[M_LOC] = Strsave(M_DEFAULT_TYPE);
  info[M_CONTACT] = Strsave(M_DEFAULT_TYPE);
  info[M_USE] = Strsave("0");
  info[M_STAT] = Strsave("1");
  info[M_SUBNET] = Strsave("NONE");
  info[M_ADDR] = Strsave("unique");
  info[M_OWNER_TYPE] = Strsave("NONE");
  info[M_OWNER_NAME] = Strsave("NONE");
  info[M_ACOMMENT] = Strsave("");
  info[M_OCOMMENT] = Strsave("");
  info[15] = info[16] = NULL;
  return info;
}

/*	Function Name: SetClusterDefaults
 *	Description: sets Cluster defaults.
 *	Arguments: info - an array to put the defaults into.
 *                 name - name of the Cluster.
 *	Returns: info - the array.
 */

static char **SetClusterDefaults(char **info, char *name)
{
  info[C_NAME] = Strsave(name);
  info[C_DESCRIPT] = Strsave(C_DEFAULT_DESCRIPT);
  info[C_LOCATION] = Strsave(C_DEFAULT_LOCATION);
  info[C_MODBY] = info[C_MODTIME] = info[C_MODWITH] = info[C_END] = NULL;
  return info;
}

/*	Function Name: SetSubnetDefaults
 *	Description: sets Subnet defaults.
 *	Arguments: info - an array to put the defaults into.
 *                 name - name of the Subnet.
 *	Returns: info - the array.
 */

static char **SetSubnetDefaults(char **info, char *name)
{
  char buf[256];

  info[C_NAME] = Strsave(name);
  info[SN_DESC] = Strsave("");
  sprintf(buf, "%ld", ntohl(inet_addr("18.255.0.0")));
  info[SN_ADDRESS] = Strsave(buf);
  sprintf(buf, "%ld", ntohl(inet_addr("255.255.0.0")));
  info[SN_MASK] = Strsave(buf);
  sprintf(buf, "%ld", ntohl(inet_addr(S_DEFAULT_LOW)));
  info[SN_LOW] = Strsave(buf);
  sprintf(buf, "%ld", ntohl(inet_addr(S_DEFAULT_HIGH)));
  info[SN_HIGH] = Strsave(buf);
  info[SN_PREFIX] = Strsave("");
  info[SN_ACE_TYPE] = Strsave("LIST");
  info[SN_ACE_NAME] = Strsave("network");
  info[SN_MODBY] = info[SN_MODTIME] = info[SN_MODWITH] = info[SN_END] = NULL;
  return info;
}

/* -------------------- General Functions -------------------- */

static char aliasbuf[256];

static char *PrintAliases(char **info)
{
  if (strlen(aliasbuf) == 0)
    sprintf(aliasbuf, "Aliases:  %s", info[0]);
  else
    {
      strcat(aliasbuf, ", ");
      strcat(aliasbuf, info[0]);
    }
}


/*	Function Name: PrintMachInfo
 *	Description: This function Prints out the Machine info in
 *                   a coherent form.
 *	Arguments: info - array of information about a machine.
 *	Returns: The name of the Machine
 */

static char *PrintMachInfo(char **info)
{
  char buf[BUFSIZ], tbuf[256];
  char *args[3];
  struct qelem *elem = NULL;
  int stat;

  Put_message("");
  sprintf(buf, "Machine:  %s", info[M_NAME]);
  Put_message(buf);
  args[0] = "*";
  args[1] = info[M_NAME];
  if ((stat = do_mr_query("get_hostalias", 2, args, StoreInfo, (char *)&elem)))
    {
      if (stat != MR_NO_MATCH)
	com_err(program_name, stat, " looking up aliases");
    }
  else
    {
      aliasbuf[0] = 0;
      Loop(QueueTop(elem), (void *) PrintAliases);
      FreeQueue(elem);
      Put_message(aliasbuf);
    }
  sprintf(tbuf, "%s %s", info[M_OWNER_TYPE],
	  strcmp(info[M_OWNER_TYPE], "NONE") ? info[M_OWNER_NAME] : "");
  sprintf(buf, "Address:  %-16s    Network:    %-16s",
	  info[M_ADDR], info[M_SUBNET]);
  Put_message(buf);
  sprintf(buf, "Owner:    %-16s    Use data:   %s", tbuf, info[M_INUSE]);
  Put_message(buf);
  sprintf(buf, "Status:   %-16s    Changed:    %s",
	  MacState(atoi(info[M_STAT])), info[M_STAT_CHNG]);
  Put_message(buf);
  Put_message("");

  sprintf(buf, "Vendor:   %-16s    Model:      %-20s  OS:  %s",
	  info[M_VENDOR], info[M_MODEL], info[M_OS]);
  Put_message(buf);
  sprintf(buf, "Location: %-16s    Contact:    %-20s  Opt: %s",
	  info[M_LOC], info[M_CONTACT], info[M_USE]);
  Put_message(buf);
  sprintf(buf, "\nAdm cmt: %s", info[M_ACOMMENT]);
  Put_message(buf);
  sprintf(buf, "Op cmt:  %s", info[M_OCOMMENT]);
  Put_message(buf);
  Put_message("");
  sprintf(buf, "Created  by %s on %s", info[M_CREATOR], info[M_CREATED]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[M_MODBY], info[M_MODTIME], info[M_MODWITH]);
  Put_message(buf);
  return info[M_NAME];
}

/*	Function Name: PrintCname
 *	Description: Prints the Data on a host alias
 *	Arguments: info a pointer to the data array.
 *	Returns: The name of the alias.
 */

static char *PrintCname(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Alias: %-32s Canonical Name: %s", info[0], info[1]);
  Put_message(buf);
  return info[0];
}

/*	Function Name: PrintClusterInfo
 *	Description: This function Prints out the cluster info
 *                   in a coherent form.
 *	Arguments: info - array of information about a cluster.
 *	Returns: The name of the cluster.
 */

static char *PrintClusterInfo(char **info)
{
  char buf[BUFSIZ];

  Put_message("");
  sprintf(buf, "Cluster:     %s", info[C_NAME]);
  Put_message(buf);
  sprintf(buf, "Description: %s", info[C_DESCRIPT]);
  Put_message(buf);
  sprintf(buf, "Location:    %s", info[C_LOCATION]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[C_MODBY], info[C_MODTIME], info[C_MODWITH]);
  Put_message(buf);
  return info[C_NAME];
}

/*	Function Name: PrintClusterData
 *	Description: Prints the Data on a cluster
 *	Arguments: info a pointer to the data array.
 *	Returns: The name of the cluster.
 */

static char *PrintClusterData(char **info)
{
  char buf[BUFSIZ];

  Put_message("");
  sprintf(buf, "Cluster: %-20s Label: %-15s Data: %s",
	  info[CD_NAME], info[CD_LABEL], info[CD_DATA]);
  Put_message(buf);
  return info[CD_NAME];
}

/*	Function Name: PrintMCMap
 *	Description: Prints the data about a machine to cluster mapping.
 *	Arguments: info a pointer to the data array.
 *	Returns: none
 */

static char *PrintMCMap(char **info)
{
  char buf[BUFSIZ];
  sprintf(buf, "Cluster: %-30s Machine: %-20s",
	  info[MAP_CLUSTER], info[MAP_MACHINE]);
  Put_message(buf);
  return "";			/* Used by QueryLoop(). */
}

/*	Function Name: PrintSubnetInfo
 *	Description: This function Prints out the subnet info
 *                   in a coherent form.
 *	Arguments: info - array of information about a subnet.
 *	Returns: The name of the subnet.
 */

static char *PrintSubnetInfo(char **info)
{
  char buf[BUFSIZ];
  struct in_addr addr, mask, low, high;

  Put_message("");
  sprintf(buf, "        Network:  %s", info[SN_NAME]);
  Put_message(buf);
  sprintf(buf, "    Description:  %s", info[SN_DESC]);
  Put_message(buf);
  addr.s_addr = htonl(atoi(info[SN_ADDRESS]));
  mask.s_addr = htonl(atoi(info[SN_MASK]));
  low.s_addr = htonl(atoi(info[SN_LOW]));
  high.s_addr = htonl(atoi(info[SN_HIGH]));
  /* screwy sequence is here because inet_ntoa returns a pointer to
     a static buf.  If it were all one sprintf, the last value would
     appear 4 times. */
  sprintf(buf, "        Address:  %s        Mask:  ", inet_ntoa(addr));
  strcat(buf, inet_ntoa(mask));
  strcat(buf, "\n           High:  ");
  strcat(buf, inet_ntoa(high));
  strcat(buf, "       Low:  ");
  strcat(buf, inet_ntoa(low));
  Put_message(buf);
  sprintf(buf, "Hostname prefix:  %s", info[SN_PREFIX]);
  Put_message(buf);
  sprintf(buf, "          Owner:  %s %s\n", info[SN_ACE_TYPE],
	  strcmp(info[SN_ACE_TYPE], "NONE") ? info[SN_ACE_NAME] : "");
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[SN_MODBY], info[SN_MODTIME], info[SN_MODWITH]);
  Put_message(buf);
  return info[SN_NAME];
}

/*	Function Name: GetMCInfo.
 *	Description: This function stores info about a machine.
 *                   type - type of data we are trying to retrieve.
 *                   name1 - the name of argv[0] for the call.
 *                   name2 - the name of argv[1] for the call.
 *	Returns: the top element of a queue containing the data or NULL.
 */

struct qelem *GetMCInfo(int type, char *name1, char *name2)
{
  int stat;
  struct qelem *elem = NULL;
  char *args[5];

  switch (type)
    {
    case MACHINE:
      args[0] = name1;
      args[1] = args[2] = args[3] = "*";
      if ((stat = do_mr_query("get_host", 4, args,
			      StoreInfo, (char *)&elem)))
	{
	  if (stat == MR_NO_MATCH)
	    {
	      char buf[128];
	      sprintf(buf, "Machine '%s' is not in the database.", name1);
	      Put_message(buf);
	    }
	  else
	    com_err(program_name, stat, " in get_machine.");
	  return NULL;
	}
      break;
    case CNAME:
      args[0] = name1;
      args[1] = name2;
      if ((stat = do_mr_query("get_hostalias", 2, args,
			      StoreInfo, (char *)&elem)))
	{
	  com_err(program_name, stat, " in get_hostalias.");
	  return NULL;
	}
      break;
    case SUBNET:
      if ((stat = do_mr_query("get_subnet", 1, &name1,
			      StoreInfo, (char *)&elem)))
	{
	  if (stat == MR_NO_MATCH)
	    {
	      char buf[128];
	      sprintf(buf, "Network '%s' is not in the database.", name1);
	      Put_message(buf);
	    }
	  else
	    com_err(program_name, stat, " in get_subnet.");
	  return NULL;
	}
      break;
    case CLUSTER:
      if ((stat = do_mr_query("get_cluster", 1, &name1,
			      StoreInfo, (char *)&elem)))
	{
	  com_err(program_name, stat, " in get_cluster.");
	  return NULL;
	}
      break;
    case MAP:
      args[MAP_MACHINE] = name1;
      args[MAP_CLUSTER] = name2;
      if ((stat = do_mr_query("get_machine_to_cluster_map", 2, args,
			      StoreInfo, (char *)&elem)))
	{
	  com_err(program_name, stat, " in get_machine_to_cluster_map.");
	  return NULL;
	}
      break;
    case DATA:
      args[CD_NAME] = name1;
      args[CD_LABEL] = name2;
      if ((stat = do_mr_query("get_cluster_data", 2, args,
			      StoreInfo, (char *)&elem)))
	{
	  com_err(program_name, stat, " in get_cluster_data.");
	  return NULL;
	}
    }
  return QueueTop(elem);
}

/*	Function Name: AskMCDInfo.
 *	Description: This function askes the user for information about a
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the information to ask about
 *                 type - type of information - MACHINE
 *                                              CLUSTER
 *                                              DATA
 *                 name - T/F : change the name of this type.
 *	Returns: none.
 */

char **AskMCDInfo(char **info, int type, Bool name)
{
  char temp_buf[BUFSIZ], *newname, *oldnewname;

  switch (type)
    {
    case MACHINE:
      sprintf(temp_buf, "\nSetting the information for the Machine %s...",
	      info[M_NAME]);
      break;
    case SUBNET:
      sprintf(temp_buf, "Setting the information for the Network %s...",
	      info[SN_NAME]);
      break;
    case CLUSTER:
      sprintf(temp_buf, "Setting the information for the Cluster %s...",
	      info[C_NAME]);
      break;
    case DATA:
      sprintf(temp_buf, "Setting the Data for the Cluster %s...",
	      info[CD_NAME]);
      break;
    }
  Put_message(temp_buf);

  if (name)
    {
      switch (type)
	{
	case MACHINE:
	  newname = Strsave(info[M_NAME]);
	  if (GetValueFromUser("The new name for this machine? ", &newname) ==
	      SUB_ERROR)
	    return NULL;
	  oldnewname = Strsave(newname);
	  newname = canonicalize_hostname(newname);
	  if (strcasecmp(newname, oldnewname) && *oldnewname != '"')
	    {
	      sprintf(temp_buf, "Warning: '%s' canonicalized to '%s'\n",
		      oldnewname, newname);
	      Put_message(temp_buf);
	    }
	  free(oldnewname);
	  break;
	case SUBNET:
	  newname = Strsave(info[SN_NAME]);
	  if (GetValueFromUser("The new name for this network? ", &newname) ==
	      SUB_ERROR)
	    return NULL;
	  break;
	case CLUSTER:
	  newname = Strsave(info[C_NAME]);
	  if (GetValueFromUser("The new name for this cluster? ", &newname) ==
	      SUB_ERROR)
	    return NULL;
	  break;
	default:
	  Put_message("Unknown type in AskMCDInfo, programmer botch");
	  return NULL;
	}
    }

  switch (type)
    {
    case MACHINE:
      if (GetValueFromUser("Machine's vendor", &info[M_VENDOR]) == SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Machine's model", &info[M_MODEL]) == SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Machine's operating system", &info[M_OS]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Machine's location", &info[M_LOC]) == SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Machine's contact", &info[M_CONTACT]) ==
	  SUB_ERROR)
	return NULL;
      while (1)
	{
	  int i;
	  if (GetValueFromUser("Machine's status (? for help)",
				 &info[M_STAT]) == SUB_ERROR)
	    return NULL;
	  if (isdigit(info[M_STAT][0]))
	    break;
	  Put_message("Valid status numbers:");
	  for (i = 0; i < 4; i++)
	    Put_message(states[i]);
	}

      /* there appears to be some argument mismatch between the client
       * and the server.. so here is this argument shuffler.
       * I have since modified this to always shuffle the arguments..
       * not just do so when performing a modify all fields request.
       * The SetMachinedefaults() has been changed to reflect this.
       * pray for us and may we attain enlightenment through structures.
       */

      if (name)
	{
	  /* info did not come from SetMachineDefaults(), which does not
	   * initialize entry 8 (M_STAT_CHNG), therefore we can
	   * free it.
	   */
	  /* This is an update of an existing machine and the structure
	   * was filled in thru a query to the db which does fill in this
	   * field.
	   */
	  free(info[8]);
	}

      info[8] = info[M_SUBNET];
      info[9] = info[M_ADDR];
      info[10] = info[M_OWNER_TYPE];
      info[11] = info[M_OWNER_NAME];
      info[12] = info[M_ACOMMENT];
      info[13] = info[M_OCOMMENT];

      if (name)
	{
	  if (GetValueFromUser("Machine's network (or 'none')", &info[8])
	      == SUB_ERROR)
	    return NULL;
	}
      if (GetValueFromUser("Machine's address (or 'unassigned' or 'unique')",
			   &info[9]) == SUB_ERROR)
	return NULL;
      if (GetTypeFromUser("Machine's owner type", "ace_type", &info[10]) ==
	  SUB_ERROR)
	return NULL;
      if (strcmp(info[10], "NONE") &&
	  GetValueFromUser("Owner's Name", &info[11]) == SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Administrative comment", &info[12]) == SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Operational comment", &info[13]) == SUB_ERROR)
	return NULL;
      info[14] = NULL;
      FreeAndClear(&info[15], TRUE);
      FreeAndClear(&info[16], TRUE);
      break;
    case SUBNET:
      if (GetValueFromUser("Network description", &info[SN_DESC]) == SUB_ERROR)
	return NULL;
      if (GetAddressFromUser("Network address", &info[SN_ADDRESS]) == SUB_ERROR)
	return NULL;
      if (GetAddressFromUser("Network mask", &info[SN_MASK]) == SUB_ERROR)
	return NULL;
      if (atoi(info[SN_LOW]) == ntohl(inet_addr(S_DEFAULT_LOW)))
	{
	  struct in_addr low;
	  unsigned long mask, addr;

	  addr = atoi(info[SN_ADDRESS]);
	  mask = atoi(info[SN_MASK]);
	  low.s_addr = atoi(info[SN_LOW]);
	  low.s_addr = (low.s_addr & ~mask) | (addr & mask);
	  free(info[SN_LOW]);
	  sprintf(temp_buf, "%d", low.s_addr);
	  info[SN_LOW] = strsave(temp_buf);
	}
      if (GetAddressFromUser("Lowest assignable address", &info[SN_LOW]) ==
	  SUB_ERROR)
	return NULL;
      if (atoi(info[SN_HIGH]) == ntohl(inet_addr(S_DEFAULT_HIGH)))
	{
	  struct in_addr high;
	  unsigned long mask, addr;

	  addr = atoi(info[SN_ADDRESS]);
	  mask = atoi(info[SN_MASK]);
	  high.s_addr = atoi(info[SN_HIGH]);
	  high.s_addr = (high.s_addr & ~mask) | (addr & mask);
	  free(info[SN_HIGH]);
	  sprintf(temp_buf, "%d", high.s_addr);
	  info[SN_HIGH] = strsave(temp_buf);
	}
      if (GetAddressFromUser("Highest assignable address", &info[SN_HIGH]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Hostname prefix", &info[SN_PREFIX]) == SUB_ERROR)
	return NULL;
      if (GetTypeFromUser("Owner type", "ace_type", &info[SN_ACE_TYPE]) ==
	  SUB_ERROR)
	return NULL;
      if (strcmp(info[SN_ACE_TYPE], "NONE") &&
	  GetValueFromUser("Owner name", &info[SN_ACE_NAME]) == SUB_ERROR)
	return NULL;
      FreeAndClear(&info[SN_MODTIME], TRUE);
      FreeAndClear(&info[SN_MODBY], TRUE);
      FreeAndClear(&info[SN_MODWITH], TRUE);
      break;
    case CLUSTER:
      if (GetValueFromUser("Cluster's Description:", &info[C_DESCRIPT]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("Cluster's Location:", &info[C_LOCATION]) ==
	  SUB_ERROR)
	return NULL;
      FreeAndClear(&info[C_MODTIME], TRUE);
      FreeAndClear(&info[C_MODBY], TRUE);
      FreeAndClear(&info[C_MODWITH], TRUE);
      break;
    case DATA:
      if (GetValueFromUser("Label defining this data?", &info[CD_LABEL]) ==
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("The data itself ? ", &info[CD_DATA]) == SUB_ERROR)
	return NULL;
      break;
    }

  /*
   * Slide the newname into the #2 slot, this screws up all future references
   * to this list.
   */
  if (name)
    SlipInNewName(info, newname);

  return info;
}

/* -----------  Machine Menu ----------- */

/*	Function Name: ShowMachineInfo
 *	Description: This function shows the information about a machine.
 *	Arguments: argc, argv - the name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

int ShowMachineInfo(int argc, char **argv)
{
  struct qelem *top;
  char *tmpname;

  tmpname = canonicalize_hostname(strsave(argv[1]));
  top = GetMCInfo(MACHINE, tmpname, NULL);
  Loop(top, ((void *) PrintMachInfo));
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: ShowMachineQuery
 *	Description: This function shows the information about a machine.
 *		or group of machines, which may be selected through a
 *		number of criteria.
 *	Arguments: argc, argv - the name of the machine in argv[1],
 *		the address of the machine in argv[2],
 *		the location of the machine in argv[3],
 *		and the contact name in argv[4].
 *	     any of these may be wildcards.
 *	Returns: DM_NORMAL.
 */

int ShowMachineQuery(int argc, char **argv)
{
  int stat;
  struct qelem *top, *elem = NULL;
  char *args[5];

  if (!strcmp(argv[1], "") && !strcmp(argv[2], "") &&
      !strcmp(argv[3], "") && !strcmp(argv[4], ""))
    {
      Put_message("You must specify at least one parameter of the query.");
      return DM_NORMAL;
    }

  if (*argv[1])
    args[0] = canonicalize_hostname(strsave(argv[1]));
  else
    args[0] = "*";
  if (*argv[2])
    args[1] = argv[2];
  else
    args[1] = "*";
  if (*argv[3])
    args[2] = argv[3];
  else
    args[2] = "*";
  if (*argv[4])
    args[3] = argv[4];
  else
    args[3] = "*";

  if ((stat = do_mr_query("get_host", 4, args, StoreInfo,
			  (char *)&elem)))
    {
      if (stat == MR_NO_MATCH)
	Put_message("No machine(s) found matching query in the database.");
      else
	com_err(program_name, stat, " in get_machine.");
      return DM_NORMAL;
    }
  top = QueueTop(elem);
  Loop(top, ((void *) PrintMachInfo));
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: AddMachine
 *	Description: This function adds a new machine to the database.
 *	Arguments: argc, argv - the name of the network in argv[1].
 *	Returns: DM_NORMAL.
 */

int AddMachine(int argc, char **argv)
{
  char **args, *info[MAX_ARGS_SIZE], *name, buf[256], *xargs[5];
  char **rinfo;
  struct qelem *elem = NULL;
  int stat;

  if (!ValidName(argv[1]))	/* Checks for wildcards. */
    return DM_NORMAL;

  /*
   * get the network record
   */

  if (strcasecmp(argv[1], "none") &&
      (stat = do_mr_query("get_subnet", 1, &argv[1],
			  StoreInfo, (char *)&elem)))
    {
      if (stat == MR_NO_MATCH)
	{
	  char buf[128];
	  sprintf(buf, "Network '%s' is not in the database.", argv[1]);
	  Put_message(buf);
	} else
	  com_err(program_name, stat, " in get_subnet.");
      return DM_NORMAL;
    }

  /*
   * Check to see if this machine already exists.
   */

  name = strsave(""); /* want to put prefix here */
  if (GetValueFromUser("Machine name", &name) == SUB_ERROR)
    return 0;

  name = canonicalize_hostname(strsave(name));

  xargs[0] = name;
  xargs[1] = xargs[2] = xargs[3] = "*";
  if (!(stat = do_mr_query("get_host", 4, xargs, NullFunc, NULL)))
    {
      sprintf(buf, "The machine '%s' already exists.", name);
      Put_message(buf);
      free(name);
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat,
	      " while checking machine '%s' in AddMachine.", name);
      free(name);
      return DM_NORMAL;
    }
  rinfo = SetMachineDefaults(info, name);
  rinfo[M_SUBNET] = strsave(argv[1]);
  if (!(args = AskMCDInfo(rinfo, MACHINE, FALSE)))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  /*
   * Actually create the new Machine.
   */

  if ((stat = do_mr_query("add_host", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in AddMachine.");

  FreeInfo(info);
  free(name);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateMachine
 *	Description: Performs the actual update of the machine data.
 *	Arguments: info - the information on the machine to update.
 *                 junk - an UNUSED Boolean.
 *	Returns: none.
 */

static void RealUpdateMachine(char **info, Bool junk)
{
  register int stat;
  char **args = AskMCDInfo(info, MACHINE, TRUE);
  if (!args)
    {
      Put_message("Aborted.");
      return;
    }
  if ((stat = do_mr_query("update_host", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in UpdateMachine.");
  else
    Put_message("Machine successfully updated.");
}

/*	Function Name: UpdateMachine
 *	Description: This function adds a new machine to the database.
 *	Arguments: argc, argv - the name of the machine in argv[1].
 *	Returns: DM_NORMAL.
 */

int UpdateMachine(int argc, char **argv)
{
  struct qelem *top;
  char *tmpname;

  tmpname = canonicalize_hostname(strsave(argv[1]));
  top = GetMCInfo(MACHINE, tmpname, NULL);
  QueryLoop(top, NullPrint, RealUpdateMachine, "Update the machine");

  FreeQueue(top);
  free(tmpname);
  return DM_NORMAL;
}

/*	Function Name: CheckAndRemoveFromCluster
 *	Description: This func tests to see if a machine is in a cluster.
 *                   and if so then removes it
 *	Arguments: name - name of the machine (already Canonocalized).
 *                 ask_user- query the user before removing if from clusters?
 *	Returns: MR_ERROR if machine left in a cluster, or mr_error.
 */

int CheckAndRemoveFromCluster(char *name, Bool ask_user)
{
  register int stat, ret_value;
  Bool delete_it;
  char *args[10], temp_buf[BUFSIZ], *ptr;
  struct qelem *top, *elem = NULL;

  ret_value = SUB_NORMAL;	/* initialize ret_value. */
  args[0] = name;
  args[1] = "*";
  stat = do_mr_query("get_machine_to_cluster_map", 2, args,
		     StoreInfo, (char *)&elem);
  if (stat && stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in get_machine_to_cluster_map.");
      return DM_NORMAL;
    }
  if (stat == MR_SUCCESS)
    {
      elem = top = QueueTop(elem);
      if (ask_user)
	{
	  sprintf(temp_buf, "%s is assigned to the following clusters.", name);
	  Put_message(temp_buf);
	  Loop(top, (void *) PrintMCMap);
	  ptr = "Remove this machine from ** ALL ** these clusters?";
	  if (YesNoQuestion(ptr, FALSE) == TRUE) /* may return -1. */
	    delete_it = TRUE;
	  else
	    {
	      Put_message("Aborting...");
	      FreeQueue(top);
	      return SUB_ERROR;
	    }
	}
      else
	delete_it = TRUE;

      if (delete_it)
	{
	  while (elem)
	    {
	      char **info = (char **) elem->q_data;
	      if ((stat = do_mr_query("delete_machine_from_cluster",
				       2, info, Scream, NULL)))
		{
		  ret_value = SUB_ERROR;
		  com_err(program_name, stat,
			  " in delete_machine_from_cluster.");
		  sprintf(temp_buf,
			  "Machine %s ** NOT ** removed from cluster %s.",
			  info[MAP_MACHINE], info[MAP_CLUSTER]);
		  Put_message(temp_buf);
		}
	      elem = elem->q_forw;
	    }
	}
    }
  return ret_value;
}

/*	Function Name: RealDeleteMachine
 *	Description: Actually Deletes the Machine.
 *	Arguments: info - nescessary information stored as an array of char *'s
 *                 one_machine - a boolean, true if there is only one item in
 *                               the query.
 *	Returns: none.
 */

static void RealDeleteMachine(char **info, Bool one_machine)
{
  register int stat;
  char temp_buf[BUFSIZ];

  sprintf(temp_buf, "Are you sure you want to delete the machine %s (y/n)? ",
	  info[M_NAME]);
  if (!one_machine || Confirm(temp_buf))
    {
      if (CheckAndRemoveFromCluster(info[M_NAME], TRUE) != SUB_ERROR)
	{
	  if ((stat = do_mr_query("delete_host", 1,
				  &info[M_NAME], Scream, NULL)))
	    {
	      com_err(program_name, stat, " in DeleteMachine.");
	      sprintf(temp_buf, "%s ** NOT ** deleted.",
		      info[M_NAME]);
	      Put_message(temp_buf);
	    }
	  else
	    {
	      sprintf(temp_buf, "%s successfully Deleted.", info[M_NAME]);
	      Put_message(temp_buf);
	    }
	}
    }
}

/*	Function Name: DeleteMachine
 *	Description: This function removes a machine from the data base.
 *	Arguments: argc, argv - the machines name int argv[1].
 *	Returns: DM_NORMAL.
 */

/* Perhaps we should remove the cluster if it has no machine now. */

int DeleteMachine(int argc, char **argv)
{
  struct qelem *top;
  char *tmpname;

  tmpname = canonicalize_hostname(strsave(argv[1]));
  top = GetMCInfo(MACHINE, tmpname, (char *) NULL);
  QueryLoop(top, PrintMachInfo, RealDeleteMachine, "Delete the machine");
  FreeQueue(top);
  free(tmpname);
  return DM_NORMAL;
}


char *partial_canonicalize_hostname(char *s)
{
  char buf[256], *cp;
  static char *def_domain = NULL;
  struct hostent *hp;
  struct utsname name;

  if (!def_domain)
    {
      uname(&name);
      hp = gethostbyname(name.nodename);
      cp = strchr(hp->h_name, '.');
      if (cp)
	def_domain = strsave(++cp);
      else
	def_domain = "";
    }

  if (strchr(s, '.') || strchr(s, '*'))
    return s;
  sprintf(buf, "%s.%s", s, def_domain);
  free(s);
  return strsave(buf);
}


/*	Function Name: ShowCname
 *	Description: This function shows machine aliases
 *	Arguments: argc, argv - the alias argv[1], the real name in argv[2]
 *	Returns: DM_NORMAL.
 */

int ShowCname(int argc, char **argv)
{
  struct qelem *top;
  char *tmpalias, *tmpname;

  tmpalias = partial_canonicalize_hostname(strsave(argv[1]));
  tmpname = canonicalize_hostname(strsave(argv[2]));
  top = GetMCInfo(CNAME, tmpalias, tmpname);
  Put_message("");		/* blank line on screen */
  Loop(top, ((void *) PrintCname));
  FreeQueue(top);
  return DM_NORMAL;
}


int AddCname(int argc, char **argv)
{
  int stat;
  char *args[10];

  args[0] = partial_canonicalize_hostname(strsave(argv[1]));
  args[1] = canonicalize_hostname(strsave(argv[2]));
  stat = do_mr_query("add_hostalias", 2, args, Scream, NULL);
  switch (stat)
    {
    case MR_SUCCESS:
      break;
    case MR_EXISTS:
      Put_message("That alias name is already in use.");
      break;
    case MR_PERM:
      Put_message("Permission denied.  "
		  "(Regular users can only add two aliases to a host.");
      break;
    default:
      com_err(program_name, stat, " in add_hostalias");
    }
  return DM_NORMAL;
}


int DeleteCname(int argc, char **argv)
{
  int stat;
  char *machine, *cluster, temp_buf[BUFSIZ], *args[10];
  Bool add_it, one_machine, one_cluster;
  struct qelem *melem, *mtop, *celem, *ctop;

  args[0] = partial_canonicalize_hostname(strsave(argv[1]));
  args[1] = canonicalize_hostname(strsave(argv[2]));
  stat = do_mr_query("delete_hostalias", 2, args, Scream, NULL);
  if (stat)
    com_err(program_name, stat, " in delete_hostalias");
  return DM_NORMAL;
}


/*	Function Name: AddMachineToCluster
 *	Description: This function adds a machine to a cluster
 *	Arguments: argc, argv - The machine name is argv[1].
 *                              The cluster name in argv[2].
 *	Returns: DM_NORMAL.
 */

int AddMachineToCluster(int argc, char **argv)
{
  int stat;
  char *machine, *cluster, temp_buf[BUFSIZ], *args[10];
  Bool add_it, one_machine, one_cluster;
  struct qelem *melem, *mtop, *celem, *ctop;

  machine = canonicalize_hostname(strsave(argv[1]));
  if (strcasecmp(machine, argv[1]) && *argv[1] != '"')
    {
      sprintf(temp_buf, "Warning: '%s' canonicalized to '%s'.",
	      argv[1], machine);
      Put_message(temp_buf);
    }
  cluster = argv[2];

  celem = ctop = GetMCInfo(CLUSTER, cluster, NULL);
  melem = mtop = GetMCInfo(MACHINE, machine, NULL);
  free(machine);

  one_machine = (QueueCount(mtop) == 1);
  one_cluster = (QueueCount(ctop) == 1);

  /* No good way to use QueryLoop() here, sigh */

  while (melem)
    {
      char **minfo = (char **) melem->q_data;
      while (celem)
	{
	  char **cinfo = (char **) celem->q_data;
	  if (one_machine && one_cluster)
	    add_it = TRUE;
	  else
	    {
	      sprintf(temp_buf, "Add machine %s to cluster %s (y/n/q) ?",
		      minfo[M_NAME], cinfo[C_NAME]);
	      switch (YesNoQuitQuestion(temp_buf, FALSE))
		{
		case TRUE:
		  add_it = TRUE;
		  break;
		case FALSE:
		  add_it = FALSE;
		  break;
		default:
		  Put_message("Aborting...");
		  FreeQueue(ctop);
		  FreeQueue(mtop);
		  return DM_NORMAL;
		}
	    }
	  if (add_it)
	    {
	      args[0] = minfo[M_NAME];
	      args[1] = cinfo[C_NAME];
	      stat = do_mr_query("add_machine_to_cluster", 2, args,
				 Scream, NULL);
	      switch (stat)
		{
		case MR_SUCCESS:
		  break;
		case MR_EXISTS:
		  sprintf(temp_buf, "%s is already in cluster %s",
			  minfo[M_NAME], cinfo[C_NAME]);
		  Put_message(temp_buf);
		  break;
		default:
		  com_err(program_name, stat, " in AddMachineToCluster.");
		  break;
		}
	    }
	  celem = celem->q_forw;
	}
      celem = ctop;		/* reset cluster element. */
      melem = melem->q_forw;
    }
  FreeQueue(ctop);
  FreeQueue(mtop);
  return DM_NORMAL;
}

/*	Function Name: RealRemoveMachineFromCluster
 *	Description: This function actually removes the machine from its
 *                   cluster.
 *	Arguments: info - all information nescessary to perform the removal.
 *                 one_map - True if there is only one case, and we should
 *                           confirm.
 *	Returns: none.
 */

static void RealRemoveMachineFromCluster(char **info, Bool one_map)
{
  char temp_buf[BUFSIZ];
  register int stat;

  sprintf(temp_buf, "Remove %s from the cluster %s",
	  info[MAP_MACHINE], info[MAP_CLUSTER]);
  if (!one_map || Confirm(temp_buf))
    {
      if ((stat = do_mr_query("delete_machine_from_cluster", 2,
			      info, Scream, NULL)))
	com_err(program_name, stat, " in delete_machine_from_cluster");
      else
	{
	  sprintf(temp_buf, "%s has been removed from the cluster %s.",
		  info[MAP_MACHINE], info[MAP_CLUSTER]);
	  Put_message(temp_buf);
	}
    }
  else
    Put_message("Machine not removed.");
}

/*	Function Name: RemoveMachineFromCluster
 *	Description: Removes this machine form a specific cluster.
 *	Arguments: argc, argv - Name of machine in argv[1].
 *                              Name of cluster in argv[2].
 *	Returns: none.
 */

int RemoveMachineFromCluster(int argc, char **argv)
{
  struct qelem *elem = NULL;
  char buf[BUFSIZ], * args[10];
  register int stat;

  args[MAP_MACHINE] = canonicalize_hostname(strsave(argv[1]));
  if (strcasecmp(args[MAP_MACHINE], argv[1]) && *argv[1] != '"')
    {
      sprintf(buf, "Warning: '%s' canonicalized to '%s'.",
	      argv[1], args[MAP_MACHINE]);
      Put_message(buf);
    }
  args[MAP_CLUSTER] = argv[2];
  args[MAP_END] = NULL;

  stat = do_mr_query("get_machine_to_cluster_map", CountArgs(args), args,
		     StoreInfo, (char *)&elem);
  if (stat == MR_NO_MATCH)
    {
      sprintf(buf, "The machine %s is not is the cluster %s.",
	      args[MAP_MACHINE], args[MAP_CLUSTER]);
      Put_message(buf);
      free(args[MAP_MACHINE]);
      return DM_NORMAL;
    }
  if (stat != MR_SUCCESS)
    com_err(program_name, stat, " in delete_machine_from_cluster");

  elem = QueueTop(elem);
  QueryLoop(elem, PrintMCMap, RealRemoveMachineFromCluster,
	    "Remove this machine from this cluster");

  FreeQueue(elem);
  free(args[MAP_MACHINE]);
  return DM_NORMAL;
}

/* ---------- Subnet Menu -------- */

/*	Function Name: ShowSubnetInfo
 *	Description: Gets information about a subnet given its name.
 *	Arguments: argc, argc - the name of the subnet in in argv[1].
 *	Returns: DM_NORMAL.
 */

int ShowSubnetInfo(int argc, char **argv)
{
  struct qelem *top;

  top = GetMCInfo(SUBNET, argv[1], (char *) NULL);
  Loop(top, (void *) PrintSubnetInfo);
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: AddSubnet
 *	Description: Creates a new subnet.
 *	Arguments: argc, argv - the name of the new subnet is argv[1].
 *	Returns: DM_NORMAL.
 */

int AddSubnet(int argc, char **argv)
{
  char **args, *info[MAX_ARGS_SIZE], *name = argv[1];
  int stat;

  /*
   * Check to see if this subnet already exists.
   */
  if (!ValidName(name))
    return DM_NORMAL;

  if ((stat = do_mr_query("get_subnet", 1, &name,
			  NullFunc, NULL)) == MR_SUCCESS)
    {
      Put_message("This subnet already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddSubnet.");
      return DM_NORMAL;
    }
  if (!(args = AskMCDInfo(SetSubnetDefaults(info, name), SUBNET, FALSE)))
    {
      Put_message("Aborted.");
      FreeInfo(info);
      return DM_NORMAL;
    }

  /*
   * Actually create the new Subnet.
   */
  if ((stat = do_mr_query("add_subnet", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in AddSubnet.");

  FreeInfo(info);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateSubnet
 *	Description: This function actually performs the subnet update.
 *	Arguments: info - all information nesc. for updating the subnet.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

static void RealUpdateSubnet(char **info, Bool junk)
{
  register int stat;
  char **args = AskMCDInfo(info, SUBNET, TRUE);
  if (!args)
    {
      Put_message("Aborted.");
      return;
    }
  if ((stat = do_mr_query("update_subnet", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in UpdateSubnet.");
  else
    Put_message("Subnet successfully updated.");
}

/*	Function Name: UpdateSubnet
 *	Description: This Function Updates a subnet
 *	Arguments: name of the subnet in argv[1].
 *	Returns: DM_NORMAL.
 */

int UpdateSubnet(int argc, char **argv)
{
  struct qelem *top;
  top = GetMCInfo(SUBNET, argv[1], NULL);
  QueryLoop(top, NullPrint, RealUpdateSubnet, "Update the subnet");

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: RealDeleteSubnet
 *	Description: Actually performs the subnet deletion.
 *	Arguments: info - all information about this subnet.
 *                 one_subnet - If true then there was only one subnet in
 *                               the queue, and we should confirm.
 *	Returns: none.
 */

static void RealDeleteSubnet(char **info, Bool one_subnet)
{
  register int stat;
  char temp_buf[BUFSIZ];

  sprintf(temp_buf,
	  "Are you sure the you want to delete the subnet %s (y/n) ?",
	  info[C_NAME]);
  if (!one_subnet || Confirm(temp_buf))
    {
      if ((stat = do_mr_query("delete_subnet", 1,
			      &info[C_NAME], Scream, NULL)))
	{
	  com_err(program_name, stat, " in delete_subnet.");
	  sprintf(temp_buf, "Subnet %s ** NOT ** deleted.", info[C_NAME]);
	  Put_message(temp_buf);
	}
      else
	{
	  sprintf(temp_buf, "subnet %s successfully deleted.",
		  info[C_NAME]);
	  Put_message(temp_buf);
	}
    }
}

/*	Function Name: DeleteSubnet
 *	Description: This function removes a subnet from the database.
 *	Arguments: argc, argv - the name of the subnet is stored in argv[1].
 *	Returns: DM_NORMAL.
 */

int DeleteSubnet(int argc, char **argv)
{
  struct qelem *top;

  top = GetMCInfo(SUBNET, argv[1], NULL);
  QueryLoop(top, PrintSubnetInfo, RealDeleteSubnet, "Delete the subnet");

  FreeQueue(top);
  return DM_NORMAL;
}

/* ---------- Cluster Menu -------- */

/*	Function Name: ShowClusterInfo
 *	Description: Gets information about a cluser given its name.
 *	Arguments: argc, argc - the name of the cluster in in argv[1].
 *	Returns: DM_NORMAL.
 */

int ShowClusterInfo(int argc, char **argv)
{
  struct qelem *top;

  top = GetMCInfo(CLUSTER, argv[1], NULL);
  Loop(top, (void *) PrintClusterInfo);
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: AddCluster
 *	Description: Creates a new cluster.
 *	Arguments: argc, argv - the name of the new cluster is argv[1].
 *	Returns: DM_NORMAL.
 */

int AddCluster(int argc, char **argv)
{
  char **args, *info[MAX_ARGS_SIZE], *name = argv[1];
  int stat;

  /*
   * Check to see if this cluster already exists.
   */
  if (!ValidName(name))
    return DM_NORMAL;

  if ((stat = do_mr_query("get_cluster", 1, &name,
			  NullFunc, NULL)) == MR_SUCCESS)
    {
      Put_message("This cluster already exists.");
      return DM_NORMAL;
    }
  else if (stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in AddCluster.");
      return DM_NORMAL;
    }
  if (!(args = AskMCDInfo(SetClusterDefaults(info, name), CLUSTER, FALSE)))
    {
      Put_message("Aborted.");
      FreeInfo(info);
      return DM_NORMAL;
    }

  /*
   * Actually create the new Cluster.
   */
  if ((stat = do_mr_query("add_cluster", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in AddCluster.");

  FreeInfo(info);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateCluster
 *	Description: This function actually performs the cluster update.
 *	Arguments: info - all information nesc. for updating the cluster.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

static void RealUpdateCluster(char **info, Bool junk)
{
  register int stat;
  char **args = AskMCDInfo(info, CLUSTER, TRUE);

  if (!args)
    {
      Put_message("Aborted.");
      return;
    }
  if ((stat = do_mr_query("update_cluster", CountArgs(args),
			  args, Scream, NULL)))
    com_err(program_name, stat, " in UpdateCluster.");
  else
    Put_message("Cluster successfully updated.");
}

/*	Function Name: UpdateCluster
 *	Description: This Function Updates a cluster
 *	Arguments: name of the cluster in argv[1].
 *	Returns: DM_NORMAL.
 */

int UpdateCluster(int argc, char **argv)
{
  struct qelem *top;
  top = GetMCInfo(CLUSTER, argv[1], NULL);
  QueryLoop(top, NullPrint, RealUpdateCluster, "Update the cluster");

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: CheckAndRemoveMachine
 *	Description: This function checks and removes all machines from a
 *                   cluster.
 *	Arguments: name - name of the cluster.
 *                 ask_first - if TRUE, then we will query the user, before
 *                             deletion.
 *	Returns: SUB_ERROR if all machines not removed.
 */

int CheckAndRemoveMachines(char *name, Bool ask_first)
{
  register int stat, ret_value;
  Bool delete_it;
  char *args[10], temp_buf[BUFSIZ], *ptr;
  struct qelem *top, *elem = NULL;

  ret_value = SUB_NORMAL;
  args[MAP_MACHINE] = "*";
  args[MAP_CLUSTER] = name;
  stat = do_mr_query("get_machine_to_cluster_map", 2, args,
		     StoreInfo, (char *)&elem);
  if (stat && stat != MR_NO_MATCH)
    {
      com_err(program_name, stat, " in get_machine_to_cluster_map.");
      return DM_NORMAL;
    }
  if (stat == MR_SUCCESS)
    {
      elem = top = QueueTop(elem);
      if (ask_first)
	{
	  sprintf(temp_buf, "The cluster %s has the following machines in it:",
		  name);
	  Put_message(temp_buf);
	  while (elem)
	    {
	      char **info = (char **) elem->q_data;
	      Print(1, &info[MAP_MACHINE], (char *) NULL);
	      elem = elem->q_forw;
	    }
	  ptr = "Remove ** ALL ** these machines from this cluster?";

	  if (YesNoQuestion(ptr, FALSE) == TRUE) /* may return -1. */
	    delete_it = TRUE;
	  else
	    {
	      Put_message("Aborting...");
	      FreeQueue(top);
	      return SUB_ERROR;
	    }
	}
      else
	delete_it = TRUE;

      if (delete_it)
	{
	  elem = top;
	  while (elem)
	    {
	      char **info = (char **) elem->q_data;
	      if ((stat = do_mr_query("delete_machine_from_cluster",
				      2, info, Scream, NULL)))
		{
		  ret_value = SUB_ERROR;
		  com_err(program_name, stat,
			  " in delete_machine_from_cluster.");
		  sprintf(temp_buf,
			  "Machine %s ** NOT ** removed from cluster %s.",
			  info[MAP_MACHINE], info[MAP_CLUSTER]);
		  Put_message(temp_buf);
		}
	      elem = elem->q_forw;
	    }
	}
    }
  return ret_value;
}

/*	Function Name: RealDeleteCluster
 *	Description: Actually performs the cluster deletion.
 *	Arguments: info - all information about this cluster.
 *                 one_cluster - If true then there was only one cluster in
 *                               the queue, and we should confirm.
 *	Returns: none.
 */

static void RealDeleteCluster(char **info, Bool one_cluster)
{
  register int stat;
  char temp_buf[BUFSIZ];

  sprintf(temp_buf,
	  "Are you sure the you want to delete the cluster %s (y/n) ?",
	  info[C_NAME]);
  if (!one_cluster || Confirm(temp_buf))
    {
      if (CheckAndRemoveMachines(info[C_NAME], TRUE) != SUB_ERROR)
	{
	  if ((stat = do_mr_query("delete_cluster", 1,
				  &info[C_NAME], Scream, NULL)))
	    {
	      com_err(program_name, stat, " in delete_cluster.");
	      sprintf(temp_buf, "Cluster %s ** NOT ** deleted.", info[C_NAME]);
	      Put_message(temp_buf);
	    }
	  else
	    {
	      sprintf(temp_buf, "cluster %s successfully deleted.",
		      info[C_NAME]);
	      Put_message(temp_buf);
	    }
	}
    }
}

/*	Function Name: DeleteCluster
 *	Description: This function removes a cluster from the database.
 *	Arguments: argc, argv - the name of the cluster is stored in argv[1].
 *	Returns: DM_NORMAL.
 */

int DeleteCluster(int argc, char **argv)
{
  struct qelem *top;

  top = GetMCInfo(CLUSTER, argv[1], NULL);
  QueryLoop(top, PrintClusterInfo, RealDeleteCluster, "Delete the cluster");

  FreeQueue(top);
  return DM_NORMAL;
}

/* ----------- Cluster Data Menu -------------- */

/*	Function Name: ShowClusterData
 *	Description: This function shows the services for one cluster.
 *	Arguments: argc, argv - The name of the cluster is argv[1].
 *                              The label of the data in argv[2].
 *	Returns: DM_NORMAL.
 */

int ShowClusterData(int argc, char **argv)
{
  struct qelem *elem, *top;
  char **info;

  top = elem = GetMCInfo(DATA, argv[1], argv[2]);
  while (elem)
    {
      info = (char **) elem->q_data;
      PrintClusterData(info);
      elem = elem->q_forw;
    }
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: AddClusterData
 *	Description: This function adds some data to the cluster.
 *	Arguments: argv, argc:   argv[1] - the name of the cluster.
 *                               argv[2] - the label of the data.
 *                               argv[3] - the data.
 *	Returns: DM_NORMAL.
 */

int AddClusterData(int argc, char **argv)
{
  int stat;

  if ((stat = do_mr_query("add_cluster_data", 3, argv + 1,
			  Scream, (char *) NULL)))
    com_err(program_name, stat, " in AddClusterData.");
  return DM_NORMAL;
}

/*	Function Name: RealRemoveClusterData
 *	Description: actually removes the cluster data.
 *	Arguments: info - all info necessary to remove the cluster, in an array
 *                        of strings.
 *                 one_item - if true then the queue has only one elem and we
 *                            should confirm.
 *	Returns: none.
 */

static void RealRemoveClusterData(char **info, Bool one_item)
{
  register int stat;
  char *temp_ptr;

  Put_message(" ");
  temp_ptr = "Are you sure that you want to remove this cluster data (y/n) ?";
  PrintClusterData(info);
  if (!one_item || Confirm(temp_ptr))
    {
      if ((stat = do_mr_query("delete_cluster_data", 3, info,
			      Scream, (char *) NULL)))
	{
	  com_err(program_name, stat, " in DeleteClusterData.");
	  Put_message("Data not removed.");
	}
      else
	Put_message("Removal successful.");
    }
}

/*	Function Name: RemoveClusterData
 *	Description: This function removes data on a given cluster.
 *	Arguments: argv, argc:   argv[1] - the name of the cluster.
 *                               argv[2] - the label of the data.
 *                               argv[3] - the data.
 *	Returns: DM_NORMAL.
 */

int RemoveClusterData(int argc, char **argv)
{
  struct qelem *top;

  top = GetMCInfo(DATA, argv[1], argv[2]);
  QueryLoop(top, PrintClusterData, RealRemoveClusterData,
	    "Remove data from cluster");

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: MachineToClusterMap
 *	Description: This Retrieves the mapping between machine and cluster
 *	Arguments: argc, argv - argv[1] -> machine name or wildcard.
 *                              argv[2] -> cluster name or wildcard.
 *	Returns: none.
 */

int MachineToClusterMap(int argc, char **argv)
{
  struct qelem *elem, *top;
  char *tmpname, temp_buf[256];

  tmpname = canonicalize_hostname(strsave(argv[1]));
  if (strcasecmp(tmpname, argv[1]) && *argv[1] != '"')
    {
      sprintf(temp_buf, "Warning: '%s' canonicalized to '%s'.",
	      argv[1], tmpname);
      Put_message(temp_buf);
    }
  top = elem = GetMCInfo(MAP, tmpname, argv[2]);

  Put_message("");		/* blank line on screen */
  while (elem)
    {
      char **info = (char **) elem->q_data;
      PrintMCMap(info);
      elem = elem->q_forw;
    }

  FreeQueue(top);
  free(tmpname);
  return DM_NORMAL;
}
