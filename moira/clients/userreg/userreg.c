/*
 * $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v $
 * $Author: danw $
 * $Locker:  $
 * $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.33 1998-01-06 20:39:44 danw Exp $ 
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char    *rcsid_userreg_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/userreg.c,v 1.33 1998-01-06 20:39:44 danw Exp $";
#endif

#include <mit-copyright.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>
#include <ctype.h>
#include <kadm.h>
#include <kadm_err.h>
#include <krb.h>
#include <des.h>
#include <errno.h>
#include "userreg.h"
#include "ureg_err.h"

/* 7.2 release compatibility */
#ifndef KADM_INSECURE_PW
#define KADM_INSECURE_PW	(-1783126240L)
#endif

#define EXIT -1


struct user     user, db_user;
struct alias    alias;
char		realm[REALM_SZ];
jmp_buf         redo;
int             restart();

extern int      errno;
int             user_is_valid = 0;
int		user_has_login = 0;
int             already_registered = 0;
int		enrollment = 0;
extern char *disabled();
char typed_mit_id[100];

fix_display(sig)
{
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  sigaction(sig, &act, NULL);
  noraw();
  echo();
  endwin();
  exit(1);
}

int main(int argc, char **argv)
{
  int ntimes;
  int reencrypt;
  char line[100], *when, *msg;
  int status;
  char tmpfirst[100], tmplast[100], tmpmid[100];
  struct sigaction act;

  status = ureg_init();
  if (status)
    {
      com_err(argv[0], status, "while trying to initialize");
      exit(1);
    }

  if (when = disabled(&msg))
    {
      printf("We're sorry, the registration service is unavailable right now\n");
      if (msg)
	printf("because %s\n", msg);
      printf("You should be able to register after %s", when);
      printf("\nPress ENTER or RETURN to continue ");
      fflush(stdout);
      getchar();
      exit(0);
    }

  /* stash the realm for later use */
  if ((status = krb_get_lrealm(realm, 1)) != KSUCCESS)
    {
      printf("System error; please try another workstation.");
      exit(1);
    }

  setup_display();

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGHUP, &act, NULL);

  while (1)
    {
      setjmp(redo);
      reset();
      ntimes = 0;

      display_text(WELCOME, "");

      gfirst();
      gmi();
      glast();
      strcpy(tmpfirst, user.u_first);
      strcpy(tmplast, user.u_last);
      FixName(tmplast, tmpfirst, user.u_last, user.u_first, tmpmid);
      redisp();

      gmitid();

      while (dolook())
	{
	  ntimes++;
	  if (ntimes > 3)
	    {
	      display_text_line(0);
	      display_text_line("Sorry, you still cannot be found in the database.");
	      display_text_line(" ");
	      display_text_line("Please call an accounts consultant at x3-1325 for help.");
	      wait_for_user();
	      break;
	    }
	  display_text_line(0);	/* clear the window */
	  display_text_line("You could not be found in the database.");
	  display_text_line("Do you want to change some input "
			    "(for example, the spelling");
	  display_text_line("of part of your name) and try again?");
	  if (askyn("Do you want to try again ? ") == YES)
	    {
	      reencrypt = 0;
	      display_text_line(0);
	      sprintf(line, "You entered your first name as \"%s\"",
		      user.u_first);
	      display_text_line(line);
	      if (askyn("Do you want to change it? ") == YES)
		{
		  gfirst();
		  strcpy(tmpfirst, user.u_first);
		  strcpy(tmplast, user.u_last);
		  FixName(tmplast, tmpfirst, user.u_last, user.u_first,
			  tmpmid);
		  redisp();
		  reencrypt = 1;
		}
	      display_text_line(0);
	      sprintf(line, "You entered your middle initial as \"%s\"",
		      user.u_mid_init);
	      display_text_line(line);
	      if (askyn("Do you want to change it? ") == YES)
		gmi();
	      display_text_line(0);
	      sprintf(line, "You entered your family name as \"%s\"",
		      user.u_last);
	      display_text_line(line);
	      if (askyn("Do you want to change it? ") == YES)
		{
		  glast();
		  strcpy(tmpfirst, user.u_first);
		  strcpy(tmplast, user.u_last);
		  FixName(tmplast, tmpfirst, user.u_last, user.u_first,
			  tmpmid);
		  redisp();
		  reencrypt = 1;
		}
	      display_text_line(0);
	      sprintf(line, "You entered your MIT id number as \"%s\"",
		      typed_mit_id);
	      display_text_line(line);
	      if (askyn("Do you want to change it? ") == YES)
		{
		  gmitid();
		  reencrypt = 0;
		}
	      if (reencrypt)
		{
		  EncryptID(user.u_mit_id, typed_mit_id,
			    user.u_first, user.u_last);
		}
	    }
	  else
	    break;
	}
      if (!user_is_valid || already_registered)
	{
	  qexit();
	  continue;
	}
      redisp();
      if (!user_has_login)
	{
	  if (negotiate_login() == -1)
	    {
	      qexit();
	      continue;
	    }
	}
      if (negotiate_passwd() == -1)
	{
	  qexit();
	  continue;
	}
      sleep(1);
      if (!enrollment)
	display_text(FINISHED, user.u_login);
      else
	{
	  display_text(FINISHEDE, user.u_login);
	  if (askyn("Do you wish to set your mailbox now? (Yes or No) "))
	    {
	      clear();
	      refresh();
	      noraw();
	      echo();
	      kinit(user.u_login, user.u_password);
	      system(NAMESPACE_PROG);
	      dest_tkt();
	      qexit();
	      exit(0);
	    }
	}
      wait_for_user();
      qexit();
      break;
    }
  restore_display();
  exit(0);
}

