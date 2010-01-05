/* $Id$
 *
 *	This is the file pobox.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for handling the poboxes.
 *
 *	Created: 	7/10/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>
#include <mrclient.h>
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

/*	Function Name: PrintPOBox
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this PO_box.
 *	Returns: MR_CONT
 */

static void PrintPOBox(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "User: %-10s Box: %-35s Type: %s", info[PO_NAME],
	  info[PO_BOX], info[PO_TYPE]);
  Put_message(buf);
  if (info[6])
    {
      sprintf(buf, "Address: %s", info[PO_ADDR]);
      Put_message(buf);
      sprintf(buf, MOD_FORMAT, info[5], info[4], info[6]);
      Put_message(buf);
    }
  else
    {
      sprintf(buf, MOD_FORMAT, info[4], info[3], info[5]);
      Put_message(buf);
    }
}

/*	Function Name: RealPrintPOMachines
 *	Description: Actually does the printing for PrintPOMachines.
 *	Arguments: info, - name of machines in info[1].
 *	Returns: none.
 */

static void RealPrintPOMachines(char **info)
{
  Print(1, info + 1, NULL);
}

/*	Function Name: PrintPOMachines
 *	Description: Prints all current post offices.
 *	Arguments: none.
 *	Returns: SUB_ERROR if the machines could not be printed.
 */

static int PrintPOMachines(void)
{
  int status;
  static char *args[] = {"pop", NULL};
  struct mqelem *top = NULL;

  if ((status = do_mr_query("get_server_locations", CountArgs(args), args,
			    StoreInfo, &top)))
    {
      com_err(program_name, status, " in get_server_locations.");
      return SUB_ERROR;
    }

  top = QueueTop(top);
  Loop(top, RealPrintPOMachines);
  FreeQueue(top);
  return SUB_NORMAL;
}

/*	Function Name: GetUserPOBox
 *	Description: prints the users POBox information.
 *	Arguments: argc, argv - name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int GetUserPOBox(int argc, char **argv)
{
  int status;
  struct mqelem *top = NULL;
  char buf[BUFSIZ];

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  switch ((status = do_mr_query("get_pobox", 1, argv + 1, StoreInfo, &top)))
    {
    case MR_NO_MATCH:
      Put_message("This user has no P.O. Box.");
      break;
    case MR_SUCCESS:
      sprintf(buf, "Current pobox for user %s: \n", argv[1]);
      Put_message("");
      top = QueueTop(top);
      Loop(top, PrintPOBox);	/* should only return 1 box. */
      FreeQueue(top);
      break;
    default:
      com_err(program_name, status, " in get_pobox.");
    }
  return DM_NORMAL;
}

/*	Function Name: GetNewLocalPOBox
 *	Description: get the machine for a new local pop Box for the user.
 *	Arguments: local_user - name of the local user.
 *	Returns: machine - name of the machine for then new pop box, or NULL.
 */

static char *GetNewLocalPOBox(char *local_user)
{
  char temp_buf[BUFSIZ];

  sprintf(temp_buf, "%s %s", "Pick one of the following",
	  "machines for this user's Post Office.");
  Put_message(temp_buf);
  Put_message("");
  if (PrintPOMachines() == SUB_NORMAL)
    {
      Put_message("");
      if (!Prompt_input("Which Machine? ", temp_buf, BUFSIZ))
	return (char *) SUB_ERROR;
      return canonicalize_hostname(strdup(temp_buf));
    }
  Put_message("Could not get machines to choose from, quitting.");
  return (char *) SUB_ERROR;
}

static int InsertSortedImapPart(int argc, char **argv, void *arg)
{
  struct mqelem *new = NULL;
  struct mqelem *elem, *prev, **parts = arg;
  int avail = atoi(argv[NFS_SIZE]) - atoi(argv[NFS_ALLOC]);

  /* Dup the argv into a new mqelem. */
  StoreInfo(argc, argv, &new);

  if (!*parts)
    {
      *parts = new;
      return MR_CONT;
    }

  /* Find the right place in parts for it. */
  elem = *parts;
  prev = NULL;
  while (elem)
    {
      char **xargv = elem->q_data;
      if (atoi(xargv[NFS_SIZE]) - atoi(xargv[NFS_ALLOC]) < avail)
	break;
      prev = elem;
      elem = elem->q_forw;
    }

  if (prev)
    AddQueue(new, prev);
  else
    {
      new->q_forw = *parts;
      (*parts)->q_back = new;
      *parts = new;
    }

  return MR_CONT;
}

