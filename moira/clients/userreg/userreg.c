/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v $
 * $Author: wesommer $
 * $Locker:  $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.3 1987-07-20 13:15:00 wesommer Exp $ 
 */

#ifndef lint
static char    *rcsid_userreg_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.3 1987-07-20 13:15:00 wesommer Exp $";
#endif	lint

#include <curses.h>
#include "userreg.h"
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>

#define EXIT -1


struct user     user, db_user;
struct alias    alias;
char            typed_mit_id[100];

jmp_buf         redo;
int             restart();

extern int      errno;
int             user_is_valid = 0;
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
					"You entered your last name as \"%s\"", user.u_last);
				display_text_line(line);
				if (askyn("Do you want to change it? ") == YES) {
					glast();
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
		if (negotiate_login_and_password() == -1) {
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
	result = sms_connect();
	timer_on();
	if (result == SUCCESS) {
		timer_off();
		result = get_user_by_mit_id(&db_user, user.u_mit_id);
		sms_disconnect();
		timer_on();
		if (result == SUCCESS) {
			/* check to see if the first and last names match */
			if (lenient_strcmp(user.u_first, db_user.u_first))
				return (1);
			if (lenient_strcmp(user.u_last, db_user.u_last))
				return (1);
			/* at this point the user has been verified */
			bcopy((char *) &db_user, (char *) &user, sizeof(user));
			user_is_valid = 1;
			if (user.u_status & 1) {
				already_registered = 1;
				/*
				 * we have to reset this so we dont ask for a
				 * new username 
				 */
				user_is_valid = 0;
				display_text_line("You are already registered.  An account for you probably already exists");
				display_text_line("on one of the W20 systems (if not, it will appear within 12 hours).");
				display_text_line("Refer to the documents 'Essential Athena' and 'Essential Unix'");
				sprintf(line, "for help logging in.  Remember, the username you chose was '%s'",
					db_user.u_login);
				display_text_line(line);
				redisp();
				sleep(5);
				return (0);
			}
			display_text_line("You have been located in the user registration database.");
			sleep(1);
			return (0);
		}
	}
	if (result == FAILURE) {
		display_text(NETWORK_DOWN);
		sleep(5);
		restart();
	}
	return (1);
}

negotiate_login_and_password()
{
	register int    result, result2;
	int             same;
	char            line[100];
	char            old_login[LOGIN_SIZE];
	char            old_password[PASSWORD_SIZE];

	display_text(USERNAME_BLURB);
	strcpy(old_login, user.u_login);
	sprintf(line, "We suggest the username '%s'.", user.u_login);
	display_text_line(line);
	display_text_line(
			  "If you are sure you would prefer another username,  feel free to enter");
	display_text_line(
			  "one,  but  remember  that it will remain with you for all the time you");
	display_text_line("are at MIT.");

get_login:
	while (1) {
		glogin();
		/*
		 * compare the typed username with the one we have already *
		 * reserved. We use lenient_strcmp because the username we *
		 * get form the database may have extra spaces... 
		 */
		if (same = !lenient_strcmp(old_login, user.u_login)) {
			break;	/* he didnt change the default */
		}
		display_text_line(0);
		display_text_line("Looking up that username in the database...  This may take a few minutes.");
		timer_off();
		result = sms_connect();
		timer_on();
		if (result == SUCCESS) {
			timer_off();
			result = get_user_by_login(&db_user, user.u_login);
			result2 = get_alias_by_name(&alias, user.u_login);
			sms_disconnect();
			timer_on();
		}
		if (result == FAILURE || result2 == FAILURE) {
			display_text(NETWORK_DOWN);
			sleep(5);
			restart();
		} else if (result != NOT_FOUND || result2 != NOT_FOUND) {
			strcpy(user.u_login, old_login);
			redisp();
			display_text_line("I'm sorry, but that username is already taken.  Please try again.");
			continue;
		} else {	/* yay! we can go ahead and change it! */
			/*
			 * technically, this is not sound. Somebody may have
			 * grabbed the username before us. It's unlikely, so
			 * we just hope it won't happen. 
			 */
			display_text_line(0);
			sprintf(line, "No one else is using \"%s\" as a username so you can have it.", user.u_login);
			display_text_line(line);
			if (askyn("Are you sure that is the one you want? ") == NO) {
				display_text_line(0);
				if (askyn("Do you want to abort this session? ") == YES) {
					display_text_line(" ");
					display_text_line(" ");
					display_text_line("aborting...");
					return (-1);
				}
				display_text_line(0);
				redisp();
				continue;
			}
			break;
		}
	}

get_password:

	/* at this point we have successfully negotiated a username */

	sprintf(line, "O.K. your username will be \"%s.\"", user.u_login);
	display_text_line(0);
	display_text_line(line);
	redisp();
	sleep(3);
	display_text_line(0);
	display_text(PASSWORD_BLURB);
	gpass();
	display_text_line("You password is now established.");
	/*
	 * we now make sure that the user must type his old password the next
	 * time he tries to change it 
	 */
	already_registered = 1;
	user.u_status = 1;
	display_text_line("Storing information in the database...  This may take a few minutes.");
	/*
	 * Check one more time to make sure someone hasn't grabbed that login
	 * since last time we checked. 
	 */
	if (!same) {		/* Recheck username if it was changed. */
		timer_off();
		result = sms_connect();
		timer_on();
		if (result == SUCCESS) {
			timer_off();
			result = get_user_by_login(&db_user, user.u_login);
			result2 = get_alias_by_name(&alias, user.u_login);
			sms_disconnect();
			timer_on();
		}
		if (result == FAILURE || result2 == FAILURE) {
			display_text(NETWORK_DOWN);
			return (-1);
		} else if (result != NOT_FOUND || result2 != NOT_FOUND) {
			strcpy(user.u_login, old_login);
			redisp();
			display_text_line(0);
			display_text_line("You're not going to believe this, but someone just now took the username");
			display_text_line("you wanted.");
			display_text_line(" ");
			display_text_line("You will have to choose a new username.");
			goto get_login;
		}
	}
	/*
	 * Do the database update and hope nobody grabs the same login before
	 * we do.  Its really not very likely that they will. 
	 */
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
	redisp();
}

glast()
{
	/* input the last name */
	char            buf[100];

	signal(SIGALRM, restart);
	input("Enter last Name:", buf, 100, LASTNAME_TIMEOUT);
	strncpy(user.u_last, buf, LAST_NAME_SIZE);
	user.u_last[LAST_NAME_SIZE - 1] = '\0';
	redisp();
}

gpass()
{
	/* input password */
	char            old_password[PASSWORD_SIZE];
	char            new_password[PASSWORD_SIZE];
	char            new_password_again[PASSWORD_SIZE];

	if (already_registered) {	/* we have to ask for his old
					 * password */
		signal(SIGALRM, restart);
		input_no_echo("Enter old Password:", old_password,
			      PASSWORD_SIZE, OLD_PASSWORD_TIMEOUT);
		if (strcmp(user.u_password, crypt(old_password,
						  user.u_password))) {
			display_text_line("Sorry, that does not match your old password.");
			display_text_line("In order to get a new one, you have to know the old one.");
			return (1);
		}
	}
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
	/*
	 * give him a new password. It looks funny because we are trying to
	 * get a semi_random key without a lot of hair by using the login
	 * name as a seed 
	 */

	strcpy(user.u_password, crypt(new_password, user.u_login));
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
		if (!islower(*nbuf)) {
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
	if (strlen(user.u_login) < 2) {
		goto input_login;
	}
	/* This part added to fix home directories -- asp */
	strcpy(user.u_home_dir, "/mit/");
	user.u_home_dir[5] = user.u_login[0];
	user.u_home_dir[6] = '/';
	user.u_home_dir[7] = user.u_login[1];
	user.u_home_dir[8] = '/';
	strcpy(user.u_home_dir + 9, user.u_login);

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
	/*
	 * replaces a user in the database. If there is an error, it informs
	 * the user and calls qexit(); It returns only if is is successful 
	 */

	register int    result, uid;

	signal(SIGALRM, SIG_IGN);	/* Don't bother me */
	uid = getuid();
	setruid(0);		/* Look, I'm root! */
	timer_off();
	result = sms_connect();
	timer_on();
	if (result == SUCCESS) {
		timer_off();
		result = replace_user_admin(&user);
		sms_disconnect();
		timer_on();
	}
	setruid(uid);		/* Back to normal */
	signal(SIGALRM, restart);

	if (result == FAILURE) {
		display_text(NETWORK_DOWN);
		return (-1);
	}
	if (result != SUCCESS) {
		display_text_line(0);
		display_text_line("There was an unknown error in trying to give you that username.");
		display_text_line("The database was not updated.");
		display_text_line("Please try again later.");
		printf("Replace error %d\n", result);
		return (-1);
	}
	/* finally, if we get to here, there are no problems */
	else {
		return (0);
	}
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
