/* $Id$
 *
 *	This is the file utils.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains:  Many useful utility functions.
 *
 *	Created: 	4/25/88
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

#include <sys/types.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>		/* for gethostbyname. */
#endif /* _WIN32 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

RCSID("$HeadURL$ $Id$");

/*	Function Name: AddQueue
 *	Description: Adds an element to a queue
 *	Arguments: elem, pred - element and its predecessor.
 *	Returns: none.
 */

void AddQueue(struct mqelem *elem, struct mqelem *pred)
{
  if (!pred)
    {
      elem->q_forw = NULL;
      elem->q_back = NULL;
      return;
    }
  elem->q_back = pred;
  elem->q_forw = pred->q_forw;
  pred->q_forw = elem;
}

/*	Function Name: RemoveQueue
 *	Description: removes an element from a queue.
 *	Arguments: elem.
 *	Returns: none.
 */

static void RemoveQueue(struct mqelem *elem)
{
  if (elem->q_forw)
    elem->q_forw->q_back = elem->q_back;
  if (elem->q_back)
    elem->q_back->q_forw = elem->q_forw;
}


/* CopyInfo: allocates memory for a copy of a NULL terminated array of
 * strings <and returns a pointer to the copy.
 */

char **CopyInfo(char **info)
{
  char **ret;
  int i;

  ret = malloc(sizeof(char *) * (CountArgs(info) + 1));
  if (!ret)
    return ret;
  for (i = 0; info[i]; i++)
    ret[i] = strdup(info[i]);
  ret[i] = NULL;
  return ret;
}


/*	Function Name: FreeInfo
 *	Description: Frees all elements of a NULL terminated arrary of char*'s
 *	Arguments: info - array who's elements we are to free.
 *	Returns: none.
 */

void FreeInfo(char **info)
{
  while (*info)
    FreeAndClear(info++, TRUE);
}

/*	Function Name: FreeAndClear        - I couldn't resist the name.
 *	Description: Clears pointer and optionially frees it.
 *	Arguments: pointer - pointer to work with.
 *                 free_it - if TRUE then free pointer.
 *	Returns: none.
 */

void FreeAndClear(char **pointer, Bool free_it)
{
  if (!*pointer)
    return;
  else if (free_it)
    free(*pointer);
  *pointer = NULL;
}

/*	Function Name: QueueTop
 *	Description: returns a mqelem pointer that points to the top of
 *                   a queue.
 *	Arguments: elem - any element of a queue.
 *	Returns: top element of a queue.
 */

struct mqelem *QueueTop(struct mqelem *elem)
{
  if (!elem)		/* NULL returns NULL.  */
    return NULL;
  while (elem->q_back)
    elem = elem->q_back;
  return elem;
}

/*	Function Name: FreeQueueElem
 *	Description: Frees one element of the queue.
 *	Arguments: elem - the elem to free.
 *	Returns: none
 */

static void FreeQueueElem(struct mqelem *elem)
{
  char **info = elem->q_data;

  if (info)
    {
      FreeInfo(info); /* free info fields */
      free(elem->q_data);		/* free info array itself. */
    }
  RemoveQueue(elem);		/* remove this element from the queue */
  free(elem);			/* free its space. */
}

/*	Function Name: FreeQueue
 *	Description: Cleans up the queue
 *	Arguments: elem - any element of the queue.
 *	Returns: none.
 */

void FreeQueue(struct mqelem *elem)
{
  struct mqelem *temp, *local = QueueTop(elem);

  while (local)
    {
      temp = local->q_forw;
      FreeQueueElem(local);
      local = temp;
    }
}

/*	Function Name: QueueCount
 *	Description: Counts the number of elements in a queue
 *	Arguments: elem - any element in the queue.
 *	Returns: none.
 */

int QueueCount(struct mqelem *elem)
{
  int count = 0;
  elem = QueueTop(elem);
  while (elem)
    {
      count++;
      elem = elem->q_forw;
    }
  return count;
}

