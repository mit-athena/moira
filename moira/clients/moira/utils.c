#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v 1.26 1990-05-02 13:13:32 mar Exp $";
#endif lint

/*	This is the file utils.c for the MOIRA Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the MOIRA database.
 *	It Contains:  Many useful utility functions.
 *	
 *	Created: 	4/25/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v 1.26 1990-05-02 13:13:32 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <stdio.h>
#include <strings.h>
#include <moira.h>
#include <moira_site.h>
#include <menu.h>
#include <ctype.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <netdb.h>		/* for gethostbyname. */

/*	Function Name: AddQueue
 *	Description: Adds an element to a queue
 *	Arguments: elem, pred - element and its predecessor.
 *	Returns: none.
 */

static void
AddQueue(elem, pred)
struct qelem * elem, *pred;
{
    if (pred == NULL) {
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

static void
RemoveQueue(elem)
struct qelem *elem;
{
    if (elem->q_forw != NULL) 
	(elem->q_forw)->q_back = elem->q_back;
    if (elem->q_back != NULL)
	(elem->q_back)->q_forw = elem->q_forw;
}

/*	Function Name: FreeInfo
 *	Description: Frees all elements of a NULL terminated arrary of char*'s
 *	Arguments: info - array who's elements we are to free.
 *	Returns: none.
 */

void
FreeInfo(info)
char ** info;
{
    while (*info != NULL)
	FreeAndClear(info++, TRUE);
}

/*	Function Name: FreeAndClear        - I couldn't resist the name.
 *	Description: Clears pointer and optionially frees it.
 *	Arguments: pointer - pointer to work with.
 *                 free_it - if TRUE then free pointer.
 *	Returns: none.
 */

void
FreeAndClear(pointer, free_it)
char ** pointer;
Bool free_it;
{
    if (*pointer == NULL)
	return;
    else if (free_it)
	free(*pointer);
    *pointer = NULL;
}

/*	Function Name: QueueTop
 *	Description: returns a qelem pointer that points to the top of
 *                   a queue.
 *	Arguments: elem - any element of a queue.
 *	Returns: top element of a queue.
 */
    
struct qelem * 
QueueTop(elem)
struct qelem * elem;
{
    if (elem == NULL)		/* NULL returns NULL.  */
	return(NULL);
    while (elem->q_back != NULL) 
	elem = elem->q_back;
    return(elem);
}

/*	Function Name: FreeQueueElem
 *	Description: Frees one element of the queue.
 *	Arguments: elem - the elem to free.
 *	Returns: none
 */

static void
FreeQueueElem(elem)
struct qelem * elem;
{
    char ** info = (char **) elem->q_data;

    if (info != (char **) NULL) {
	FreeInfo( info ); /* free info fields */
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

void
FreeQueue(elem)
struct qelem * elem;
{
    struct qelem *temp, *local = QueueTop(elem); 

    while(local != NULL) {
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

int
QueueCount(elem)
struct qelem * elem;
{
    int count = 0;
    elem = QueueTop(elem);
    while (elem != NULL) {
	count ++;
	elem = elem->q_forw;
    }
    return(count);
}

/*	Function Name: StoreInfo
 *	Description: Stores information from an moira query into a queue.
 *	Arguments: argc, argv, - information returned from the query returned
 *                               in argv.
 *                 data - the previous element on the queue, this data will be
 *                        stored in a qelem struct immediatly after this elem.
 *                        If NULL then a new queue will be created.  This value
 *                        is updated to the current element at the end off the
 *                        call.
 *	Returns: MR_CONT, or MR_ABORT if it has problems.
 */

int
StoreInfo(argc, argv, data)
int argc;
char ** argv;
char * data;
{
    char ** info = (char **) malloc( MAX_ARGS_SIZE * sizeof(char *));
    struct qelem ** old_elem = (struct qelem **) data;
    struct qelem * new_elem = (struct qelem *) malloc (sizeof (struct qelem));
    int count;

    if ( (new_elem == (struct qelem *) NULL) || (info == (char **) NULL) ) {
	Put_message("Could Not allocate more memory.");
	FreeQueue(*old_elem);
	*old_elem = (struct qelem *) NULL;
	return(MR_ABORT);
    }

    for (count = 0; count < argc; count++)
	info[count] = Strsave(argv[count]);
    info[count] = NULL;		/* NULL terminate this sucker. */

    new_elem->q_data = (char *) info;
    AddQueue(new_elem, *old_elem);

    *old_elem = new_elem;
    return(MR_CONT);
}

/*	Function Name: CountArgs
 *	Description:  Retrieve the number of args in a null terminated
 *                     arglist.
 *	Arguments: info - the argument list.
 *	Returns: number if args in the list.
 */

int
CountArgs(info)
char ** info;
{
    int number = 0;
    
    while (*info != NULL) {
	number++;
	info++;
    }

    return(number);
}    

/*	Function Name: Scream
 *	Description: Bitch Loudly and exit, it is intended as a callback
 *                   function for queries that should never return a value.
 *	Arguments: none
 *	Returns: doesn't exit.
 */

int
Scream()
{
    com_err(program_name, 0,
	    "\nA Moira update returned a value -- programmer botch\n");
    mr_disconnect();
    exit(1);
}

/*	Function Name: PromptWithDefault
 *	Description: allows a user to be prompted for input, and given a 
 *                   default choice.
 *	Arguments: prompt - the prompt string.
 *                 buf, buflen - buffer to be returned and its MAX size?
 *                 default value for the answer.
 *	Returns: zero on failure
 */

int
PromptWithDefault(prompt, buf, buflen, def)
char *prompt, *buf;
int buflen;
char *def;
{
    char tmp[BUFSIZ];
    int ans;

    if (parsed_argc > 0) {
	parsed_argc--;
	strncpy(buf, parsed_argv[0], buflen);
	sprintf(tmp, "%s: %s", prompt, buf);
	Put_message(tmp);
	parsed_argv++;
	return(1);
    }

    (void) sprintf(tmp, "%s [%s]: ", prompt, def ? def : "");
    ans = Prompt_input(tmp, buf, buflen);
    if (ans == 0) {
	if (YesNoQuestion("Are you sure you want to exit", 1)) {
	    Cleanup_menu();
	    exit(0);
	}
	Put_message("Continuing input...");
	return(PromptWithDefault(prompt, buf, buflen, def));
    }
    if (IS_EMPTY(buf))
	(void) strcpy(buf, def);
    else if (!strcmp(buf, "\"\""))
	*buf = 0;
    return(ans);
}

/*	Function Name: YesNoQuestion
 *	Description: This prompts the user for the answer to a yes-no or
 *                   true-false question.
 *	Arguments: prompt - the prompt for the user.
 *                 bool_def - the default value either TRUE or FALSE.
 *	Returns: TRUE or FALSE or -1 on error
 */

Bool
YesNoQuestion(prompt, bool_def)
char *prompt;
int bool_def;
{
    char ans[2];

    while (TRUE) {
	if (!PromptWithDefault(prompt, ans, 2, bool_def ? "y" : "n"))
	    return(-1);
	switch (ans[0]) {
	case 'n':
	case 'N':
	    return(FALSE);
	case 'y':
	case 'Y':
	    return(TRUE);
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

Bool
YesNoQuitQuestion(prompt, bool_def)
char *prompt;
int bool_def;
{
    char ans[2];

    while (TRUE) {
	if (!PromptWithDefault(prompt, ans, 2, bool_def ? "y" : "n"))
	    return(-1);
	switch (ans[0]) {
	case 'n':
	case 'N':
	    return(FALSE);
	case 'y':
	case 'Y':
	    return(TRUE);
	case 'q':
	case 'Q':
	    return(-1);
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

Bool
Confirm(prompt)
char * prompt;
{
  return( !verbose || (YesNoQuestion(prompt,FALSE) == TRUE) );
}

/*	Function Name: ValidName
 *	Description: This function checks to see if we have a valid list name.
 *	Arguments: s - the list name.
 *	Returns: TRUE if Valid.
 */

Bool
ValidName(s)
char *s;
{
    if (IS_EMPTY(s))
	Put_message("Please use a non-empty name.");
    else if (index(s, ' '))
	Put_message("You cannot use space (' ') in this name.");
    else if (index(s, '*') || index(s, '?') || index(s, '['))
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

int 
ToggleVerboseMode()
{

  verbose = !verbose;

  if (verbose)
    Put_message("Delete functions will first confirm\n");
  else
    Put_message("Delete functions will be silent\n");
    
  return(DM_NORMAL);
}

/*	Function Name: NullFunc
 *	Description:  dummy callback routine 
 *	Arguments: none
 *	Returns: MR_CONT
 */

int
NullFunc()
{
    return(MR_CONT);
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

void
SlipInNewName(info, name)
char ** info;
char * name;
{
    register int i;

    /* This also pushes the NULL down. */
    for (i = CountArgs(info); i > 0; i--) { 
	info[i+1] = info[i];
    }
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

int
GetValueFromUser(prompt, pointer)
char * prompt, ** pointer;
{
    char buf[BUFSIZ];

    if (PromptWithDefault(prompt, buf, BUFSIZ, *pointer) == 0)
	return(SUB_ERROR);

/* 
 * If these are the same then there is no need to allocate a new string.
 *
 * a difference that makes no difference, IS no difference.
 */

    if (strcmp(buf, *pointer) != 0) { 
	if (*pointer != NULL)
	    free(*pointer);
	*pointer = Strsave(buf);
    }
    return(SUB_NORMAL);
}

/*	Function Name: GetYesNoValueFromUser
 *	Description: This function gets a value from a user for the field
 *                   specified.
 *	Arguments: prompt - prompt for user.
 *                 pointer - pointer to default value, will be returned
 *                          as new value.
 *	Returns: SUB_ERROR if break hit (^C).
 */

int
GetYesNoValueFromUser(prompt, pointer)
char * prompt, ** pointer;
{
    char user_prompt[BUFSIZ];
    Bool default_val;

    if ( strcmp (*pointer, DEFAULT_YES) == 0 )
	default_val = TRUE;
    else
	default_val = FALSE;
    
    sprintf(user_prompt, "%s (y/n)", prompt);

    switch (YesNoQuestion(user_prompt, default_val)) {
    case TRUE:
	if (*pointer != NULL)
	    free(*pointer);
	*pointer = Strsave(DEFAULT_YES);
	break;
    case FALSE:
	if (*pointer != NULL)
	    free(*pointer);
	*pointer = Strsave(DEFAULT_NO);
	break;
    case -1:
    default:
	return(SUB_ERROR);
    }
    return(SUB_NORMAL);
}

/*	Function Name: GetFSVal
 *	Description: asks about a specific filesystem value.
 *	Arguments: name - string for this type of filesystem.
 *                 mask - mask for this type of filesystem.
 *                 current - current filesystem state. (for defaults).
 *                 new - new filesystem state.
 *	Returns: TRUE if successful.
 */

static Bool
GetFSVal(name, mask, current, new)
char * name;
int mask, current, *new;
{
    char temp_buf[BUFSIZ];
    sprintf(temp_buf, "Is this a %s filsystem", name);
    switch (YesNoQuestion(temp_buf, ( (mask & current) == mask) )) {
    case TRUE:
	*new |= mask;
	break;
    case FALSE:
	break;			/* zero by default. */
    default:
	return(FALSE);
    }
    return(TRUE);
}

/*	Function Name: GetFSTypes
 *	Description: Allows user to specify filsystem types.
 *	Arguments: current - current value of filsystem, freed here.
 *	Returns: SUB_ERROR on ^C.
 */

int
GetFSTypes(current, options)
char **  current;
int options;
{
    int c_value, new_val = 0;	/* current value of filesys type (int). */
    char ret_value[BUFSIZ];

    if (*current == NULL)
	c_value = 0;
    else 
	c_value = atoi(*current);

    if (GetFSVal("student", MR_FS_STUDENT, c_value, &new_val) == FALSE)
	return(SUB_ERROR);
    if (GetFSVal("faculty", MR_FS_FACULTY, c_value, &new_val) == FALSE)
	return(SUB_ERROR);
    if (GetFSVal("staff", MR_FS_STAFF, c_value, &new_val) == FALSE)
	return(SUB_ERROR);
    if (GetFSVal("miscellaneous", MR_FS_MISC, c_value, &new_val) == FALSE)
	return(SUB_ERROR);
    if (options) {
	if (GetFSVal("Group Quotas", MR_FS_GROUPQUOTA, c_value, &new_val) ==
	    FALSE)
	  return(SUB_ERROR);
    }

    FreeAndClear(current, TRUE);
    sprintf(ret_value, "%d", new_val);
    *current = Strsave(ret_value);
    return(SUB_NORMAL);
}

/*	Function Name: Strsave
 *	Description: save a string.
 *	Arguments: string  - the string to save.
 *	Returns: The malloced string, now safely saved, or NULL.
 */

char *
Strsave(str)
char *str;
{
    register char *newstr = malloc((unsigned) strlen(str) + 1);

    if (newstr == (char *) NULL)
	return ((char *) NULL);
    else
	return (strcpy(newstr, str));
}

/*	Function Name: Print
 *	Description: prints out all the arguments on a single line.
 *	Arguments: argc, argv - the standard MR arguments.
 *                 callback - the callback function - NOT USED.
 *	Returns: MR_CONT
 */

/* ARGSUSED */
int
Print(argc, argv, callback)
int argc;
char **argv, *callback;
{
    char buf[BUFSIZ];
    register int i;

    found_some = TRUE;
    (void) strcpy(buf,argv[0]);	/* no newline 'cause Put_message adds one */
    for (i = 1; i < argc; i++)
	(void) sprintf(buf,"%s %s",buf,argv[i]);
    (void) Put_message(buf);

    return (MR_CONT);
}

/*	Function Name: PrintByType
 *	Description: This function prints all members of the type specified
 *                   by the callback arg, unless the callback is NULL, in which
 *                   case it prints all members.
 *	Arguments: argc, argc - normal arguments for mr_callback function. 
 *                 callback - either a type of member or NULL.
 *	Returns: MR_CONT or MR_QUIT.
 */

/*ARGSUSED*/
int
PrintByType(argc, argv, callback)
int argc;
char **argv, *callback;
{
    if (callback == NULL)
	return( Print(argc, argv, callback) );
    if (strcmp(argv[0], callback) == 0) 
	return( Print(argc, argv, callback) );
    return(MR_CONT);
}

/*	Function Name: PrintHelp
 *	Description: Prints Help Information in a NULL terminated
 *                   char **.
 *	Arguments: message.
 *	Returns: DM_NORMAL.
 */

int
PrintHelp(message)
char ** message;
{
    register int i;

    for (i = 0; i < CountArgs(message); i++)
      Put_message(message[i]);

    return(DM_NORMAL);
}

/*	Function Name: Loop
 *	Description: This function goes through the entire queue, and
 *                   and executes the given function on each element.
 *	Arguments: elem - top element of the queue.
 *                 func - the function to execute.
 *	Returns: none.
 */

void
Loop(elem, func)
FVoid func;
struct qelem * elem;
{
    while (elem != NULL) {
	char ** info = (char **) elem->q_data;
	(*func) (info);
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

void
QueryLoop(elem, print_func, op_func, query_string)
struct qelem *elem;
FVoid op_func;
FCharStar print_func;
char * query_string;
{
    Bool one_item;
    char temp_buf[BUFSIZ], *name;

    elem = QueueTop(elem);
    one_item = (QueueCount(elem) == 1);
    while (elem != NULL) {
	char **info = (char **) elem->q_data;
	
	if (one_item)
	    (*op_func) (info, one_item);
	else {
	    name = (*print_func) (info); /* call print function. */
	    sprintf(temp_buf,"%s %s (y/n/q)", query_string, name);
	    switch(YesNoQuitQuestion(temp_buf, FALSE)) {
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

char *
NullPrint(info)
char ** info;
{
    return(info[NAME]);
}


/*	Function Name: GetTypeValues
 *	Description: gets legal values for a typed object, keeping a cache
 *	Arguments: type name
 *	Returns: argv of values
 */

struct qelem *
GetTypeValues(tname)
char *tname;
{
    int stat;
    char *argv[3], *p, **pp, *strsave();
    struct qelem *elem, *oelem;
    static struct qelem *cache = NULL;
    struct cache_elem { char *cache_name; struct qelem *cache_data; } *ce;

    for (elem = cache; elem; elem = elem->q_forw) {
	ce = (struct cache_elem *)elem->q_data;
	if (!strcmp(ce->cache_name, tname))
	    return(ce->cache_data);
    }

    argv[0] = tname;
    argv[1] = "TYPE";
    argv[2] = "*";
    elem = NULL;
    if (stat = do_mr_query("get_alias", 3, argv, StoreInfo, (char *)&elem)) {
	com_err(program_name, stat, " in GetTypeValues");
	return(NULL);
    }
    oelem = elem;
    for (elem = QueueTop(elem); elem; elem = elem->q_forw) {
	pp = (char **) elem->q_data;
	p = strsave(pp[2]);
	FreeInfo(pp);
	elem->q_data = p;
    }
    elem = (struct qelem *) malloc(sizeof(struct qelem));
    ce = (struct cache_elem *) malloc(sizeof(struct cache_elem));
    ce->cache_name = strsave(tname);
    ce->cache_data = QueueTop(oelem);
    elem->q_data = (char  *)ce;
    AddQueue(elem, cache);
    cache = QueueTop(elem);
    return(ce->cache_data);
}


/*	Function Name: GetTypeFromUser
 *	Description: gets a typed value from the user
 *	Arguments: prompt string, type name, buffer pointer
 *	Returns: 
 */

GetTypeFromUser(prompt, tname, pointer)
char *prompt;
char *tname;
char  **pointer;
{
    char def[BUFSIZ], buffer[BUFSIZ], *p, *argv[3];
    struct qelem *elem;
    int stat;

    strcpy(def, *pointer);
    strcpy(buffer, prompt);
    strcat(buffer, " (");
    for (elem = GetTypeValues(tname); elem; elem = elem->q_forw) {
	strcat(buffer, elem->q_data);
	if (elem->q_forw)
	    strcat(buffer, ", ");
    }
    strcat(buffer, ")");
    if (strlen(buffer) > 64)
	sprintf(buffer, "%s (? for help)", prompt);
    GetValueFromUser(buffer, pointer);
    if (**pointer == '?') {
	sprintf(buffer, "Type %s is one of:", tname);
	Put_message(buffer);
	for (elem = GetTypeValues(tname); elem; elem = elem->q_forw) {
	    Put_message(elem->q_data);
	}
	*pointer = strsave(def);
	return(GetTypeFromUser(prompt, tname, pointer));
    }
    for (elem = GetTypeValues(tname); elem; elem = elem->q_forw) {
	if (!strcasecmp(elem->q_data, *pointer)) {
	    strcpy(*pointer, elem->q_data);
	    return(SUB_NORMAL);
	}
    }
    sprintf(buffer, "\"%s\" is not a legal value for %s.  Use one of:",
	    *pointer, tname);
    Put_message(buffer);
    for (elem = GetTypeValues(tname); elem; elem = elem->q_forw) {
	Put_message(elem->q_data);
    }
    sprintf(buffer, "Are you sure you want \"%s\" to be a legal %s",
	    *pointer, tname);
    if (YesNoQuestion("Do you want this to be a new legal value", 0) &&
	YesNoQuestion(buffer, 0)) {
	argv[0] = tname;
	argv[1] = "TYPE";
	argv[2] = *pointer;
	/* don't uppercase access flags.  Do uppercase everything else */
	if (strncmp(tname, "fs_access", 9))
	  for (p = argv[2]; *p; p++)
	    if (islower(*p))
	      *p = toupper(*p);
	if (stat = do_mr_query("add_alias", 3, argv, Scream, NULL)) {
	    com_err(program_name, stat, " in add_alias");
	} else {
	    elem = (struct qelem *) malloc(sizeof(struct qelem));
	    elem->q_data = strsave(*pointer);
	    AddQueue(elem, GetTypeValues(tname));
	    Put_message("Done.");
	}
    }
    *pointer = strsave(def);
    return(GetTypeFromUser(prompt, tname, pointer));
}


do_mr_query(name, argc, argv, proc, hint)
char *name;
int argc;
char **argv;
int (*proc)();
char *hint;
{
    int status;
    extern char *whoami, *moira_server;

    status = mr_query(name, argc, argv, proc, hint);
    if (status != MR_ABORTED && status != MR_NOT_CONNECTED)
      return(status);
    status = mr_connect(moira_server);
    if (status) {
	com_err(whoami, status, " while re-connecting to server %s",
		moira_server);
	return(MR_ABORTED);
    }
    status = mr_auth(whoami);
    if (status) {
	com_err(whoami, status, " while re-authenticating to server %s",
		moira_server);
	mr_disconnect();
	return(MR_ABORTED);
    }
    status = mr_query(name, argc, argv, proc, hint);
    return(status);
}

