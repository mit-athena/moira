/* $Id$
 *
 *	This is the file delete.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: functions for deleting users and lists.
 *
 *	Created: 	5/18/88
 *	By:		Chris D. Peterson
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

RCSID("$HeadURL$ $Id$");

int CheckListForDeletion(char *name, Bool verbose);
void CheckAce(char *type, char *name, Bool verbose);
int CheckIfAce(char *name, char *type, Bool verbose);
int RemoveItemFromLists(char *name, char *type, struct mqelem **elem,
			int verbose);
int RemoveMembersOfList(char *name, Bool verbose);
int DeleteUserGroup(char *name, Bool verbose);
int DeleteHomeFilesys(char *name, Bool verbose);
void AttemptToDeleteList(char **list_info, Bool ask_first);

/*	Function Name: CheckListForDeletion
 *	Description: Check one of the lists in which we just removed a member.
 *                   if the list is empty then it will delete it.
 *	Arguments: name - name of the list to check.
 *                 verbose - verbose mode?
 *	Returns: none.
 */

int CheckListForDeletion(char *name, Bool verbose)
{
  struct mqelem *elem = NULL;
  int status;
  char *args[2], buf[BUFSIZ], **info;

  if ((status = do_mr_query("count_members_of_list", 1, &name, StoreInfo,
			    &elem)))
    {
      com_err(program_name, status, " in DeleteList (count_members_of_list).");
      return SUB_NORMAL;
    }
  info = elem->q_data;
  if (!strcmp(info[NAME], "0"))
    {
      if (verbose)
	{
	  sprintf(buf, "Delete the empty list %s? ", name);
	  switch (YesNoQuestion(buf, FALSE))
	    {
	    case TRUE:
	      break;
	    case FALSE:
	      Put_message("Not deleting this list.");
	      FreeQueue(elem);
	      return SUB_NORMAL;
	    default:
	      Put_message("Aborting Deletion!");
	      FreeQueue(elem);
	      return SUB_ERROR;
	    }
	}
      args[0] = "foo";		/* not used. */
      args[1] = name;
      DeleteList(2, args);
    }
  FreeQueue(elem);
  return SUB_NORMAL;
}

/*	Function Name: CheckAce
 *	Description: Checks an ace to see of we should delete it.
 *	Arguments: type - the type of this ace.
 *                 name - the name of the ace.
 *                 verbose - query user?
 *	Returns: none.
 */

void CheckAce(char *type, char *name, Bool verbose)
{
  char *args[2], buf[BUFSIZ];
  int status;

  if (strcmp(type, "LIST"))
    return;		/* If the ace is not a list the ignore it. */

  args[0] = type;
  args[1] = name;
  status = do_mr_query("get_ace_use", 2, args, NULL, NULL);
  if (status != MR_NO_MATCH)
    return;			/* If this query fails the ace will
				   not be deleted even if it is empty. */
  if (verbose)
    {
      sprintf(buf, "Delete the unused Access Control Entity (ACE) %s? ", name);
      if (YesNoQuestion(buf, FALSE) != TRUE)
	{
	  Put_message("Aborting Deletion!");
	  return;
	}
    }
  /*
   * Delete the ACE.
   *
   * NOTE: Delete list expects only the name of the list to delete in argv[1].
   *       since, 'args' already satisfies this, there is no need to create
   *       a special argument list.
   */
  DeleteList(2, args);
}


/*	Function Name: CheckIfAce
 *	Description: Checks to see if this is an ace of another data object.
 *	Arguments: name - name of the object.
 *	Returns: SUB_ERROR if this list is an ace, or if the query did not
 *               succeed.
 */