/*	Function Name: StoreInfo
 *	Description: Stores information from a moira query into a queue.
 *	Arguments: argc, argv, - information returned from the query returned
 *                               in argv.
 *                 data - the previous element on the queue, this data will be
 *                        stored in a mqelem struct immediatly after this elem.
 *                        If NULL then a new queue will be created.  This value
 *                        is updated to the current element at the end off the
 *                        call.
 *	Returns: MR_CONT, or MR_ABORT if it has problems.
 */

int StoreInfo(int argc, char **argv, void *data)
{
  char **info = malloc(MAX_ARGS_SIZE * sizeof(char *));
  struct mqelem **old_elem = data;
  struct mqelem *new_elem = malloc(sizeof(struct mqelem));
  int count;

  if (!new_elem || !info)
    {
      Put_message("Could Not allocate more memory.");
      FreeQueue(*old_elem);
      *old_elem = NULL;
      return MR_ABORT;
    }

  for (count = 0; count < argc; count++)
    info[count] = strdup(argv[count]);
  info[count] = NULL;		/* NULL terminate this sucker. */

  new_elem->q_data = info;
  AddQueue(new_elem, *old_elem);

  *old_elem = new_elem;
  return MR_CONT;
}

/*	Function Name: CountArgs
 *	Description:  Retrieve the number of args in a null terminated
 *                     arglist.
 *	Arguments: info - the argument list.
 *	Returns: number if args in the list.
 */

int CountArgs(char **info)
{
  int number = 0;

  while (*info)
    {
      number++;
      info++;
    }

  return number;
}

/*	Function Name: PromptWithDefault
 *	Description: allows a user to be prompted for input, and given a
 *                   default choice.
 *	Arguments: prompt - the prompt string.
 *                 buf, buflen - buffer to be returned and its MAX size?
 *                 default value for the answer.
 *	Returns: zero on failure
 */

int PromptWithDefault(char *prompt, char *buf, int buflen, char *def)
{
  char tmp[BUFSIZ];
  int ans;

  if (parsed_argc > 0)
    {
      parsed_argc--;
      strncpy(buf, parsed_argv[0], buflen);
      sprintf(tmp, "%s: %s", prompt, buf);
      Put_message(tmp);
      parsed_argv++;
      return 1;
    }

  sprintf(tmp, "%s [%s]: ", prompt, def ? def : "");
  ans = Prompt_input(tmp, buf, buflen);
  if (IS_EMPTY(buf))
    strcpy(buf, def);
  else if (!strcmp(buf, "\"\""))
    *buf = 0;
  return ans;
}

/*	Function Name: YesNoQuestion
 *	Description: This prompts the user for the answer to a yes-no or
 *                   true-false question.
 *	Arguments: prompt - the prompt for the user.
 *                 bool_def - the default value either TRUE or FALSE.
 *	Returns: TRUE or FALSE or -1 on error
 */

Bool YesNoQuestion(char *prompt, int bool_def)
{
  char ans[2];

  while (TRUE)
    {
      if (!PromptWithDefault(prompt, ans, 2, bool_def ? "y" : "n"))
	return -1;
      switch (ans[0])
	{
	case 'n':
	case 'N':
	  return FALSE;
	case 'y':
	case 'Y':
	  return TRUE;
	default:
	  Put_message("Please answer 'y' or 'n'.");
	  break;
	}
    }
}

/*	Function Name: YesNoQuitQuestion
 *	Description: This prompts the user for the answer to a yes-no or
 *                   true-false question, with a quit option.
 *	Arguments: prompt - the prompt for the user.
 *                 bool_def - the default value either TRUE or FALSE.
 *	Returns: TRUE or FALSE or -1 on error or QUIT
 *      NOTE: It is not possible to have quit the default, but then I don't
 *            seem to need this functionality.
 */

Bool YesNoQuitQuestion(char *prompt, int bool_def)
{
  char ans[2];

  while (TRUE)
    {
      if (!PromptWithDefault(prompt, ans, 2, bool_def ? "y" : "n"))
	return -1;
      switch (ans[0])
	{
	case 'n':
	case 'N':
	  return FALSE;
	case 'y':
	case 'Y':
	  return TRUE;
	case 'q':
	case 'Q':
	  return -1;
	default:
	  Put_message("Please answer 'y', 'n' or 'q'.");
	  break;
	}
    }
}

