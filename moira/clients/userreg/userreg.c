/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v $
 * $Author: mar $
 * $Locker:  $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.18 1990-04-09 14:29:41 mar Exp $ 
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char    *rcsid_userreg_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.18 1990-04-09 14:29:41 mar Exp $";
#endif	lint

#include <mit-copyright.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>
#include <ctype.h>
#include <krb.h>
#include <errno.h>
#include "userreg.h"
#include "ureg_err.h"

#define EXIT -1


struct user     user, db_user;
struct alias    alias;
char            typed_mit_id[100];

jmp_buf         redo;
int             restart();

extern int      errno;
int             user_is_valid = 0;
int		user_has_login = 0;
int             already_registered = 0;
int		enrollment = 0;
extern char *disabled();

fix_display(sig)
{
	signal(sig, SIG_IGN);	/* let us clean up, please */
	noraw();
	echo();
	endwin();
	exit(1);
}

main(argc, argv)
	int             argc;
	char          **argv;
{
	register int    ntimes;
	register int    reencrypt;
	char            line[100], *when, *msg;
	int status;
	char tmpfirst[100], tmplast[100], tmpmid[100];
	
	status = ureg_init();
	if (status) {
		com_err(argv[0], status, "while trying to initialize");
		exit(1);
	}

	if (when = disabled(&msg)) {
	    printf("We're sorry, the registration service is unavailable right now\n");
	    if (msg)
	      printf("because %s\n", msg);
	    printf("You should be able to register after %s", when);
	    printf("\nPress ENTER or RETURN to continue ");
	    fflush(stdout);
	    getchar();
	    exit(0);
	}

	setup_display();

	signal(SIGINT, fix_display);
	signal(SIGQUIT, fix_display);
	signal(SIGHUP, fix_display);

	while (1) {
		setjmp(redo);
		reset();
		ntimes = 0;

		display_text(WELCOME);

		gfirst();
		gmi();
		glast();
		strcpy(tmpfirst, user.u_first);
		strcpy(tmplast, user.u_last);
		FixName(tmplast, tmpfirst, user.u_last, user.u_first, tmpmid);
		redisp();
		
		gmitid();
		
		while (dolook()) {
			ntimes++;
			if (ntimes > 3) {
			    	display_text_line(0);
				display_text_line("Sorry, you still cannot be found in the database.");
				display_text_line(" ");
				display_text_line("Please call the account administrator at x3-1325 for help.");
				wait_for_user();
				break;
			}
			display_text_line(0);	/* clear the window */
			display_text_line("You could not be found in the database.");
			display_text_line("Do you want to change some input (for example, the spelling");
			display_text_line("of part of your name) and try again?");
			if (askyn("Do you want to try again ? ") == YES) {
				reencrypt = 0;
				display_text_line(0);
				sprintf(line,
					"You entered your first name as \"%s\"", user.u_first);
				display_text_line(line);
				if (askyn("Do you want to change it? ") == YES) {
					gfirst();
					strcpy(tmpfirst, user.u_first);
					strcpy(tmplast, user.u_last);
					FixName(tmplast, tmpfirst, user.u_last, user.u_first, tmpmid);
					redisp();
					reencrypt = 1;
				}
				display_text_line(0);
				sprintf(line,
					"You entered your middle initial as \"%s\"", user.u_mid_init);
				display_text_line(line);
				if (askyn("Do you want to change it? ") == YES)
					gmi();
				display_text_line(0);
				sprintf(line,
					"You entered your family name as \"%s\"", user.u_last);
				display_text_line(line);
				if (askyn("Do you want to change it? ") == YES) {
					glast();
					strcpy(tmpfirst, user.u_first);
					strcpy(tmplast, user.u_last);
					FixName(tmplast, tmpfirst, user.u_last, user.u_first, tmpmid);
					redisp();
					reencrypt = 1;
				}
				display_text_line(0);
				sprintf(line,
					"You entered your MIT id number as \"%s\"", typed_mit_id);
				display_text_line(line);
				if (askyn("Do you want to change it? ") == YES) {
					gmitid();
					reencrypt = 0;
				}
				if (reencrypt) {
				    EncryptID(user.u_mit_id, typed_mit_id,
					      user.u_first, user.u_last);
				}
			} else
				break;
		}
		if (!user_is_valid || already_registered) {
			qexit();
			continue;
		}
		redisp();
		if (!user_has_login) {
			if (negotiate_login() == -1) {
				qexit();
				continue;
			}
		}
		if (negotiate_passwd() == -1) {
			qexit();
			continue;
		}
		sleep(1);
		display_text_line(0);
		if (!enrollment)
		  display_text_line("You are now registered to get an Athena account.");
		sprintf(line, "Please remember your username of \"%s\" and the password",
			user.u_login);
		display_text_line(line);
		display_text_line("you typed in earlier.");
		display_text_line("");
		if (!enrollment)
		  display_text_line("Your account should be created by tomorrow\n");
		
		display_text_line("");
		display_text_line("You are now finished. Thank you!");
		wait_for_user();
		qexit();
		break;
	}
	restore_display();
	exit(0);
}