static int AddImapPartitions(char *server, struct mqelem **parts)
{
  char *argv[2];
  int status;

  argv[0] = server;
  argv[1] = "*";

  status = do_mr_query("get_nfsphys", 2, argv, InsertSortedImapPart, parts);
  if (status)
    {
      com_err(program_name, status, " in AddImapPartitions");
      return SUB_ERROR;
    }
  return SUB_NORMAL;
}

char *CreateImapBox(char *user)
{
  int status;
  struct mqelem *elem = NULL, *servers = NULL, *partitions = NULL;
  char *server = NULL, *partition = NULL;
  char *argv[11], *fsname, temp_buf[BUFSIZ];
  static char *default_imap_quota = NULL;

  if (!default_imap_quota)
    {
      char **vargv;
      argv[0] = "def_imap_quota";
      status = do_mr_query("get_value", 1, argv, StoreInfo, &elem);
      if (status)
	{
	  com_err(program_name, status, " getting default IMAP quota");
	  return (char *)SUB_ERROR;
	}
      vargv = elem->q_data;
      default_imap_quota = strdup(vargv[0]);
      FreeQueue(elem);
    }

  argv[0] = "POSTOFFICE";
  status = do_mr_query("get_server_locations", 1, argv, StoreInfo, &servers);
  if (status)
    {
      com_err(program_name, status, " in GetImapBox.");
      return (char *)SUB_ERROR;
    }
  servers = QueueTop(servers);

  /* Get an IMAP server. */
  while (!server)
    {
      server = strdup("[ANY]");
      if (GetValueFromUser("IMAP Server? ['?' for a list]", &server) !=
	  SUB_NORMAL)
	{
	  free(server);
	  FreeQueue(servers);
	  return (char *)SUB_ERROR;
	}

      if (!strcmp(server, "?"))
	{
	  elem = servers;
	  while (elem)
	    {
	      char **sargv = elem->q_data;
	      sprintf(temp_buf, "  %s\n", sargv[1]);
	      Put_message(temp_buf);
	      elem = elem->q_forw;
	    }
	  free(server);
	  server = NULL;
	}
    }
  server = canonicalize_hostname(server);

  /* Get the partitions on that server. */
  if (!strcasecmp(server, "[ANY]"))
    {
      char **sargv;

      elem = servers;
      while (elem && !status)
	{
	  sargv = elem->q_data;
	  status = AddImapPartitions(sargv[1], &partitions);
	  elem = elem->q_forw;
	}

      if (partitions)
	{
	  sargv = partitions->q_data;
	  server = strdup(sargv[NFS_NAME]);
	  partition = strdup(sargv[NFS_DIR]);
	}
    }
  else
    status = AddImapPartitions(server, &partitions);
  partitions = QueueTop(partitions);

  FreeQueue(servers);
  if (status || !partitions)
    {
      if (!partitions)
	com_err(program_name, 0, "No registered nfsphys on %s.", server);
      else
	FreeQueue(partitions);
      free(server);
      return (char *)SUB_ERROR;
    }

  /* Pick a partition */
  while (!partition)
    {
      char **pargv = partitions->q_data;
      partition = strdup(pargv[NFS_DIR]);
      if (GetValueFromUser("Partition? ['?' for a list]", &partition) !=
	  SUB_NORMAL)
	{
	  free(server);
	  free(partition);
	  FreeQueue(partitions);
	  return (char *)SUB_ERROR;
	}

      elem = partitions;
      if (!strcmp(partition, "?"))
	{
	  while (elem)
	    {
	      char **pargv = elem->q_data;
	      sprintf(temp_buf, "  %s (%s available, %d free)",
		      pargv[NFS_DIR], pargv[NFS_SIZE],
		      atoi(pargv[NFS_SIZE]) - atoi(pargv[NFS_ALLOC]));
	      Put_message(temp_buf);
	      elem = elem->q_forw;
	    }
	  free(partition);
	  partition = NULL;
	}
      else
	{
	  while (elem)
	    {
	      char **pargv = elem->q_data;
	      if (!strcmp(partition, pargv[NFS_DIR]))
		break;
	      elem = elem->q_forw;
	    }
	  if (!elem)
	    {
	      com_err(program_name, 0, "No such partition %s", partition);
	      free(partition);
	      partition = NULL;
	    }
	}
    }
  FreeQueue(partitions);

  fsname = malloc(strlen(user) + 4);
  sprintf(fsname, "%s.po", user);
  argv[FS_NAME] = fsname;
  argv[FS_TYPE] = "IMAP";
  argv[FS_MACHINE] = server;
  argv[FS_PACK] = partition;
  argv[FS_M_POINT] = "";
  argv[FS_ACCESS] = "w";
  argv[FS_COMMENTS] = "IMAP box";
  argv[FS_OWNER] = user;
  argv[FS_OWNERS] = "wheel";
  argv[FS_CREATE] = "1";
  argv[FS_L_TYPE] = "USER";

  status = do_mr_query("add_filesys", 11, argv, NULL, NULL);
  free(server);
  free(partition);

  if (status)
    {
      com_err(program_name, status, " creating IMAP filesys in CreateImapBox");
      free(fsname);
      return (char *)SUB_ERROR;
    }

  argv[Q_FILESYS] = fsname;
  argv[Q_TYPE] = "USER";
  argv[Q_NAME] = user;
  argv[Q_QUOTA] = default_imap_quota;

  status = do_mr_query("add_quota", 4, argv, NULL, NULL);
  if (status)
    {
      com_err(program_name, status, " setting quota in CreateImapBox");
      free(fsname);
      return (char *)SUB_ERROR;
    }
  else
    return fsname;
}