/*	Function Name: Confirm
 *	Description:   This function asks the user to confirm the action
 *                     he is about to take, used for things like deleting.
 *	Arguments: prompt - the prompt string.
 *	Returns:   TRUE/FALSE - wether or not the confirmation occured.
 */

Bool Confirm(char *prompt)
{
  return !verbose || (YesNoQuestion(prompt, FALSE) == TRUE);
}

/*	Function Name: ValidName
 *	Description: This function checks to see if we have a valid list name.
 *	Arguments: s - the list name.
 *	Returns: TRUE if Valid.
 */

Bool ValidName(char *s)
{
  if (IS_EMPTY(s))
    Put_message("Please use a non-empty name.");
  else if (strchr(s, ' '))
    Put_message("You cannot use space (' ') in this name.");
  else if (strchr(s, '*') || strchr(s, '?'))
    Put_message("Wildcards not accepted here.");
  else
    return TRUE;
  return FALSE;
}

/*	Function Name: ToggleVerboseMode
 *	Description: This function toggles the verbose mode.
 *	Arguments: none
 *	Returns: DM_NORMAL.
 */

int ToggleVerboseMode(int argc, char **argv)
{
  verbose = !verbose;

  if (verbose)
    Put_message("Delete functions will first confirm");
  else
    Put_message("Delete functions will be silent");

  return DM_NORMAL;
}

/*	Function Name: SlipInNewName
 *	Description: Slips the new name into the number 2 slot of a list, and
 *                   returns a pointer to the new list.
 *	Arguments: info - list that needs this name slipped into it.
 *                 name - the name to slip into the list.
 *	Returns: a pointer to the new list.
 *      NOTE:  This screws up the numbers of the elements of the array in a
 *             big way.
 */

void SlipInNewName(char **info, char *name)
{
  int i;

  /* This also pushes the NULL down. */
  for (i = CountArgs(info); i > 0; i--)
    info[i + 1] = info[i];
  info[1] = name;	/* now slip in the name. */
}

/*	Function Name: GetValueFromUser
 *	Description: This function gets a value from a user for the field
 *                   specified.
 *	Arguments: prompt - prompt for user.
 *                 pointer - pointer to default value, will be returned
 *                          as new value.
 *	Returns: SUB_ERROR if break hit (^C).
 */

int GetValueFromUser(char *prompt, char **pointer)
{
  char buf[BUFSIZ];

  if (PromptWithDefault(prompt, buf, BUFSIZ, *pointer) == 0)
    return SUB_ERROR;

  /*
   * If these are the same then there is no need to allocate a new string.
   *
   * a difference that makes no difference, IS no difference.
   */

  if (*pointer)
    {
      if (strcmp(buf, *pointer))
	{
	  free(*pointer);
	  *pointer = strdup(buf);
	}
    }
  return SUB_NORMAL;
}

/*	Function Name: GetYesNoValueFromUser
 *	Description: This function gets a value from a user for the field
 *                   specified.
 *	Arguments: prompt - prompt for user.
 *                 pointer - pointer to default value, will be returned
 *                          as new value.
 *	Returns: SUB_ERROR if break hit (^C).
 */

int GetYesNoValueFromUser(char *prompt, char **pointer)
{
  char user_prompt[BUFSIZ];
  Bool default_val;

  if (!strcmp(*pointer, DEFAULT_YES))
    default_val = TRUE;
  else
    default_val = FALSE;

  sprintf(user_prompt, "%s (y/n)", prompt);

  switch (YesNoQuestion(user_prompt, default_val))
    {
    case TRUE:
      if (*pointer)
	free(*pointer);
      *pointer = strdup(DEFAULT_YES);
      break;
    case FALSE:
      if (*pointer)
	free(*pointer);
      *pointer = strdup(DEFAULT_NO);
      break;
    case -1:
    default:
      return SUB_ERROR;
    }
  return SUB_NORMAL;
}

/*	Function Name: GetFSVal
 *	Description: asks about a specific filesystem value.
 *	Arguments: name - string for this type of filesystem.
 *                 mask - mask for this type of filesystem.
 *                 current - current filesystem state. (for defaults).
 *                 new - new filesystem state.
 *	Returns: TRUE if successful.
 */

