/* $Id $
 *
 * Talk to the Moira database to change a person's GECOS information.
 *
 * chfn with no modifiers changes the information of the user who is
 * running the program.
 *
 * If a commandline argument is given, it is taken to be the username
 * of the user whose information is to be changed.
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>
#include <moira_site.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <krb.h>

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/chfn.c,v 1.16 1998-02-05 22:50:54 danw Exp $");

#define FALSE 0
#define TRUE 1

char *whoami;

struct finger_info {
  char *fullname;
  char *nickname;
  char *home_address;
  char *home_phone;
  char *office_address;
  char *office_phone;
  char *mit_department;
  char *mit_year;
};

int usage(void);
int leave(int status);
int chfn(char *uname);
int get_user_info(int argc, char *argv[], void *message);
char *ask(char *question, char *def_val, int phone_num);
void get_new_info(struct finger_info *old_info, struct finger_info *new_info);

int main(int argc, char *argv[])
{
  char pname[ANAME_SZ];
  char *uname = pname;
  int k_errno;

  initialize_krb_error_table();

  if ((whoami = strrchr(argv[0], '/')) == NULL)
    whoami = argv[0];
  else
    whoami++;

  if (argc > 2)
    usage();

  if (argc == 2)
    uname = argv[1];
  else
    {
      /* Do it right; get name from kerberos ticket file rather than
	 from passord file. */

      if ((k_errno = tf_init(TKT_FILE, R_TKT_FIL)))
	{
	  com_err(whoami, k_errno, "reading ticket file");
	  exit(1);
	}

      if ((k_errno = tf_get_pname(pname)))
	{
	  com_err(whoami, k_errno, "getting kerberos principal name");
	  exit(1);
	}

      tf_close();
    }

  exit(chfn(uname));
}

/* This should be called rather than exit once connection to moira server
   has been established. */
int leave(int status)
{
  mr_disconnect();
  exit(status);
}

int chfn(char *uname)
{
  int status;			/* general purpose exit status */
  int q_argc;			/* argc for mr_query */
  char *q_argv[F_END];		/* argv for mr_query */
  char *motd;			/* for Moira server status */
  int i;

  struct finger_info old_info;
  struct finger_info new_info;

  /* Try each query.  If we ever fail, print error message and exit. */

  status = mr_connect(NULL);
  if (status)
    {
      com_err(whoami, status, "while connecting to Moira");
      exit(1);
    }

  status = mr_motd(&motd);
  if (status)
    {
      com_err(whoami, status, "unable to check server status");
      leave(1);
    }
  if (motd)
    {
      fprintf(stderr, "The Moira server is currently unavailable:\n%s\n",
	      motd);
      leave(1);
    }

  status = mr_auth("chfn");	/* Don't use argv[0] - too easy to fake */
  if (status)
    {
      com_err(whoami, status,
	      "while authenticating -- run \"kinit\" and try again.");
      leave(1);
    }

  /* First, do an access check. */

  q_argv[F_NAME] = uname;
  for (i = F_NAME + 1; i < F_MODTIME; i++)
    q_argv[i] = "junk";
  q_argc = F_MODTIME;		/* one more than the last updatable field */

  if ((status = mr_access("update_finger_by_login", q_argc, q_argv)))
    {
      com_err(whoami, status, "; finger\ninformation not changed.");
      leave(2);
    }

  printf("Changing finger information for %s.\n", uname);

  /* Get information */

  q_argv[NAME] = uname;
  q_argc = NAME + 1;
  if ((status = mr_query("get_finger_by_login", q_argc, q_argv,
			 get_user_info, &old_info)))
    {
      com_err(whoami, status, "while getting user information.");
      leave(2);
    }

  /* Get the new information from the user */

  get_new_info(&old_info, &new_info);

  /* Do the update */

  printf("Changing finger information...\n");

  q_argv[F_NAME] = uname;
  q_argv[F_FULLNAME] = new_info.fullname;
  q_argv[F_NICKNAME] = new_info.nickname;
  q_argv[F_HOME_ADDR] = new_info.home_address;
  q_argv[F_HOME_PHONE] = new_info.home_phone;
  q_argv[F_OFFICE_ADDR] = new_info.office_address;
  q_argv[F_OFFICE_PHONE] = new_info.office_phone;
  q_argv[F_MIT_DEPT] = new_info.mit_department;
  q_argv[F_MIT_AFFIL] = new_info.mit_year;
  q_argc = F_MODTIME;		/* First non-update query argument */

  if ((status = mr_query("update_finger_by_login", q_argc, q_argv,
			 NULL, NULL)))
    {
      com_err(whoami, status, "while updating finger information.");
      leave(1);
    }

  printf("Finger information updated succesfully.\n");

  return 0;
}