/*	Function Name: SetUserPOBox
 *	Description: Addes or Chnages the P.O. Box for a user.
 *	Arguments: argc, argv - the login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int SetUserPOBox(int argc, char **argv)
{
  int status;
  char *type, temp_buf[BUFSIZ], *local_user, *args[10], *box;
  struct mqelem *top = NULL;

  local_user = argv[1];
  if (!ValidName(local_user))
    return DM_NORMAL;

  /* Print the current PO Box info */
  switch ((status = do_mr_query("get_pobox", 1, argv + 1, StoreInfo, &top)))
    {
    case MR_SUCCESS:
      sprintf(temp_buf, "Current pobox for user %s: \n", local_user);
      Put_message("");
      top = QueueTop(top);
      Loop(top, PrintPOBox);	/* should only return 1 box. */
      FreeQueue(top);
      break;
    case MR_NO_MATCH:
      Put_message("This user has no P.O. Box.");
      break;
    default:
      com_err(program_name, status, " in get_pobox.");
      return DM_NORMAL;
    }

  Put_message("");

  sprintf(temp_buf, "Assign %s a local PO Box (y/n)", local_user);
  switch (YesNoQuestion(temp_buf, TRUE))
    {
    case TRUE:
      switch (YesNoQuestion("Use Previous Local Box (y/n)", TRUE))
	{
	case TRUE:
	  switch ((status = do_mr_query("set_pobox_pop", 1,
					&local_user, NULL, NULL)))
	    {
	    case MR_SUCCESS:
	      return DM_NORMAL;
	    case MR_MACHINE:
	      sprintf(temp_buf, "%s did not have a previous local PO Box.",
		      local_user);
	      Put_message(temp_buf);
	    default:
	      com_err(program_name, status, " in set_pobox_pop.");
	      return DM_NORMAL;
	    }
	  /* Fall through from MR_MACHINE case. */
	case FALSE:
	  type = strdup("IMAP");
	  if (GetValueFromUser("Kind of Local PO Box?", &type) == SUB_ERROR)
	    {
	      free(type);
	      return DM_NORMAL;
	    }
	  if (!strcasecmp(type, "POP"))
	    {
	      free(type);
	      type = "POP";
	      if ((box = GetNewLocalPOBox(local_user)) == (char *) SUB_ERROR)
		return DM_NORMAL;
	    }
	  else if (!strcasecmp(type, "IMAP"))
	    {
	      free(type);
	      type = "IMAP";
	      switch (YesNoQuestion("Create IMAP filesystem (y/n)", TRUE))
		{
		case TRUE:
		  if ((box = CreateImapBox(local_user)) == (char *) SUB_ERROR)
		    return DM_NORMAL;
		  break;
		case FALSE:
		  box = malloc(strlen(local_user) + 4);
		  sprintf(box, "%s.po", local_user);
		  break;
		default:
		  return DM_NORMAL;
		}
	    }
	  else if (!strcasecmp(type, "EXCHANGE"))
	    {
	      free(type);
	      type = "EXCHANGE";
	      box = "EXCHANGE.MIT.EDU";
	    }
	  else
	    {
	      sprintf(temp_buf, "Unknown local PO Box type %s", type);
	      Put_message(temp_buf);
	      free(type);
	      return DM_NORMAL;
	    }
	  break;
	default:
	  return DM_NORMAL;
	}
      break;
    case FALSE:
      type = "SMTP";
      sprintf(temp_buf, "Set up a foreign PO Box for %s (y/n)", local_user);
      switch (YesNoQuestion(temp_buf, TRUE))
	{
	case TRUE:
	  if (!Prompt_input("Foreign PO Box for this user? ", temp_buf, BUFSIZ))
	    return DM_NORMAL;
	  if (mrcl_validate_pobox_smtp(local_user, temp_buf, &box) !=
	      MRCL_SUCCESS)
	    {
	      if (mrcl_get_message())
		Put_message(mrcl_get_message());
	      return DM_NORMAL;
	    }
	  break;
	case FALSE:
	default:
	  return DM_NORMAL;	/* ^C hit. */
	}
      break;
    default:			/* ^C hit. */
      Put_message("Aborted.");
      return DM_NORMAL;
    }

  args[PO_NAME] = local_user;
  args[PO_TYPE] = type;
  args[PO_BOX] = box;
  args[3] = NULL;
  if ((status = do_mr_query("set_pobox", CountArgs(args), args,
			    NULL, NULL)))
    com_err(program_name, status, " in ChangeUserPOBox");
  else
    Put_message("PO Box assigned.");
  free(box);

  return DM_NORMAL;
}

