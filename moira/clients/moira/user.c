/* $Id$
 *
 *	This is the file user.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: Functions for manipulating user information.
 *
 *	Created: 	5/9/88
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

RCSID("$HeadURL$ $Id$");

void CorrectCapitalization(char **name);
char **AskUserInfo(char **info, Bool name);
struct mqelem *GetUserInfo(int type, char *name1, char *name2);
static void PrintLogin(char **info);
struct mqelem *GetUserBySponsor(char *type, char *name);

#define LOGIN 0
#define UID   1
#define BY_NAME  2
#define CLASS 3
#define ID 4

#ifdef ATHENA
#define DEFAULT_SHELL "/bin/athena/bash"
#else
#define DEFAULT_SHELL "/bin/bash"
#endif
#define DEFAULT_CLASS "?"

#define DEFAULT_WINCONSOLESHELL "cmd"
#define DEFAULT_WINHOMEDIR "[DFS]"
#define DEFAULT_WINPROFILEDIR "[DFS]"

/*	Function Name: UserState
 *	Description: Convert a numeric state into a descriptive string.
 *	Arguments: state value
 *	Returns: pointer to statically allocated string.
 */

static char *states[] = {
  "Registerable (0)",
  "Active (1)",
  "Half Registered (2)",
  "Deleted (3)",
  "Not registerable (4)",
  "Enrolled/Registerable (5)",
  "Enrolled/Not Registerable (6)",
  "Half Enrolled (7)",
  "Registerable, Kerberos only (8)",
  "Active, Kerberos only (9)"
};

static char *UserState(int state)
{
  static char buf[BUFSIZ];

  if (state < 0 || state >= US_END)
    {
      sprintf(buf, "Unknown (%d)", state);
      return buf;
    }
  return states[state];
}


/*	Function Name: PrintUserName
 *	Description: Print name of a user.
 *	Arguments: info - the information about a user.
 *	Returns: none.
 */

