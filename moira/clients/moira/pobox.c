#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v 1.22 1998-01-05 19:52:10 danw Exp $";
#endif

/*	This is the file pobox.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains: Functions for handling the poboxes.
 *
 *	Created: 	7/10/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/pobox.c,v 1.22 1998-01-05 19:52:10 danw Exp $
 *
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#define FOREIGN_BOX ("SMTP")
#define LOCAL_BOX ("POP")

/*	Function Name: PrintPOBox
 *	Description: Yet another specialized print function.
 *	Arguments: info - all info about this PO_box.
 *	Returns: MR_CONT
 */

static void PrintPOBox(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Address: %-10s Box: %-35s Type: %s", info[PO_NAME],
	  info[PO_BOX], info[PO_TYPE]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[4], info[3], info[5]);
  Put_message(buf);
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
  register int status;
  static char *args[] = {"pop", NULL};
  struct qelem *top = NULL;

  if ((status = do_mr_query("get_server_locations", CountArgs(args), args,
			    StoreInfo, (char *)&top)))
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
  register int status;
  struct qelem *top = NULL;
  char buf[BUFSIZ];

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  switch ((status = do_mr_query("get_pobox", 1, argv + 1, StoreInfo,
				(char *)&top)))
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
      return canonicalize_hostname(strsave(temp_buf));
    }
  Put_message("Could not get machines to choose from, quitting.");
  return (char *) SUB_ERROR;
}

/*	Function Name: SetUserPOBox
 *	Description: Addes or Chnages the P.O. Box for a user.
 *	Arguments: argc, argv - the login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int SetUserPOBox(int argc, char **argv)
{
  register int status;
  char *type, temp_buf[BUFSIZ], *local_user, *args[10], box[BUFSIZ];
  char *temp_box;
  struct qelem *top = NULL;
  local_user = argv[1];

  if (!ValidName(local_user))
    return DM_NORMAL;

  /* Print the current PO Box info */
  switch ((status = do_mr_query("get_pobox", 1, argv + 1, StoreInfo,
				(char *)&top)))
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

  sprintf(temp_buf, "Assign %s a local PO Box (y/n)", local_user);
  switch (YesNoQuestion(temp_buf, TRUE))
    {
    case TRUE:
      type = LOCAL_BOX;
      switch (YesNoQuestion("Use Previous Local Box (y/n)", TRUE))
	{
	case TRUE:
	  switch ((status = do_mr_query("set_pobox_pop", 1,
					&local_user, Scream, NULL)))
	    {
	    case MR_SUCCESS:
	      return DM_NORMAL;
	    case MR_MACHINE:
	      sprintf(temp_buf, "%s did not have a previous local PO Box.",
		      local_user);
	      Put_message(temp_buf);
	      if ((temp_box = GetNewLocalPOBox(local_user)) !=
		  (char *) SUB_ERROR)
		{
		  strcpy(box, temp_box);
		  free(temp_box);
		}
	      else
		return DM_NORMAL;
	      break;
	    default:
	      com_err(program_name, status, " in set_pobox_pop.");
	      return DM_NORMAL;
	    }
	  break;
	case FALSE:
	  if ((temp_box = GetNewLocalPOBox(local_user)) !=
	      (char *) SUB_ERROR)
	    {
	      strcpy(box, temp_box);
	      free(temp_box);
	    }
	  else
	    return DM_NORMAL;
	  break;
	default:
	  return DM_NORMAL;
	}
      break;
    case FALSE:
      type = FOREIGN_BOX;
      sprintf(temp_buf, "Set up a foreign PO Box for %s (y/n)", local_user);
      switch (YesNoQuestion(temp_buf, TRUE))
	{
	case TRUE:
	  if (!Prompt_input("Foreign PO Box for this user? ", box, BUFSIZ))
	    return DM_NORMAL;
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
  args[PO_END] = NULL;
  if ((status = do_mr_query("set_pobox", CountArgs(args), args,
			    Scream, NULL)))
    com_err(program_name, status, " in ChangeUserPOBox");
  else
    Put_message("PO Box assigned.");

  return DM_NORMAL;
}

/*	Function Name: RemoveUserPOBox
 *	Description: Removes this users POBox.
 *	Arguments: argc, argv - name of user in argv[1].
 *	Returns: DM_NORMAL.
 */

int RemoveUserPOBox(int argc, char **argv)
{
  register int status;
  char temp_buf[BUFSIZ];

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  sprintf(temp_buf, "Are you sure that you want to remove %s's PO Box (y/n)",
	  argv[1]);

  if (Confirm(temp_buf))
    {
      if ((status = do_mr_query("delete_pobox", 1, argv + 1,
				Scream, NULL)))
	com_err(program_name, status, " in delete_pobox.");
      else
	Put_message("PO Box removed.");
    }
  return DM_NORMAL;
}