static Bool GetFSVal(char *name, int mask, int current, int *new)
{
  char temp_buf[BUFSIZ];
  sprintf(temp_buf, "Is this a %s filsystem", name);
  switch (YesNoQuestion(temp_buf, ((mask & current) == mask)))
    {
    case TRUE:
      *new |= mask;
      break;
    case FALSE:
      break;			/* zero by default. */
    default:
      return FALSE;
    }
  return TRUE;
}

/*	Function Name: GetFSTypes
 *	Description: Allows user to specify filsystem types.
 *	Arguments: current - current value of filsystem, freed here.
 *	Returns: SUB_ERROR on ^C.
 */

int GetFSTypes(char **current, int options)
{
  int c_value, new_val = 0;	/* current value of filesys type (int). */
  char ret_value[BUFSIZ];

  if (!*current)
    c_value = 0;
  else
    c_value = atoi(*current);

  if (GetFSVal("student", MR_FS_STUDENT, c_value, &new_val) == FALSE)
    return SUB_ERROR;
  if (GetFSVal("faculty", MR_FS_FACULTY, c_value, &new_val) == FALSE)
    return SUB_ERROR;
  if (GetFSVal("staff", MR_FS_STAFF, c_value, &new_val) == FALSE)
    return SUB_ERROR;
  if (GetFSVal("miscellaneous", MR_FS_MISC, c_value, &new_val) == FALSE)
    return SUB_ERROR;
  if (options)
    {
      if (GetFSVal("Group Quotas", MR_FS_GROUPQUOTA, c_value, &new_val) ==
	  FALSE)
	return SUB_ERROR;
    }

  FreeAndClear(current, TRUE);
  sprintf(ret_value, "%d", new_val);
  *current = strdup(ret_value);
  return SUB_NORMAL;
}

/* atot: convert ASCII integer unix time into human readable date string */

char *atot(char *itime)
{
  time_t time;
  char *ct;

  time = (time_t) atoi(itime);
  ct = ctime(&time);
  ct[24] = 0;
  return &ct[4];
}


/*	Function Name: Print
 *	Description: prints out all the arguments on a single line.
 *	Arguments: argc, argv - the standard Moira arguments.
 *                 callback - the callback function - NOT USED.
 *	Returns: MR_CONT
 */

int Print(int argc, char **argv, void *callback)
{
  char buf[BUFSIZ];
  int i;

  found_some = TRUE;
  strcpy(buf, argv[0]);	/* no newline 'cause Put_message adds one */
  for (i = 1; i < argc; i++)
    sprintf(buf, "%s %s", buf, argv[i]);
  Put_message(buf);

  return MR_CONT;
}

/*	Function Name: PrintByType
 *	Description: This function prints all members of the type specified
 *                   by the callback arg, unless the callback is NULL, in which
 *                   case it prints all members.
 *	Arguments: argc, argc - normal arguments for mr_callback function.
 *                 callback - either a type of member or NULL.
 *	Returns: MR_CONT or MR_QUIT.
 */

int PrintByType(int argc, char **argv, void *callback)
{
  if (!callback)
    return Print(argc, argv, callback);
  if (!strcmp(argv[0], callback))
    return Print(argc, argv, callback);
  return MR_CONT;
}

/*	Function Name: PrintHelp
 *	Description: Prints Help Information in a NULL terminated
 *                   char **.
 *	Arguments: message.
 *	Returns: DM_NORMAL.
 */

int PrintHelp(char **message)
{
  int i;

  for (i = 0; i < CountArgs(message); i++)
    Put_message(message[i]);

  return DM_NORMAL;
}

/*	Function Name: Loop
 *	Description: This function goes through the entire queue, and
 *                   and executes the given function on each element.
 *	Arguments: elem - top element of the queue.
 *                 func - the function to execute.
 *	Returns: none.
 */

void Loop(struct mqelem *elem, void (*func)(char **))
{
  while (elem)
    {
      (*func) (elem->q_data);
      elem = elem->q_forw;
    }
}


