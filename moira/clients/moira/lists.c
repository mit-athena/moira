/* $Id$
 *
 *	This is the file lists.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: All list manipulation functions, except delete.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <mrclient.h>
#include <moira_site.h>
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

struct mqelem *GetListInfo(int type, char *name1, char *name2);
char **AskListInfo(char **info, Bool name);
int AddList(int argc, char **argv);
void ListMembersByType(char *type, int tags);
int GetMemberInfo(char *action, char **ret_argv);

#define LIST    0
#define MEMBERS 1
#define GLOM    2
#define ACE_USE 3

#define DEFAULT_ACTIVE            DEFAULT_YES
#define DEFAULT_PUBLIC            DEFAULT_YES
#define DEFAULT_HIDDEN            DEFAULT_NO
#define DEFAULT_MAILLIST          DEFAULT_YES
#define DEFAULT_GROUP             DEFAULT_NO
#define DEFAULT_GID               UNIQUE_GID
#define DEFAULT_NFSGROUP          DEFAULT_NO
#define DEFAULT_MAILMAN           DEFAULT_NO
#define DEFAULT_MAILMAN_SERVER    "[ANY]"
#define DEFAULT_ACE_TYPE          "user"
#define DEFAULT_ACE_NAME          (user)
#define DEFAULT_MEMACE_TYPE       "NONE"
#define DEFAULT_MEMACE_NAME       "NONE"
#define DEFAULT_DESCRIPTION       DEFAULT_COMMENT

/* globals only for this file. */

static char current_list[BUFSIZ];


/*	Function Name: PrintListAce
 *	Description: This function prints the list ace information.
 *	Arguments: info - an info structure.
 *	Returns: none.
 */

static void PrintListAce(char **info)
{
  char buf[BUFSIZ];

  sprintf(buf, "Item: %-20s Name: %s", info[ACE_TYPE], info[ACE_NAME]);
  Put_message(buf);
}

/*	Function Name: PrintListInfo
 *	Description: This function Prints out the List info in a coherent form.
 *	Arguments: info - the List info.
 *	Returns: none.
 */

static void PrintListInfo(char **info)
{
  char buf[BUFSIZ];

  Put_message(" ");
  sprintf(buf, "%20sList: %s", "", info[L_NAME]);
  Put_message(buf);
  sprintf(buf, "Description: %s", info[L_DESC]);
  Put_message(buf);
  if (atoi(info[L_MAILLIST]))
    Put_message("This list is a mailing list.");
  else
    Put_message("This list is NOT a mailing list.");
  if (atoi(info[L_GROUP]))
    {
      sprintf(buf, "This list is a Group%s and its ID number is %s",
	      atoi(info[L_NFSGROUP]) ? " and an NFS Group," : "",
	      info[L_GID]);
      Put_message(buf);
    }
  else
    Put_message("This list is NOT a Group.");

  if (atoi(info[L_MAILMAN]))
    {
      sprintf(buf, "This list is a Mailman list on server %s",
	      info[L_MAILMAN_SERVER]);
      Put_message(buf);
    }

  if (!strcmp(info[L_ACE_TYPE], "NONE"))
    Put_message("This list has no Administrator, how strange?!");
  else
    {
      sprintf(buf, "The Administrator of this list is the %s: %s",
	      info[L_ACE_TYPE], info[L_ACE_NAME]);
      Put_message(buf);
    }

  if (strcmp(info[L_MEMACE_TYPE], "NONE"))
    {
      sprintf(buf, "The Membership Administrator of this list is the %s: %s",
	      info[L_MEMACE_TYPE], info[L_MEMACE_NAME]);
      Put_message(buf);
    }

  sprintf(buf, "This list is: %s, %s, and %s",
	  atoi(info[L_ACTIVE]) ? "active" : "inactive",
	  atoi(info[L_PUBLIC]) ? "public" : "private",
	  atoi(info[L_HIDDEN]) ? "hidden" : "visible");
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[L_MODBY], info[L_MODTIME], info[L_MODWITH]);
  Put_message(buf);
}

/*	Function Name: GetListInfo
 *	Description: Stores all info about a group of lists in a queue.
 *	Arguments: type - type of info to store.
 *                 name - name of the info.
 *	Returns: the first element in the queue.
 */

struct mqelem *GetListInfo(int type, char *name1, char *name2)
{
  char *args[2];
  struct mqelem *elem = NULL;
  int status;

