/*
 * Copyright 1987 by the Massachusetts Institute of Technology. For copying
 * and distribution information, see the file "mit-copyright.h". 
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.11 1989-08-28 23:26:42 mar Exp $
 * $Author: mar $
 *
 */

#ifndef lint
static char *rcsid_chpobox_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.11 1989-08-28 23:26:42 mar Exp $";
#endif not lint

/*
 * Talk to the SMS database to change a person's home mail machine. This may
 * be an Athena machine, or a completely arbitrary address.
 * 
 * chpobox with no modifiers reports the current mailbox.
 * 
 * chpobox -s [address] means set the mailbox to this address.
 *
 * chpobox -p restores the pobox to a previous POP box, if there was one.
 *
 * chpobox -u [user] is needed if you are logged in as one user, but
 * are trying to change the email address of another.  You must have
 * Kerberos tickets as the person whose address you are trying to
 * change, or the attempt will fail.
 */

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

/* SMS includes */
#include <sms.h>
#include <sms_app.h>
#include "mit-copyright.h"

char *getlogin();
char *malloc();
char *whoami;
uid_t getuid();

int getopt();

static int match;


main(argc, argv)
    int argc;
    char *argv[];
{
    struct passwd *pwd;
    char *smsarg[3], buf[BUFSIZ];
    char *potype(), *index();
    char *address, *uname, *machine, *motd;
    uid_t u;
    char *canonicalize_hostname();
    int get_pobox(), scream();
    int c, setflag, prevpop, usageflag, status;

    extern int optind;
    extern char *optarg;

    c = setflag = prevpop = usageflag = 0;
    address = uname = (char *) NULL;
    u = getuid();

    if ((whoami = rindex(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;

    if (argc > 5) {
	usage();
    }

    while ((c = getopt(argc, argv, "s:pu:")) != EOF)
	switch (c) {

	case 's':
	    if (prevpop)
		usageflag++;
	    else {
		setflag++;
		strcpy(buf, optarg);
		address = buf;
	    }
	    break;
	case 'p':
	    if (setflag)
		usageflag++;
	    else {
		prevpop++;
	    }
	    break;
	case 'u':
	    uname = strsave(optarg);
	    break;
	default:
	    usageflag++;
	    break;
	}
    if (argc == 2 && optind == 1 && !uname) {
	uname = argv[optind++];
    }
    if (usageflag || optind != argc) {
	usage();
    }
    if (!uname) {
	if ((uname = getlogin()) == NULL) {
	    usage();
	}
	if (uname[0] == '\0') {
	    pwd = getpwuid((int) u);
	    (void) strcpy(uname, pwd->pw_name);
	}
    }
    smsarg[0] = uname;

    status = sms_connect(SMS_SERVER);
    if (status) {
	com_err(whoami, status, " while connecting to Moira");
	exit(1);
    }

    status = sms_motd(&motd);
    if (status) {
	sms_disconnect();
        com_err(whoami, status, " unable to check server status");
	exit(1);
    }
    if (motd) {
	fprintf(stderr, "The Moira server is currently unavailable:\n%s\n", motd);
	sms_disconnect();
	exit(1);
    }

    status = sms_auth("chpobox");
    if (status) {
	com_err(whoami, status, " while authenticating -- run \"kinit\" and try again.");
	sms_disconnect();
	exit(1);
    }

    if (setflag) {
	/* Address is of form user@host.  Split it up. */
	if (!address) {
	    fprintf(stderr, "%s: no address was specified.\n", whoami);
	    goto show;
	}
	machine = index(address, '@');
	if (machine) {
	    *machine++ = '\0';		/* get rid of the @ sign */
	    machine = strtrim(machine);	/* get rid of whitespace */
	} else {
	    fprintf(stderr, "%s: no at sign (@) in address \"%s\"\n",
		    whoami, address);
	    goto show;
	}
	smsarg[2] = canonicalize_hostname(strsave(machine));
	smsarg[1] = potype(smsarg[2]);
	if (!strcmp(smsarg[1], "POP")) {
	    if (strcmp(address, uname)) {
		fprintf(stderr,
			"%s: the name on the POP box must match the username\n",
			whoami);
		goto show;
	    }
	} else if (!strcmp(smsarg[1], "LOCAL")) {
	    strcat(address, "@");
	    strcat(address, smsarg[2]);
	    smsarg[2] = address;
	    if ((address = index(address, '@')) &&
		(address = index(address, '.')))
	      *address = 0;
	    strcat(smsarg[2], ".LOCAL");
	    smsarg[1] = "SMTP";
	} else if (smsarg[1]) {
	    if (*machine != '"' && strcasecmp(smsarg[2], machine))
	      printf("Warning: hostname %s canonicalized to %s\n",
		     machine, smsarg[2]);
	    strcat(address, "@");
	    strcat(address, smsarg[2]);
	    smsarg[2] = address;
	} else
	  goto show;
	status = sms_query("set_pobox", 3, smsarg, scream, NULL);
	if (status)
	  com_err(whoami, status,
		  " while setting pobox for %s to type %s, box %s",
		  smsarg[0], smsarg[1], smsarg[2]);
    } else if (prevpop) {
	status = sms_query("set_pobox_pop", 1, smsarg, scream, NULL);
	if (status == SMS_MACHINE) {
	    fprintf(stderr,
		    "Moira has no record of a previous POP box for %s\n", uname);
	} else if (status != 0)
	  com_err(whoami, status, " while setting pobox");
    }

    /*
     * get current box
     */
show:
    status = sms_query("get_pobox", 1, smsarg, get_pobox, NULL);
    if (status == SMS_NO_MATCH)
      printf("User %s has no pobox.\n", uname);
    else if (status != 0)
      com_err(whoami, status, " while retrieving current mailbox");
    sms_disconnect();
    exit(0);
}


scream()
{
    com_err(whoami, 0, "Unexpected return value from Moira -- programmer botch");
    sms_disconnect();
    exit(1);
}

/*
 * get_pobox gets all your poboxes and displays them.
 */

/* ARGSUSED */
int
get_pobox(argc, argv, callarg)
    int argc;
    char **argv, *callarg;
{
    if (!strcmp(argv[1], "POP"))
      printf("User %s, Type %s, Box: %s@%s\n",
	     argv[0], argv[1], argv[0], argv[2]);
    else
      printf("User %s, Type %s, Box: %s\n",
	     argv[0], argv[1], argv[2]);
    printf("  Modified by %s on %s with %s\n", argv[4], argv[3], argv[5]);
    return (0);
}

/*
 * given a canonicalized machine name, ask the SMS server if it is of type
 * pop, or of type local -- if neither, we assume that it's of type foreign. 
 */
char *
potype(machine)
    char *machine;
{
    char *service[1];
    int check_match(), status;

    match = 0;
    service[0] = "POP";
    status = sms_query("get_server_locations", 1, service,
		       check_match, machine);
    if (status && (status != SMS_NO_MATCH)) {
	com_err(whoami, status, " while reading list of POP servers");
	return(NULL);
    }
    if (match)
	return ("POP");

    service[0] = "LOCAL";
    status = sms_query("get_server_locations", 1, service,
		       check_match, machine);
    if (status && (status != SMS_NO_MATCH)) {
	com_err(whoami, status, " while reading list of LOCAL servers");
	return(NULL);
    }
    if (match)
	return ("LOCAL");
    else
	return ("SMTP");
}

/* ARGSUSED */
int
check_match(argc, argv, callback)
    int argc;
    char **argv, *callback;
{
    if (match)
	return (0);

    if (strcasecmp(argv[1], callback) == 0)
	match = 1;

    return (0);
}

usage()
{
    fprintf(stderr, "Usage: %s [-s address] [-p] [-u user]\n", whoami);
    exit(1);
}
