/* $Id: main.c,v 1.32 1998-03-26 21:07:19 danw Exp $
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
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/main.c,v 1.32 1998-03-26 21:07:19 danw Exp $");

static void ErrorExit(char *buf, int status);
static void Usage(void);
static void Signal_Handler(void);
static void CatchInterrupt(void);

char *whoami;			/* used by menu.c ugh!!! */
char *moira_server;
int interrupt = 0;

extern Menu moira_top_menu, list_menu, user_menu, dcm_menu;

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
  char *motd, **arg, pname[ANAME_SZ];
  struct sigaction act;

  if ((status = tf_init(TKT_FILE, R_TKT_FIL)) ||
      (status = tf_get_pname(pname)))
    {
      com_err(whoami, status, "reading Kerberos ticket file");
      exit(1);
    }
  tf_close();
  user = pname;

  if (!(program_name = strrchr(argv[0], '/')))
    program_name = argv[0];
  else
    program_name++;
  program_name = strdup(program_name);
  whoami = strdup(program_name); /* used by menu.c,  ugh !!! */

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
		  Usage();
	    }
	  else
	    Usage();
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
  act.sa_handler = Signal_Handler;
  sigaction(SIGHUP, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  if (use_menu)
    sigaction(SIGINT, &act, NULL);
  else
    {
      act.sa_handler = CatchInterrupt;
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
