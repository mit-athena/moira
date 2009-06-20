/* $Id: namespace.c,v 1.17 2009-05-04 20:49:10 zacheiss Exp $
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

RCSID("$Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/namespace.c,v 1.17 2009-05-04 20:49:10 zacheiss Exp $");

static void ErrorExit(char *buf, int status);
static void Usage(void);
static void Signal_Handler(void);
static void CatchInterrupt(void);
int NewListHelp(int argc, char **argv);

char *whoami;			/* used by menu.c ugh!!! */
char *moira_server;
int interrupt = 0;

/*
 * List Information Menu
 */

Menu list_info_menu = {
  NULLFUNC,
  NULLFUNC,
  "List Information Menu",
  3,
  {
    SIMPLEFUNC("member", "Show all lists to which a given member belongs",
	       ListByMember),
    SIMPLEFUNC("admin", "Show all items which a given member can administer",
	       ListByAdministrator),
    SIMPLEFUNC("public", "Show all public mailing lists",
	       ListAllPublicMailLists),
  }
};

/*
 * List Member Menu
 */

Menu list_member_menu = {
  ListmaintMemberMenuEntry,
  ListmaintMemberMenuExit,
  NULL,
  4,
  {
    SIMPLEFUNC("add", "Add a member to this list", AddMember),
    SIMPLEFUNC("remove", "Remove a member from this list", DeleteMember),
    SIMPLEFUNC("show", "Show the members of this list", ListAllMembers),
    SIMPLEFUNC("verbose", "Toggle Verbosity of Delete", ToggleVerboseMode)
  }
};

/*
 * List Menu
 */

Menu list_menu = {
  NULLFUNC,
  NULLFUNC,
  "List Menu",
  6,
  {
    { ShowListInfo, NULLMENU, 2, {
      {"show", "Display information about a list"},
      {"list name", "Name of list: "}
    } },
    { UpdateList, NULLMENU, 2, {
      {"update", "Update characteristics of a list"},
      {"list name", "Name of list: "}
    } },
    SIMPLEFUNC("query_remove",
	       "Interactively remove a member from all lists",
	       InterRemoveItemFromLists),
    { NULLFUNC, &list_member_menu, 2, {
      {"members", "Member Menu - Change/Show Members of a List."},
      {"list name", "Name of list: "}
    } },
    SUBMENU("lists", "Find Mailing Lists Menu",
	    &list_info_menu),
    SIMPLEFUNC("help", "Print Help", NewListHelp)
  }
};

/*
 * Post Office Box Menu
 */

Menu pobox_menu = {
  NULLFUNC,
  NULLFUNC,
  "Mail Forwarding Menu",
  3,
  {
    {GetUserPOBox, NULLMENU, 2, {
      {"show", "Show a user's post office box"},
      {"user name", "user name: "}
    } },
    {SetUserPOBox, NULLMENU, 2, {
      {"set", "Set a user's post office box (mail forwarding)"},
      {"user name", "user name: "}
    } },
    {RemoveUserPOBox, NULLMENU, 2, {
      {"remove", "Remove a user's post office box"},
      {"user name", "user name: "}
    } },
  }
};

/*
 * Miscellaneous Menu
 */

Menu misc_menu = {
  NULLFUNC,
  NULLFUNC,
  "Miscellaneous Menu",
  2,
  {
    SIMPLEFUNC("statistics", "Show database statistics", TableStats),
    SIMPLEFUNC("clients", "Show active Moira clients", ShowClients),
  }
};


Menu namespace_menu = {
  NULLFUNC,
  NULLFUNC,
  "Campus Namespace Database Manipulation",
  4,
  {
    SUBMENU("mail", "Mail Forwarding", &pobox_menu),
    SUBMENU("list", "Mailing Lists", &list_menu),
    {ShowUserByLogin, NULLMENU, 2, {
       {"account", "Show user account information"},
       {"login name", "Desired login name: "}
     } },
    SUBMENU("misc", "Miscellaneous", &misc_menu)
  }
};

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
  char *motd, **arg;
  struct sigaction act;

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
	    if (arg - argv < argc - 1)
	      {
		++arg;
		moira_server = *arg;
	      }
	    else
	      Usage();
	  else
	    Usage();
	}
    }

  if (mrcl_connect(moira_server, program_name, QUERY_VERSION, 0)
      != MRCL_SUCCESS)
    exit(1);

  if ((status = mr_krb5_auth(program_name)))
    {
      if (status == MR_UNKNOWN_PROC)
	status = mr_auth(program_name);

      if (status)
	{
	  if (status == MR_USER_AUTH)
	    {
	      char buf[BUFSIZ];
	      com_err(program_name, status, "\nPress [RETURN] to continue");
	      fgets(buf, BUFSIZ, stdin);
	    }
	  else
	    ErrorExit("\nAuthorization failed -- please run kinit", status);
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
#ifdef HAVE_CURSES
  if (use_menu)
    sigaction(SIGINT, &act, NULL);
  else
#endif
    {
      act.sa_handler = CatchInterrupt;
      sigaction(SIGINT, &act, NULL);
    }

  menu = &namespace_menu;

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
  fprintf(stderr, "Usage: %s [-nomenu]\n", program_name);
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
#ifdef HAVE_CURSES
  if (use_menu)
    Cleanup_menu();
#endif
  mr_disconnect();
  exit(1);
}


static void CatchInterrupt(void)
{
  Put_message("Interrupt! Press RETURN to continue");
  interrupt = 1;
}


/* Dummy routine to be able to link against the rest of the moira client */

int DeleteUser(int argc, char **argv)
{
  return DM_QUIT;
}


int NewListHelp(int argc, char **argv)
{
  static char *message[] = {
    "A list can be a mailing list, an Athena group list, or both.  Each",
    "list has an owner and members.  The owner of a list may be the list",
    "itself, another list, or a user.  The members of a list can be users",
    "(login names), other lists, or foreign address strings.  You can use",
    "certain keys to do the following:",
    "    Refresh the screen - Type ctrl-L.",
    "    Escape from a function - Type ctrl-C.",
    NULL,
  };

  return PrintHelp(message);
}