int CheckIfAce(char *name, char *type, Bool verbose)
{
  char *args[2], buf[BUFSIZ], **info;
  struct mqelem *local, *elem;
  int status;
  elem = NULL;

  args[0] = type;
  args[1] = name;
  switch ((status = do_mr_query("get_ace_use", 2, args, StoreInfo, &elem)))
    {
    case MR_NO_MATCH:
      return DM_NORMAL;
    case MR_SUCCESS:
      local = elem = QueueTop(elem);
      info = local->q_data;
      if (QueueCount(elem) == 1 &&
	  !strcmp(info[0], "LIST") &&
	  !strcmp(info[1], name))
	{
	  FreeQueue(elem);
	  return DM_NORMAL;
	}
      if (verbose)
	{
	  sprintf(buf, "%s %s %s", type, name,
		  "is the ACE for the following data objects:");
	  Put_message(buf);
	  Put_message("");
	  for (; local != NULL; local = local->q_forw)
	    {
	      info = local->q_data;
	      if (!strcmp(info[0], "LIST") &&
		  !strcmp(info[1], name))
		continue;
	      Print(CountArgs(info), info, NULL);
	    }
	  Put_message("");
	  Put_message("The ACE for each of these items must be changed before");
	  sprintf(buf, "the %s %s can be deleted.\n", type, name);
	  Put_message(buf);
	}
      break;
    default:
      com_err(program_name, status, " in CheckIfAce (get_ace_use).");
      return SUB_ERROR;
    }
  FreeQueue(elem);
  return SUB_ERROR;
}

/*	Function Name: RemoveItemFromLists
 *	Description: this function removes a list from all other lists of
 *                   which it is a member.
 *	Arguments: name - name of the item
 *                 elem - a pointer to a queue element. RETURNED
 *                 verbose - verbose mode.
 *	Returns: SUB_ERROR if there is an error.
 */

int RemoveItemFromLists(char *name, char *type, struct mqelem **elem,
			int verbose)
{
  struct mqelem *local;
  char *args[10], temp_buf[BUFSIZ];
  int lists;
  int status;

  args[0] = type;
  args[1] = name;
  *elem = NULL;

  /*
   * Get all list of which this item is a member, and store them in a queue.
   */

  status = do_mr_query("get_lists_of_member", 2, args, StoreInfo, elem);

  if (status == MR_NO_MATCH)
    return SUB_NORMAL;

  if (status != MR_SUCCESS)
    {
      com_err(program_name, status, " in DeleteList (get_lists_of_member).");
      return SUB_ERROR;
    }

  /*
   * If verbose mode, ask user of we should remove our list from
   * all these lists.
   */

  local = *elem = QueueTop(*elem);
  lists = QueueCount(*elem);
  if (lists == 0)
    return SUB_NORMAL;
  if (verbose)
    {
      sprintf(temp_buf, "%s %s is a member of %d other list%s.\n", type,
	      name, lists, ((lists == 1) ? "" : "s"));
      Put_message(temp_buf);
      while (local)
	{
	  char **info = local->q_data;
	  Print(1, &info[GLOM_NAME], (char *) NULL);
	    local = local->q_forw;
	}
	Put_message(" ");	/* Blank Line. */
	sprintf(temp_buf, "Remove %s %s from these lists? ", type, name);
	if (YesNoQuestion(temp_buf, FALSE) != TRUE)
	  {
	    Put_message("Aborting...");
	    FreeQueue(*elem);
	    *elem = NULL;
	    return SUB_ERROR;
	  }
    }

  /*
   * Remove this list from all lists that it is a member of.
   */

  local = *elem;
  args[DM_MEMBER] = name;
  args[DM_TYPE] = type;
  while (local)
    {
      char **info = local->q_data;
      args[DM_LIST] = info[GLOM_NAME];
      if ((status = do_mr_query("delete_member_from_list",
				3, args, NULL, NULL)))
	{
	  com_err(program_name, status, " in delete_member\nAborting\n");
	  FreeQueue(*elem);
	  return SUB_ERROR;
	}
      local = local->q_forw;
    }
  return SUB_NORMAL;
}

/*	Function Name: RemoveMembersOfList
 *	Description: Deletes the members of the list.
 *	Arguments: name - name of the list.
 *                 verbose - query user, about deletion?
 *	Returns: SUB_ERROR - if we could not delete, or the user abouted.
 */

