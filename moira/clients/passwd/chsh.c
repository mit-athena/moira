/*
 * Copyright 1988 by the Massachusetts Institute of Technology. For copying
 * and distribution information, see the file "mit-copyright.h". 
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chsh.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chsh.c,v 1.13 1995-11-21 16:55:17 jweiss Exp $
 * $Author: jweiss $
 *
 */

#ifndef lint
static char *rcsid_chsh_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chsh.c,v 1.13 1995-11-21 16:55:17 jweiss Exp $";
#endif not lint

/*
 * Talk to the MOIRA database to change a person's login shell.  The chosen
 * shell must exist.  A warning will be issued if the shell is not in 
 * /etc/shells.
 * 
 * chsh with no modifiers changes the shell of the user who is running
 * the program.
 * 
 * If a commandline argument is given, it is taken to be the username
 * of the user whose shell is to be changed.
 *
 */

#ifdef POSIX
#include <unistd.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <krb.h>
#include <ctype.h>
#include <errno.h>

/* MOIRA includes */
#include <moira.h>
#include <moira_site.h>
#include "mit-copyright.h"

char *whoami;
char *getusershell();

extern char *krb_err_txt[];

main(argc, argv)
    int argc;
    char *argv[];
{
    char pname[ANAME_SZ];
    char *uname = pname;
    int k_errno;
    char *whoami;

    if ((whoami = strrchr(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;

    if (argc > 2) {
	usage();
    }
    
    if (argc == 2)
	uname = argv[1];
    else
    {
	/* Do it right; get name from kerberos ticket file rather than 
	   from passord file. */
	
	if (k_errno = tf_init(TKT_FILE, R_TKT_FIL)) {
	    fprintf(stderr, "%s: %s\n", whoami, krb_err_txt[k_errno]);
	    exit(1);
	}
	
	if (k_errno = tf_get_pname(pname)) {
	    fprintf(stderr, "%s: %s\n", whoami, krb_err_txt[k_errno]);
	    exit(1);
	}

	tf_close();
    }
    
    exit(chsh(uname));
}

leave(status)
  int status;
  /* This should be called rather than exit once connection to moira server
     has been established. */
{
    mr_disconnect();
    exit(status);
}

scream()
{
    com_err(whoami, 0, "Unexpected return value from Moira -- programmer botch");
    leave(1);
}

chsh(uname)
  char *uname;
{
    int status;			/* general purpose exit status */
    int q_argc;			/* argc for mr_query */
    char *q_argv[U_END];	/* argv for mr_query */
    char *motd;			/* determine MR server status */

    int got_one = 0;		/* have we got a new shell yet? */
    char shell[BUFSIZ];		/* the new shell */
    int get_shell();
    void check_shell();		/* make sure new shell is valid */

    /* Try each query.  If we ever fail, print error message and exit. */

    status = mr_connect(NULL);
    if (status) {
	com_err(whoami, status, " while connecting to Moira");
	exit(1);
    }

    status = mr_motd(&motd);
    if (status) {
        com_err(whoami, status, " unable to check server status");
	leave(1);
    }
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	leave(1);
    }

    status = mr_auth("chsh");	/* Don't use argv[0] - too easy to fake */
    if (status) {
	com_err(whoami, status, 
		" while authenticating -- run \"kinit\" and try again.");
	leave(1);
    }

    /* First, do an access check */

    q_argv[USH_NAME] = uname;
    q_argv[USH_SHELL] = "junk";
    q_argc = USH_END;

    if (status = mr_access("update_user_shell", q_argc, q_argv))
    {
	com_err(whoami, status, "; shell not\nchanged.");
	leave(2);
    }

    printf("Changing login shell for %s.\n", uname);

    /* Display current information */

    q_argv[NAME] = uname;
    q_argc = NAME + 1;

    if ((status = mr_query("get_user_account_by_login", q_argc, q_argv, 
			    get_shell, (char *) uname)))
    {
	com_err(whoami, status, " while getting user information.");
	leave(2);
    }

    /* Ask for new shell */
    while (!got_one)
    {
	printf("New shell: ");
	if (fgets(shell, sizeof(shell), stdin) == NULL)
	  leave(0);
	got_one = (strlen(shell) > 1);
    }

    shell[strlen(shell) - 1] = 0;	/* strip off newline */

    /* Make sure we have a valid shell.  This routine could exit */
    check_shell(shell);

    /* Change shell */

    printf("Changing shell to %s...\n", shell);

    q_argv[USH_NAME] = uname;
    q_argv[USH_SHELL] = shell;	
    q_argc = USH_END;
    if (status = mr_query("update_user_shell", q_argc, q_argv, 
			   scream, (char *) NULL))
    {
	com_err(whoami, status, " while changing shell.");
	leave(2);
    }

    printf("Shell successfully changed.\n");
    mr_disconnect();

    return(0);
}

get_shell(argc, argv, uname)
  int argc;
  char **argv;
  char *uname;			/* for sanity checking */
{
    /* We'll just take the first information we get since login names 
       cannot be duplicated in the database. */
    
    if (argc < U_END || strcmp(argv[U_NAME], uname))
    {
	fprintf(stderr, "Some internal error has occurred.  Try again.\n");
	leave(3);
    }
    
    printf("Account information last changed on %s\n", argv[U_MODTIME]);
    printf("by user %s with %s.\n", argv[U_MODBY], argv[U_MODWITH]);
    printf("Current shell for %s is %s.\n", uname, argv[U_SHELL]); 
    
    return(MR_ABORT);		/* Don't pay attention to other matches. */
}
    
void check_shell(shell)
  char *shell;
{
    char *valid_shell;
    int ok = 0;

    while (valid_shell = getusershell()) {
	if (strcmp(shell, valid_shell) == NULL) {
	    ok = 1;
	    break;
	}
	else if (strcmp(shell, 1+strrchr(valid_shell, '/')) == NULL) {
	    ok = 1;
	    (void) strcpy(shell, valid_shell);
	    break;
	}
    }
    
    if (!ok) {
	if (shell[0] != '/') {
	    fprintf(stderr, "%s it not a standard shell.  ", shell);
	    fprintf(stderr, "You may choose to use a nonstandard\n");
	    fprintf(stderr, "shell, but you must specify its complete ");
	    fprintf(stderr, "path name.\n");
	    leave(2);
	}
	else if (access(shell, X_OK)) {
	    fprintf(stderr, "%s is not available.\n", shell);
	    leave(2);
	}
	else {
	    printf("%s exists but is an unusual choice.\n", shell);
	    printf("Try again if it is not what you wanted.\n");
	}
    }
}	

usage()
{
    fprintf(stderr, "Usage: %s [user]\n", whoami);
    exit(1);
}

#if defined(ultrix) || defined(_AIX) || defined(sgi)
char *getusershell()
{
    static int count = 1;

    switch (count++) {
    case 1:
	return("/bin/sh");
    case 2:
	return("/bin/csh");
    case 3:
	return("/bin/athena/tcsh");
    case 4:
	return(NULL);
    default:
	count = 1;
	return(getusershell());
    }
}
#endif
