#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.15 1989-08-25 12:36:29 mar Exp $";
#endif lint

/*	This is the file main.c for the Moira Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the Moira database.
 *	It Contains: The main driver for the Moira Client.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.15 1989-08-25 12:36:29 mar Exp $
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
char * moira_server;

extern Menu sms_top_menu, list_menu, user_menu, dcm_menu;

#ifndef DEBUG
static void SignalHandler();
#endif DEBUG

static void ErrorExit(), Usage();
char *getlogin();
uid_t getuid();
struct passwd *getpwuid();

Bool use_menu = TRUE;		/* whether or not we are using a menu. */

/*	Function Name: main
 *	Description: The main driver for the Moira Client.
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
    char *motd, **arg;

    if ((user = getlogin()) == NULL) 
	user = getpwuid((int) getuid())->pw_name;
    user = (user && strlen(user)) ? Strsave(user) : "";

    if ((program_name = rindex(argv[0], '/')) == NULL)
      program_name = argv[0];
    else
      program_name++;
    program_name = Strsave(program_name);
    whoami = Strsave(program_name); /* used by menu.c,  ugh !!! */

    verbose = TRUE;
    arg = argv;
    moira_server = SMS_SERVER;

    while (++arg - argv < argc) {
	if (**arg == '-') {
	    if (!strcmp(*arg, "-nomenu"))
	      use_menu = FALSE;
	    else if (!strcmp(*arg, "-db"))
	      if (arg - argv < argc - 1) {
		  ++arg;
		  moira_server = *arg;
	      } else
		Usage(argv);
	    else
	      Usage(argv);
	}
    }

    if ( status = sms_connect(moira_server) ) 
	ErrorExit("\nConnection to Moira server failed", status);

    if ( status = sms_motd(&motd) )
        ErrorExit("\nUnable to check server status", status);
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	sms_disconnect();
	exit(1);
    }

    if ( status = sms_auth(program_name) ) {
	if (status == SMS_USER_AUTH) {
	    char buf[BUFSIZ];
	    com_err(program_name, status, "\nPress [RETURN] to continue");
	    gets(buf);
	} else
	  ErrorExit("\nAuthorization failed -- please run kinit", status);
    }

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
    else if (!strcmp(program_name, "dcmmaint"))
      menu = &dcm_menu;
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
