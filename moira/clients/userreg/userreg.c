/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v $
 * $Author: mar $
 * $Locker:  $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.7 1988-07-31 17:31:09 mar Exp $ 
 */

#ifndef lint
static char    *rcsid_userreg_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.7 1988-07-31 17:31:09 mar Exp $";
#endif	lint

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
	char            line[100];
	int status;
	char tmpfirst[100], tmplast[100], tmpmid[100];
	
	status = ureg_init();
	if (status) {
		com_err(argv[0], status, "while trying to initialize");
		exit(1);
	}

	setup_display();

	signal(SIGINT, fix_display);
	signal(SIGQUIT, fix_display);
	signal(SIGHUP, fix_display);

	do {
		
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
				display_text_line("Sorry, you still cannot be found in the database.");
				display_text_line(" ");
				display_text_line("Please call the account administrator at x1325 for help.");
				sleep(8);
				break;
			}
			display_text_line(0);	/* clear the window */
			display_text_line("I'm afraid I was unable to find you in the database.");
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
					encrypt_mitid();
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
		display_text_line("You are now registered to get an Athena account.");
		sprintf(line, "Please remember your username of \"%s\" and the password",
			user.u_login);
		display_text_line(line);
		display_text_line("you typed in earlier.");
		display_text_line("");
		display_text_line("Your account should be created within about 12 hours\n");
		
		display_text_line("");
		display_text_line("You are now finished. Thank you!");
		sleep(5);
		qexit();
	} while (getuid() == 0);/* Loop forever if we're root */
	restore_display();
	exit(0);
}

reset()
{
	reset_display();
	bzero(&user, sizeof(user));
	user_is_valid = 0;
	already_registered = 0;
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
	display_text_line(
	 "Looking you up in the database.... This may take a few minutes.");

	timer_off();
	result = verify_user(user.u_first, user.u_last,
			     typed_mit_id, user.u_mit_id, db_user.u_login);
	timer_on();
	switch(result) {
	case 0:
		display_text_line("You have been located in the user registration database.");
		user_is_valid = 1;
		user_has_login = 0;
		sleep(1);
		return 0;
	case UREG_NO_PASSWD_YET:
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
		display_text_line("Refer to the documents 'Essential Athena' and 'Essential Unix'");
		strcpy(user.u_login, db_user.u_login);
		sprintf(line, "for help logging in.  Remember, the username you chose was '%s'",
			db_user.u_login);
		display_text_line(line);
		redisp();
		sleep(5);
		return (0);
	case UREG_USER_NOT_FOUND:
		return (1);

	case ECONNREFUSED:
	case ETIMEDOUT:
		display_text(NETWORK_DOWN);
		return (0);
		
	default:
		display_text_line("An unexpected error occurred while trying to access the database");
		display_text_line(error_message(result));
		redisp();
		sleep(5);
		return(1);
	}
}