reset()
{
	reset_display();
	bzero(&user, sizeof(user));
	user_is_valid = 0;
	already_registered = 0;
	enrollment = 0;
	redisp();
}

dolook()
{
	/* do the database lookup */

	char            line[100];
	register int    result;
	if (user_is_valid) {
		/* !! */
		return (0);
	}
	display_text_line(0);
	display_text_line("Looking you up in the database.... This may take from 30 seconds to 10");
	display_text_line("minutes, depending on how busy the system is at the moment.");

	timer_off();
	result = verify_user(user.u_first, user.u_last,
			     typed_mit_id, user.u_mit_id, db_user.u_login);
	wfeep();
	display_text_line(0);
	timer_on();
	switch(result) {
	case 0:
	case UREG_ENROLLED:
		display_text_line("You have been located in the user registration database.");
		user_is_valid = 1;
		user_has_login = 0;
		sleep(1);
		return 0;
	case UREG_NO_PASSWD_YET:
	case UREG_HALF_ENROLLED:
		user_is_valid = 1;
		user_has_login = 1;
		display_text_line ("You have chosen a login name, but you have not yet chosen a password.");
		sprintf(line, "Remember: the username you chose was '%s'",
			db_user.u_login);
		strcpy(user.u_login, db_user.u_login);
		display_text_line(line);
		redisp();
		sleep(5);
		return (0);
		
	case UREG_ALREADY_REGISTERED:
		already_registered = 1;
		/*
		 * we have to reset this so we dont ask for a
		 * new username 
		 */	
		user_is_valid = 0;
		display_text_line("You are already registered.  An account for you probably already exists");
		display_text_line("(if not, it will appear within 12 hours).");
		display_text_line("");
		display_text_line("Refer to the document 'Getting Started on Athena' for help logging in.");
		strcpy(user.u_login, db_user.u_login);
		sprintf(line, "Remember, the username you chose was '%s'",
			db_user.u_login);
		display_text_line(line);
		redisp();
		wait_for_user();
		return (0);
	case UREG_DELETED:
		display_text(DELETED_ACCT);
		wait_for_user();
		restore_display();
		exit(0);
	case UREG_NOT_ALLOWED:
		display_text(OFFER_ENROLL);
		redisp();
		if (!askyn("Continue choosing a name and password (Y/N)? ")) {
		    already_registered = 1;
		    return(0);
		}
		user_has_login = 0;
		user_is_valid = 1;
		enrollment = 1;
		return(0);
	case UREG_ENROLL_NOT_ALLOWED:
		display_text(NOT_ALLOWED);
		wait_for_user();
		restore_display();
		exit(0);
	case UREG_KRB_TAKEN:
		display_text(IMPROPER_LOGIN);
		wait_for_user();
		return(0);
	case UREG_USER_NOT_FOUND:
		return (1);

	case ECONNREFUSED:
	case ETIMEDOUT:
	case UREG_MISC_ERROR:
		display_text(NETWORK_DOWN);
		display_text_line(" ");
		sprintf(line, "The specific error was: %s",
			error_message(result));
		display_text_line(line);
		wait_for_user();
		return (0);
		
	default:
		display_text_line("An unexpected error occurred while trying to access the database");
		display_text_line(error_message(result));
		redisp();
		wait_for_user();
		return(1);
	}
}