  switch (type)
    {
    case LIST:
      args[0] = name1;
      if ((status = do_mr_query("get_list_info", 1, args, StoreInfo, &elem)))
	{
	  com_err(program_name, status, " in get_list_info");
	  return NULL;
	}
      break;
    case MEMBERS:
      args[0] = name1;
      if ((status = do_mr_query("get_members_of_list", 1, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status, " in get_members_of_list");
	  return NULL;
	}
      break;
    case GLOM:
      args[0] = name1;
      args[1] = name2;
      if (!strcmp(name1, "MACHINE"))
	  args[1] = canonicalize_hostname(strdup(name2));
      if ((status = do_mr_query("get_lists_of_member", 2, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status, " in get_list_of_members");
	  return NULL;
	}
      break;
    case ACE_USE:
      args[0] = name1;
      args[1] = name2;
      if ((status = do_mr_query("get_ace_use", 2, args, StoreInfo, &elem)))
	{
	  com_err(program_name, status, " in get_ace_use");
	  return NULL;
	}
      break;
    }
  return QueueTop(elem);
}

/*	Function Name: AskListInfo.
 *	Description: This function askes the user for information about a
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the
 *                             info into.
 *                 name - add a newname field? (T/F)
 *	Returns: SUB_ERROR or SUB_NORMAL.
 */

char **AskListInfo(char **info, Bool name)
{
  char temp_buf[BUFSIZ], *newname;
  int status;

  Put_message(" ");
  sprintf(temp_buf, "Setting information of list %s.", info[L_NAME]);
  Put_message(temp_buf);
  Put_message(" ");

  if (name)
    {
      while (1)
	{
	  newname = strdup(info[L_NAME]);
	  if (GetValueFromUser("The new name for this list", &newname) ==
	      SUB_ERROR)
	    return NULL;
	  if (ValidName(newname))
	    break;
	}
    }
  if (GetYesNoValueFromUser("Is this an active list", &info[L_ACTIVE]) ==
      SUB_ERROR)
    return NULL;
  if (GetYesNoValueFromUser("Is this a public list", &info[L_PUBLIC]) ==
      SUB_ERROR)
    return NULL;
  if (GetYesNoValueFromUser("Is this a hidden list", &info[L_HIDDEN]) ==
      SUB_ERROR)
    return NULL;
  if (GetYesNoValueFromUser("Is this a maillist", &info[L_MAILLIST]) ==
      SUB_ERROR)
    return NULL;
  if (GetYesNoValueFromUser("Is this a group", &info[L_GROUP]) == SUB_ERROR)
    return NULL;
  if (atoi(info[L_GROUP]))
    {
      if (GetYesNoValueFromUser("Is this an NFS group", &info[L_NFSGROUP]) == 
	  SUB_ERROR)
	return NULL;
      if (GetValueFromUser("What is the GID for this group.", &info[L_GID]) ==
	  SUB_ERROR)
	return NULL;
    }

  if (GetYesNoValueFromUser("Is this a Mailman list", &info[L_MAILMAN]) == 
      SUB_ERROR)
    return NULL;
  if (atoi(info[L_MAILMAN]))
    {
      if (GetValueFromUser("Mailman server", &info[L_MAILMAN_SERVER]) == 
	  SUB_ERROR)
	return NULL;
      info[L_MAILMAN_SERVER] = canonicalize_hostname(info[L_MAILMAN_SERVER]);
      
      /* Change default owner */
      strcpy(info[L_ACE_TYPE], "LIST");
      strcpy(info[L_ACE_NAME], "mailman");
    }

  do {
    if (GetTypeFromUser("What Type of Administrator", "ace_type",
			&info[L_ACE_TYPE]) == SUB_ERROR)
      return NULL;
    if (strcasecmp(info[L_ACE_TYPE], "none"))
      {
	sprintf(temp_buf, "Which %s will be the administrator of this list: ",
		info[L_ACE_TYPE]);
	if (GetValueFromUser(temp_buf, &info[L_ACE_NAME]) == SUB_ERROR)
	  return NULL;
      }
    else
      {
	Put_message("Setting the administrator of this list to 'NONE'");
	Put_message("will make you unable to further modify the list.");
	if (YesNoQuestion("Do you really want to do this?", FALSE) == TRUE)
	  break;
      }
  } while (!strcasecmp(info[L_ACE_TYPE], "none"));
	
  if (!strcasecmp(info[L_ACE_TYPE], "kerberos"))
    {
      char *canon;
      
      status = mrcl_validate_kerberos_member(info[L_ACE_NAME], &canon);
      if (mrcl_get_message())
	Put_message(mrcl_get_message());
      if (status == MRCL_REJECT)
	return NULL;
      free(info[L_ACE_NAME]);
      info[L_ACE_NAME] = canon;
    }

  do {
    if (GetTypeFromUser("What Type of Membership Administrator", "ace_type",
			&info[L_MEMACE_TYPE]) == SUB_ERROR)
      return NULL;
    if (strcasecmp(info[L_MEMACE_TYPE], "none"))
      {
	sprintf(temp_buf, "Which %s will be the membership administrator of this list: ",
		info[L_MEMACE_TYPE]);
	if (GetValueFromUser(temp_buf, &info[L_MEMACE_NAME]) == SUB_ERROR)
	  return NULL;
      }
    else if (atoi(info[L_MAILMAN]))
      {
	Put_message("Setting the Membership Administrator of a Mailman list to 'NONE'");
	Put_message("means no one will receive the list administrator password.");
	if (YesNoQuestion("Do you really want to do this?", FALSE) == TRUE)
	  break;
      }
  } while ((!strcasecmp(info[L_MEMACE_TYPE], "none")) && 
	   atoi(info[L_MAILMAN]));

  if (!strcasecmp(info[L_MEMACE_TYPE], "kerberos"))
    {
      char *canon;

      status = mrcl_validate_kerberos_member(info[L_MEMACE_NAME], &canon);
      if (mrcl_get_message())
	Put_message(mrcl_get_message());
      if (status == MRCL_REJECT)
	return NULL;
      free(info[L_MEMACE_NAME]);
      info[L_MEMACE_NAME] = canon;
    }
  if (GetValueFromUser("Description: ", &info[L_DESC]) == SUB_ERROR)
    return NULL;

  FreeAndClear(&info[L_MODTIME], TRUE);
  FreeAndClear(&info[L_MODBY], TRUE);
  FreeAndClear(&info[L_MODWITH], TRUE);
  /*
   * Slide the newname into the #2 slot, this screws up all future references
   * to this list.
   */
  if (name) 			/* slide the newname into the #2 slot. */
    SlipInNewName(info, newname);

  return info;
}

/* -------------- List functions. -------------- */

/*	Function Name: ShowListInfo.
 *	Description: shows info on a list.
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM status code.
 */

int ShowListInfo(int argc, char **argv)
{
  struct mqelem *top, *list;

  top = list = GetListInfo(LIST, argv[1], NULL);
  while (list)
    {
      PrintListInfo(list->q_data);
      list = list->q_forw;
    }

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateList
 *	Description: performs the actual update of the list.
 *	Arguments: info - all information needed to update the list.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

static void RealUpdateList(char **info, Bool junk)
{
  int stat;
  char **args;
  struct mqelem *elem = NULL;

  if (!(args = AskListInfo(info, TRUE)))
    {
      Put_message("Aborted.");
      return;
    }

  /*
   * If the new list name is less than 8 characters, make sure it doesn't
   * collide with a username.
   */
  if ((strlen(args[2]) <= 8) &&
      do_mr_query("get_user_account_by_login", 1, args + 1,
		  StoreInfo, &elem) != MR_NO_MATCH)
    {
      char buf[256];

      sprintf(buf, "\nA user by the name `%s' already exists in the database.",
	      args[1]);
      Put_message(buf);
      Loop(QueueTop(elem), FreeInfo);
      FreeQueue(elem);
      if (YesNoQuestion("Do you still want to rename this list to that name",
			FALSE) != TRUE)
	{
	  Put_message("List ** NOT ** Updated.");
	  return;
	}
    }

  if ((stat = do_mr_query("update_list", CountArgs(args), args,
			  NULL, NULL)) != MR_SUCCESS)
    {
      com_err(program_name, stat, " in UpdateList.");
      Put_message("List ** NOT ** Updated.");
    }
  else
    Put_message("List successfully updated.");
}

/*	Function Name: UpdateList
 *	Description: updates the information on a list.
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: DM Status code.
 */

int UpdateList(int argc, char **argv)
{
  struct mqelem *top;

  top = GetListInfo(LIST, argv[1], (char *) NULL);
  QueryLoop(top, NullPrint, RealUpdateList, "Update the list");

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: SetDefaults
 *	Description: sets defaults for AddList function
 *	Arguments: info - the array to add them to.
 *                 name - name of the program to add.
 *	Returns: defaults - the default information.
 */

static char **SetDefaults(char **info, char *name)
{
  info[L_NAME] =     strdup(name);
  info[L_ACTIVE] =   strdup(DEFAULT_ACTIVE);
  info[L_PUBLIC] =   strdup(DEFAULT_PUBLIC);
  info[L_HIDDEN] =   strdup(DEFAULT_HIDDEN);
  info[L_MAILLIST] = strdup(DEFAULT_MAILLIST);
  info[L_GROUP] =    strdup(DEFAULT_GROUP);
  info[L_GID] =      strdup(DEFAULT_GID);
  info[L_NFSGROUP] = strdup(DEFAULT_NFSGROUP);
  info[L_MAILMAN]  = strdup(DEFAULT_MAILMAN);
  info[L_MAILMAN_SERVER] = strdup(DEFAULT_MAILMAN_SERVER);
  info[L_ACE_TYPE] = strdup(DEFAULT_ACE_TYPE);
  info[L_ACE_NAME] = strdup(DEFAULT_ACE_NAME);
  info[L_MEMACE_TYPE] = strdup(DEFAULT_MEMACE_TYPE);
  info[L_MEMACE_NAME] = strdup(DEFAULT_MEMACE_NAME);
  info[L_DESC] =     strdup(DEFAULT_DESCRIPTION);
  info[L_MODTIME] = info[L_MODBY] = info[L_MODWITH] = info[L_END] = NULL;
  return info;
}

/*	Function Name: AddList
 *	Description:
 *	Arguments: argc, argv - name of list in argv[1].
 *	Returns: SUB_ERROR if list not created.
 */

int AddList(int argc, char **argv)
{
  static char *info[MAX_ARGS_SIZE], **add_args;
  int status, ret_code = SUB_NORMAL;
  struct mqelem *elem = NULL;

  if (!ValidName(argv[1]))
    return DM_NORMAL;
  status = do_mr_query("get_list_info", 1, argv + 1, NULL, NULL);
  if (status != MR_NO_MATCH)
    {
      if (status == MR_SUCCESS)
	Put_message("This list already exists.");
      else
	com_err(program_name, status, " in AddList.");
      return SUB_ERROR;
    }

  if (status = mr_access("add_list", L_MODTIME, SetDefaults(info, argv[1]))) 
    {
      com_err(program_name, status, " in AddList.");
      return SUB_ERROR;
    }

  /*
   * If the listname is less than 8 characters, make sure it doesn't
   * collide with a username.
   */
  if ((strlen(argv[1]) <= 8) &&
      do_mr_query("get_user_account_by_login", 1, argv + 1,
		  StoreInfo, &elem) != MR_NO_MATCH)
    {
      char buf[256];

      sprintf(buf, "\nA user by the name `%s' already exists in the database.",
	      argv[1]);
      Put_message(buf);
      Loop(QueueTop(elem), FreeInfo);
      FreeQueue(elem);
      if (YesNoQuestion("Create a list with the same name", FALSE) != TRUE)
	return SUB_ERROR;
    }

  if (!(add_args = AskListInfo(SetDefaults(info, argv[1]), FALSE)))
    {
      Put_message("Aborted.");
      return SUB_ERROR;
    }

  if ((status = do_mr_query("add_list", CountArgs(add_args), add_args,
			    NULL, NULL)) != MR_SUCCESS)
    {
      com_err(program_name, status, " in AddList.");
      Put_message("List Not Created.");
      ret_code = SUB_ERROR;
    }

  if (atoi(add_args[L_MAILMAN]))
    {
      char mailman_address[256], buf[1024];

      status = do_mr_query("get_list_info", 1, add_args, StoreInfo, &elem);
      if (status)
	  com_err(program_name, status, "while retrieving list information.");
      else
	{
	  strcpy(mailman_address, add_args[0]);
	  strcat(mailman_address, "@");
	  strcat(mailman_address, ((char **)elem->q_data)[L_MAILMAN_SERVER]);
	  sprintf(buf, "Add STRING %s to LIST %s", mailman_address,
		  add_args[0]);
	  if (YesNoQuestion(buf, TRUE) == TRUE)
	    {
	      char *args[3];
	      args[0] = add_args[0];
	      args[1] = "STRING";
	      args[2] = mailman_address;
	      
	      status = do_mr_query("add_member_to_list", CountArgs(args), args,
				   NULL, NULL);
	      if (status)
		com_err(program_name, status, "while adding member to list.");
	    }
	}
    }

  FreeInfo(info);
  return ret_code;
}

/*	Function Name: Instructions
 *	Description: This func prints out instruction on manipulating lists.
 *	Arguments: none
 *	Returns: DM Status Code.
 */

int ListHelp(int argc, char **argv)
{
  static char *message[] = {
    "Listmaint handles the creation, deletion, and updating of lists.",
    "A list can be a mailing list, a group list, or both.",
    "The concept behind lists is that a list has an owner",
    "- administrator -  and members.",
    "The administrator of a list may be another list.",
    "The members of a list can be users (login names), other lists,",
    "or address strings.",
    "You can use certain keys to do the following:",
    "    Refresh the screen - Type ctrl-L.",
    "    Escape from a function - Type ctrl-C.",
    "    Suspend the program (temporarily) - Type ctrl-Z.",
    NULL,
  };

  return PrintHelp(message);
}

/*-*-* LISTMAINT UPDATE MENU *-*-*/

/*	Function Name: ListmaintMemberMenuEntry
 *	Description: entry routine into the listmaint member menu.
 *	Arguments: m - the member menu.
 *                 argc, argv - name of the list in argv[1].
 *	Returns: none.
 */

int ListmaintMemberMenuEntry(Menu *m, int argc, char **argv)
{
  char temp_buf[BUFSIZ];
  char *list_name = argv[1];
  int stat;

  if (!ValidName(list_name))
    return DM_QUIT;

  if (*argv[0] == 'a')
    {	/* add_list */
      if (AddList(argc, argv) == SUB_ERROR)
	return DM_QUIT;
      sprintf(temp_buf, "List '%s' created. Do you want to %s", list_name,
	      "change its membership (y/n)? ");
      if (YesNoQuestion(temp_buf, TRUE) != TRUE)
	return DM_QUIT;
    }
  else
    /* All we want to know is if it exists. */
    switch ((stat = do_mr_query("count_members_of_list", 1, argv + 1,
				NULL, NULL)))
      {
      case MR_SUCCESS:
	break;
      case MR_LIST:
	Put_message("This list does not exist.");
	return DM_QUIT;
      case MR_PERM:
	Put_message("You are not allowed to view this list.");
	break;
      default:
	com_err(program_name, stat, " in get_list_info");
	return DM_QUIT;
      }

  sprintf(temp_buf, "Change/Display membership of '%s'", list_name);
  m->m_title = strdup(temp_buf);
  strcpy(current_list, list_name);
  return DM_NORMAL;
}

/*	Function Name: ListmaintMemberMenuExit
 *	Description: This is the function called when the member menu is
 *                   exited, it frees the memory that is storing the name.
 *	Arguments: m - the menu
 *	Returns: DM_NORMAL
 */

int ListmaintMemberMenuExit(Menu *m)
{
  free(m->m_title);
  strcpy(current_list, "");
  return DM_NORMAL;
}

/*	Function Name: ListMembersByType
 *	Description: This function lists the users of a list by type.
 *	Arguments: type - the type of the list "USER", "LIST", or "STRING".
 *		   tags - whether or not to display tags
 *	Returns: none.
 *      NOTE: if type is NULL, all lists members are listed.
 */

void ListMembersByType(char *type, int tags)
{
  char temp_buf[BUFSIZ];
  int status;
  char *args[10];

  args[0] = current_list;
  args[1] = NULL;

  found_some = FALSE;
  if ((status = do_mr_query(tags ? "get_tagged_members_of_list" :
			    "get_members_of_list", CountArgs(args),
			    args, PrintByType, type)))
    com_err(program_name, status, " in ListMembersByType");
  if (!found_some)
    {
      if (!type)
	Put_message("List is empty (no members).");
      else
	{
	  sprintf(temp_buf, "No %s Members", type);
	  Put_message(temp_buf);
	}
    }
}

/*	Function Name: ListAllMembers
 *	Description: lists all members of the current list.
 *	Arguments:
 *	Returns: DM_NORMAL
 */

int ListAllMembers(int argc, char **argv)
{
  ListMembersByType(NULL, 0);
  return DM_NORMAL;
}

/*	Function Name: ListUserMembers
 *	Description: This function lists all members of a list of type "USER".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ListUserMembers(int argc, char **argv)
{
  ListMembersByType("USER", 0);
  return DM_NORMAL;
}

/*	Function Name: ListListMembers
 *	Description: This function lists all members of a list of type "LIST".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ListListMembers(int argc, char **argv)
{
  ListMembersByType("LIST", 0);
  return DM_NORMAL;
}

/*	Function Name: ListStringMembers
 *	Description:This function lists all members of a list of type "STRING".
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ListStringMembers(int argc, char **argv)
{
  ListMembersByType("STRING", 0);
  return DM_NORMAL;
}

/*	Function Name: GetMemberInfo
 *	Description: This function gets the information needed to
 *                   add or delete a user from a list.
 *	Arguments: argc, argv - standard.
 *                 action - name of the action to be performed either
 *                          "add" or "delete".
 *                 ret_argc, ret_argv - the returned value of argc and argv.
 *	Returns: SUB_ERROR or SUB_NORMAL.
 */

int GetMemberInfo(char *action, char **ret_argv)
{
  char temp_buf[BUFSIZ];

  ret_argv[LM_LIST] = strdup(current_list);

  ret_argv[LM_TYPE] = strdup("user");
  if (GetTypeFromUser("Type of member", "member", &ret_argv[LM_TYPE]) ==
      SUB_ERROR)
    return SUB_ERROR;

  sprintf(temp_buf, "Name of %s to %s", ret_argv[LM_TYPE], action);
  ret_argv[LM_MEMBER] = strdup(user);
  if (GetValueFromUser(temp_buf, &ret_argv[LM_MEMBER]) == SUB_ERROR)
    return SUB_ERROR;
  ret_argv[LM_END] = NULL;		/* NULL terminate this list. */

  if (strcasecmp(ret_argv[LM_TYPE], "string") &&
      !ValidName(ret_argv[LM_MEMBER]))
    {
      FreeInfo(ret_argv);
      return SUB_ERROR;
    }
  return SUB_NORMAL;
}

/*	Function Name: AddMember
 *	Description: This function adds a member to a list.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int AddMember(int argc, char **argv)
{
  char *args[10], temp_buf[BUFSIZ], *p;
  int status;
  struct mqelem *mailhubs, *elem;

  if (GetMemberInfo("add", args) == SUB_ERROR)
    return DM_NORMAL;

  if (!strcmp(args[LM_TYPE], "STRING"))
    {
      status = mrcl_validate_string_member(args[LM_MEMBER]);
      if (status != MRCL_SUCCESS)
	Put_message(mrcl_get_message());
      if (status == MRCL_REJECT)
	return DM_NORMAL;
    }
  else if (!strcmp(args[LM_TYPE], "KERBEROS"))
    {
      char *canon;

      status = mrcl_validate_kerberos_member(args[LM_MEMBER], &canon);
      if (mrcl_get_message())
	Put_message(mrcl_get_message());
      if (status == MRCL_REJECT)
	return DM_NORMAL;
      free(args[LM_MEMBER]);
      args[LM_MEMBER] = canon;
    }
  else if (!strcmp(args[LM_TYPE], "MACHINE"))
    {
      char *canon;
      canon = canonicalize_hostname(strdup(args[LM_MEMBER]));
      free(args[LM_MEMBER]);
      args[LM_MEMBER] = canon;
    }
  else if (!strcmp(args[LM_TYPE], "ID"))
    {
      char *canon_type, *canon_member;

      status = mrcl_validate_id_member(args[LM_TYPE], &canon_type, args[LM_MEMBER], &canon_member);
      if (mrcl_get_message())
	Put_message(mrcl_get_message());
      if (status == MRCL_REJECT)
	return DM_NORMAL;
      free(args[LM_TYPE]);
      args[LM_TYPE] = canon_type;
      free(args[LM_MEMBER]);
      args[LM_MEMBER] = canon_member;
    }

  if ((status = do_mr_query("add_member_to_list", CountArgs(args), args,
			    NULL, NULL)) != MR_SUCCESS)
    {
      if (status == MR_EXISTS)
	{
	  sprintf(temp_buf, "The %s %s is already a member of LIST %s.",
		  args[LM_TYPE], args[LM_MEMBER], args[LM_LIST]);
	  Put_message(temp_buf);
	}
      else
	com_err(program_name, status, " in AddMember");
    }

  FreeInfo(args);
  return DM_NORMAL;
}

/*	Function Name: DeleteMember
 *	Description: This function deletes a member from a list.
 *	Arguments: none.
 *	Returns: DM_NORMAL
 */

int DeleteMember(int argc, char **argv)
{
  char *args[10];
  int status;

  if (GetMemberInfo("delete", args) == SUB_ERROR)
    return DM_NORMAL;

  if (Confirm("Are you sure you want to delete this member?"))
    {
      if ((status = do_mr_query("delete_member_from_list", CountArgs(args),
				args, NULL, NULL)))
	{
	  if ((status == MR_STRING || status == MR_NO_MATCH) &&
	      !strcmp(args[LM_TYPE], "KERBEROS"))
	    {
	      char *canon;
	      mrcl_validate_kerberos_member(args[LM_MEMBER], &canon);
	      if (mrcl_get_message())
		{
		  free(args[LM_MEMBER]);
		  args[LM_MEMBER] = canon;
		  if (do_mr_query("delete_member_from_list", CountArgs(args),
				  args, NULL, NULL) == MR_SUCCESS)
		    {
		      Put_message(mrcl_get_message());
		      status = MR_SUCCESS;
		    }
		}
	    }
	  else if ((status == MR_MACHINE || status == MR_NO_MATCH) &&
		   !strcmp(args[LM_TYPE], "MACHINE"))
	    {
	      char *canon;
	      canon = canonicalize_hostname(args[LM_MEMBER]);
	      args[LM_MEMBER] = canon;
	      if (do_mr_query("delete_member_from_list", CountArgs(args),
			      args, NULL, NULL) == MR_SUCCESS)
		status = MR_SUCCESS;
	    }
	}
      if (status)
	com_err(program_name, status, " in DeleteMember");
      else
	Put_message("Deletion Completed.");
    }
  else
    Put_message("Deletion has been Aborted.");

  FreeInfo(args);
  return DM_NORMAL;
}

/*	Function Name: TagMember
 *	Description: Add a tag to a list member
 *	Arguments:
 *	Returns: DM_NORMAL
 */

int TagMember(int argc, char **argv)
{
  char *args[10];
  int status;

  if (GetMemberInfo("tag", args) == SUB_ERROR)
    return DM_NORMAL;

  args[LM_TAG] = strdup("");
  if (GetValueFromUser("Tag" , &args[LM_TAG]) == SUB_ERROR)
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  args[LM_TAG_END] = NULL;		/* NULL terminate this list. */

  if ((status = do_mr_query("tag_member_of_list", CountArgs(args),
			    args, NULL, NULL)))
    com_err(program_name, status, " in TagMember");

  FreeInfo(args);
  return DM_NORMAL;
}

/*	Function Name: ListAllMembers
 *	Description: lists all members of the current list.
 *	Arguments:
 *	Returns: DM_NORMAL
 */

int ListMembersWithTags(int argc, char **argv)
{
  ListMembersByType(NULL, 1);
  return DM_NORMAL;
}

/*	Function Name: InterRemoveItemFromLists
 *	Description: This function allows interactive removal of an item
 *                   (user, string, list) for all list  that it is on.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 *      NOTES: QueryLoop() does not work here because info does not have
 *             enough information in it to delete the member from the list.
 */

int InterRemoveItemFromLists(int argc, char **argv)
{
  int status;
  char *type, *name, *args[10], buf[BUFSIZ];
  struct mqelem *top, *elem;

  type = strdup("USER");
  if (GetTypeFromUser("Type of member", "member", &type) == SUB_ERROR)
    return DM_NORMAL;

  sprintf(buf, "Name of %s", type);
  name = strdup(user);
  if (GetValueFromUser(buf, &name) == SUB_ERROR)
    return DM_NORMAL;

  if (!ValidName(name))
    return DM_NORMAL;

  top = elem = GetListInfo(GLOM, type, name);

  while (elem)
    {
      char line[BUFSIZ];
      char **info = elem->q_data;
      sprintf(line, "Delete %s %s from the list \"%s\" (y/n/q)? ", type,
	      name, info[GLOM_NAME]);
      switch (YesNoQuitQuestion(line, FALSE))
	{
	case TRUE:
	  Put_message("deleting...");
	  args[DM_LIST] = info[GLOM_NAME];
	  args[DM_TYPE] = type;
	  args[DM_MEMBER] = name;
	  if (!strcmp("MACHINE", type))
	      args[DM_MEMBER] = canonicalize_hostname(strdup(name));
	  if ((status = do_mr_query("delete_member_from_list", 3, args,
				    NULL, NULL)))
	    {
	      /* should probabally check to delete list. */
	      com_err(program_name, status, " in delete_member");
	    }
	  break;
	case FALSE:
	  break;
	default:
	  Put_message("Aborting...");
	  FreeQueue(top);
	  return DM_NORMAL;
	}
      elem = elem->q_forw;
    }
  FreeQueue(top);
  return DM_NORMAL;
}

/*-*-* LIST MENU *-*-*/

/*	Function Name: ListByMember
 *	Description: This gets all lists that a given member is a member of.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int ListByMember(int argc, char **argv)
{
  char buf[BUFSIZ], temp_buf[BUFSIZ], *type, *name, **info;
  Bool maillist, group, neither;
  struct mqelem *top, *elem;

  type = strdup("USER");
  if (GetTypeFromUser("Type of member", "member", &type) == SUB_ERROR)
    return DM_NORMAL;

  sprintf(buf, "Name of %s", type);
  name = strdup(user);
  if (GetValueFromUser(buf, &name) == SUB_ERROR)
    return DM_NORMAL;

  switch (YesNoQuestion("Do you want a recursive search (y/n)", TRUE))
    {
    case TRUE:
      sprintf(temp_buf, "R%s", type);	/* "USER" to "RUSER" etc. */
      free(type);
      type = strdup(temp_buf);
      break;
    case FALSE:
      break;
    default:
      return DM_NORMAL;
    }

  if ((maillist = YesNoQuestion("Show Lists that are Maillists (y/n) ?",
				TRUE)) == -1)
    return DM_NORMAL;
  if ((group = YesNoQuestion("Show Lists that are Groups (y/n) ?",
			     TRUE)) == -1)
    return DM_NORMAL;

  if ((neither = YesNoQuestion("Show Lists that are neither Maillists nor Groups (y/n) ?",
			       TRUE)) == -1)
    return DM_NORMAL;

  elem = top = GetListInfo(GLOM, type, name);

  while (elem)
    {
      info = elem->q_data;
      if ((maillist == TRUE && !strcmp(info[GLOM_MAILLIST], "1")) ||
	  (group == TRUE && !strcmp(info[GLOM_GROUP], "1")))
	Put_message(info[GLOM_NAME]);
      if (neither == TRUE && !strcmp(info[GLOM_MAILLIST], "0") &&
	   !strcmp(info[GLOM_GROUP], "0"))
	Put_message(info[GLOM_NAME]);
      elem = elem->q_forw;
    }
  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: ListByAdministrator
 *	Description: This function prints all lists which a given user or
 *                   group administers.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int ListByAdministrator(int argc, char **argv)
{
  char buf[BUFSIZ], temp_buf[BUFSIZ], *type, *name;
  struct mqelem *top;

  type = strdup("USER");
  if (GetTypeFromUser("Type of member", "member", &type) == SUB_ERROR)
    return DM_NORMAL;

  sprintf(buf, "Name of %s", type);
  name = strdup(user);
  if (GetValueFromUser(buf, &name) == SUB_ERROR)
    return DM_NORMAL;

  switch (YesNoQuestion("Do you want a recursive search (y/n)", FALSE))
    {
    case TRUE:
      sprintf(temp_buf, "R%s", type);	/* "USER" to "RUSER" etc. */
      free(type);
      type = strdup(temp_buf);
      break;
    case FALSE:
      break;
    default:
      return DM_NORMAL;
    }

  top = GetListInfo(ACE_USE, type, name);
  Loop(top, PrintListAce);

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: ListAllPublicMailLists
 *	Description: This function lists all public mailing lists.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ListAllPublicMailLists(int argc, char **argv)
{
  int status;
  static char *args[] = {
    "TRUE",			/* active */
    "TRUE",			/* public */
    "FALSE",			/* hidden */
    "TRUE",			/* maillist */
    "DONTCARE",			/* group. */
  };

  if (YesNoQuestion("This query will take a while. Do you wish to continue?",
		    TRUE) == TRUE)
    {
      if ((status = do_mr_query("qualified_get_lists", 5, args,
				Print, NULL)) != MR_SUCCESS)
	com_err(program_name, status, " in ListAllGroups");
    }

  return DM_NORMAL;
}