static void PrintUserName(char **info)
{
  char buf[BUFSIZ], print_buf[BUFSIZ];
  sprintf(buf, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
  sprintf(print_buf, "%-40s User Name: %s", buf, info[U_NAME]);
  Put_message(print_buf);
}

/*	Function Name: PrintUserInfo
 *	Description: Prints Information about a user.
 *	Arguments: info - an argument list with the user information
 *                          in it.
 *	Returns: none
 */

static void PrintUserInfo(char **info)
{
  char name[BUFSIZ], buf[BUFSIZ], sponsor[BUFSIZ];
  int status;

  sprintf(name, "%s, %s %s", info[U_LAST], info[U_FIRST], info[U_MIDDLE]);
  sprintf(buf, "Login name: %-20s Full name: %s", info[U_NAME], name);
  Put_message(buf);
  sprintf(buf, "User id: %-23s Login shell: %-10s",
	  info[U_UID], info[U_SHELL]);
  Put_message(buf);
  sprintf(buf, "Class: %-25s Windows Console Shell: %-10s",
	  info[U_CLASS], info[U_WINCONSOLESHELL]);
  Put_message(buf);
  sprintf(sponsor, "%s %s", info[U_SPONSOR_TYPE], info[U_SPONSOR_NAME]);
  sprintf(buf, "Sponsor: %-23s Expiration date: %s", sponsor,  info[U_EXPIRATION]);
  Put_message(buf);
  sprintf(buf, "Alternate Email: %s", info[U_ALT_EMAIL]);
  Put_message(buf);
  sprintf(buf, "Alternate Phone: %s", info[U_ALT_PHONE]);
  Put_message(buf);
  sprintf(buf, "Account is: %-20s MIT ID number: %s",
	  UserState(atoi(info[U_STATE])), info[U_MITID]);
  Put_message(buf);
  sprintf(buf, "Windows Home Directory: %s", info[U_WINHOMEDIR]);
  Put_message(buf);
  sprintf(buf, "Windows Profile Directory: %s", info[U_WINPROFILEDIR]);
  Put_message(buf);
  status = atoi(info[U_STATE]);
  if (status == 0 || status == 2)
    {
      sprintf(buf, "User %s secure Account Coupon to register",
	      atoi(info[U_SECURE]) ? "needs" : "does not need");
      Put_message(buf);
    }
  sprintf(buf, "Comments: %s", info[U_COMMENT]);
  Put_message(buf);
  sprintf(buf, "Created  by %s on %s.", info[U_CREATOR], info[U_CREATED]);
  Put_message(buf);
  sprintf(buf, MOD_FORMAT, info[U_MODBY], info[U_MODTIME], info[U_MODWITH]);
  Put_message(buf);
}

/*	Function Name: SetUserDefaults
 *	Description: Sets the default values for add user.
 *	Arguments: info - a blank user info array of char *'s.
 *	Returns: args - the filled info structure.
 */

static char **SetUserDefaults(char **info)
{
  info[U_NAME] = strdup(UNIQUE_LOGIN);
  info[U_UID] = strdup(UNIQUE_UID);
  info[U_SHELL] = strdup(DEFAULT_SHELL);
  info[U_WINCONSOLESHELL] = strdup(DEFAULT_WINCONSOLESHELL);
  info[U_LAST] = strdup(DEFAULT_NONE);
  info[U_FIRST] = strdup(DEFAULT_NONE);
  info[U_MIDDLE] = strdup(DEFAULT_NONE);
  info[U_STATE] = strdup(DEFAULT_NO);
  info[U_MITID] = strdup(DEFAULT_NONE);
  info[U_CLASS] = strdup(DEFAULT_CLASS);
  info[U_COMMENT] = strdup("");
  info[U_SIGNATURE] = strdup("");
  info[U_SECURE] = strdup("0");
  info[U_WINHOMEDIR] = strdup(DEFAULT_WINHOMEDIR);
  info[U_WINPROFILEDIR] = strdup(DEFAULT_WINPROFILEDIR);
  info[U_SPONSOR_TYPE] = strdup("NONE");
  info[U_SPONSOR_NAME] = strdup("NONE");
  info[U_EXPIRATION] = strdup("");
  info[U_ALT_EMAIL] = strdup("");
  info[U_ALT_PHONE] = strdup("");
  info[U_MODTIME] = info[U_MODBY] = info[U_MODWITH] = info[U_END] = NULL;
  info[U_CREATED] = info[U_CREATOR] = NULL;
  return info;
}


/* Check that the supplied name follows the capitalization rules, and
 * offer to correct it if not.
 */

void CorrectCapitalization(char **name)
{
  char temp_buf[BUFSIZ], fixname[BUFSIZ];

  strcpy(fixname, *name);
  FixCase(fixname);
  if (strcmp(fixname, *name))
    {
      Put_message("You entered a name which does not follow the capitalization conventions.");
      sprintf(temp_buf, "Correct it to \"%s\"", fixname);
      if (YesNoQuestion(temp_buf, 0) == TRUE)
	{
	  free(*name);
	  *name = strdup(fixname);
	}
    }
}


/*	Function Name: AskUserInfo.
 *	Description: This function askes the user for information about a
 *                   machine and saves it into a structure.
 *	Arguments: info - a pointer the the structure to put the info into.
 *                 flags - Flags asking us which info we want.
 *	Returns: the args to pass to the query.
 *      NOTES: the return args are not necessarily in the correct order to
 *             use the #defined names (e.g args[UID] is not the uid anymore).
 */

char **AskUserInfo(char **info, Bool name)
{
  int state;
  char temp_buf[BUFSIZ], *newname;

  if (name)
    {
      sprintf(temp_buf, "\nChanging Attributes of user %s.\n", info[U_NAME]);
      Put_message(temp_buf);
    }
  else
    {
      struct mqelem *elem = NULL;
      char *argv[3];

      if (GetValueFromUser("User's last name", &info[U_LAST]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_LAST]);
      if (GetValueFromUser("User's first name", &info[U_FIRST]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_FIRST]);
      if (GetValueFromUser("User's middle name", &info[U_MIDDLE]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_MIDDLE]);
      argv[0] = info[U_FIRST];
      argv[1] = info[U_LAST];
      if (do_mr_query("get_user_account_by_name", 2, argv,
		      StoreInfo, &elem) == MR_SUCCESS)
	{
	  Put_message("A user by that name already exists in the database.");
	  Loop(QueueTop(elem), PrintUserInfo);
	  Loop(QueueTop(elem), FreeInfo);
	  FreeQueue(elem);
	  if (YesNoQuestion("Add new user anyway", TRUE) != TRUE)
	    return NULL;
	}
    }
  if (name)
    {
      newname = strdup(info[U_NAME]);
      if (GetValueFromUser("The new login name for this user", &newname) ==
	  SUB_ERROR)
	return NULL;
    }
  else if (GetValueFromUser("Login name for this user", &info[U_NAME]) ==
	   SUB_ERROR)
    return NULL;

  strcpy(temp_buf, info[U_UID]);
  if (GetValueFromUser("User's UID", &info[U_UID]) == SUB_ERROR)
    return NULL;
  if (strcmp(info[U_UID], UNIQUE_UID) && strcmp(info[U_UID], temp_buf))
    {
      struct mqelem *elem = NULL;
      if (do_mr_query("get_user_account_by_uid", 1, &info[U_UID],
		      StoreInfo, &elem) == MR_SUCCESS)
	{
	  Put_message("A user with that uid already exists in the database.");
	  Loop(QueueTop(elem), PrintUserInfo);
	  Loop(QueueTop(elem), FreeInfo);
	  FreeQueue(elem);
	  if (YesNoQuestion("Add new user anyway", TRUE) != TRUE)
	    return NULL;
	}
    }

  if (GetValueFromUser("User's shell", &info[U_SHELL]) == SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Windows console shell", &info[U_WINCONSOLESHELL])
      == SUB_ERROR)
    return NULL;
  if (name)
    {
      if (GetValueFromUser("User's last name", &info[U_LAST]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_LAST]);
      if (GetValueFromUser("User's first name", &info[U_FIRST]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_FIRST]);
      if (GetValueFromUser("User's middle name", &info[U_MIDDLE]) == SUB_ERROR)
	return NULL;
      CorrectCapitalization(&info[U_MIDDLE]);
    }
  while (1)
    {
      int i;
      if (GetValueFromUser("User's status (? for help)", &info[U_STATE]) ==
	  SUB_ERROR)
	return NULL;
      if (isdigit(info[U_STATE][0]))
	break;
      Put_message("Valid status numbers:");
      for (i = 0; i < US_END; i++)
	{
	  sprintf(temp_buf, "  %d: %s", i, states[i]);
	  Put_message(temp_buf);
	}
    }
  if (GetValueFromUser("User's MIT ID number", &info[U_MITID]) == SUB_ERROR)
    return NULL;
  RemoveHyphens(info[U_MITID]);
  if (GetTypeFromUser("User's MIT Year (class)", "class", &info[U_CLASS]) ==
      SUB_ERROR)
    return NULL;
  if (GetValueFromUser("Comments", &info[U_COMMENT]) == SUB_ERROR)
    return NULL;

  if (GetValueFromUser("Windows Home Directory", &info[U_WINHOMEDIR]) ==
      SUB_ERROR)
    return NULL;

  if (GetValueFromUser("Windows Profile Directory", &info[U_WINPROFILEDIR]) ==
      SUB_ERROR)
    return NULL;

  if (GetTypeFromUser("User's sponsor type", "ace_type", &info[U_SPONSOR_TYPE])
      == SUB_ERROR)
    return NULL;
  if (strcmp(info[U_SPONSOR_TYPE], "NONE") &&
      GetValueFromUser("Sponsor's Name", &info[U_SPONSOR_NAME]) == SUB_ERROR)
    return NULL;

  if (GetValueFromUser("Expiration date", &info[U_EXPIRATION]) == SUB_ERROR)
    return NULL;

  if (GetValueFromUser("Alternate Email", &info[U_ALT_EMAIL]) == SUB_ERROR)
    return NULL;

  if (GetValueFromUser("Alternate Phone", &info[U_ALT_PHONE]) == SUB_ERROR)
    return NULL;

  state = atoi(info[U_STATE]);
  if (!name || state == 0 || state == 2)
    {
      if (YesNoQuestion("User needs secure Account Coupon to register",
			atoi(info[U_SECURE]) ? TRUE : FALSE) == FALSE)
	{
	  free(info[U_SECURE]);
	  info[U_SECURE] = strdup("0");
	}
      else
	{
	  free(info[U_SECURE]);
	  info[U_SECURE] = strdup("1");
	}
    }

  info[U_SIGNATURE] = strdup("");

  FreeAndClear(&info[U_MODTIME], TRUE);
  FreeAndClear(&info[U_MODBY], TRUE);
  FreeAndClear(&info[U_MODWITH], TRUE);

  /*
   * Slide the newname into the #2 slot, this screws up all future references
   * to this list, since we slip the pointer into a info list it gets freed
   * when the rest of the list gets freed.
   */
  if (name)
    SlipInNewName(info, newname);

  return info;
}

/*	Function Name: GetUserInfo
 *	Description: Stores the user information in a queue.
 *	Arguments: type - type of field given to get info, one of:
 *                        LOGIN, UID, BY_NAME, CLASS.
 *                 name1 - name of thing specified by type (wildcards okay)
 *                 name2 - other name, only used in get user by first and last.
 *                         (wildcards okay).
 *	Returns: the first element of the queue containing the user info.
 *
 */

struct mqelem *GetUserInfo(int type, char *name1, char *name2)
{
  char *args[2];
  int status;
  struct mqelem *elem = NULL;

  switch (type)
    {
    case LOGIN:
      args[0] = name1;
      if ((status = do_mr_query("get_user_account_by_login", 1, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status,
		  " when attempting to get_user_account_by_login.");
	  return NULL;
	}
      break;
    case UID:
      args[0] = name1;
      if ((status = do_mr_query("get_user_account_by_uid", 1, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status,
		  " when attempting to get_user_account_by_uid.");
	  return NULL;
	}
      break;
    case BY_NAME:
      args[0] = name1;
      args[1] = name2;
      if ((status = do_mr_query("get_user_account_by_name", 2, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status,
		  " when attempting to get_user_account_by_name.");
	  return NULL;
	}
      break;
    case CLASS:
      args[0] = name1;
      if ((status = do_mr_query("get_user_account_by_class", 1, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status,
		  " when attempting to get_user_account_by_class.");
	  return NULL;
	}
      break;
    case ID:
      args[0] = name1;
      if ((status = do_mr_query("get_user_account_by_id", 1, args,
				StoreInfo, &elem)))
	{
	  com_err(program_name, status,
		  " when attempting to get_user_account_by_id.");
	  return NULL;
	}
      break;
    }
  return QueueTop(elem) ;
}

/*	Function Name: AddNewUser
 *	Description: Adds a new user to the database.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int AddNewUser(int argc, char **argv)
{
  int status;
  char **args, *info[MAX_ARGS_SIZE];

  if (!(args = AskUserInfo(SetUserDefaults(info), FALSE)))
    {
      Put_message("Aborted.");
      return DM_NORMAL;
    }
  if ((status = do_mr_query("add_user_account", CountArgs(args),
			    args, NULL, NULL)))
    com_err(program_name, status, " in add_user_account");
  else
    Put_message("New user added to database.");
  FreeInfo(args);
  return DM_NORMAL;
}


/*	Function Name: GetLoginName
 *	Description: Asks the user for a login name and reserves
 *                   it with kerberous.
 *	Arguments: none.
 *	Returns: a malloced login name for the user.
 */

static char *GetLoginName(void)
{
  char *name;

  name = strdup("");
  if (GetValueFromUser("Login name for this user? ", &name) == SUB_ERROR)
    return NULL;
  Put_message("KERBEROS code not added, did not reserve name with kerberos.");
  return name;
}


/*	Function Name: ChooseUser
 *	Description: Choose a user from a list and return the uid.
 *	Arguments: top - a queue of user information.
 *	Returns: uid - the malloced uid of the user that was chosen.
 */

static char *ChooseUser(struct mqelem *elem)
{
  while (elem)
    {
      char **info = elem->q_data;
      PrintUserInfo(info);
      switch (YesNoQuitQuestion("Is this the user you want (y/n/q)", FALSE))
	{
	case TRUE:
	  return strdup(info[U_UID]);
	case FALSE:
	  break;
	default:		/* quit or ^C. */
	  return NULL;
	}
      elem = elem->q_forw;
    }
  return NULL;
}

/*	Function Name: GetUidNumberFromName
 *	Description: Gets the users uid number, from the name.
 *	Arguments: none.
 *	Returns: uid - a malloced string containing the uid.
 */

static char *GetUidNumberFromName(void)
{
  char *args[5], *uid, first[BUFSIZ], last[BUFSIZ];
  int status;
  struct mqelem *top = NULL;

  if (!Prompt_input("First Name: ", first, BUFSIZ))
    return NULL;
  if (!Prompt_input("Last  Name: ", last, BUFSIZ))
    return NULL;
  FixCase(first);
  FixCase(last);

  args[0] = first;
  args[1] = last;

  switch ((status = do_mr_query("get_user_account_by_name", 2, args,
				StoreInfo, &top)))
    {
    case MR_SUCCESS:
      break;
    case MR_NO_MATCH:
      Put_message("There is no user in the database with that name.");
      return NULL;
    default:
      com_err(program_name, status, " in get_account_user_by_name.");
      return NULL;
    }

  top = QueueTop(top);
  if (QueueCount(top) == 1) /* This is a unique name. */
    {
      char **info = top->q_data;
      Put_message("User ID Number retrieved for the user: ");
      Put_message("");
      PrintUserName(info);
      uid = strdup(info[U_UID]);
      FreeQueue(top);
      return strdup(uid);
    }

  Put_message("That name is not unique, choose the user that you want.");
  uid = ChooseUser(top);
  FreeQueue(top);
  return uid;
}

/*	Function Name: SetUserPassword
 *	Description: Set the new kerberos password for this user.
 *	Arguments: name - kerberos principle name for this user, (login name).
 *	Returns: none.
 */

static void SetUserPassword(char *name)
{
  name = name;			/* make saber happy. */
  Put_message("Kerberos password not changed, code non-existant.");
  /* clever message to call account_admin, if this fails. */
}

/*	Function Name:  GiveBackLogin
 *	Description: Gives back previously reserved kerberous principle.
 *	Arguments: name - principle to give back.
 *	Returns: void.
 */

static void GiveBackLogin(char *name)
{
  name = name;			/* make saber happy. */
  Put_message("kerberos code not implemented, name not given back.");
  /* send mail to db maintainer if this fails. */
}

/*	Function Name: RegisterUser
 *	Description: This function registers a user.
 *	Arguments: none.
 *	Returns: DM_NORMAL.
 */

int RegisterUser(int argc, char **argv)
{
  char *args[MAX_ARGS_SIZE];
  char *login, *potype = NULL;
  char temp_buf[BUFSIZ];
  int status, i;

  for (i = 0; i < MAX_ARGS_SIZE; i++)
    args[i] = NULL;

  Put_message("This function has NO kerberos support, so strange things");
  Put_message("may happen if you use it to register a user.");

  switch (YesNoQuestion("Do you know the users UID Number (y/n)", FALSE))
    {
    case TRUE:
      Prompt_input("What is the UID number of the user? ", temp_buf, BUFSIZ);
      args[0] = strdup(temp_buf);
      break;
    case FALSE:
      if (!(args[0] = GetUidNumberFromName()))
	return DM_NORMAL;
      break;
    default:
      return DM_NORMAL;
    }

  sprintf(temp_buf, "u%s", args[0]);
  login = strdup(temp_buf);
  if (GetValueFromUser("Login name for this user? ", &login) == SUB_ERROR)
    {
      args[1] = login;
      FreeInfo(args);	   /* This work because the NULL temination is ok. */
      return DM_NORMAL;
    }
  Put_message("KERBEROS code not added, did not reserve name with kerberos.");
  args[1] = login;
  
  sprintf(temp_buf, "EXCHANGE");
  potype = strdup(temp_buf);
  if (GetValueFromUser("P.O. Box Type for this user? ", &potype) == SUB_ERROR)
    {
      args[2] = potype;
      FreeInfo(args);
      return DM_NORMAL;
    }
  if (strcmp(potype, "POP") && strcmp(potype, "IMAP") && strcmp(potype, "EXCHANGE"))
    {
      sprintf(temp_buf, "Unknown P.O. Box type.");
      Put_message(temp_buf);
      FreeInfo(args);
      return DM_NORMAL;
    }
  args[2] = potype;
  args[3] = NULL;

  switch ((status = do_mr_query("register_user", CountArgs(args),
				args, NULL, NULL)))
    {
    case MR_SUCCESS:
      sprintf(temp_buf, "User %s successfully registered.", login);
      Put_message(temp_buf);
      SetUserPassword(login);
      break;
    case MR_IN_USE:
      GiveBackLogin(login);
      sprintf(temp_buf, "The username %s is already in use.", login);
      Put_message(temp_buf);
      break;
    default:
      com_err(program_name, status, " in register_user");
      break;
    }
  FreeInfo(args);
  return DM_NORMAL;
}

/*	Function Name: RealUpdateUser
 *	Description: actuall updates the user information.
 *	Arguments: info - all current information for the user fields.
 *                 junk - an UNUSED boolean.
 *	Returns: none.
 */

static void RealUpdateUser(char **info, Bool junk)
{
  int status;
  char error_buf[BUFSIZ];
  char **args = AskUserInfo(info, TRUE);

  if (!args)
    {
      Put_message("Aborted.");
      return;
    }
  if ((status = do_mr_query("update_user_account", CountArgs(args),
			    args, NULL, NULL)))
    {
      com_err(program_name, status, " in ModifyFields");
      sprintf(error_buf, "User %s not updated due to errors.", info[NAME]);
      Put_message(error_buf);
    }
}

/*	Function Name: UpdateUser
 *	Description: Modify some of the information about a user.
 *	Arguments: argc, argv - login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int UpdateUser(int argc, char **argv)
{
  struct mqelem *elem;

  elem = GetUserInfo(LOGIN, argv[1], NULL);
  QueryLoop(elem, NullPrint, RealUpdateUser, "Update the user");

  FreeQueue(elem);
  return DM_NORMAL;
}

/*	Function Name: RealDeactivateUser
 *	Description: sets the user's status to 3.
 *	Arguments: info - all current information for the user fields
 *		   one_item - indicates the user hasn't been queried yet
 *	Returns: none.
 */

static void RealDeactivateUser(char **info, Bool one_item)
{
  int status;
  char txt_buf[BUFSIZ];
  char *qargs[2], **args;
  struct mqelem *elem = NULL;

  if (one_item)
    {
      sprintf(txt_buf, "Deactivate user %s (y/n)", info[NAME]);
      if (YesNoQuestion(txt_buf, FALSE) != TRUE)
	return;
    }

  qargs[0] = info[NAME];
  qargs[1] = "3";
  if ((status = do_mr_query("update_user_status", 2, qargs, NULL, NULL)))
    {
      com_err(program_name, status, " in update_user_status");
      sprintf(txt_buf, "User %s not deactivated due to errors.", info[NAME]);
      Put_message(txt_buf);
    }
  else if (YesNoQuestion("Also deactivate matching list and filesystem (y/n)",
			 FALSE) == TRUE)
    {
      status = do_mr_query("get_list_info", 1, &(info[NAME]), StoreInfo,
			   &elem);
      if (status == MR_SUCCESS)
	{
	  args = QueueTop(elem)->q_data;
	  free(args[L_ACTIVE]);
	  args[L_ACTIVE] = strdup("0");
	  FreeAndClear(&args[L_MODTIME], TRUE);
	  FreeAndClear(&args[L_MODBY], TRUE);
	  FreeAndClear(&args[L_MODWITH], TRUE);
	  SlipInNewName(args, strdup(args[L_NAME]));
	  if ((status = do_mr_query("update_list", CountArgs(args), args,
				    NULL, NULL)))
	    {
	      com_err(program_name, status, " updating list, "
		      "not deactivating list or filesystem");
	      FreeInfo(args);
	      FreeQueue(elem);
	      return;
	    }
	  FreeInfo(args);
	  FreeQueue(elem);
	  elem = NULL;
	}
      else if (status != MR_NO_MATCH)
	{
	  com_err(program_name, status, " getting list info, "
		  "not deactivating list or filesystem");
	  return;
	}

      if ((status = do_mr_query("get_filesys_by_label", 1, &(info[NAME]),
				StoreInfo, &elem)))
	{
	  com_err(program_name, status, " getting filsys info, "
		  "not deactivating filesystem");
	  return;
	}
      args = QueueTop(elem)->q_data;
      free(args[FS_TYPE]);
      args[FS_TYPE] = strdup("ERR");
      free(args[FS_COMMENTS]);
      args[FS_COMMENTS] = strdup("Locker disabled; call 3-1325 for help");
      FreeAndClear(&args[FS_MODTIME], TRUE);
      FreeAndClear(&args[FS_MODBY], TRUE);
      FreeAndClear(&args[FS_MODWITH], TRUE);
      SlipInNewName(args, strdup(args[FS_NAME]));
      if ((status = do_mr_query("update_filesys", CountArgs(args), args,
				NULL, NULL)))
	{
	  com_err(program_name, status, " updating filesystem, "
		  "not deactivating filesystem");
	  FreeInfo(args);
	  FreeQueue(elem);
	  return;
	}
      FreeInfo(args);
      FreeQueue(elem);
    }
}


/*	Function Name: DeactivateUser
 *	Description: sets the user's status to 3.
 *	Arguments: argc, argv - login name of the user in argv[1].
 *	Returns: DM_NORMAL.
 */

int DeactivateUser(int argc, char **argv)
{
  struct mqelem *elem;

  elem = GetUserInfo(LOGIN, argv[1], NULL);
  QueryLoop(elem, NullPrint, RealDeactivateUser, "Deactivate user");

  FreeQueue(elem);
  return DM_NORMAL;
}


/* ------------------------- Top Menu ------------------------- */

/* DeleteUser() in delete.c */

/*	Function Name: DeleteUserByUid
 *	Description: Deletes the user given a uid number.
 *	Arguments: argc, argv - uid if user in argv[1].
 *	Returns: DM_NORMAL.
 *      NOTES: This just gets the username from the mr server
 *             and performs a DeleteUser().
 */

int DeleteUserByUid(int argc, char **argv)
{
  int status;
  struct mqelem *elem = NULL;
  char **info;

  if (!ValidName(argv[1]))
    return DM_NORMAL;

  if ((status = do_mr_query("get_user_account_by_uid", 1, argv + 1, StoreInfo,
			    &elem)))
    com_err(program_name, status, " in get_user_account_by_uid");

  info = elem->q_data;
  argv[1] = info[U_NAME];

  DeleteUser(argc, argv);
  return DM_NORMAL;
}

/* ------------------------- Show User Information ------------------------- */

/*	Function Name: ShowUserByLogin
 *	Description: Shows user information given a login name.
 *	Arguments: argc, argv - login name in argv[1].
 *	Returns: DM_NORMAL
 */

int ShowUserByLogin(int argc, char *argv[])
{
  struct mqelem *top, *elem;

  elem = top = GetUserInfo(LOGIN, argv[1], NULL);
  Loop(elem, PrintUserInfo);

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: RetrieveUserByName
 *	Description: Show information on a user give fist and/or last name.
 *	Arguments: argc, argv - argv[1] - first name.
 *                              argv[2] - last name.
 *	Returns: DM_NORMAL.
 */

int ShowUserByName(int argc, char *argv[])
{
  struct mqelem *top;
  char buf[BUFSIZ];

  top = GetUserInfo(BY_NAME, argv[1], argv[2]);

  if (!top)		/* if there was an error then return. */
    return DM_NORMAL;

  if (!PromptWithDefault("Print full information, or just the names (f/n)?",
			 buf, 2, "f"))
    return DM_NORMAL;

  switch (buf[0])
    {
    case 'F':
    case 'f':
      Loop(top, PrintUserInfo);
      break;
    case 'N':
    case 'n':
      Loop(top, PrintUserName);
      break;
    }

  FreeQueue(top);
  return DM_NORMAL;
}

/*	Function Name: ShowUserByClass
 *	Description: Shows real and login names of all users in class.
 *	Arguments: argc, argv - argv[1] contains the class.
 *	Returns: none.
 */

int ShowUserByClass(int argc, char **argv)
{
  struct mqelem *top;

  if (YesNoQuestion("This will take a long time.  Are you sure", 0) == FALSE)
    return DM_NORMAL;
  top = GetUserInfo(CLASS, argv[1], NULL);
  Loop(top, PrintUserName);

  FreeQueue(top);
  return DM_NORMAL;
}


/*	Function Name: ShowUserById
 *	Description: Shows user information given an ID number.
 *	Arguments: argc, argv - ID number in argv[1].
 *	Returns: DM_NORMAL
 */

int ShowUserById(int argc, char *argv[])
{
  struct mqelem *top, *elem;

  elem = top = GetUserInfo(ID, argv[1], NULL);
  Loop(elem, PrintUserInfo);

  FreeQueue(top);
  return DM_NORMAL;
}


/*	Function Name: GetKrbmap
 *	Description: Shows user <-> Kerberos mappings
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

int GetKrbmap(int argc, char **argv)
{
  int stat;
  struct mqelem *elem = NULL, *top;
  char buf[BUFSIZ];

  if ((stat = do_mr_query("get_kerberos_user_map", 2, &argv[1],
			  StoreInfo, &elem)))
    {
      com_err(program_name, stat, " in GetKrbMap.");
      return DM_NORMAL;
    }

  top = elem = QueueTop(elem);
  Put_message("");
  while (elem)
    {
      char **info = elem->q_data;
      sprintf(buf, "User: %-9s Principal: %s",
	      info[KMAP_USER], info[KMAP_PRINCIPAL]);
      Put_message(buf);
      elem = elem->q_forw;
    }

  FreeQueue(QueueTop(top));
  return DM_NORMAL;
}


/*	Function Name: AddKrbmap
 *	Description: Add a new user <-> Kerberos mapping
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

int AddKrbmap(int argc, char **argv)
{
  int stat;

  if (!strchr(argv[KMAP_PRINCIPAL + 1], '@'))
    {
      Put_message("Please specify a realm for the kerberos principal.");
      return DM_NORMAL;
    }
  if ((stat = do_mr_query("add_kerberos_user_map", 2, &argv[1],
			  NULL, NULL)))
    {
      com_err(program_name, stat, " in AddKrbMap.");
      if (stat == MR_EXISTS)
	Put_message("No user or principal may have more than one mapping.");
    }
  return DM_NORMAL;
}


/*	Function Name: DeleteKrbmap
 *	Description: Remove a user <-> Kerberos mapping
 *	Arguments: argc, argv - argv[1] contains the user login name,
 *		argv[2] contains the principal
 *	Returns: none.
 */

int DeleteKrbmap(int argc, char **argv)
{
  int stat;

  if ((stat = do_mr_query("delete_kerberos_user_map", 2, &argv[1],
			  NULL, NULL)))
    com_err(program_name, stat, " in DeleteKrbMap.");
  return DM_NORMAL;
}

int GetUserReservations(int argc, char **argv)
{
  int stat;
  struct mqelem *elem = NULL, *top;
  char buf[BUFSIZ];

  if ((stat = do_mr_query("get_user_reservations", 1, &argv[1],
			  StoreInfo, &elem)))
    {
      com_err(program_name, stat, " in GetUserReservations.");
      return DM_NORMAL;
    }

  top = elem = QueueTop(elem);
  Put_message("");
  if (!elem)
    Put_message("No reservations for that user.");
  while (elem)
    {
      char **info = elem->q_data;
      sprintf(buf, "Reservation: %s", info[0]);
      Put_message(buf);
      elem = elem->q_forw;
    }

  FreeQueue(QueueTop(top));
  return DM_NORMAL;  
}

int AddUserReservation(int argc, char **argv)
{
  int stat;
  char buf[BUFSIZ];
  
  switch (stat = do_mr_query("add_user_reservation", 2, &argv[1],
			     NULL, NULL))
    {
    case MR_SUCCESS:
      break;
    case MR_STRING:
      sprintf(buf, "The reservation %s is not valid.", argv[2]);
      Put_message(buf);
      PrintReservationTypes();
      break;
    default:
      com_err(program_name, stat, " in AddUserReservation.");
      break;
    }
  
  return DM_NORMAL;
}

int DelUserReservation(int argc, char **argv)
{
  int stat;
  char buf[BUFSIZ];

  switch (stat = do_mr_query("delete_user_reservation", 2, &argv[1],
		     NULL, NULL))
    {
    case MR_SUCCESS:
      break;
    case MR_STRING:
      sprintf(buf, "The reservation %s is not valid.", argv[2]);
      Put_message(buf);
      PrintReservationTypes();
      break;
    default:
      com_err(program_name, stat, " in DelUserReservation.");
      break;
    }
  
  return DM_NORMAL;
}

int GetUserByReservation(int argc, char **argv)
{
  int stat;
  struct mqelem *elem = NULL, *top;
  char buf[BUFSIZ];

  switch (stat = do_mr_query("get_user_by_reservation", 1, &argv[1],
		     StoreInfo, &elem))
    {
    case MR_SUCCESS:
      break;
    case MR_STRING:
      sprintf(buf, "The reservation %s is not valid.", argv[1]);
      Put_message(buf);
      PrintReservationTypes();
      return DM_NORMAL;
    default:
      com_err(program_name, stat, " in GetUserByReservation.");
      return DM_NORMAL;
    }
  
  top = elem = QueueTop(elem);
  Put_message("");
  while (elem)
    {
      char **info = elem->q_data;
      sprintf(buf, "User: %s", info[0]);
      Put_message(buf);
      elem = elem->q_forw;
    }

  FreeQueue(QueueTop(top));
  return DM_NORMAL;  
}

void PrintReservationTypes(void)
{
  int stat;
  struct mqelem *elem = NULL, *top;
  char buf[BUFSIZ];
  char *qargs[2];

  Put_message("Valid types of reservations are: ");
  Put_message("");
  qargs[0] = "*";
  qargs[1] = "RESERVE";
  qargs[2] = "*";
  if ((stat = do_mr_query("get_alias", 3, &qargs[0],
			  StoreInfo, &elem)))
    {
      com_err(program_name, stat, "in PrintReservationTypes.");
    }
  top = elem = QueueTop(elem);
  while (elem)
    {
      char **info = elem->q_data;
      sprintf(buf, "%s", info[2]);
      Put_message(buf);
      elem = elem->q_forw;
    }

  FreeQueue(QueueTop(top));  
}

int UserBySponsor(int argc, char **argv)
{
  char buf[BUFSIZ], temp_buf[BUFSIZ], *type, *name;
  struct mqelem *top;

  type = strdup("USER");
  if (GetTypeFromUser("Type of sponsor", "search_ace_type", &type) == SUB_ERROR)
    return DM_NORMAL;

  sprintf(buf, "Name of %s", type);
  name = strdup(user);
  if (GetValueFromUser(buf, &name) == SUB_ERROR)
    return DM_NORMAL;

  switch (YesNoQuestion("Do you want a recursive search (y/n)", FALSE))
    {
    case TRUE:
      sprintf(temp_buf, "R%s", type);     /* "USER to "RUSER", etc. */
      free(type);
      type = strdup(temp_buf);
      break;
    case FALSE:
      break;
    default:
      return DM_NORMAL;
    }

  top = GetUserBySponsor(type, name);
  Loop(top, PrintLogin);

  FreeQueue(top);
  return DM_NORMAL;
}

static void PrintLogin(char **info)
{
     char buf[BUFSIZ];

     sprintf(buf, "Login: %s", info[U_NAME]);
     Put_message(buf);
}

struct mqelem *GetUserBySponsor(char *type, char *name)
{
  char *args[2];
  struct mqelem *elem = NULL;
  int status;

  args[0] = type;
  args[1] = name;
  if ((status = do_mr_query("get_user_account_by_sponsor", 2, args, StoreInfo,
			    &elem)))
    {
      com_err(program_name, status, " in get_user_account_by_sponsor");
      return NULL;
    }
  return QueueTop(elem);
}