int get_user_info(int argc, char *argv[], void *message)
{
  struct finger_info *old_info = message;

  if (argc != F_END)
    {
      fprintf(stderr, "Some internal error occurred; try again.\n");
      leave(3);
    }

  printf("Info last changed on %s by user %s with %s.\n",
	 argv[F_MODTIME], argv[F_MODBY], argv[F_MODWITH]);

  old_info->fullname = strdup(argv[F_FULLNAME]);
  old_info->nickname = strdup(argv[F_NICKNAME]);
  old_info->home_address = strdup(argv[F_HOME_ADDR]);
  old_info->home_phone = strdup(argv[F_HOME_PHONE]);
  old_info->office_address = strdup(argv[F_OFFICE_ADDR]);
  old_info->office_phone = strdup(argv[F_OFFICE_PHONE]);
  old_info->mit_department = strdup(argv[F_MIT_DEPT]);
  old_info->mit_year = strdup(argv[F_MIT_AFFIL]);

  /* Only pay attention to the first match since login names are
     unique in the database. */
  return MR_ABORT;
}

char *ask(char *question, char *def_val, int phone_num)
{
  static char buf[BUFSIZ];
  int ok = FALSE;
  char *result;
  int i;
  int dashes = FALSE;

#define BLANK "none"

  while (!ok)
    {
      ok = TRUE;
      printf("%s [%s]: ", question, def_val);
      if (!fgets(buf, sizeof(buf), stdin))
	leave(0);
      buf[strlen(buf) - 1] = '\0';
      if (strlen(buf) == 0)
	result = def_val;
      else if (!strcasecmp(buf, BLANK))
	result = "";
      else
	result = buf;

      for (i = 0; i < strlen(buf); i++)
	{
	  switch (buf[i])
	    {
	    case '"':
	      printf("'\"' is not allowed.\n");
	      ok = FALSE;
	      break;
	    case ',':
	      printf("',' is not allowed.\n");
	      ok = FALSE;
	      break;
	    case ':':
	      printf("':' is not allowed.\n");
	      ok = FALSE;
	      break;
	    default:
	      if (iscntrl(buf[i]))
		{
		  printf("Control characters are not allowed.\n");
		  ok = FALSE;
		  break;
		}
	    }
	  if (!ok)
	    break;
	}

      if (phone_num && ok)
	{
	  for (i = 0; i < strlen(result); i++)
	    {
	      if (!isdigit(result[i]) && (result[i] != '-'))
		{
		  printf("Phone numbers can contain only digits.\n");
		  ok = FALSE;
		  break;
		}
	      if (result[i] == '-')
		dashes = TRUE;
            }
        }
    }

  /* Remove dashes if necessary */
  if (dashes && result == buf)
    {
      char *tmp1, *tmp2;
      tmp1 = tmp2 = (char *)buf;
      do
	{
	  if (*tmp1 != '-')
	    *tmp2++ = *tmp1;
	}
      while (*tmp1++);
    }

    return result;
}

void get_new_info(struct finger_info *old_info, struct finger_info *new_info)
{
  printf("Default values are printed inside of '[]'.\n");
  printf("To accept the default, type <return>.\n");
  printf("To have a blank entry, type the word '%s'.\n\n", BLANK);

#define GETINFO(m, v, n) new_info->v = strdup(ask(m, old_info->v, n))

  GETINFO("Full name", fullname, FALSE);
  GETINFO("Nickname", nickname, FALSE);
  GETINFO("Home address (Ex: EC Bemis 514)", home_address, FALSE);
  GETINFO("Home phone number (Ex: 3141592)", home_phone, TRUE);
  GETINFO("Office address (Exs: E40-342 or 2-108)",
	  office_address, FALSE);
  GETINFO("Office phone (Ex: 3-7619)", office_phone, TRUE);
  GETINFO("MIT department (Exs: 9, Biology, Information Services)",
	  mit_department, FALSE);
  GETINFO("MIT year (Exs: 1989, '91, Faculty, Grad)", mit_year, FALSE);
}

int usage(void)
{
  fprintf(stderr, "Usage: %s [user]\n", whoami);
  exit(1);
}