negotiate_login()
{
	register int    result, result2;
	int             same;
	char            line[100];
	char            old_login[LOGIN_SIZE];
	char            old_password[PASSWORD_SIZE];
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
			  "If you are sure you would prefer another username,  feel free to enter");
	display_text_line(
			  "a different one,  but  remember  that it will remain with you for all the ");
	display_text_line("time you are at MIT.");

	while (1) {
		glogin();

		display_text_line(0);
		display_text_line("Looking up that username in the database...  This may take a few minutes.");
		timer_off();
		/*
		 * Rather than bother SMS with a bunch of different
		 * usernames, all in use, we first try and see if this
		 * guy is known to Kerberos.
		 */
		if ((result = get_krbrlm(realm, 1)) != KSUCCESS) {
		    display_text_line("Can't get kerberos realm, giving up");
		    continue;
		}
		result = get_in_tkt(user.u_login, "", realm, "krbtgt", realm, "");
		timer_on();
		if (result != KDC_PR_UNKNOWN) {
		in_use:
			strcpy(user.u_login, old_login);
			redisp();
			display_text_line("I'm sorry, but that username is already taken.  Please try again with a different username");
			continue;
		}

		/*
		 * If he isn't, let's try through SMS.
		 */
		timer_off();
		result = grab_login(user.u_first, user.u_last,
				    typed_mit_id, user.u_mit_id,
				    user.u_login);
		timer_on();
		if (result != 0) {
			if (result == UREG_LOGIN_USED) goto in_use;
			display_text(NETWORK_DOWN);
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

}

gfirst()
{
	/* input the first name */
	char            buf[100];

	signal(SIGALRM, restart);
	input("Enter first Name:", buf, 100, FIRSTNAME_TIMEOUT);
	strncpy(user.u_first, buf, FIRST_NAME_SIZE);
	user.u_first[FIRST_NAME_SIZE - 1] = '\0';
	canon_name(user.u_first);
	redisp();
}

glast()
{
	/* input the last name */
	char            buf[100];

	signal(SIGALRM, restart);
	input("Enter family Name:", buf, 100, LASTNAME_TIMEOUT);
	strncpy(user.u_last, buf, LAST_NAME_SIZE);
	user.u_last[LAST_NAME_SIZE - 1] = '\0';
	canon_name(user.u_last);
	redisp();
}

gpass()
{
	/* input password */
	char            old_password[PASSWORD_SIZE];
	char            new_password[PASSWORD_SIZE];
	char            new_password_again[PASSWORD_SIZE];

do_input:
	signal(SIGALRM, restart);
	input_no_echo("Enter password:", new_password,
		      PASSWORD_SIZE, NEW_PASSWORD_TIMEOUT);
	if (strlen(new_password) < 4) {
		display_text_line("Please use a password of at least 4 characters.");
		goto do_input;
	}
	signal(SIGALRM, restart);
	input_no_echo("Enter password again:", new_password_again,
		      PASSWORD_SIZE, REENTER_PASSWORD_TIMEOUT);
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
	char            buf[100];
	register char  *nbuf = buf;

input_login:
	i = 0;
	user.u_login[0] = '\0';
	nbuf = &buf[0];
	signal(SIGALRM, restart);
	input("Enter username:", buf, 100, USERNAME_TIMEOUT);
	while (*nbuf != '\0') {
		if (!islower(*nbuf) && !isdigit(*nbuf)
		    && (*nbuf != '_') && (*nbuf != '.')) {
			display_text_line("Your username must be all lowercase alphabetic characters.");
			goto input_login;
		}
		user.u_login[i] = *nbuf++;
		i++;
		if (i > LOGIN_SIZE - 1) {
			display_text_line("Your username must be no more than 8 characters long.");
			goto input_login;
		}
	}
	if (i != 0) {
		user.u_login[i] = '\0';
	}
	if (strlen(user.u_login) < 3) {
		display_text_line("Your username must be 3 or more characters long.\n");
		goto input_login;
	}
	redisp();
}

gmitid()
{
	/* get mid id */
	register int    i;
	char            buf[100];
	register char  *nbuf = buf;

input_mit_id:
	signal(SIGALRM, restart);
	input("Enter MIT Id:", buf, 100, MITID_TIMEOUT);
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
	encrypt_mitid();
}

encrypt_mitid()
{
	char            salt[3];

	make_salt(salt, user.u_first, user.u_last);
	strcpy(user.u_mit_id, crypt(&typed_mit_id[2], salt));
}


#define _tolower(c) ((c)|0x60)

make_salt(salt, first, last)
	char           *salt, *first, *last;
{
	salt[0] = _tolower(last[0]);
	salt[1] = _tolower(first[0]);
}


gmi()
{
	/* get middle initial */
	char            buf[100];

	signal(SIGALRM, restart);
	input("Enter Middle Initial:", buf, 100, MI_TIMEOUT);
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
	sleep(6);		/* give the user a chance to see the screen */
	display_text_line(0);
	return (EXIT);
}


do_replace()
{
	int status;
	
	/*
	 * replaces a user in the database. If there is an error, it informs
	 * the user and calls qexit(); It returns only if is is successful 
	 */
	timer_off();
	status = set_password(user.u_first, user.u_last, typed_mit_id,
			      user.u_mit_id, user.u_password);
	timer_on();
	if (status) {
		display_text (NETWORK_DOWN);
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