negotiate_login()
{
	register int    result;
	char            line[100];
	char            old_login[LOGIN_SIZE+2];
	char		realm[REALM_SZ];
	int		i;
	char *cp;
	
	display_text(USERNAME_BLURB);
	cp = user.u_login;
	if (isalpha(user.u_first[0]))
		*cp++ = user.u_first[0];
	if (isalpha(user.u_mid_init[0]))
		*cp++ = user.u_mid_init[0];
	
	for (i = 0; user.u_last[i] && cp - user.u_login < 8; i++) {
	        if (isalpha(user.u_last[i])) *cp++ = user.u_last[i];
	}

	for (i=0; user.u_login[i]; i++)
		if (isupper(user.u_login[i]))
			user.u_login[i]=tolower(user.u_login[i]);
	
	strcpy(old_login, user.u_login);
	sprintf(line, "We suggest the username '%s'.", user.u_login);
	display_text_line(line);
	display_text_line(
			  "If you are sure you would prefer another username, feel free to enter");
	display_text_line(
			  "a different one.  Keep in mind that it will remain with you for all the ");
	display_text_line("time you are at MIT.");

	while (1) {
		glogin();

		display_text_line(0);
		display_text_line("Trying to assign that username...  This may take a few minutes.");
		timer_off();
		/*
		 * Rather than bother SMS with a bunch of different
		 * usernames, all in use, we first try and see if this
		 * guy is known to Kerberos.
		 */
		if ((result = krb_get_lrealm(realm, 1)) != KSUCCESS) {
		    display_text_line("System error; please try another workstation.");
		    continue;
		}
		result = krb_get_pw_in_tkt(user.u_login, "", realm, 
					   "krbtgt", realm, 1, "");
		timer_on();
		if (result != KDC_PR_UNKNOWN) {
		in_use:
			strcpy(user.u_login, old_login);
			redisp();
			display_text_line("That username is already taken.  Please try again with a different username");
			continue;
		}

		/*
		 * If he isn't, let's try through SMS.
		 */
		timer_off();
		if (!enrollment)
		  result = grab_login(user.u_first, user.u_last,
				      typed_mit_id, user.u_mit_id,
				      user.u_login);
		else
		  result = enroll_login(user.u_first, user.u_last,
					typed_mit_id, user.u_mit_id,
					user.u_login);
		wfeep();
		timer_on();
		if (result != 0) {
		    	char buf[BUFSIZ];

			if (result == UREG_LOGIN_USED) goto in_use;
			display_text(NETWORK_DOWN);
			display_text_line(" ");
			sprintf(buf, "The specific error was: %s",
				error_message(result));
			display_text_line(buf);
			wait_for_user();
			return (qexit());
		} else break;
	}
	/* at this point we have successfully negotiated a username */

	sprintf(line, "O.K. your username will be \"%s\".", user.u_login);
	display_text_line(0);
	display_text_line(line);
	redisp();
	sleep(3);
	return 0;
}
negotiate_passwd()
{
	display_text_line(0);
	display_text(PASSWORD_BLURB);

	gpass();
	display_text_line("Storing password in the database...  This may take a few minutes.");
	if (do_replace()) {
		return (-1);
	}
	display_text_line("done.");
	return(0);
}

gfirst()
{
	/* input the first name */
	char            buf[FIRST_NAME_SIZE+2];

	signal(SIGALRM, fix_display);
	input("Enter first Name:", buf, FIRST_NAME_SIZE+1,
	      FIRSTNAME_TIMEOUT, TRUE);
	strncpy(user.u_first, buf, FIRST_NAME_SIZE);
	user.u_first[FIRST_NAME_SIZE - 1] = '\0';
	canon_name(user.u_first);
	redisp();
}

glast()
{
	/* input the last name */
	char            buf[LAST_NAME_SIZE+2];

	signal(SIGALRM, restart);
	input("Enter family Name:", buf, LAST_NAME_SIZE+1,
	      LASTNAME_TIMEOUT, FALSE);
	strncpy(user.u_last, buf, LAST_NAME_SIZE);
	user.u_last[LAST_NAME_SIZE - 1] = '\0';
	canon_name(user.u_last);
	redisp();
}

gpass()
{
	/* input password */
	char            new_password[PASSWORD_SIZE * 2 + 1];
	char            new_password_again[PASSWORD_SIZE * 2 + 1];

do_input:
	signal(SIGALRM, restart);
	input_no_echo("Enter password:", new_password,
		      enrollment ? PASSWORD_SIZE * 2 : PASSWORD_SIZE,
		      NEW_PASSWORD_TIMEOUT);
	if (strlen(new_password) < 4) {
		display_text_line("Please use a password of at least 4 characters.");
		goto do_input;
	}
	wfeep();
	signal(SIGALRM, restart);
	input_no_echo("Enter your password again:", new_password_again,
		      enrollment ? PASSWORD_SIZE * 2 : PASSWORD_SIZE,
		      REENTER_PASSWORD_TIMEOUT);
	if (strcmp(new_password, new_password_again)) {
		display_text_line("Sorry, the two passwords you just typed in don't match.");
		display_text_line("Please try again.");
		goto do_input;
	}

	strcpy(user.u_password, new_password);
	redisp();
}

glogin()
{
	/* get login name */
	register int    i;
	char            buf[LOGIN_SIZE+2];
	register char  *nbuf = buf;

input_login:
	i = 0;
	user.u_login[0] = '\0';
	nbuf = &buf[0];
	signal(SIGALRM, restart);
	input("Enter username:", buf, LOGIN_SIZE, USERNAME_TIMEOUT, FALSE);
	if (!islower(*nbuf) && !isdigit(*nbuf)) {
	    user.u_login[0] = 0;
	    display_text_line("Your username must start with a lower case letter or number.");
	    goto input_login;
	}
	while (*nbuf != '\0') {
		if (!islower(*nbuf) && !isdigit(*nbuf)
		    && (*nbuf != '_')) {
			user.u_login[0] = 0;
			display_text_line("Your username must be all lowercase letters or numbers.");
			goto input_login;
		}
		user.u_login[i] = *nbuf++;
		i++;
		if (i > LOGIN_SIZE) {
			user.u_login[0] = 0;
			display_text_line("Your username must be no more than 8 characters long.");
			goto input_login;
		}
	}
	if (i != 0) {
		user.u_login[i] = '\0';
	}
	if (strlen(user.u_login) < 3) {
		user.u_login[0] = 0;
		display_text_line("Your username must be 3 or more characters long.\n");
		goto input_login;
	}
	redisp();
}

