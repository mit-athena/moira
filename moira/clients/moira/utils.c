#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v 1.4 1988-06-29 20:13:16 kit Exp $";
#endif lint

/*	This is the file utils.c for allmaint, the SMS client that allows
 *      a user to maintaint most important parts of the SMS database.
 *	It Contains: Many utilities used by allmaint.
 *	
 *	Created: 	4/25/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v $
 *      $Author: kit $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/utils.c,v 1.4 1988-06-29 20:13:16 kit Exp $
 *	
 *  	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */


#include <stdio.h>
#include <strings.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "allmaint.h"
#include "allmaint_funcs.h"
#include "globals.h"
#include "infodefs.h"

#include <netdb.h>		/* for gethostbyname. */
/* #include <ctype.h> */
/* #include <varargs.h> */
/* #include <sys/types.h> */

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

void
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

/*	Function Name: AddQueue
 *	Description: Adds an element to a queue
 *	Arguments: elem, pred - element and its predecessor.
 *	Returns: none.
 */

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

RemoveQueue(elem)
struct qelem *elem;
{
    if (elem->q_forw != NULL) 
	(elem->q_forw)->q_back = elem->q_back;
    if (elem->q_back != NULL)
	(elem->q_back)->q_forw = elem->q_forw;
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
	return(SMS_ABORT);
    }

    for (count = 0; count < argc; count++)
	info[count] = Strsave(argv[count]);
    info[count] = NULL;		/* NULL terminate this sucker. */

    new_elem->q_data = (char *) info;
    AddQueue(new_elem, *old_elem);

    *old_elem = new_elem;
    return(SMS_CONT);
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
	    "\nAn SMS update returned a value -- programmer botch\n");
    sms_disconnect();
    exit(1);
}

/*	Function Name: PromptWithDefault
 *	Description: allows a user to be prompted for input, and given a 
 *                   default choice.
 *	Arguments: prompt - the prompt string.
 *                 buf, buflen - buffer to be returned and its MAX size?
 *                 default value for the answer.
 *	Returns: the value returned by prompt_input.
 */

int
PromptWithDefault(prompt, buf, buflen, def)
char *prompt, *buf;
int buflen;
char *def;
{
    char tmp[BUFSIZ];
    int ans;

    (void) sprintf(tmp, "%s [%s]: ", prompt, def ? def : "");
    ans = Prompt_input(tmp, buf, buflen);
    if (IS_EMPTY(buf))
	(void) strcpy(buf, def);
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
    char ans;

    while (TRUE) {
	if (!PromptWithDefault(prompt, &ans, 1, bool_def ? "y" : "n"))
	    return(-1);
	switch (ans) {
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
    char ans;

    while (TRUE) {
	if (!PromptWithDefault(prompt, &ans, 1, bool_def ? "y" : "n"))
	    return(-1);
	switch (ans) {
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

int
Confirm(prompt)
char * prompt;
{
  return( !verbose || YesNoQuestion(prompt,FALSE) );
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
    else if (index(s, WILDCARD))
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
 *	Returns: SMS_CONT
 */

int
NullFunc()
{
    return(SMS_CONT);
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

SlipInNewName(info, name)
char ** info;
char * name;
{
    register int i;

    /* This also pushes the NULL down. */
    for (i = CountArgs(info); i > 1; i--) { 
	info[i] = info[i-1];
    }
    info[1] = name;	/* now slip in the name. */
}

/*	Function Name: GetValueFromUser
 *	Description: This function gets a value from a user for the field
 *                   specified.
 *	Arguments: prompt - prompt for user.
 *                 pointer - pointer to default value, will be returned
 *                          as new value.
 *	Returns: none.
 */

void
GetValueFromUser(prompt, pointer)
char * prompt, ** pointer;
{
    char buf[BUFSIZ];

    PromptWithDefault(prompt, buf, BUFSIZ, *pointer);
    if (*pointer != NULL)
	free(*pointer);
    *pointer = Strsave(buf);
}

/*	Function Name: CanonicalizeHostname
 *	Description: This function takes a machine name and canonicalize's it.
 *	Arguments: machine - name of the machine to work on.
 *	Returns: new name or NULL if nameserver returns error
 */

char *
CanonicalizeHostname(machine)
char *machine;
{
    struct hostent *hostinfo;

    hostinfo = gethostbyname(machine);
/* If this fails then we just return what we were passed. */
    if (hostinfo != (struct hostent *) NULL)
	machine = hostinfo->h_name;
    return (machine);
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

/*	Function Name: EncryptMITID
 *	Description: Encrypts an mit ID number. 
 *	Arguments: sbuf - the buffer to return the encrypted number in.
 *                 idnumber - the id number (string).
 *                 first, last - name of the person.
 *	Returns: none.
 */

void
EncryptMITID(sbuf, idnumber, first, last)
char *sbuf, *idnumber, *first, *last;
{
    char salt[3];
    extern char *crypt();

#define _tolower(c) ((c)|0x60)

    salt[0] = _tolower(last[0]);
    salt[1] = _tolower(first[0]);
    salt[2] = 0;

    (void) strcpy(sbuf, crypt(&idnumber[2], salt));
}

/*	Function Name: RemoveHyphens
 *	Description: Removes all hyphens from the string passed to it.
 *	Arguments: str - the string to remove the hyphes from
 *	Returns: none
 */

void
RemoveHyphens(str)
char *str;
{
    char *hyphen;

    while ((hyphen = index(str, '-')) != NULL)
	(void) strcpy(hyphen, hyphen + 1);
}

/*	Function Name: Print
 *	Description: prints out all the arguments on a single line.
 *	Arguments: argc, argv - the standard SMS arguments.
 *                 callback - the callback function - NOT USED.
 *	Returns: SMS_CONT
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

    return (SMS_CONT);
}

/*	Function Name: PrintByType
 *	Description: This function prints all members of the type specified
 *                   by the callback arg, unless the callback is NULL, in which
 *                   case it prints all members.
 *	Arguments: argc, argc - normal arguments for sms_callback function. 
 *                 callback - either a type of member or NULL.
 *	Returns: SMS_CONT or SMS_QUIT.
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
	return( Print(1, argv + 1, callback) );
    return(SMS_CONT);
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
    Print(CountArgs(message), message, (char *) NULL);
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

    one_item = (QueueCount(elem) == 1);
    while (elem != NULL) {
	char **info = (char **) elem->q_data;
	
	if (one_item)
	    (*op_func) (info, one_item);
	else {
	    name = (*print_func) (info); /* call print function. */
	    sprintf(temp_buf,"%s %s (y/n/q) ?", query_string, name);
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

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