int RemoveMembersOfList(char *name, Bool verbose)
{
  char buf[BUFSIZ], *args[10];
  struct mqelem *local, *elem = NULL;
  int status, members;
  /*
   * Get the members of this list.
   */
  status = do_mr_query("get_members_of_list", 1, &name, StoreInfo, &elem);
  if (status == MR_NO_MATCH)
    return SUB_NORMAL;

  if (status)
    {
      com_err(program_name, status, " in DeleteList (get_members_of_list).");
      return SUB_ERROR;
    }
  /*
   * If verbose mode, then ask the user if we should delete.
   */
  local = elem = QueueTop(elem);
  if (!(members = QueueCount(elem)))
    return SUB_NORMAL;
  if (verbose)
    {
      sprintf(buf, "List %s has %d member%s:", name, QueueCount(elem),
	      ((members == 1) ? "" : "s"));
      Put_message(buf);
      Put_message(" ");	/* Blank Line. */
      while (local)
	{
	  char **info = local->q_data;
	  Print(CountArgs(info), info, NULL);
	  local = local->q_forw;
	}
      Put_message(" ");	/* Blank Line. */
      sprintf(buf, "Remove th%s member%s from list %s? ",
	      ((members == 1) ? "is" : "ese"),
	      ((members == 1) ? "" : "s"), name);
      if (YesNoQuestion(buf, FALSE) != TRUE)
	{
	  Put_message("Aborting...");
	  FreeQueue(elem);
	  return SUB_ERROR;
	}
    }
  /*
   * Perform The Removal.
   */
  local = elem;
  args[0] = name;
  while (local)
    {
      char **info = local->q_data;
      args[1] = info[0];
      args[2] = info[1];
      if ((status = do_mr_query("delete_member_from_list",
				3, args, NULL, NULL)))
	{
	  com_err(program_name, status, " in delete_member\nAborting\n");
	  FreeQueue(elem);
	  return SUB_ERROR;
	}
      local = local->q_forw;
    }
  return SUB_NORMAL;
}

/*	Function Name: DeleteUserGroup
 *	Description: Deletes the list given by name if it exists.
 *                   intended to be used to delete user groups
 *	Arguments: name - the name of the list to delete.
 *                 verbose - flag that if TRUE queries the user to
 *                           ask if list should be deleted.
 *	Returns: MR_ERROR if there is an error.
 */

int DeleteUserGroup(char *name, Bool verbose)
{
  int status, ans;
  char buf[BUFSIZ], *args[10];

  status = do_mr_query("get_list_info", 1, &name, NULL, NULL);
  if (!status)
    {
      if (verbose)
	{
	  sprintf(buf, "There is also a list named %s, delete it?", name);
	  ans = YesNoQuestion(buf, FALSE);
	  if (ans == FALSE)
	    {
	      Put_message("Leaving group alone.");
	      return SUB_NORMAL;
	    }
	  if (ans < 0)
	    {
	      Put_message("Aborting...");
	      return SUB_ERROR;
	    }
	}
      /* ans == TRUE  || ~verbose */
      args[0] = "foo";	/* not used. */
      args[1] = name;
      DeleteList(2, args);
    }
  else if (status != MR_NO_MATCH)
    {
      com_err(program_name, status, " Aborting Delete User.");
      return SUB_ERROR;
    }
  return SUB_NORMAL;
}

/*	Function Name: DeleteHomeFilesys
 *	Description: Delete the home filesystem for the named user.
 *	Arguments: name - name of the user (and filsystem) to delete.
 *                 verbose - if TRUE query user.
 *	Returns: SUB_NORMAL if home filesystem deleted, or nonexistant.
 */

int DeleteHomeFilesys(char *name, Bool verbose)
{
  int status;
  char buf[BUFSIZ];

  switch ((status = do_mr_query("get_filesys_by_label", 1, &name, NULL, NULL)))
    {
    case MR_NO_MATCH:
      break;
    case MR_SUCCESS:
      if (verbose)
	{
	  sprintf(buf, "Delete the filesystem named %s (y/n)?", name);
	  switch (YesNoQuestion(buf, FALSE))
	    {
	    case FALSE:
	      Put_message("Filesystem Not Deleted, continuing...");
	      return SUB_NORMAL;
	    case TRUE:
	      break;
	    default:
	      Put_message("Filesystem Not Deleted, aborting...");
	      return SUB_ERROR;
	    }
	}
      if ((status = do_mr_query("delete_filesys", 1, &name, NULL,
				NULL)) != MR_SUCCESS)
	{
	  com_err(program_name, status, " in delete_filesys.");
	  return SUB_ERROR;
	}
      else
	Put_message("Filesystem Successfully Deleted.");
      break;
    default:
      com_err(program_name, status, " in get_filesystem_by_label).");
      return SUB_ERROR;
    }
  return SUB_NORMAL;
}