gmitid()
{
	/* get mid id */
	register int    i;
	char            buf[15];
	register char  *nbuf = buf;

input_mit_id:
	signal(SIGALRM, restart);
	input("Enter MIT Id:", buf, 14, MITID_TIMEOUT, FALSE);
	i = 0;
	nbuf = &buf[0];
	while (*nbuf != '\0') {
		if (*nbuf == ' ' || *nbuf == '-') {
			nbuf++;
			continue;
		}
		if (!isdigit(*nbuf)) {
			i = 0;
			break;
		}
		typed_mit_id[i] = *nbuf;
		nbuf++;
		i++;
	}
	if (i != 9) {
		display_text_line(
				  "Your MIT id must be a nine-digit number.  Please try again.");
		goto input_mit_id;
	}
	typed_mit_id[9] = '\0';
	redisp();
	EncryptID(user.u_mit_id, typed_mit_id, user.u_first, user.u_last);
}

gmi()
{
	/* get middle initial */
	char            buf[MID_INIT_SIZE+2];

	signal(SIGALRM, restart);
	input("Enter Middle Initial:", buf, MID_INIT_SIZE+1, MI_TIMEOUT, TRUE);
	strncpy(user.u_mid_init, buf, MID_INIT_SIZE);
	user.u_mid_init[MID_INIT_SIZE - 1] = '\0';
	canon_name(user.u_mid_init);
	redisp();
}

qexit()
{
	/* exit quickly, not saving anything in the database */
	bzero(&user, sizeof(user));
	typed_mit_id[0] = '\0';
	user_is_valid = 0;
	already_registered = 0;
	enrollment = 0;
	sleep(2);		/* give the user a chance to see the screen */
	display_text_line(0);
	return (EXIT);
}


do_replace()
{
	int status;
	char buf[100];
	
	/*
	 * replaces a user in the database. If there is an error, it informs
	 * the user and calls qexit(); It returns only if is is successful 
	 */
	timer_off();
	if (!enrollment)
	  status = set_password(user.u_first, user.u_last, typed_mit_id,
				user.u_mit_id, user.u_password);
	else
	  status = get_krb(user.u_first, user.u_last, typed_mit_id,
				user.u_mit_id, user.u_password);
	wfeep();
	timer_on();
	if (status) {
		display_text (NETWORK_DOWN);
		display_text_line(" ");
		sprintf(buf, "The specific error was: %s",
			error_message(status));
		display_text_line(buf);
		wait_for_user();
		return (-1);
	} else return 0;
}

#define _toupper(c) ((c) & ~0x20)

lenient_strcmp(string1, string2)
	register char  *string1, *string2;
{
	/*
	 * a primitive case insensitive string comparison. It returns only 0
	 * if the strings are equal (ignoring case) and 1 if they are
	 * different. Also ignores spaces. 
	 */

	while (1) {
		if (*string1 == '\0' && *string2 == '\0') {
			return (0);
		}
		if (*string1 == ' ' || *string1 == '.' || *string1 == '-' || *string1 == '\'') {
			string1++;
			continue;
		}
		if (*string2 == ' ' || *string2 == '.' || *string2 == '-' || *string2 == '\'') {
			string2++;
			continue;
		}
		if (_toupper(*string1) != _toupper(*string2)) {
			return (1);
		}
		string1++;
		string2++;
	}
}

/*
 * Input timeout handler.  Loop back to asking for the first name. 
 */

restart()
/* Go to asking for first name. */
{
	qexit();
	longjmp(redo);
}

canon_name(cp)
	register char *cp;
{
	register char *p2 = cp;
      
	/* Trim whitespace off both ends. */
	for (; *p2 && isspace(*p2); p2++) ;
	if (*p2) {
		strcpy(cp, p2);
		p2 = cp + strlen(cp);
		--p2;
		while (p2 >= cp && isspace(*p2)) *(--p2) = '\0';
	}
	/* Make it capitalized */
	for (p2=cp; *p2; p2++) {
		if (isupper(*p2)) *p2 = tolower(*p2);
	}
	if (islower(*cp)) *cp=toupper(*cp);
}
