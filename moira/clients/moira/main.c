#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.29 1998-01-07 17:13:00 danw Exp $";
#endif

/*	This is the file main.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: The main driver for the Moira Client.
 *
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.29 1998-01-07 17:13:00 danw Exp $
 *
 *  	Copyright 1988 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution
 *	see the file mit-copyright.h
 */

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <moira.h>
#include <menu.h>
#include <krb_et.h>

#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

char *whoami;			/* used by menu.c ugh!!! */
char *moira_server;
int interrupt = 0;

extern Menu moira_top_menu, list_menu, user_menu, dcm_menu;

static void Signal_Handler(), CatchInterrupt();

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

void main(int argc, char **argv)
{
  int status;
  Menu *menu;
  char *motd, **arg;
  struct sigaction act;

  if (!(user = getlogin()))
    user = getpwuid(getuid())->pw_name;
  user = (user && strlen(user)) ? Strsave(user) : "";

  if (!(program_name = strrchr(argv[0], '/')))
    program_name = argv[0];
  else
    program_name++;
  program_name = Strsave(program_name);
  whoami = Strsave(program_name); /* used by menu.c,  ugh !!! */

  verbose = TRUE;
  arg = argv;
  moira_server = NULL;

  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (!strcmp(*arg, "-nomenu"))
	    use_menu = FALSE;
	  else if (!strcmp(*arg, "-menu"))
	    use_menu = TRUE;
	  else if (!strcmp(*arg, "-db"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  moira_server = *arg;
		} else
		  Usage(argv);
	    }
	  else
	    Usage(argv);
	}
    }

  if ((status = mr_connect(moira_server)))
    ErrorExit("\nConnection to Moira server failed", status);

  if ((status = mr_motd(&motd)))
    ErrorExit("\nUnable to check server status", status);
  if (motd)
    {
      fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
	      motd);
      mr_disconnect();
      exit(1);
    }

  if ((status = mr_auth(program_name)))
    {
      if (status == MR_USER_AUTH)
	{
	  char buf[BUFSIZ];
	  com_err(program_name, status, "\nPress [RETURN] to continue");
	  fgets(buf, BUFSIZ, stdin);
	}
      else
	{
	  if (status >= ERROR_TABLE_BASE_krb &&
	      status <= ERROR_TABLE_BASE_krb + 256)
	    ErrorExit("\nAuthorization failed -- please run kinit", status);
	  else
	    ErrorExit("\nAuthorization failed.", status);
	}
    }

  /*
   * These signals should not be set until just before we fire up the menu
   * system.
   */

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) Signal_Handler;
  sigaction(SIGHUP, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  if (use_menu)
    sigaction(SIGINT, &act, NULL);
  else
    {
      act.sa_handler = (void (*)()) CatchInterrupt;
      sigaction(SIGINT, &act, NULL);
    }

  initialize_gdss_error_table();

  if (!strcmp(program_name, "listmaint"))
    menu = &list_menu;
  else if (!strcmp(program_name, "usermaint"))
    menu = &user_menu;
  else if (!strcmp(program_name, "dcmmaint"))
    menu = &dcm_menu;
  else
    menu = &moira_top_menu;

  if (use_menu)		/* Start menus that execute program */
    {
      Start_paging();
      Start_menu(menu);
      Stop_paging();
    }
  else			/* Start program without menus. */
    Start_no_menu(menu);

  mr_disconnect();
  exit(0);
}

/*	Function Name: ErrorExit
 *	Description: This function does the error handling and exits.
 *	Arguments: buf - the error message to print.
 *                 status - the error code.
 *	Returns: doesn't return.
 */

static void ErrorExit(char *buf, int status)
{
  com_err(program_name, status, buf);
  mr_disconnect();
  exit(1);
}

/*	Function Name: usage
 *	Description: Prints usage info and then exits.
 *	Arguments: none
 *	Returns: doesn't return.
 */

static void Usage(void)
{
  fprintf(stderr, "Usage: %s [-nomenu | -menu] [-db server[:port]]\n",
	  program_name);
  exit(1);
}

/*	Function Name: Signal_Handler
 *	Description: This function cleans up from a signal interrupt.
 *	Arguments: none.
 *	Returns: doesn't
 */

static void Signal_Handler(void)
{
  Put_message("Signal caught - exiting");
  if (use_menu)
    Cleanup_menu();
  mr_disconnect();
  exit(1);
}


static void CatchInterrupt(void)
{
  Put_message("Interrupt! Press RETURN to continue");
  interrupt = 1;
}