reset(void)
{
  reset_display();
  memset(&user, 0, sizeof(user));
  user_is_valid = 0;
  already_registered = 0;
  enrollment = 0;
  redisp();
}

int dolook(void)
{
  /* do the database lookup */

  char line[100];
  int result;

  if (user_is_valid)
    {
      /* !! */
      return 0;
    }
  display_text_line(0);
  display_text_line("Looking you up in the database.... "
		    "This may take from a few seconds to a few");
  display_text_line("minutes, depending on how busy the system is "
		    "at the moment.");

  timer_off();
  result = verify_user(user.u_first, user.u_last, typed_mit_id,
		       user.u_mit_id, db_user.u_login);
  wfeep();
  display_text_line(0);
  timer_on();
  switch(result)
    {
    case 0:
    case UREG_ENROLLED:
      display_text_line("You have been located in the user registration database.");
      user_is_valid = 1;
      user_has_login = 0;
      sleep(1);
      return 0;
    case UREG_HALF_ENROLLED:
      enrollment = 1;
      /* fall through to: */
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
      return 0;

    case UREG_ALREADY_REGISTERED:
      already_registered = 1;
      /*
       * we have to reset this so we dont ask for a
       * new username
       */
      user_is_valid = 0;
      display_text_line("You are already registered.  "
			"An account for you probably already exists");
      display_text_line("(if not, it will appear within 24 hours).");
      display_text_line("");
      display_text_line("Refer to the document 'Working on Athena' "
			"for help logging in.");
      strcpy(user.u_login, db_user.u_login);
      sprintf(line, "Remember, the username you chose was '%s'",
	      db_user.u_login);
      display_text_line(line);
      redisp();
      wait_for_user();
      return 0;
    case UREG_DELETED:
      display_text(DELETED_ACCT, db_user.u_login);
      wait_for_user();
      restore_display();
      exit(0);
    case UREG_NOT_ALLOWED:
      display_text(OFFER_ENROLL, db_user.u_login);
      redisp();
      if (!askyn("Continue choosing a name and password (Y/N)? "))
	{
	  already_registered = 1;
	  return 0;
	}
      user_has_login = 0;
      user_is_valid = 1;
      enrollment = 1;
      return 0;
    case UREG_ENROLL_NOT_ALLOWED:
      display_text(NOT_ALLOWED, db_user.u_login);
      wait_for_user();
      restore_display();
      exit(0);
    case UREG_KRB_TAKEN:
      display_text(IMPROPER_LOGIN, db_user.u_login);
      wait_for_user();
      return 0;
    case UREG_USER_NOT_FOUND:
      return 1;

    case ECONNREFUSED:
    case ETIMEDOUT:
    case UREG_MISC_ERROR:
      display_text(NETWORK_DOWN, db_user.u_login);
      display_text_line(" ");
      sprintf(line, "The specific error was: %s", error_message(result));
      display_text_line(line);
      wait_for_user();
      return 0;

    default:
      display_text_line("An unexpected error occurred while trying to "
			"access the database");
      display_text_line(error_message(result));
      redisp();
      wait_for_user();
      return 1;
    }
}


