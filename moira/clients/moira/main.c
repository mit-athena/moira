#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.7 1988-07-30 17:30:17 mar Exp $";
#endif lint

/*	This is the file main.c for the SMS Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the SMS database.
 *	It Contains: The main driver for the SMS Client.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.7 1988-07-30 17:30:17 mar Exp $
 *	
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sms.h>
#include <menu.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

char * whoami;			/* used by menu.c ugh!!! */

extern Menu sms_top_menu, list_menu, user_menu;

#ifndef DEBUG
static void SignalHandler();
#endif DEBUG

static void ErrorExit(), Usage();
char *getlogin();
uid_t getuid();
struct passwd *getpwuid();

Bool use_menu = TRUE;		/* whether or not we are using a menu. */

/*	Function Name: main
 *	Description: The main driver for the SMS Client.
 *	Arguments: argc, argv - standard command line args.
 *	Returns: doesn't return.
 */

void
main(argc, argv)
    int argc;
    char ** argv;
{
    int status;
    Menu *menu;

    if ((user = getlogin()) == NULL) 
	user = getpwuid((int) getuid())->pw_name;
    user = (user && strlen(user)) ? Strsave(user) : "";

    init_sms_err_tbl();
    init_krb_err_tbl();
    verbose = TRUE;

    switch (argc) {
    case 2:
      if (strcmp(argv[1], "-nomenu") == 0)
	use_menu = FALSE;
      else 
	Usage();
				/* Fall Through. */
    case 1:
      if ((program_name = rindex(argv[0], '/')) == NULL)
	program_name = argv[0];
      else
	program_name++;
      break;
    default:
      Usage();
      break;
    }

    program_name = Strsave(program_name);
    whoami = Strsave(program_name); /* used by menu.c,  ugh !!! */

    if ( status = sms_connect() ) 
	ErrorExit("\nConnection to SMS server failed", status);

    if ( status = sms_auth(program_name) ) 
	ErrorExit("\nAuthorization failed -- please run kinit", status);

/*
 * These signals should not be set until just before we fire up the menu
 * system. 
 */

#ifndef DEBUG
    (void) signal(SIGHUP, SignalHandler);
    (void) signal(SIGINT, SignalHandler); 
    (void) signal(SIGQUIT, SignalHandler);
#endif DEBUG

    if (!strcmp(program_name, "listmaint"))
      menu = &list_menu;
    else if (!strcmp(program_name, "usermaint"))
      menu = &user_menu;
    else
      menu = &sms_top_menu;

    if (use_menu) {		/* Start menus that execute program */
        Start_paging();
	Start_menu(menu);
	Stop_paging();
    }
    else			/* Start program without menus. */
	Start_no_menu(menu);

    sms_disconnect();
    exit(0);
}

/*	Function Name: ErrorExit
 *	Description: This function does the error handling and exits.
 *	Arguments: buf - the error message to print.
 *                 status - the error code.
 *	Returns: doesn't return.
 */

static void
ErrorExit(buf,status)
int status;
char * buf;    
{
    com_err(program_name, status, buf);
    sms_disconnect();
    exit(1);
}

/*	Function Name: usage
 *	Description: Prints usage info and then exits.
 *	Arguments: none
 *	Returns: doesn't return.
 */

static void
Usage()
{
    fprintf(stderr, "Usage: %s [-nomenu]\n", program_name);
    exit(1);
}

#ifndef DEBUG
/*	Function Name: SignalHandler
 *	Description: This function cleans up from a signal interrupt.
 *	Arguments: none.
 *	Returns: doesn't
 */

static void
SignalHandler()
{
    Put_message("Signal caught - exiting");
    if (use_menu)
      Cleanup_menu();
    sms_disconnect();
    exit(1);
}
#endif DEBUG