/*	Function Name: QueryLoop
 *	Description: This functions loops through a queue containing
 *                   information about some item that we want to perform
 *                   an operation on, and then calls the correct routine
 *                   perform that operation.
 *	Arguments: top - top of the queue of information.
 *                 print_func - print function.
 *                 op_function - operation to be performed.
 *                 query_string - string the prompts the user whether or not
 *                                to perform this operation.
 *	Returns: none.
 *      NOTES:
 *               print_opt - should expect one arguent, the info array
 *                           of char *'s.
 *                           is expected to return the name of the item.
 *               op_func   - should expect two arguments.
 *                           1) the info array of char *'s.
 *                           2) a boolean the is true if there only
 *                              one item in this queue, used for delete
 *                              confirmation.
 *               query_string - this should be of such a form that when the
 *                              name of the object and '(y/n/q) ?' are appended
 *                              then it should still make sense, an example is
 *                              "Delete the list"
 */

void QueryLoop(struct mqelem *elem, char * (*print_func)(char **),
	       void (*op_func)(char **, Bool), char *query_string)
{
  Bool one_item;
  char temp_buf[BUFSIZ], *name;

  elem = QueueTop(elem);
  one_item = (QueueCount(elem) == 1);
  while (elem)
    {
      char **info = elem->q_data;

      if (one_item)
	(*op_func) (info, one_item);
      else
	{
	  name = (*print_func) (info); /* call print function. */
	  sprintf(temp_buf, "%s %s (y/n/q)", query_string, name);
	  switch (YesNoQuitQuestion(temp_buf, FALSE))
	    {
	    case TRUE:
	      (*op_func) (info, one_item);
	      break;
	    case FALSE:
	      break;
	    default:		/* Quit. */
	      Put_message("Aborting...");
	      return;
	    }
	}
      elem = elem->q_forw;
    }
}

/*	Function Name: NullPrint
 *	Description: print function that returns nothing.
 *	Arguments: info - a pointer to the info array - Not used.
 *	Returns: none.
 */

char *NullPrint(char **info)
{
  return info[NAME];
}


/*	Function Name: GetTypeValues
 *	Description: gets legal values for a typed object, keeping a cache
 *	Arguments: type name
 *	Returns: argv of values
 */

struct mqelem *GetTypeValues(char *tname)
{
  int stat;
  char *argv[3], *p, **pp;
  struct mqelem *elem, *oelem;
  static struct mqelem *cache = NULL;
  struct cache_elem {
    char *cache_name;
    struct mqelem *cache_data;
  } *ce;

  for (elem = cache; elem; elem = elem->q_forw)
    {
      ce = elem->q_data;
      if (!strcmp(ce->cache_name, tname))
	return ce->cache_data;
    }

  argv[0] = tname;
  argv[1] = "TYPE";
  argv[2] = "*";
  elem = NULL;
  if ((stat = do_mr_query("get_alias", 3, argv, StoreInfo, &elem)))
    {
      com_err(program_name, stat, " in GetTypeValues");
      return NULL;
    }
  oelem = elem;
  for (elem = QueueTop(elem); elem; elem = elem->q_forw)
    {
      pp = elem->q_data;
      p = strdup(pp[2]);
      FreeInfo(pp);
      elem->q_data = p;
    }
  elem = malloc(sizeof(struct mqelem));
  ce = malloc(sizeof(struct cache_elem));
  ce->cache_name = strdup(tname);
  ce->cache_data = QueueTop(oelem);
  elem->q_data = ce;
  AddQueue(elem, cache);
  cache = QueueTop(elem);
  return ce->cache_data;
}


/*	Function Name: GetTypeFromUser
 *	Description: gets a typed value from the user
 *	Arguments: prompt string, type name, buffer pointer
 *	Returns: SUB_ERROR if ^C, SUB_NORMAL otherwise
 */