/* Get a login name from the user and register it.  There are several steps
 * to this: display help message, get name, check name, display confirmation
 * message, get confirmation, register name.
 */

int negotiate_login(void)
{
  int result, i;
  char line[100], old_login[LOGIN_SIZE + 2];
  char *error, *cp;

  /* build suggested username */
  cp = user.u_login;
  if (isalpha(user.u_first[0]))
    *cp++ = user.u_first[0];
  if (isalpha(user.u_mid_init[0]))
    *cp++ = user.u_mid_init[0];
  for (i = 0; user.u_last[i] && cp - user.u_login < 8; i++)
    {
      if (isalpha(user.u_last[i]))
	*cp++ = user.u_last[i];
    }
  for (i = 0; user.u_login[i]; i++)
    {
      if (isupper(user.u_login[i]))
	user.u_login[i] = tolower(user.u_login[i]);
    }
  strcpy(old_login, user.u_login);

  /* print message */
  display_text(USERNAME_BLURB, user.u_login);

again:
  /* get name from user */
  glogin();

  display_text_line(0);
  display_text_line("Testing that username...");
  error = "";
  if (strlen(user.u_login) < 3)
    error = "Your username must be at least 3 characters long.";
  else if (strlen(user.u_login) > 8)
    error = "Your username cannot be longer than 8 characters.";
  else if (!isalpha(user.u_login[0]) || !islower(user.u_login[0]))
    error = "Your username must start with a lowercase letter.";
  else
    for (i = 1; i < strlen(user.u_login); i++)
      {
	if (!islower(user.u_login[i]) &&
	    !isdigit(user.u_login[i]) &&
	    user.u_login[i] != '_')
	  error = "Your username must contain only lower case letters, "
	    "numbers, and underscore";
      }
  timer_off();
  result = krb_get_pw_in_tkt(user.u_login, "", realm, "krbtgt", realm, 1, "");
  timer_on();
  if (*error == 0 && result != KDC_PR_UNKNOWN)
    error = "That username is already taken.";

  /* if it's bad, get another name from user */
  if (*error)
    {
      strcpy(user.u_login, old_login);
      redisp();
      display_text_line(error);
      display_text_line("Please choose another username.");
      goto again;
    }

  /* name is OK, make sure */
  display_text(USERNAME_BLURB2, user.u_login);
  if (!askyn("Do you want to register this username? (Yes or No) "))
    goto again;

  display_text_line(0);
  display_text_line("Trying to assign that username...  "
		    "This may take a few minutes.");

  /* Do It! */
  timer_off();
  if (!enrollment)
    {
      result = grab_login(user.u_first, user.u_last, typed_mit_id,
			  user.u_mit_id, user.u_login);
    }
  else
    {
      result = enroll_login(user.u_first, user.u_last, typed_mit_id,
			    user.u_mit_id, user.u_login);
    }
  wfeep();
  timer_on();
  if (result != 0)
    {
      char buf[BUFSIZ];

      if (result == UREG_LOGIN_USED)
	{
	  /* name was in moira but not kerberos */
	  error = "Sorry, that username really was in use after all.";
	  strcpy(user.u_login, old_login);
	  redisp();
	  display_text_line(error);
	  display_text_line("Please choose another username.");
	  goto again;
	}

      display_text(NETWORK_DOWN, "");
      display_text_line(" ");
      sprintf(buf, "The specific error was: %s", error_message(result));
      display_text_line(buf);
      wait_for_user();
      return qexit();
    }

  /* at this point we have successfully negotiated a username */
  sprintf(line, "O.K. your username will be \"%s\".", user.u_login);
  display_text_line(0);
  display_text_line(line);
  redisp();
  sleep(3);
  return 0;
}


