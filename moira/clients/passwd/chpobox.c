/*
 * Copyright 1987 by the Massachusetts Institute of Technology. For copying
 * and distribution information, see the file "mit-copyright.h". 
 *
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.5 1988-08-04 13:22:25 mar Exp $
 * $Author: mar $
 *
 */

#ifndef lint
static char *rcsid_chpobox_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chpobox.c,v 1.5 1988-08-04 13:22:25 mar Exp $";
#endif not lint

/*
 * Talk to the SMS database to change a person's home mail machine. This may
 * be an Athena machine, or a completely arbitrary address.
 * 
 * chpobox with no modifiers reports all current mailboxes.
 * 
 * chpobox -a [address] means add (not replace) a mailbox to the ones
 * the user already has.  Complains if the user tries to give herself
 * more than one mailbox of type pop.
 *
 * chpobox -d [address] means delete the specified mailbox from the list.
 * Complains if this would mean leaving the user with no mailbox at all. 
 *
 * chpobox -u [user] is needed if you are logged in as one user, but
 * are trying to change the email address of another.  You must have
 * Kerberos tickets as the person whose address you are trying to
 * change, or the attempt will fail.
 *
 * [address] must always have an @ sign in it. 
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <strings.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>

/* SMS includes */
#include <sms.h>
#include <sms_app.h>
#include "mit-copyright.h"

char *getlogin();
char *malloc();
char *strcpy();
char *strcat();
char *whoami;

int getopt();
int status;

#define MAXBOX 16

extern int h_errno;
static int match;
static struct pobox_values boxes[MAXBOX];
static int nboxes = 0;

uid_t getuid();

#define DOMAIN ".MIT.EDU"
#define NET_ADDRESS_SIZE 500	/* You would not believe the length of some
				 * addresses.. */
main(argc, argv)
    char *argv[];