/*	Function Name: SplitUserPOBox
 *	Description: Splits the user's PO Box between local and SMTP
 *	Arguments: argc, argv - name of user in argv[1].
 *	Returns: DM_NORMAL.
 */

int SplitUserPOBox(int argc, char **argv)
{
  char temp_buf[BUFSIZ], *args[3], *box;
  int status;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if (!Prompt_input("Foreign PO Box for this user? ", temp_buf, BUFSIZ))
    return DM_NORMAL;
  if (mrcl_validate_pobox_smtp(argv[1], temp_buf, &box) !=
      MRCL_SUCCESS)
    {
      if (mrcl_get_message())
	Put_message(mrcl_get_message());
      return DM_NORMAL;
    }

  args[0] = argv[1];
  args[1] = "SPLIT";
  args[2] = box;

  status = do_mr_query("set_pobox", 3, args, NULL, NULL);
  if (status == MR_MACHINE)
    Put_message("User has no local PO Box--PO Box unchanged.");
  else if (status)
    com_err(program_name, status, " in SplitUserPOBox");
  else
    Put_message("PO Box split.");
  free(box);

  return DM_NORMAL;
}

/*	Function Name: RemoveUserPOBox
 *	Description: Removes this users POBox.
 *	Arguments: argc, argv - name of user in argv[1].
 *	Returns: DM_NORMAL.
 */

int RemoveUserPOBox(int argc, char **argv)
{
  int status;
  char temp_buf[BUFSIZ];

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  sprintf(temp_buf, "Are you sure that you want to remove %s's PO Box (y/n)",
	  argv[1]);

  if (Confirm(temp_buf))
    {
      if ((status = do_mr_query("delete_pobox", 1, argv + 1,
				NULL, NULL)))
	com_err(program_name, status, " in delete_pobox.");
      else
	Put_message("PO Box removed.");
    }
  return DM_NORMAL;
}