int GetTypeFromUser(char *prompt, char *tname, char **pointer)
{
  char def[BUFSIZ], buffer[BUFSIZ], *p, *argv[3];
  struct mqelem *elem;
  int stat;

  strcpy(def, *pointer);
  strcpy(buffer, prompt);
  strcat(buffer, " (");
  for (elem = GetTypeValues(tname); elem; elem = elem->q_forw)
    {
      /* Make sure we don't blow up and get too long a prompt */
      if (strlen(buffer) > 64)
	break;
      strcat(buffer, elem->q_data);
      if (elem->q_forw)
	strcat(buffer, ", ");
    }
  strcat(buffer, ")");
  /* Trim the prompt if it is too long */
  if (strlen(buffer) > 64)
    sprintf(buffer, "%s (? for help)", prompt);
  if (GetValueFromUser(buffer, pointer) == SUB_ERROR)
    return SUB_ERROR;
  if (**pointer == '?')
    {
      sprintf(buffer, "Type %s is one of:", tname);
      Put_message(buffer);
      for (elem = GetTypeValues(tname); elem; elem = elem->q_forw)
	Put_message(elem->q_data);
      *pointer = strdup(def);
      return GetTypeFromUser(prompt, tname, pointer);
    }
  for (elem = GetTypeValues(tname); elem; elem = elem->q_forw)
    {
      if (!strcasecmp(elem->q_data, *pointer))
	{
	  strcpy(*pointer, elem->q_data);
	  return SUB_NORMAL;
	}
    }
  sprintf(buffer, "\"%s\" is not a legal value for %s.  Use one of:",
	  *pointer, tname);
  Put_message(buffer);
  for (elem = GetTypeValues(tname); elem; elem = elem->q_forw)
    Put_message(elem->q_data);
  sprintf(buffer, "Are you sure you want \"%s\" to be a legal %s",
	  *pointer, tname);
  if (YesNoQuestion("Do you want this to be a new legal value", 0) == TRUE &&
      YesNoQuestion(buffer, 0) == TRUE)
    {
      argv[0] = tname;
      argv[1] = "TYPE";
      argv[2] = *pointer;
      /* don't uppercase access flags.  Do uppercase everything else */
      if (strncmp(tname, "fs_access", 9))
	{
	  for (p = argv[2]; *p; p++)
	    {
	      if (islower(*p))
		*p = toupper(*p);
	    }
	}
      if ((stat = do_mr_query("add_alias", 3, argv, NULL, NULL)))
	com_err(program_name, stat, " in add_alias");
      else
	{
	  elem = malloc(sizeof(struct mqelem));
	  elem->q_data = strdup(*pointer);
	  AddQueue(elem, GetTypeValues(tname));
	  Put_message("Done.");
	}
    }
  *pointer = strdup(def);
  return GetTypeFromUser(prompt, tname, pointer);
}


/*	Function Name: GetAddressFromUser
 *	Description: gets an IP address from the user
 *	Arguments: prompt string, buffer pointer
 *		buffer contains default value as long int
 *	Returns: SUB_ERROR if ^C, SUB_NORMAL otherwise
 */

int GetAddressFromUser(char *prompt, char **pointer)
{
  char *value, buf[256];
  struct in_addr addr;
  int ret;

  addr.s_addr = htonl(atoi(*pointer));
  value = strdup(inet_ntoa(addr));
  ret = GetValueFromUser(prompt, &value);
  if (ret == SUB_ERROR)
    return SUB_ERROR;
  addr.s_addr = inet_addr(value);
  free(*pointer);
  sprintf(buf, "%d", ntohl(addr.s_addr));
  *pointer = strdup(buf);
  return SUB_NORMAL;
}


int do_mr_query(char *name, int argc, char **argv,
		int (*proc)(int, char **, void *), void *hint)
{
  int status;
  extern char *whoami, *moira_server;

  refresh_screen();
  status = mr_query(name, argc, argv, proc, hint);
  if (status != MR_ABORTED && status != MR_NOT_CONNECTED)
    return status;
  status = mrcl_connect(moira_server, whoami, QUERY_VERSION, 0);
  if (status != MRCL_SUCCESS)
    {
      com_err(whoami, status, " while re-connecting to server %s",
	      moira_server);
      return MR_ABORTED;
    }
  status = mr_krb5_auth(whoami);
  
  if (status == MR_UNKNOWN_PROC)
    status = mr_auth(whoami);

  if (status)
    {
      com_err(whoami, status, " while re-authenticating to server %s",
	      moira_server);
      mr_disconnect();
      return MR_ABORTED;
    }
  status = mr_query(name, argc, argv, proc, hint);
  return status;
}