{
    struct passwd *pwd;
    struct pobox_values getnewmach(), pobox;
    char *smsarg[1], buf[BUFSIZ];
    char *strsave(), *trim(), *in(), *canon();
    char *address, *uname, *machine;
    char **return_args, **crunch_pobox_args();
    uid_t u;
    int get_pobox(), scream();
    int c, delflag, add, usageflag;

    extern int optind;
    extern char *optarg;

    init_sms_err_tbl();
    init_krb_err_tbl();
    c = delflag = add = usageflag = 0;
    address = uname = (char *) NULL;
    u = getuid();

    if ((whoami = rindex(argv[0], '/')) == NULL)
	whoami = argv[0];
    else
	whoami++;

    if (argc > 5) {
	usage();
    }

    while ((c = getopt(argc, argv, "d:a:u:")) != EOF)
	switch (c) {

	case 'd':
	    if (add)
		usageflag++;
	    else {
		delflag++;
		address = strsave(optarg);
	    }
	    break;
	case 'a':
	    if (delflag)
		usageflag++;
	    else {
		add++;
		address = strsave(optarg);
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

    status = sms_connect();
    if (status) {
	(void) sprintf(buf, "\nConnection to SMS server failed.");
	goto punt;
    }

    status = sms_auth("chpobox");
    if (status) {
	(void) sprintf(buf, "\nAuthorization failed -- please \
run \"kinit\" and try again.");
	goto punt;
    }

    /*
     * set up some bogus arguments to feed to sms_access 
     */
    pobox.login = uname;
    pobox.type = "SMTP";
    pobox.box = "foo";
    return_args = crunch_pobox_args(pobox);
    /*
     * do an access check. 
     */
    status = sms_access("set_pobox", 3, return_args);
    if (status) {
	(void) sprintf(buf, "\nUnauthorized attempt to modify %s's \
email address.", uname);
	goto punt;
    }

    /*
     * get a list of current boxes
     */
    status = sms_query("get_pobox", 1, smsarg, get_pobox, NULL);
    if (status && status != SMS_NO_MATCH) {
	com_err(whoami, status, "while retrieving current mailboxes\n");
	goto punt;
    }

    /*
     * address, if it exists, is of form user@host.  It needs to be split up. 
     */
    if (address) {
	machine = index(address, '@');
	if (machine) {
	    *machine++ = '\0';	/* get rid of the @ sign */
	    machine = trim(machine);	/* get rid of whitespace */
	}
	else {
	    fprintf(stderr, "%s: no at sign (@) in address\n",
		    whoami);
	    sms_disconnect();
	    exit(1);
	}			/* now machine points to the machine name,
				 * and address points to the "box" name */
	pobox.machine = canon(machine);	/* canonicalize the machine name */
	if (pobox.machine == (char *) NULL) {/* nameserver failed in canon */
	    (void) sprintf(buf, "\nNameserver lookup \
failed.\nI cannot change your mailbox at this time.  Please try again \
later.\n");
	    goto punt;
	}
	pobox.type = in(pobox.machine);	/* find out the type	 */
	pobox.login = uname;	/* whose is changing?		 */
	pobox.box = address;	/* to what mailbox?		 */
    }

    if (add) {
	if (strcmp(pobox.type, "pop") == 0) {
	    int i;
	    /*
	     * check to be sure that they're not getting more than one. If
	     * they are, punt. 
	     */
	    for (i = 0; i < nboxes; i++) {
		if (!strcmp(boxes[i].type, "POP")) {
		    printf("%s already has a pobox on %s\n",
			   uname, boxes[i].machine);
		    goto punt;
		}
	    }
	}
	printf("Adding email address %s@%s for %s.",
	       pobox.box, pobox.machine, uname);

	return_args = crunch_pobox_args(pobox);
	status = sms_query("add_pobox", 4, return_args, scream,
			   (char *) NULL);
	if (status) {
	    (void) sprintf(buf, "\nWrite failed to SMS \
database.");
	    goto punt;
	}
	printf("..done.\n");
	sms_disconnect();
	exit(0);
    }
    else if (delflag) {
	/* check to make sure that the
	 * user isn't being left without a mailbox. */
	if (nboxes < 2) {
	    printf("That would leave %s without a mailbox\n", uname);
	    goto punt;
	}
	printf("Deleting email address %s@%s for %s.", pobox.box,
	       pobox.machine, uname);
	return_args = crunch_pobox_args(pobox);
	status = sms_query("delete_pobox", 4, return_args, scream,
			   (char *) NULL);
	if (status) {
	    (void) sprintf(buf, "\nWrite failed to SMS \
database.");
	    goto punt;
	}
	printf("..done.\n");
	sms_disconnect();
	exit(0);
    }

    printf("Current mail address for %s is: ", uname);
    if (nboxes == 0)
	printf("  None\n");
    else {
	printf("%s\n  last modified on %s by user %s using %s\n",
	      boxes[0].box, boxes[0].modtime, boxes[0].modby, boxes[0].modwith);
    }

    sms_disconnect();
    exit(0);

punt:
    if (status)
	com_err(whoami, status, buf);
    sms_disconnect();
    exit(1);
}

scream()
{
    com_err(whoami, status, "Unexpected return value from SMS -- \
programmer botch\n");
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
    struct pobox_values *pobox = &boxes[nboxes++];

    pobox->type = strsave(argv[1]);
    pobox->box = strsave(argv[2]);
    pobox->modtime = strsave(argv[3]);
    pobox->modby = strsave(argv[4]);
    pobox->modwith = strsave(argv[5]);

    return (0);
}

char **
crunch_pobox_args(in)
    struct pobox_values in;
{
    char **out;

    out = (char **) malloc((unsigned) sizeof(char *) * 6);
    out[0] = in.login;
    out[1] = in.type;
    out[2] = in.box;
    out[3] = in.modtime;
    out[4] = in.modby;
    out[5] = in.modwith;
    return (out);
}

/*
 * Trim whitespace off of cp.  Side-effects cp. 
 */
char *
trim(cp)
    register char *cp;
{
    register char *ep;

    while (isspace(*cp))
	cp++;
    ep = cp;
    while (*ep)
	ep++;
    if (ep > cp) {
	for (--ep; isspace(*ep); --ep)
	    continue;
	ep++;
	*ep = '\0';
    }
    return cp;
}

/*
 * given a canonicalized machine name, ask the SMS server if it is of type
 * pop, or of type local -- if neither, we assume that it's of type foreign. 
 */
char *
in(machine)
    char *machine;
{
    char *service[1], buf[BUFSIZ];
    int check_match();

    match = 0;
    service[0] = strsave("pop");
    status = sms_query("get_server_locations", 1, service,
		       check_match, machine);
    if (status && (status != SMS_NO_MATCH)) {
	(void) sprintf(buf, "\nRead failed from SMS \
database.");
	goto punt;
    }
    if (match)
	return ("pop");

    service[0] = strsave("local");
    status = sms_query("get_server_locations", 1, service,
		       check_match, machine);
    if (status && (status != SMS_NO_MATCH)) {
	(void) sprintf(buf, "\nRead failed from SMS \
database.");
	goto punt;
    }
    if (match)
	return ("local");
    else
	return ("foreign");
punt:
    com_err(whoami, status, buf);
    sms_disconnect();
    exit(1);
}

/* ARGSUSED */
int
check_match(argc, argv, callback)
    int argc;
    char **argv, *callback;
{
    if (match)
	return (0);

    if (strcmp(argv[1], callback) == 0)
	match = 1;

    return (0);
}

/*
 * given a machine name, canonicalize it and return it.  Returns (char *)
 * NULL if the nameserver returns any error. 
 */
char *
canon(machine)
    char *machine;
{
    struct hostent *hostinfo;

    hostinfo = gethostbyname(machine);
    if (hostinfo != (struct hostent *) NULL)
	machine = strsave(hostinfo->h_name);
    else			/* gethostbyname failed; this should be very
				 * rare, since we're dealing with local
				 * hosts, so no fancy error recovery.
				 */
	machine = (char *) NULL;
    return (machine);
}

usage()
{
    fprintf(stderr, "Usage: %s [-d|a address] [-u user]\n", whoami);
    exit(1);
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
