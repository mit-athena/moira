#if (!defined(lint) && !defined(SABER))
  static char rcsid_module_c[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/namespace.c,v 1.4 1993-10-22 16:21:25 mar Exp $";
#endif lint

/*	This is the file main.c for the Moira Client, which allows a nieve
 *      user to quickly and easily maintain most parts of the Moira database.
 *	It Contains: The main driver for the Moira Client.
 *	
 *	Created: 	4/12/88
 *	By:		Chris D. Peterson
 *
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/namespace.c,v $
 *      $Author: mar $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/namespace.c,v 1.4 1993-10-22 16:21:25 mar Exp $
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
#include <krb.h>


#include "mit-copyright.h"
#include "defs.h"
#include "f_defs.h"
#include "globals.h"

char * whoami;			/* used by menu.c ugh!!! */
char * moira_server;
int interrupt = 0;
int NewListHelp();

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
	SIMPLEFUNC("admin",
		   "Show all items which a given member can administer",
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
    SUBMENU("mail","Mail Forwarding", &pobox_menu),
    SUBMENU("list","Mailing Lists", &list_menu),
    {ShowUserByLogin, NULLMENU, 2, {
       {"account", "Show user account information"},
       {"login name", "Desired login name: "}
     } },
    SUBMENU("misc", "Miscellaneous", &misc_menu)
  }
};


#ifndef DEBUG
static void SignalHandler(), CatchInterrupt();
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
    char pname[ANAME_SZ];
#ifdef POSIX
    struct sigaction act;
#endif

    if ((program_name = strrchr(argv[0], '/')) == NULL)
      program_name = argv[0];
    else
      program_name++;
    program_name = Strsave(program_name);
    whoami = Strsave(program_name); /* used by menu.c,  ugh !!! */

    verbose = TRUE;
    arg = argv;
    moira_server = NULL;

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

    if ( status = mr_connect(moira_server) ) 
	ErrorExit("\nConnection to Moira server failed", status);

    /* do this now since calling mr_connect initialized the krb error table
     * for us.
     */
    if ((status = tf_init(TKT_FILE, R_TKT_FIL)) ||
	(status = tf_get_pname(pname))) {
	com_err(whoami, status, "cannot find your ticket file");
	exit(1);
    }
    tf_close();
    user = Strsave(pname);

    if ( status = mr_motd(&motd) )
        ErrorExit("\nUnable to check server status", status);
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	mr_disconnect();
	exit(1);
    }

    if ( status = mr_auth(program_name) ) {
	if (status == MR_USER_AUTH) {
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
#ifdef POSIX
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler= (void (*)()) SignalHandler;
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGQUIT, &act, NULL);
    if (use_menu)
      (void) sigaction(SIGINT, &act, NULL); 
    else {
	act.sa_handler= (void (*)()) CatchInterrupt;
	(void) sigaction(SIGINT, &act, NULL); 
    }
#else
    (void) signal(SIGHUP, SignalHandler);
    (void) signal(SIGQUIT, SignalHandler);
    if (use_menu)
      (void) signal(SIGINT, SignalHandler); 
    else
      (void) signal(SIGINT, CatchInterrupt); 
#endif /* POSIX */
#endif /* DEBUG */

    menu = &namespace_menu;

    if (use_menu) {		/* Start menus that execute program */
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

static void
ErrorExit(buf,status)
int status;
char * buf;    
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
    mr_disconnect();
    exit(1);
}


static void
CatchInterrupt()
{
    Put_message("Interrupt! Press RETURN to continue");
    interrupt = 1;
}
#endif DEBUG


/* Dummy routine to be able to link against the rest of the moira client */

DeleteUser()
{
    return(DM_QUIT);
};


int
NewListHelp()
{
    static char * message[] = {
	"A list can be a mailing list, an Athena group list, or both.  Each",
	"list has an owner and members.  The owner of a list may be the list",
	"itself, another list, or a user.  The members of a list can be users",
	"(login names), other lists, or foreign address strings.  You can use",
	"certain keys to do the following:",
	"    Refresh the screen - Type ctrl-L.",
	"    Escape from a function - Type ctrl-C.",
	NULL,
    };

    return(PrintHelp(message));
}