#ifndef ATHENA
/*	Function Name: RealDeleteUser
 *	Description: Just Deletes the user.
 *	Arguments: name - name of User to delete
 *	Returns: SUB_ERROR if the deletion failed.
 */

static int RealDeleteUser(char *name)
{
  char buf[BUFSIZ];
  int status;

  if ((status = do_mr_query("delete_user", 1, &name, NULL,
			    NULL)) != MR_SUCCESS)
    {
      com_err(program_name, status, ": user not deleted");
      return SUB_ERROR;
    }
  sprintf(buf, "User %s deleted.", name);
  Put_message(buf);
  return SUB_NORMAL;
}
#endif

/*	Function Name: RealDeleteList
 *	Description: Just Deletes the list.
 *	Arguments: name - name of list to delete
 *	Returns: SUB_ERROR if the deletion failed.
 */

static int RealDeleteList(char *name)
{
  char buf[BUFSIZ];
  int status;

  if ((status = do_mr_query("delete_list", 1, &name, NULL,
			    NULL)) != MR_SUCCESS)
    {
      com_err(program_name, status, ": list not deleted");
      return SUB_ERROR;
    }
  sprintf(buf, "List %s deleted.", name);
  Put_message(buf);
  Put_message("");
  return SUB_NORMAL;
}

/*	Function Name: AttemptToDeleteList
 *	Description: Atempts to delete list, in the following manner:
 *                   1) try to delet it, if this fails in a known error then
 *                      a) try to clean up each of those known methods, or
 *                         at least explain why we failed.
 *	Arguments: list_info - info about this list.
 *                 ask_first - (T/F) query user before preparing for deletion,
 *                             and cleaning up?
 *	Returns: none - all is taken care of and error messages printed
 *                      one way or the other.
 */

void AttemptToDeleteList(char **list_info, Bool ask_first)
{
  int status;
  struct mqelem *local, *member_of;
  char *name = list_info[L_NAME];
  member_of = NULL;

  /*
   * Attempt delete. - will only work if:
   * 1) This list has no members.
   * 2) This list in a member of no other lists.
   * 3) This list is not an ace of another object.
   */

  switch ((status = do_mr_query("delete_list", 1, &name, NULL, NULL)))
    {
    case MR_SUCCESS:
      Put_message("List Sucessfully Deleted.");
      CheckAce(list_info[L_ACE_TYPE], list_info[L_ACE_NAME], ask_first);
      break;
    case MR_IN_USE:
      /*
       * This list is in use.  Try to find out why,
       * and for the cases where we have a good idea of
       * what to do we will query and then do it.
       */

      if ((CheckIfAce(name, "list", ask_first) != SUB_NORMAL) ||
	  (RemoveItemFromLists(name, "list",
			       &member_of, ask_first) != SUB_NORMAL))
	break;
      /*
       * If the list is it's own ACL, then make the person performing
       * the delete the owner before removing this person from the list
       */
      if (!strcmp(list_info[L_ACE_TYPE], "LIST") &&
	  !strcmp(list_info[L_ACE_NAME], list_info[L_NAME]))
	{
	  free(list_info[L_ACE_TYPE]);
	  free(list_info[L_ACE_NAME]);
	  list_info[L_ACE_TYPE] = strdup("USER");
	  list_info[L_ACE_NAME] = strdup(user);
	  SlipInNewName(list_info, strdup(list_info[L_NAME]));
	  if ((status = do_mr_query("update_list", CountArgs(list_info) - 3,
				    list_info, NULL, NULL)) != MR_SUCCESS)
	    {
	      com_err(program_name, status, " while updating list owner");
	      Put_message("List may be only partly deleted.");
	    }
	}
      if ((RemoveMembersOfList(name, ask_first) == SUB_NORMAL) &&
	  (RealDeleteList(name) == SUB_NORMAL))
	{		/* if... */
	  CheckAce(list_info[L_ACE_TYPE], list_info[L_ACE_NAME], ask_first);

	  local = QueueTop(member_of);
	  while (local)
	    {
	      char **info = local->q_data;
	      if (CheckListForDeletion(info[LM_LIST], ask_first) == SUB_ERROR)
		break;
	      local = local->q_forw;
	    }
	  FreeQueue(member_of);
	}
      break;
    default:
      com_err(program_name, status, " in DeleteList (delete_list).");
      break;
    }
}