int negotiate_passwd(void)
{
  char *passwd, *error;
  char old_passwd[256], fullname[256], tktstring[256], inst[INST_SZ];
  char login[ANAME_SZ], lpassword[PASSWORD_SIZE];
  int result;
  des_cblock key;
  FILE *in;

again:
  display_text(PASSWORD_BLURB, "");
  gpass();

  /* validate password */
  error = NULL;
  passwd = user.u_password;
  sprintf(fullname, "%s%s", user.u_first, user.u_last);
  if (strlen(passwd) < 6)
    error = "Please choose a longer password.";
  if (!strpasscmp(passwd, user.u_first) ||
      !strpasscmp(passwd, user.u_last) ||
      !strpasscmp(passwd, user.u_login) ||
      !strpasscmp(passwd, fullname) ||
      !strpasscmp(passwd, typed_mit_id))
    error = "Please do not use your name or ID number for your password.";
  if (!error)
    {
      in = fopen(LOGIN_INFO, "r");
      if (in)
	{
	  fgets(login, sizeof(login), in);
	  /* trim trailing newline */
	  if (strlen(login))
	    login[strlen(login) - 1] = '\0';
	  fgets(lpassword, sizeof(lpassword), in);
	  /* trim trailing newline */
	  if (strlen(lpassword))
	    lpassword[strlen(lpassword) - 1] = '\0';
	  fclose(in);
	}
      else
	{
	  strcpy(login, "moira");
	  strcpy(lpassword, "moira");
	}

      sprintf(tktstring, "/tmp/tkt_cpw_%d", getpid());
      krb_set_tkt_string(tktstring);
      des_string_to_key(passwd, key);
      inst[0] = 0;

      result = krb_get_pw_in_tkt(login, inst, realm, PWSERV_NAME,
				 KADM_SINST, 1, lpassword);
      if (result == KSUCCESS)
	result = kadm_init_link(PWSERV_NAME, KRB_MASTER, realm);
      if (result == KSUCCESS)
	result = kadm_check_pw(key, passwd, &error);
      dest_tkt();
      if (result == KADM_INSECURE_PW)
	{
	  error = "You have chosen a passsword that is in the dictionary "
	    "of commonly\nselected user passwords.  You will have to choose "
	    "a better password.";
	}
      else if (result != KSUCCESS)
	{
	  display_text(NETWORK_DOWN);
	  display_text_line(" ");
	  sprintf(fullname, "%s while verifying password",
		  error_message(result));
	  display_text_line(fullname);
	  wait_for_user();
	  return -1;
	}
    }

  if (error)
    {
      display_text_line(0);
      display_text_line(error);
      wait_for_user();
      goto again;
    }

  display_text(PASSWORD_BLURB2, "");
  strcpy(old_passwd, user.u_password);
  gpass();
  if (strcmp(old_passwd, user.u_password))
    {
      display_text_line(0);
      display_text_line("What you just typed did not match the password "
			"you gave the first time.");
      sleep(8);
      goto again;
    }

  display_text_line("Storing password in the database...  This may take "
		    "a few minutes.");
  if (do_replace())
    return -1;
  display_text_line("done.");
  return 0;
}

gfirst(void)
{
  /* input the first name */
  char buf[FIRST_NAME_SIZE + 2];
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  input("Enter first Name:", buf, FIRST_NAME_SIZE + 1,
	FIRSTNAME_TIMEOUT, TRUE);
  strncpy(user.u_first, buf, FIRST_NAME_SIZE);
  user.u_first[FIRST_NAME_SIZE - 1] = '\0';
  canon_name(user.u_first);
  redisp();
}

glast(void)
{
  /* input the last name */
  char buf[LAST_NAME_SIZE + 2];
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  input("Enter family Name:", buf, LAST_NAME_SIZE + 1,
	LASTNAME_TIMEOUT, FALSE);
  strncpy(user.u_last, buf, LAST_NAME_SIZE);
  user.u_last[LAST_NAME_SIZE - 1] = '\0';
  canon_name(user.u_last);
  redisp();
}

gpass(void)
{
  /* input password */
  char new_password[PASSWORD_SIZE + 1];
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  input_no_echo("Enter password:", new_password,
		PASSWORD_SIZE, NEW_PASSWORD_TIMEOUT);
  strcpy(user.u_password, new_password);
  redisp();
}


/* get login name */

glogin(void)
{
  char buf[LOGIN_SIZE + 2];
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  user.u_login[0] = '\0';
  input("Enter username:", buf, LOGIN_SIZE, USERNAME_TIMEOUT, FALSE);
  strcpy(user.u_login, buf);
  redisp();
}

gmitid(void)
{
  /* get mid id */
  int i;
  char buf[15];
  char *nbuf = buf;
  struct sigaction act;

input_mit_id:
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  input("Enter MIT Id:", buf, 14, MITID_TIMEOUT, FALSE);
  i = 0;
  nbuf = &buf[0];
  while (*nbuf)
    {
      if (*nbuf == ' ' || *nbuf == '-')
	{
	  nbuf++;
	  continue;
	}
      if (!isdigit(*nbuf))
	{
	  i = 0;
	  break;
	}
      typed_mit_id[i] = *nbuf;
      nbuf++;
      i++;
    }
  if (i != 9)
    {
      display_text_line("Your MIT id must be a nine-digit number.  "
			"Please try again.");
      goto input_mit_id;
    }
  typed_mit_id[9] = '\0';
  redisp();
  EncryptID(user.u_mit_id, typed_mit_id, user.u_first, user.u_last);
}

