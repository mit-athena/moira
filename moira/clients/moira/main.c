/* $Id: main.c,v 1.38 2000-03-15 22:44:03 rbasch Exp $
 *
 *	This is the file main.c for the Moira Client, which allows users
 *      to quickly and easily maintain most parts of the Moira database.
 *	It Contains: The main driver for the Moira Client.
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
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.38 2000-03-15 22:44:03 rbasch Exp $");

static void ErrorExit(char *buf, int status);
static void Usage(void);
static void Signal_Handler(int sig);
static void CatchInterrupt(int sig);
static void SetHandlers(void);

char *whoami;			/* used by menu.c ugh!!! */
char *moira_server;
int interrupt = 0;

extern Menu moira_top_menu, list_menu, user_menu, dcm_menu;

#ifdef HAVE_CURSES
Bool use_menu = TRUE;		/* whether or not we are using a menu. */
#endif

/*	Function Name: main
 *	Description: The main driver for the Moira Client.
 *	Arguments: argc, argv - standard command line args.
 *	Returns: doesn't return.
 */

int main(int argc, char **argv)
{
  int status;
  Menu *menu;
  char **arg;

  if (!(program_name = strrchr(argv[0], '/')))
    program_name = argv[0];
  else
    program_name++;
  program_name = strdup(program_name);
  whoami = strdup(program_name); /* used by menu.c,  ugh !!! */

  user = mrcl_krb_user();
  if (!user)
    exit(1);

  verbose = TRUE;
  arg = argv;
  moira_server = NULL;

  while (++arg - argv < argc)
    {
      if (**arg == '-')
	{
	  if (!strcmp(*arg, "-nomenu"))
	    {
#ifdef HAVE_CURSES
	      use_menu = FALSE;
#else
	      ;
#endif
	    }
	  else if (!strcmp(*arg, "-menu"))
	    {
#ifdef HAVE_CURSES
	      use_menu = TRUE;
#else
	      fprintf(stderr, "%s: No curses support. -menu option ignored\n",
		      whoami);
#endif
	    }
	  else if (!strcmp(*arg, "-db"))
	    {
	      if (arg - argv < argc - 1)
		{
		  ++arg;
		  moira_server = *arg;
		} else
		  Usage();
	    }
	  else
	    Usage();
	}
    }

  if (mrcl_connect(moira_server, program_name, 2, 0) != MRCL_SUCCESS)
    exit(1);

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
  SetHandlers();

  if (!strcmp(program_name, "listmaint"))
    menu = &list_menu;
  else if (!strcmp(program_name, "usermaint"))
    menu = &user_menu;
  else if (!strcmp(program_name, "dcmmaint"))
    menu = &dcm_menu;
  else
    menu = &moira_top_menu;

#ifdef HAVE_CURSES
  if (use_menu)		/* Start menus that execute program */
    {
      Start_paging();
      Start_menu(menu);
      Stop_paging();
    }
  else			/* Start program without menus. */
#endif
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

static void Signal_Handler(int sig)
{
  Put_message("Signal caught - exiting");
#ifdef HAVE_CURSES
  if (use_menu)
    Cleanup_menu();
#endif
  mr_disconnect();
  exit(1);
}


static void CatchInterrupt(int sig)
{
  Put_message("Interrupt! Press RETURN to continue");
  interrupt = 1;
}

#ifdef HAVE_POSIX_SIGNALS
static void SetHandlers(void)
{
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = Signal_Handler;
  sigaction(SIGHUP, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
#ifdef HAVE_CURSES
  if (use_menu)
    sigaction(SIGINT, &act, NULL);
  else
#endif
    {
      act.sa_handler = CatchInterrupt;
      sigaction(SIGINT, &act, NULL);
    }
}
#else
static void SetHandlers(void)
{
  signal(SIGTERM, Signal_Handler);
#ifdef HAVE_CURSES
  if (use_menu)
    signal(SIGINT, Signal_Handler);
  else
#endif
      signal(SIGINT, CatchInterrupt);
}
#endif