/*	Function Name: DeleteList
 *	Description: deletes a list
 *	Arguments: argc, argv - standard Moira argc and argv.
 *	Returns: DM Status Code.
 */

int DeleteList(int argc, char *argv[])
{
  char buf[BUFSIZ];
  struct mqelem *top, *list;
  int status;
  Bool one_list;

  list = NULL;

  switch ((status = do_mr_query("get_list_info", 1, argv + 1,
				StoreInfo, &list)))
    {
    case MR_SUCCESS:
      break;
    case MR_NO_MATCH:
    case MR_LIST:
      Put_message("There is no list that matches that name.");
      return DM_NORMAL;
    default:
      com_err(program_name, status, " in DeleteList (get_list_info).");
      return DM_NORMAL;
    }

  top = list = QueueTop(list);
  one_list = (QueueCount(list) == 1);
  while (list)
    {
      char **info = list->q_data;
      if (one_list)
	{
	  sprintf(buf, "Are you sure that you want to delete the list %s",
		  info[L_NAME]);
	  if (Confirm(buf))
	    AttemptToDeleteList(info, TRUE);
	}
      else
	{
	  sprintf(buf, "Delete the list %s", info[L_NAME]);
	  switch (YesNoQuestion(buf, FALSE))
	    {
	    case TRUE:
	      AttemptToDeleteList(info, TRUE);
	      break;
	    case FALSE:
	      break;
	    default:
	      Put_message("Aborting...");
	      FreeQueue(top);
	      return DM_QUIT;
	    }
	}
      list = list->q_forw;
    }
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: DeleteUser
 *	Description: Deletes a user from the database.
 *	Arguments: argc, argv - name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int DeleteUser(int argc, char **argv)
{
  int status;
  char buf[BUFSIZ];
  char *name = argv[1];	/* name of the user we are deleting. */
#ifndef ATHENA
  struct mqelem *local, *member_of = NULL;
#endif

  if (!ValidName(name))
    return DM_NORMAL;

  if (!Confirm("Are you sure that you want to delete this user?"))
    return DM_NORMAL;

  status = do_mr_query("delete_user", 1, &name, NULL, NULL);
  if (status != MR_IN_USE && status != 0)
    {
      com_err(program_name, status, ": user not deleted");
      return DM_NORMAL;
    }
  if (status == 0)
    {
      sprintf(buf, "User %s deleted.", name);
      Put_message(buf);
#ifdef ATHENA
      /* delete this return if the policy decision below is reversed */
      return DM_NORMAL;
#endif
    }
#ifdef ATHENA
  /* Design decision not to allow registered users to be deleted.
   */
  Put_message("Sorry, registered users cannot be deleted from the database.");
  Put_message("Deactivate the user now, and the system manager will expunge later.");
#else
  else if (status == MR_IN_USE)
    {

      /*
       * Check:
       * 1) Query - Delete home filesytem.
       * 2) Query - Delete user Group.
       * 2) Is the user an ACE of any object in the database?
       * 3) Query - Remove user from all list of which he is a member.
       *
       * If all these have been accomplished, then attempt to delete
       * the user again.
       */
      if ((DeleteHomeFilesys(name, TRUE) == SUB_ERROR) ||
	  (DeleteUserGroup(name, TRUE) == SUB_ERROR) ||
	  (CheckIfAce(name, "user", TRUE) == SUB_ERROR) ||
	  (RemoveItemFromLists(name, "user", &member_of, TRUE) == SUB_ERROR) ||
	  (RealDeleteUser(name) == SUB_ERROR))
	return DM_NORMAL;
    }

  /*
   * Query - Delete all empty lists created by removing this user from them.
   */

  local = member_of;
  while (local)
    {
      char **info = local->q_data;
      if (CheckListForDeletion(info[0], TRUE) == SUB_ERROR)
	break;
      local = local->q_forw;
    }

  FreeQueue(member_of);	/* Free memory and return. */
#endif
  return DM_NORMAL;
}