gmi(void)
{
  /* get middle initial */
  char buf[MID_INIT_SIZE + 2];
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = (void (*)()) fix_display;
  sigaction(SIGALRM, &act, NULL);
  input("Enter Middle Initial:", buf, MID_INIT_SIZE + 1, MI_TIMEOUT, TRUE);
  strncpy(user.u_mid_init, buf, MID_INIT_SIZE);
  user.u_mid_init[MID_INIT_SIZE - 1] = '\0';
  canon_name(user.u_mid_init);
  redisp();
}

int qexit(void)
{
  /* exit quickly, not saving anything in the database */
  memset(&user, 0, sizeof(user));
  typed_mit_id[0] = '\0';
  user_is_valid = 0;
  already_registered = 0;
  enrollment = 0;
  sleep(2);		/* give the user a chance to see the screen */
  display_text_line(0);
  return EXIT;
}


int do_replace(void)
{
  int status;
  char buf[100];

  /*
   * replaces a user in the database. If there is an error, it informs
   * the user and calls qexit(); It returns only if is is successful
   */
  timer_off();
  if (!enrollment)
    {
      status = set_password(user.u_first, user.u_last, typed_mit_id,
			    user.u_mit_id, user.u_password);
    }
  else
    {
      status = get_krb(user.u_first, user.u_last, typed_mit_id,
		       user.u_mit_id, user.u_password);
    }
  wfeep();
  timer_on();
  if (status)
    {
      display_text(NETWORK_DOWN);
      display_text_line(" ");
      sprintf(buf, "The specific error was: %s", error_message(status));
      display_text_line(buf);
      wait_for_user();
      return -1;
    }
  else
    return 0;
}


int kinit(char *user, char *passwd)
{
  int status;
  char inst[INST_SZ];

  inst[0] = '\0';
  status = krb_get_pw_in_tkt(user, inst, realm, "krbtgt",
			     realm, DEFAULT_TKT_LIFE, 0);
  return status;
}


#ifndef _toupper
#define _toupper(c) ((c) & ~0x20)
#endif

int lenient_strcmp(char *string1, char *string2)
{
  /*
   * a primitive case insensitive string comparison. It returns only 0
   * if the strings are equal (ignoring case) and 1 if they are
   * different. Also ignores spaces.
   */

  while (1)
    {
      if (*string1 == '\0' && *string2 == '\0')
	return 0;
      if (*string1 == ' ' || *string1 == '.' || *string1 == '-' ||
	  *string1 == '\'' || *string1 == '_')
	{
	  string1++;
	  continue;
	}
      if (*string2 == ' ' || *string2 == '.' || *string2 == '-' ||
	  *string2 == '\'' || *string2 == '_')
	{
	  string2++;
	  continue;
	}
      if (_toupper(*string1) != _toupper(*string2))
	return 1;
      string1++;
      string2++;
    }
}


/* See if the strings match in forward & reverse direction, ignoring
 * case and spaces/punctuation.
 */

int strpasscmp(char *s1, char *s2)
{
  char buf[256], *from, *to;

  if (!lenient_strcmp(s1, s2))
    return 0;
  /* if s2 is empty, say OK */
  if (!*s2)
    return 1;

  from = &s2[strlen(s2)];
  from--;
  for (to = &buf[0]; from >= s2; from--)
    *to++ = *from;

  return lenient_strcmp(s1, buf);
}


/*
 * Input timeout handler.  Loop back to asking for the first name.
 */

/* Go to asking for first name. */
restart(void)
{
  qexit();
  longjmp(redo, 1);
}

canon_name(char *cp)
{
  char *p2 = cp;

  /* Trim whitespace off both ends. */
  for (; *p2 && isspace(*p2); p2++)
    ;
  if (*p2)
    {
      strcpy(cp, p2);
      p2 = cp + strlen(cp);
      --p2;
      while (p2 >= cp && isspace(*p2))
	*(--p2) = '\0';
    }
  /* Make it capitalized */
  FixCase(cp);
}
